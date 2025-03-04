// Copyright 2021 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "Dialect/LinalgTransform/LinalgTransformOps.h"

#include <algorithm>

#include "Dialect/LinalgExt/IR/LinalgExtOps.h"
#include "Dialect/LinalgExt/Transforms/Transforms.h"
#include "Dialect/LinalgTransform/ScopedTransform.h"
#include "Dialect/LinalgTransform/TrackingListener.h"
#include "Dialect/LinalgTransform/TrackingRewriteDriver.h"
#include "Dialect/LinalgTransform/TransformOpInterface.h"
#include "FunctionHelpers.h"
#include "PDL.h"
#include "Transforms/Listener.h"
#include "mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "mlir/Conversion/AsyncToLLVM/AsyncToLLVM.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h"
#include "mlir/Conversion/LinalgToLLVM/LinalgToLLVM.h"
#include "mlir/Conversion/LinalgToStandard/LinalgToStandard.h"
#include "mlir/Conversion/MathToLLVM/MathToLLVM.h"
#include "mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "mlir/Conversion/VectorToLLVM/ConvertVectorToLLVM.h"
#include "mlir/Dialect/Async/Passes.h"
#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h"
#include "mlir/Dialect/Bufferization/Transforms/Bufferize.h"
#include "mlir/Dialect/Bufferization/Transforms/OneShotAnalysis.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/Linalg/ComprehensiveBufferize/ModuleBufferization.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Dialect/Linalg/Transforms/Hoisting.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/Dialect/PDL/IR/PDLTypes.h"
#include "mlir/Dialect/SCF/Transforms.h"
#include "mlir/Dialect/Utils/StaticValueUtils.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Transforms/InliningUtils.h"
#include "mlir/Transforms/Passes.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "linalg-transform-dialect"
#define DBGS() (llvm::dbgs() << "[" DEBUG_TYPE << "]: ")

using namespace mlir;
using namespace mlir::linalg;
using namespace mlir::iree_compiler::IREE;

#include "Dialect/LinalgTransform/LinalgTransformDialect.cpp.inc"

void transform::LinalgTransformDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Dialect/LinalgTransform/LinalgTransformOps.cpp.inc"
      >();
}

//===----------------------------------------------------------------------===//
// Functional Rewrite Helpers
//===----------------------------------------------------------------------===//

using FunctionalLinalgTransform =
    std::function<FailureOr<LinalgOp>(LinalgOp, PatternRewriter &)>;

/// Extracts a vector of int64_t from an array attribute. Asserts if the
/// attribute contains values other than integers.
static SmallVector<int64_t> extractI64Array(ArrayAttr attr) {
  SmallVector<int64_t> result;
  result.reserve(attr.size());
  for (APInt value : attr.getAsValueRange<IntegerAttr>())
    result.push_back(value.getSExtValue());
  return result;
}

/// Extracts a vector of StringRef from an array attribute. Asserts if the
/// attribute contains values other than StringRefs.
static SmallVector<StringRef> extractStringArray(ArrayAttr attr) {
  SmallVector<StringRef> result;
  result.reserve(attr.size());
  for (StringRef value : attr.getAsValueRange<StringAttr>())
    result.push_back(value);
  return result;
}

/// Extracts a vector of unsigned from an array attribute. Asserts if the
/// attribute contains values other than intergers. May truncate.
static SmallVector<unsigned> extractUIntArray(ArrayAttr attr) {
  SmallVector<unsigned> result;
  result.reserve(attr.size());
  for (APInt value : attr.getAsValueRange<IntegerAttr>())
    result.push_back(value.getZExtValue());
  return result;
}

//===---------------------------------------------------------------------===//
// ScopeOp
//===---------------------------------------------------------------------===//

void transform::ScopeOp::getSuccessorRegions(
    Optional<unsigned> index, ArrayRef<Attribute> operands,
    SmallVectorImpl<RegionSuccessor> &regions) {
  if (index)
    regions.emplace_back(getResults());
  else
    regions.emplace_back(&body());
}

//===---------------------------------------------------------------------===//
// SequenceOp
//===---------------------------------------------------------------------===//

LogicalResult transform::SequenceOp::verify() {
  WalkResult result = this->walk([](Operation *child) {
    for (OpResult result : child->getResults()) {
      if (llvm::hasNItemsOrLess(result.getUses(), 1))
        continue;
      InFlightDiagnostic diag = child->emitError()
                                << "result #" << result.getResultNumber()
                                << " has more than one use";
      for (OpOperand &use : result.getUses()) {
        diag.attachNote(use.getOwner()->getLoc())
            << "used here as operand #" << use.getOperandNumber();
      }
      return WalkResult::interrupt();
    }
    return WalkResult::advance();
  });
  return failure(result.wasInterrupted());
}

//===---------------------------------------------------------------------===//
// MatchOp
//===---------------------------------------------------------------------===//

LogicalResult transform::MatchOp::apply(TransformResults &results,
                                        TransformState &state) {
  Operation *topLevelOp = state.getTopLevel();
  FailureOr<SmallVector<Operation *>> ops = findMatchingOps(*this, topLevelOp);
  if (failed(ops))
    return failure();
  LLVM_DEBUG(DBGS() << "matched " << ops->size() << " ops\n");
  results.set(getResult().cast<OpResult>(), *ops);
  return success();
}

//===---------------------------------------------------------------------===//
// TileOp
//===---------------------------------------------------------------------===//

LogicalResult transform::TileOp::apply(TransformResults &transformResults,
                                       TransformState &state) {
  LinalgTilingOptions tilingOptions;
  SmallVector<int64_t> tileSizes = extractI64Array(sizes());
  size_t numExpectedLoops = 0;
  for (int64_t i : tileSizes)
    if (i)
      ++numExpectedLoops;

  if (!tileSizes.empty())
    tilingOptions.setTileSizes(tileSizes);
  tilingOptions.setInterchange(extractUIntArray(interchange()));
  LinalgTilingPattern pattern(getContext(), tilingOptions);
  auto functionalTile =
      [&](LinalgOp op, PatternRewriter &rewriter) -> FailureOr<TiledLinalgOp> {
    return pattern.returningMatchAndRewrite(op, rewriter);
  };

  SmallVector<Operation *> tiledLinalgOps;
  SmallVector<SmallVector<Operation *>> loops(numExpectedLoops);

  for (Operation *target : state.getPayloadOps(target())) {
    auto linalgOp = cast<linalg::LinalgOp>(target);
    FailureOr<TiledLinalgOp> tiled =
        functional::applyAt(linalgOp, functionalTile);
    if (failed(tiled))
      return linalgOp->emitOpError() << "Failed to apply tiling";

    tiledLinalgOps.push_back(tiled->op);

    if (tiled->loops.size() != numExpectedLoops) {
      // Not enough loops were generated. This usually means that the input size
      // was smaller than the tiling size.
      // TODO: LinalgTilingPattern should return failure().
      return tiled->loops.front()->emitOpError()
             << "Not enough loops generated: " << tiled->loops.size() << " vs "
             << numExpectedLoops;
    }
    for (unsigned int i = 0; i < numExpectedLoops; ++i)
      loops[i].push_back(tiled->loops[i]);
  }

  transformResults.set(tiled_linalg_op().cast<OpResult>(), tiledLinalgOps);
  for (unsigned int i = 0; i < numExpectedLoops; ++i) {
    transformResults.set(getOperation()->getOpResult(i + 1), loops[i]);
  }
  return success();
}

ParseResult transform::TileOp::parse(OpAsmParser &parser,
                                     OperationState &result) {
  OpAsmParser::UnresolvedOperand targetOperand;
  SMLoc opLoc;
  parser.getCurrentLocation(&opLoc);
  if (parser.parseOperand(targetOperand))
    return parser.emitError(opLoc, "expected `target` operand");
  if (parser.parseOptionalAttrDict(result.attributes))
    return failure();
  Type pdlOpType = parser.getBuilder().getType<pdl::OperationType>();
  result.addTypes(pdlOpType);
  Attribute sizesAttr = result.attributes.get("sizes");
  if (!sizesAttr)
    return parser.emitError(opLoc, "expected `sizes` attribute");
  auto sizesArrayAttr = sizesAttr.dyn_cast<ArrayAttr>();
  if (!sizesArrayAttr)
    return parser.emitError(opLoc, "`sizes` attribute must be an array");
  for (int64_t tileSize : extractI64Array(sizesArrayAttr)) {
    if (tileSize)
      result.addTypes(pdlOpType);
  }
  if (parser.resolveOperand(targetOperand, pdlOpType, result.operands))
    return failure();
  return success();
}

void transform::TileOp::print(OpAsmPrinter &p) {
  p << ' ';
  p << target();
  p.printOptionalAttrDict((*this)->getAttrs());
}

//===---------------------------------------------------------------------===//
// ScalarizeOp
//===---------------------------------------------------------------------===//

FailureOr<LinalgOp> transform::ScalarizeOp::applyToOne(LinalgOp target) {
  LinalgTilingOptions tilingOptions;
  tilingOptions.scalarizeDynamicDims();
  // Tiling with "scalarize_dyn_dims" actually sets the same lambda as the tile
  // sizes and asserts that it is not already set.
  SmallVector<int64_t> emptyTileSizes;
  LinalgTilingPattern pattern(getContext(), tilingOptions);
  auto maybeTiledLinalgOp =
      functional::applyReturningPatternAt(pattern, target);
  if (failed(maybeTiledLinalgOp))
    return failure();
  return maybeTiledLinalgOp->op;
}

//===---------------------------------------------------------------------===//
// FuseOp
//===---------------------------------------------------------------------===//

FailureOr<LinalgOp> transform::FuseOp::applyToOne(LinalgOp target) {
  LinalgTilingAndFusionOptions fusionOptions;
  fusionOptions.tileSizes = extractI64Array(tile_sizes());
  fusionOptions.tileInterchange = extractI64Array(tile_interchange());

  LinalgTileAndFuseTensorOpsPattern pattern(getContext(), fusionOptions);
  auto functionalFuse = [&](LinalgOp op,
                            PatternRewriter &rewriter) -> FailureOr<LinalgOp> {
    auto tileLoopNest = pattern.returningMatchAndRewrite(op, rewriter);
    if (failed(tileLoopNest))
      return failure();
    return tileLoopNest->getRootOp();
  };
  return functional::applyAt(target, functionalFuse);
}

LogicalResult transform::FuseOp::verify() {
  SmallVector<int64_t> permutation = extractI64Array(tile_interchange());
  auto sequence = llvm::to_vector(llvm::seq<int64_t>(0, permutation.size()));
  if (!std::is_permutation(sequence.begin(), sequence.end(),
                           permutation.begin(), permutation.end())) {
    return emitOpError() << "expects interchange to be a permutation, found "
                         << tile_interchange();
  }
  return success();
}

//===---------------------------------------------------------------------===//
// GeneralizeOp
//===---------------------------------------------------------------------===//

FailureOr<LinalgOp> transform::GeneralizeOp::applyToOne(LinalgOp target) {
  // Exit early if no transformation is needed.
  if (isa<GenericOp>(target))
    return target;
  return functional::applyAt(
      target, callLinalgPattern<LinalgGeneralizationPattern>(getContext()));
}

//===---------------------------------------------------------------------===//
// InterchangeOp
//===---------------------------------------------------------------------===//

FailureOr<LinalgOp> transform::InterchangeOp::applyToOne(LinalgOp target) {
  SmallVector<unsigned> interchangeVector =
      extractUIntArray(iterator_interchange());
  // Exit early if no transformation is needed.
  if (interchangeVector.empty())
    return target;
  return functional::applyAt(target,
                             callLinalgPattern<GenericOpInterchangePattern>(
                                 getContext(), interchangeVector));
}

LogicalResult transform::InterchangeOp::verify() {
  SmallVector<unsigned> permutation = extractUIntArray(iterator_interchange());
  auto sequence = llvm::to_vector(llvm::seq<unsigned>(0, permutation.size()));
  if (!std::is_permutation(sequence.begin(), sequence.end(),
                           permutation.begin(), permutation.end())) {
    return emitOpError()
           << "expects iterator_interchange to be a permutation, found "
           << iterator_interchange();
  }
  return success();
}

//===---------------------------------------------------------------------===//
// PadOp
//===---------------------------------------------------------------------===//

FailureOr<LinalgOp> transform::PadOp::applyToOne(LinalgOp target) {
  // Convert the integer packing flags to booleans.
  SmallVector<bool> packPaddings;
  for (int64_t packPadding : extractI64Array(this->pack_paddings()))
    packPaddings.push_back(static_cast<bool>(packPadding));

  // Convert the padding values to attributes.
  SmallVector<Attribute> paddingValues;
  for (auto const &it : llvm::zip(extractStringArray(this->padding_values()),
                                  target->getOperandTypes())) {
    Type elementType = getElementTypeOrSelf(std::get<1>(it));
    paddingValues.push_back(parseAttribute(std::get<0>(it), elementType));
    if (!paddingValues.back()) {
      return target->emitOpError("Could not parse padding value: ")
             << std::get<0>(it) << " to type: " << elementType;
    }
  }

  // Extract the transpose vectors.
  SmallVector<SmallVector<int64_t>> transposePaddings;
  for (Attribute transposeVector : this->transpose_paddings().cast<ArrayAttr>())
    transposePaddings.push_back(
        extractI64Array(transposeVector.cast<ArrayAttr>()));

  LinalgPaddingOptions paddingOptions;
  paddingOptions.setPaddingValues(paddingValues);
  paddingOptions.setPaddingDimensions(
      extractI64Array(this->padding_dimensions()));
  paddingOptions.setPackPaddings(packPaddings);
  paddingOptions.setHoistPaddings(extractI64Array(this->hoist_paddings()));
  paddingOptions.setTransposePaddings(transposePaddings);
  auto res = functional::applyAt(
      target,
      callLinalgPattern<LinalgPaddingPattern>(getContext(), paddingOptions));
  if (failed(res))
    return target->emitOpError()
           << "failed to apply LinalgPaddingPattern at: " << target;
  return res;
}

LogicalResult transform::PadOp::verify() {
  SmallVector<int64_t> packPaddings = extractI64Array(pack_paddings());
  if (any_of(packPaddings, [](int64_t packPadding) {
        return packPadding != 0 && packPadding != 1;
      })) {
    return emitOpError()
           << "expects pack_paddings to contain booleans (0/1), found "
           << pack_paddings();
  }
  SmallVector<int64_t> paddingDimensions =
      extractI64Array(padding_dimensions());
  if (any_of(paddingDimensions,
             [](int64_t paddingDimension) { return paddingDimension < 0; })) {
    return emitOpError()
           << "expects padding_dimensions to contain positive integers, found "
           << padding_dimensions();
  }
  SmallVector<int64_t> hoistPaddings = extractI64Array(hoist_paddings());
  if (any_of(hoistPaddings,
             [](int64_t hoistPadding) { return hoistPadding < 0; })) {
    return emitOpError()
           << "expects hoist_paddings to contain positive integers, found "
           << hoist_paddings();
  }
  ArrayAttr transposes = transpose_paddings();
  for (Attribute attr : transposes) {
    SmallVector<int64_t> transpose = extractFromI64ArrayAttr(attr);
    auto sequence = llvm::to_vector(llvm::seq<int64_t>(0, transpose.size()));
    if (!std::is_permutation(sequence.begin(), sequence.end(),
                             transpose.begin(), transpose.end())) {
      return emitOpError()
             << "expects transpose_paddings to be a permutation, found "
             << attr;
    }
  }
  return success();
}

//===---------------------------------------------------------------------===//
// DecomposeOp
//===---------------------------------------------------------------------===//

LogicalResult
transform::DecomposeOp::apply(transform::TransformResults &results,
                              transform::TransformState &state) {
  RewritePatternSet patterns(getContext());
  // TODO: make this targetable.
  populateDecomposeConvolutionPatterns(patterns, LinalgTransformationFilter());
  if (failed(applyPatternsAndFoldGreedily(state.getTopLevel(),
                                          std::move(patterns))))
    return failure();

  // TODO: make this chainable, it isn't in the original codegenstrategy.
  return success();
}

//===---------------------------------------------------------------------===//
// VectorizeOp
//===---------------------------------------------------------------------===//

static void configureVectorizationPatterns(transform::VectorizeOp vectorizeOp,
                                           RewritePatternSet &patterns) {
  MLIRContext *ctx = vectorizeOp->getContext();
  vector::populateVectorTransferPermutationMapLoweringPatterns(patterns);
  vector::populateVectorReductionToContractPatterns(patterns);
  patterns.add<linalg::LinalgCopyVTRForwardingPattern,
               linalg::LinalgCopyVTWForwardingPattern>(ctx,
                                                       /*benefit=*/2);
  vector::TransferReadOp::getCanonicalizationPatterns(patterns, ctx);
  vector::TransferWriteOp::getCanonicalizationPatterns(patterns, ctx);
  if (vectorizeOp.vectorize_padding())
    linalg::populatePadOpVectorizationPatterns(patterns);
}

/// Applies the transformation specified by the given vectorize operation to the
/// given target operation AND some related operations.Populates `results` with
/// transformation operations for further transformations if the pattern applied
/// successfully (currently, the main "contraction" op after vectorization).
static FailureOr<LinalgOp>
executeTargetedVectorizeOp(LinalgOp target,
                           linalg::transform::VectorizeOp vectorizeOp) {
  // TODO: this is copy-pasta from LinalgStrategyVectorizePass, it shouldn't be.
  MLIRContext *ctx = target->getContext();
  RewritePatternSet patterns(ctx);
  configureVectorizationPatterns(vectorizeOp, patterns);
  LinalgVectorizationPattern pattern(vectorizeOp.getContext());
  auto functionalVectorize = [&](LinalgOp op, PatternRewriter &rewriter) {
    return pattern.matchAndRewrite(op, rewriter);
  };

  /// Apply the transformations in a scope.
  return transform::scoped(
      target,
      [&](transform::ScopeOp scope, Operation *op) -> FailureOr<LinalgOp> {
        if (failed(functional::applyAt(op, functionalVectorize)) ||
            failed(applyPatternsAndFoldGreedily(scope, std::move(patterns))))
          return failure();
        // FIXME: Vectorization doesn't return anything.
        return LinalgOp();
      });

  // TODO: vectorization may fail because the op is not vectorizable, unclear
  // what to do here. We should probably report it somehow, but we may also
  // want to go on and keep the original for continuation. Should we have
  // some notion of transformation optionality vs. mandatory (like lowering)?
  // How to find ops that were not replaced?
}

LogicalResult
transform::VectorizeOp::apply(transform::TransformResults &results,
                              transform::TransformState &state) {
  if (target()) {
    SmallVector<Operation *> resultVector;
    LogicalResult res = applyTransformToEach(
        state.getPayloadOps(target()), resultVector, [&](LinalgOp target) {
          return executeTargetedVectorizeOp(target, *this);
        });

    if (failed(res))
      return failure();

    results.set(getResult(0).cast<OpResult>(), resultVector);
    return success();
  }

  MLIRContext *ctx = getContext();
  RewritePatternSet patterns(ctx);
  patterns.add<LinalgVectorizationPattern>(ctx);
  configureVectorizationPatterns(*this, patterns);
  auto &listener = state.getExtension<TrackingListener>();
  LogicalResult applicationResult = applyPatternsTrackAndFoldGreedily(
      state.getTopLevel(), listener, std::move(patterns));
  LogicalResult listenerResult = listener.checkErrorState();
  return failure(failed(applicationResult) || failed(listenerResult));
}

ParseResult transform::VectorizeOp::parse(OpAsmParser &parser,
                                          OperationState &result) {
  auto operationType = pdl::OperationType::get(parser.getContext());
  OpAsmParser::UnresolvedOperand target;
  OptionalParseResult parseResult = parser.parseOptionalOperand(target);
  if (parseResult.hasValue()) {
    if (parseResult.getValue().failed() ||
        parser.parseOptionalAttrDict(result.attributes) ||
        parser.resolveOperand(target, operationType, result.operands) ||
        parser.addTypeToList(operationType, result.types)) {
      return failure();
    }
  } else {
    if (parser.parseOptionalAttrDict(result.attributes)) {
      return failure();
    }
  }
  return success();
}

void transform::VectorizeOp::print(OpAsmPrinter &printer) {
  if (target())
    printer << " " << target() << " ";
  printer.printOptionalAttrDict(getOperation()->getAttrs());
}

//===---------------------------------------------------------------------===//
// LowerVectorsOp
//===---------------------------------------------------------------------===//

/// Returns true of the numbered vector lowering stage is included into the list
/// of stages specified on the given lowerVectors operation.
static bool stageIncluded(int stage, transform::LowerVectorsOp lowerVectorsOp) {
  for (auto s : lowerVectorsOp.stages().getAsValueRange<IntegerAttr>()) {
    if (s.getSExtValue() == stage)
      return true;
  }
  return false;
}

// Applies the transformation specified by the given lower vectors operation
/// to the given function.
LogicalResult
transform::LowerVectorsOp::apply(transform::TransformResults &results,
                                 transform::TransformState &state) {
  MLIRContext *ctx = getContext();
  RewritePatternSet patterns(ctx);

  vector::VectorTransposeLowering vectorTransposeLowering =
      llvm::StringSwitch<vector::VectorTransposeLowering>(transpose_lowering())
          .Case("eltwise", vector::VectorTransposeLowering::EltWise)
          .Case("flat_transpose", vector::VectorTransposeLowering::Flat)
          .Case("shuffle", vector::VectorTransposeLowering::Shuffle)
          .Default(vector::VectorTransposeLowering::EltWise);
  vector::VectorMultiReductionLowering vectorMultiReductionLowering =
      llvm::StringSwitch<vector::VectorMultiReductionLowering>(
          multireduction_lowering())
          .Case("innerreduction",
                vector::VectorMultiReductionLowering::InnerReduction)
          .Default(vector::VectorMultiReductionLowering::InnerParallel);
  vector::VectorContractLowering vectorContractLowering =
      llvm::StringSwitch<vector::VectorContractLowering>(contraction_lowering())
          .Case("matrixintrinsics", vector::VectorContractLowering::Matmul)
          .Case("dot", vector::VectorContractLowering::Dot)
          .Case("outerproduct", vector::VectorContractLowering::OuterProduct)
          .Default(vector::VectorContractLowering::OuterProduct);
  // TODO: fix the annoying name mismatch (vector-transfers vs vector-transfer).
  vector::VectorTransferSplit vectorTransferSplit =
      llvm::StringSwitch<vector::VectorTransferSplit>(split_transfers())
          .Case("none", vector::VectorTransferSplit::None)
          .Case("linalg-copy", vector::VectorTransferSplit::LinalgCopy)
          .Case("vector-transfers", vector::VectorTransferSplit::VectorTransfer)
          .Default(vector::VectorTransferSplit::None);

  vector::VectorTransformsOptions vectorTransformOptions;
  vectorTransformOptions.setVectorTransformsOptions(vectorContractLowering)
      .setVectorMultiReductionLowering(vectorMultiReductionLowering)
      .setVectorTransposeLowering(vectorTransposeLowering)
      .setVectorTransferSplit(vectorTransferSplit);

  VectorTransferToSCFOptions vectorTransferToSCFOptions =
      VectorTransferToSCFOptions()
          .enableFullUnroll(unroll_vector_transfers())
          .enableLowerPermutationMaps();

  int maxTransferRank = 1;

  auto avx2LoweringOptions =
      x86vector::avx2::LoweringOptions().setTransposeOptions(
          x86vector::avx2::TransposeLoweringOptions()
              .lower4x8xf32(transpose_avx2_lowering())
              .lower8x8xf32(transpose_avx2_lowering()));

  // TODO: this is copy-pasta from LinalgStrategyLowerVectorsPass, shouldn't be.
  vector::populateVectorToVectorCanonicalizationPatterns(patterns);
  if (stageIncluded(1, *this)) {
    patterns.add<mlir::vector::ContractionOpToOuterProductOpLowering,
                 mlir::vector::ContractionOpToMatmulOpLowering,
                 mlir::vector::ContractionOpLowering>(vectorTransformOptions,
                                                      ctx);
    vector::populateVectorTransferPermutationMapLoweringPatterns(patterns);
  }
  if (stageIncluded(2, *this)) {
    vector::populateVectorMultiReductionLoweringPatterns(
        patterns, vectorTransformOptions.vectorMultiReductionLowering);
  }
  if (stageIncluded(3, *this)) {
    patterns.add<vector::VectorTransferFullPartialRewriter>(
        ctx, vectorTransformOptions);
  }
  if (stageIncluded(4, *this)) {
    vector::populateVectorTransferLoweringPatterns(patterns, maxTransferRank);
  }
  if (stageIncluded(5, *this)) {
    populateVectorToSCFConversionPatterns(
        patterns, vectorTransferToSCFOptions.setTargetRank(maxTransferRank));
  }
  if (stageIncluded(6, *this)) {
    vector::populateVectorShapeCastLoweringPatterns(patterns);
  }
  if (stageIncluded(7, (*this))) {
    vector::populateVectorTransposeLoweringPatterns(patterns,
                                                    vectorTransformOptions);
    if (transpose_avx2_lowering())
      x86vector::avx2::populateSpecializedTransposeLoweringPatterns(
          patterns, avx2LoweringOptions, /*benefit=*/10);
  }

  // TODO: these transformations are currently not targeted at concrete ops.
  // LinalgTransformationFilter filter = makeTransformationFilter(target);
  if (failed(applyPatternsAndFoldGreedily(state.getTopLevel(),
                                          std::move(patterns))))
    return failure();

  // TODO: make composable...
  return success();
}

//===---------------------------------------------------------------------===//
// BufferizeOp
//===---------------------------------------------------------------------===//

LogicalResult transform::BufferizeOp::apply(transform::TransformResults &result,
                                            transform::TransformState &state) {
  PassManager pm(getContext());

  bufferization::OneShotBufferizationOptions options;
  options.memCpyFn = [](OpBuilder &builder, Location loc, Value from,
                        Value to) {
    return success(linalg::makeMemRefCopyOp(builder, loc, from, to));
  };
  pm.addPass(createLinalgComprehensiveModuleBufferizePass(options));
  if (failed(pm.run(state.getTopLevel())))
    return failure();

  // Perform buffer-level hoistings.
  state.getTopLevel()->walk(
      [&](func::FuncOp funcOp) { hoistRedundantVectorTransfers(funcOp); });
  return success();
}

//===---------------------------------------------------------------------===//
// LowerToLLVMOp
//===---------------------------------------------------------------------===//

LogicalResult
transform::LowerToLLVMOp::apply(transform::TransformResults &result,
                                transform::TransformState &state) {
  // TODO: it is feasible to scope lowering at arbitrary level and introduce
  // unrealized casts, but there needs to be the final module-wise cleanup in
  // the end. Keep module-level for now.
  PassManager pm(getContext());

  pm.addNestedPass<func::FuncOp>(createConvertVectorToSCFPass());
  pm.addNestedPass<func::FuncOp>(createConvertLinalgToLoopsPass());
  if (enable_async()) {
    pm.addPass(createAsyncToAsyncRuntimePass());
    pm.addPass(createAsyncRuntimeRefCountingPass());
    pm.addPass(createAsyncRuntimeRefCountingOptPass());
  }
  pm.addPass(createCanonicalizerPass());
  pm.addPass(createLowerAffinePass());
  pm.addPass(createConvertSCFToCFPass());
  pm.addPass(createConvertLinalgToLLVMPass());
  pm.addPass(createConvertVectorToLLVMPass(
      // clang-format off
      LowerVectorToLLVMOptions()
        .enableReassociateFPReductions(reassociate_fp_reductions())
        .enableIndexOptimizations(enable_index_optimizations())
        .enableArmNeon(enable_arm_neon())
        .enableArmSVE(enable_arm_sve())
        .enableAMX(enable_amx())
        .enableX86Vector(enable_x86vector())));
  // clang-format on
  pm.addNestedPass<func::FuncOp>(createConvertMathToLLVMPass());
  pm.addPass(createMemRefToLLVMPass());
  if (enable_async())
    pm.addPass(createConvertAsyncToLLVMPass());
  pm.addPass(createConvertFuncToLLVMPass());
  pm.addPass(createReconcileUnrealizedCastsPass());
  if (failed(pm.run(state.getTopLevel())))
    return failure();

  // Make all arguments noalias for now.
  // FIXME: this is a terrible hack!
  state.getTopLevel()->walk([](LLVM::LLVMFuncOp funcOp) {
    for (int64_t i = 0; i < funcOp.getNumArguments(); ++i) {
      if (!funcOp.getFunctionType()
               .getParamType(i)
               .isa<LLVM::LLVMPointerType>())
        continue;
      funcOp.setArgAttr(i, "llvm.noalias", UnitAttr::get(funcOp.getContext()));
    }
  });
  return success();
}

//===---------------------------------------------------------------------===//
// GetParentLoopOp
//===---------------------------------------------------------------------===//

FailureOr<scf::ForOp>
transform::GetParentLoopOp::applyToOne(Operation *source) {
  int64_t nLoops = num_loops();
  for (int64_t i = 0; i < nLoops; ++i) {
    source = source->getParentOfType<scf::ForOp>();
    if (!source) {
      emitError() << "the transformed op is enclosed by " << i << " loops, but "
                  << nLoops << " expected";
      return failure();
    }
  }
  return cast<scf::ForOp>(source);
}

//===---------------------------------------------------------------------===//
// UnrollLoopOp
//===---------------------------------------------------------------------===//

LogicalResult transform::UnrollLoopOp::applyToOne(scf::ForOp loop) {
  return loopUnrollByFactor(loop, factor());
}

//===---------------------------------------------------------------------===//
// PeelLoopOp
//===---------------------------------------------------------------------===//

FailureOr<scf::ForOp> transform::PeelLoopOp::applyToOne(scf::ForOp loop) {
  scf::ForOp result;
  IRRewriter rewriter(loop->getContext());
  LogicalResult status =
      scf::peelAndCanonicalizeForLoop(rewriter, loop, result);
  if (failed(status))
    return loop;
  return result;
}

//===---------------------------------------------------------------------===//
// PipelineLoopOp
//===---------------------------------------------------------------------===//

static void
loopScheduling(scf::ForOp forOp,
               std::vector<std::pair<Operation *, unsigned>> &schedule,
               unsigned iterationInterval, unsigned readLatency) {
  auto getLatency = [&](Operation *op) {
    if (isa<vector::TransferReadOp>(op))
      return readLatency;
    return unsigned(1);
  };

  DenseMap<Operation *, unsigned> opCycles;
  std::map<unsigned, std::vector<Operation *>> wrappedSchedule;
  for (Operation &op : forOp.getBody()->getOperations()) {
    if (isa<scf::YieldOp>(op))
      continue;
    unsigned earlyCycle = 0;
    for (Value operand : op.getOperands()) {
      Operation *def = operand.getDefiningOp();
      if (!def)
        continue;
      earlyCycle = std::max(earlyCycle, opCycles[def] + getLatency(def));
    }
    opCycles[&op] = earlyCycle;
    wrappedSchedule[earlyCycle % iterationInterval].push_back(&op);
  }
  for (auto it : wrappedSchedule) {
    for (Operation *op : it.second) {
      unsigned cycle = opCycles[op];
      schedule.push_back(std::make_pair(op, cycle / iterationInterval));
    }
  }
}

FailureOr<scf::ForOp> transform::PipelineLoopOp::applyToOne(scf::ForOp loop) {
  // TODO: make the pipelining pattern return the transformed loop.
  if (!getOperation()->getUses().empty()) {
    InFlightDiagnostic diag = emitError()
                              << "NYI: cannot target the result of pipelining";
    diag.attachNote(getOperation()->use_begin()->getOwner()->getLoc())
        << "use here";
    return failure();
  }

  scf::PipeliningOption schedule;
  schedule.getScheduleFn =
      [this](scf::ForOp forOp,
             std::vector<std::pair<Operation *, unsigned>> &schedule) mutable {
        loopScheduling(forOp, schedule, iteration_interval(), read_latency());
      };

  RewritePatternSet patterns(loop->getContext());
  scf::populateSCFLoopPipeliningPatterns(patterns, schedule);
  assert(patterns.getNativePatterns().size() == 1 &&
         "expected one pipelining pattern");
  auto functionalPattern = [&patterns](scf::ForOp forOp,
                                       PatternRewriter &rewriter) {
    RewritePattern *pattern = patterns.getNativePatterns().front().get();
    return pattern->matchAndRewrite(forOp, rewriter);
  };
  if (failed(functional::applyAt(loop, std::move(functionalPattern))))
    return failure();

  return scf::ForOp();
}

//===---------------------------------------------------------------------===//
// OutlineLoopOp
//===---------------------------------------------------------------------===//

static scf::ExecuteRegionOp outlineInExecuteRegion(RewriterBase &b,
                                                   Operation *op) {
  if (op->getNumRegions() != 1)
    return nullptr;
  OpBuilder::InsertionGuard g(b);
  b.setInsertionPoint(op);
  scf::ExecuteRegionOp executeRegionOp =
      b.create<scf::ExecuteRegionOp>(op->getLoc(), op->getResultTypes());
  {
    OpBuilder::InsertionGuard g(b);
    b.setInsertionPointToStart(&executeRegionOp.getRegion().emplaceBlock());
    Operation *clonedOp = b.cloneWithoutRegions(*op);
    Region &clonedRegion = clonedOp->getRegions().front();
    assert(clonedRegion.empty() && "expected empty region");
    b.inlineRegionBefore(op->getRegions().front(), clonedRegion,
                         clonedRegion.end());
    b.create<scf::YieldOp>(op->getLoc(), clonedOp->getResults());
  }
  b.replaceOp(op, executeRegionOp.getResults());
  return executeRegionOp;
}

static FailureOr<func::FuncOp> outlineLoop(scf::ForOp loop, StringRef funcName,
                                           transform::TransformState &state) {
  PatternRewriterListener rewriter(loop->getContext());
  auto &listener = state.getExtension<TrackingListener>();
  rewriter.addListener(&listener);
  Location loc = loop.getLoc();
  scf::ExecuteRegionOp exec = outlineInExecuteRegion(rewriter, loop);
  assert(exec && "failed to produce execute_region");
  FailureOr<func::FuncOp> outlined =
      outlineSingleBlockRegion(rewriter, loc, exec.getRegion(), funcName);
  if (failed(listener.checkErrorState()))
    return failure();
  return outlined;
}

LogicalResult
transform::OutlineLoopOp::apply(transform::TransformResults &results,
                                transform::TransformState &state) {
  SmallVector<Operation *> resultVector;
  auto res =
      applyTransformToEach(state.getPayloadOps(target()), resultVector,
                           [&](scf::ForOp loop) -> FailureOr<func::FuncOp> {
                             return outlineLoop(loop, func_name(), state);
                           });
  if (failed(res))
    return failure();
  results.set(getResult().cast<OpResult>(), resultVector);
  return success();
}

//===---------------------------------------------------------------------===//
// PrintOp
//===---------------------------------------------------------------------===//

LogicalResult transform::PrintOp::apply(transform::TransformResults &results,
                                        transform::TransformState &state) {
  if (!target()) {
    llvm::outs() << "[[[ IR printer: " << name() << " top-level ]]]\n";
    state.getTopLevel()->dump();
    return success();
  }

  llvm::outs() << "[[[ IR printer: " << name() << " single op ]]]\n";
  ArrayRef<Operation *> targets = state.getPayloadOps(target());
  targets.front()->dump();
  return success();
}

//===----------------------------------------------------------------------===//
// LinalgExt specific transforms
//===----------------------------------------------------------------------===//

LogicalResult
transform::TileToLinalgExtTileOp::apply(transform::TransformResults &results,
                                        transform::TransformState &state) {
  LinalgTilingOptions tilingOptions;
  SmallVector<int64_t> tileSizes = extractI64Array(sizes());
  if (!tileSizes.empty())
    tilingOptions.setTileSizes(tileSizes);

  LinalgExt::LinalgExtTilingPattern pattern(this->getContext(), tilingOptions);
  ArrayRef<Operation *> targets = state.getPayloadOps(target());
  auto tilingInterfaceOp = dyn_cast<TilingInterface>(targets.front());
  if (!tilingInterfaceOp) {
    targets.front()->emitError("Cannot tile op: Not a TilingInterface");
    return failure();
  }

  FailureOr<iree_compiler::IREE::LinalgExt::TilingResult> result =
      functional::applyReturningPatternAt(pattern, tilingInterfaceOp);
  if (failed(result))
    return failure();
  results.set(tiled_op().cast<OpResult>(), result->tiledOp);
  results.set(tile_op().cast<OpResult>(), result->tileOp.getOperation());
  return success();
}

FailureOr<scf::ForOp> transform::RewriteLinalgExtTileToScfForOp::applyToOne(
    LinalgExt::TileOp target) {
  LinalgExt::TileOpToSCFRewriter pattern(this->getContext());
  auto functionalRewrite =
      [&](LinalgExt::TileOp op,
          PatternRewriter &rewriter) -> FailureOr<scf::ForOp> {
    auto result = pattern.returningMatchAndRewrite(op, rewriter);
    if (failed(result))
      return failure();
    return result;
  };
  return functional::applyAt(target, functionalRewrite);
}

FailureOr<LinalgExt::InParallelOp>
transform::RewriteLinalgExtTileToInParallelOp::applyToOne(
    LinalgExt::TileOp target) {
  LinalgExt::TileOpToInParallelRewriter pattern(this->getContext());
  auto functionalRewrite =
      [&](LinalgExt::TileOp op,
          PatternRewriter &rewriter) -> FailureOr<LinalgExt::InParallelOp> {
    auto result = pattern.returningMatchAndRewrite(op, rewriter);
    if (failed(result))
      return failure();
    return result;
  };
  return functional::applyAt(target, functionalRewrite);
}

FailureOr<Operation *>
transform::RewriteLinalgExtInParallelToAsyncOp::applyToOne(
    LinalgExt::InParallelOp target) {
  LinalgExt::InParallelOpToAsyncRewriter pattern(this->getContext());
  auto functionalRewrite =
      [&](LinalgExt::InParallelOp op,
          PatternRewriter &rewriter) -> FailureOr<Operation *> {
    auto result = pattern.returningMatchAndRewrite(op, rewriter);
    if (failed(result))
      return failure();
    return result;
  };
  return functional::applyAt(target, functionalRewrite);
}

LogicalResult transform::RewriteLinalgExtInParallelToHALOp::apply(
    transform::TransformResults &results, transform::TransformState &state) {
  LinalgExt::InParallelOpToHALRewriter pattern(this->getContext());
  ArrayRef<Operation *> targets = state.getPayloadOps(target());
  return functional::applyReturningPatternAt(
      pattern, cast<LinalgExt::InParallelOp>(targets.front()));
}

FailureOr<scf::ForOp>
transform::RewriteLinalgExtInParallelToScfForOp::applyToOne(
    LinalgExt::InParallelOp target) {
  LinalgExt::InParallelOpToScfForRewriter pattern(this->getContext());
  auto functionalRewrite =
      [&](LinalgExt::InParallelOp op,
          PatternRewriter &rewriter) -> FailureOr<scf::ForOp> {
    auto result = pattern.returningMatchAndRewrite(op, rewriter);
    if (failed(result))
      return failure();
    return result;
  };
  return functional::applyAt(target, functionalRewrite);
}

#define GET_OP_CLASSES
#include "Dialect/LinalgTransform/LinalgTransformOps.cpp.inc"
