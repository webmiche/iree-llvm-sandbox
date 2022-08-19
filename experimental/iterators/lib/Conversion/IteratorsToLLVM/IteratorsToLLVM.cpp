//===-- IteratorsToLLVM.h - Conversion from Iterators to LLVM ---*- C++ -*-===//
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "iterators/Conversion/IteratorsToLLVM/IteratorsToLLVM.h"

#include "../PassDetail.h"
#include "IteratorAnalysis.h"
#include "iterators/Dialect/Iterators/IR/Iterators.h"
#include "iterators/Utils/MLIRSupport.h"
#include "mlir/Conversion/LLVMCommon/MemRefBuilder.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/Arithmetic/IR/Arithmetic.h"
#include "mlir/Dialect/Arithmetic/Utils/Utils.h"
#include "mlir/Dialect/Bufferization/IR/Bufferization.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/SCF/SCF.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/ADT/TypeSwitch.h"

namespace mlir {
class MLIRContext;
} // namespace mlir

using namespace mlir;
using namespace mlir::iterators;
using namespace mlir::LLVM;
using namespace mlir::func;
using namespace ::iterators;
using namespace std::string_literals;
using IteratorInfo = mlir::iterators::IteratorInfo;

namespace {
struct ConvertIteratorsToLLVMPass
    : public ConvertIteratorsToLLVMBase<ConvertIteratorsToLLVMPass> {
  void runOnOperation() override;
};
} // namespace

/// Maps types from the Iterators dialect to corresponding types in LLVM.
class IteratorsTypeConverter : public TypeConverter {
public:
  IteratorsTypeConverter(LLVMTypeConverter &llvmTypeConverter)
      : llvmTypeConverter(llvmTypeConverter) {
    addConversion([](Type type) { return type; });
    addConversion(convertColumnarBatchType);
    addConversion(convertTupleType);

    // Convert MemRefType using LLVMTypeConverter.
    addConversion([&](Type type) -> llvm::Optional<Type> {
      if (type.isa<MemRefType>())
        return llvmTypeConverter.convertType(type);
      return llvm::None;
    });
  }

private:
<<<<<<< HEAD
=======
  /// Maps a ColumnarBatchType to an LLVMStruct of pointers.
  static Optional<Type> convertColumnarBatchType(Type type) {
    if (auto batchType = type.dyn_cast<ColumnarBatchType>()) {
      Type i64 = IntegerType::get(type.getContext(), /*width=*/64);
      SmallVector<Type> fieldTypes{i64};
      fieldTypes.reserve(batchType.getTypes().size() + 1);
      llvm::transform(batchType.getTypes(), std::back_inserter(fieldTypes),
                      [](Type t) { return LLVMPointerType::get(t); });
      return LLVMStructType::getLiteral(type.getContext(), fieldTypes);
    }
    return llvm::None;
  }

>>>>>>> sql_iterators_runtime
  /// Maps a TupleType to a corresponding LLVMStructType.
  static Optional<Type> convertTupleType(Type type) {
    if (auto tupleType = type.dyn_cast<TupleType>()) {
      return LLVMStructType::getNewIdentified(type.getContext(), "tuple",
                                              tupleType.getTypes());
    }
    return llvm::None;
  }

  LLVMTypeConverter llvmTypeConverter;
};

/// Return a symbol reference to the printf function, inserting it into the
/// module if necessary.
static FlatSymbolRefAttr lookupOrInsertPrintf(OpBuilder &builder,
                                              ModuleOp module) {
  if (module.lookupSymbol<LLVMFuncOp>("printf"))
    return SymbolRefAttr::get(builder.getContext(), "printf");

  // Create a function declaration for printf, the signature is:
  //   * `i32 (i8*, ...)`
  LLVMPointerType charPointerType = LLVMPointerType::get(builder.getI8Type());
  LLVMFunctionType printfFunctionType =
      LLVMFunctionType::get(builder.getI32Type(), charPointerType,
                            /*isVarArg=*/true);

  // Insert the printf function into the body of the parent module.
  OpBuilder::InsertionGuard insertGuard(builder);
  builder.setInsertionPointToStart(module.getBody());
  builder.create<LLVMFuncOp>(module.getLoc(), "printf", printfFunctionType);
  return SymbolRefAttr::get(builder.getContext(), "printf");
}

/// Return a value representing an access into a global string with the given
/// name, creating the string if necessary.
static Value getOrCreateGlobalString(OpBuilder &builder, Twine name,
                                     Twine value, ModuleOp module,
                                     bool makeUnique) {
  Location loc = module->getLoc();

  // Determine name of global.
  llvm::SmallString<64> candidateName;
  name.toStringRef(candidateName);
  if (makeUnique) {
    int64_t uniqueNumber = 0;
    while (module.lookupSymbol(candidateName)) {
      (name + "." + Twine(uniqueNumber)).toStringRef(candidateName);
      uniqueNumber++;
    }
  }

  // Create the global at the entry of the module.
  GlobalOp global;
  StringAttr nameAttr = builder.getStringAttr(candidateName);
  if (!(global = module.lookupSymbol<GlobalOp>(nameAttr))) {
    StringAttr valueAttr = builder.getStringAttr(value);
    LLVMArrayType globalType =
        LLVMArrayType::get(builder.getI8Type(), valueAttr.size());
    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(module.getBody());
    global = builder.create<GlobalOp>(loc, globalType, /*isConstant=*/true,
                                      Linkage::Internal, nameAttr, valueAttr,
                                      /*alignment=*/0);
  }

  // Get the pointer to the first character in the global string.
  Value globalPtr = builder.create<AddressOfOp>(loc, global);
  Value zero = builder.create<LLVM::ConstantOp>(loc, builder.getI64Type(),
                                                builder.getI64IntegerAttr(0));
  return builder.create<GEPOp>(loc, LLVMPointerType::get(builder.getI8Type()),
                               globalPtr, ArrayRef<Value>({zero, zero}));
}

/// Lowers columnar_batch_from_memrefs to LLVM IR that extracts the bare
/// pointers and the number of elements from the given memrefs.
///
/// Possible result:
///
/// %1 = builtin.unrealized_conversion_cast %0 :
///        memref<3xi32> to !llvm.struct<(ptr<i32>, ptr<i32>, i64,
///                                       array<1 x i64>, array<1 x i64>)>
/// %2 = llvm.mlir.undef : !llvm.struct<(i64, ptr<i32>)>
/// %3 = llvm.extractvalue %1[1] :
///        !llvm.struct<(ptr<i32>, ptr<i32>, i64,
///                      array<1 x i64>, array<1 x i64>)>
/// %4 = llvm.extractvalue %1[3, 0] :
///        !llvm.struct<(ptr<i32>, ptr<i32>, i64,
///                      array<1 x i64>, array<1 x i64>)>
/// %5 = llvm.insertvalue %3, %2[1 : index] : !llvm.struct<(i64, ptr<i32>)>
/// %6 = llvm.insertvalue %4, %5[0 : index] : !llvm.struct<(i64, ptr<i32>)>
struct ColumnarBatchFromMemrefsLowering
    : public OpConversionPattern<ColumnarBatchFromMemrefsOp> {
  ColumnarBatchFromMemrefsLowering(TypeConverter &typeConverter,
                                   MLIRContext *context,
                                   PatternBenefit benefit = 1)
      : OpConversionPattern(typeConverter, context, benefit) {}

  LogicalResult match(ColumnarBatchFromMemrefsOp op) const override {
    return success();
  }

  void rewrite(ColumnarBatchFromMemrefsOp op, OpAdaptor adaptor,
               ConversionPatternRewriter &rewriter) const override {
    Location loc = op->getLoc();

    // Create empty struct for batch.
    Type batchStructType = typeConverter->convertType(op.batch().getType());
    Value batchStruct = rewriter.create<UndefOp>(loc, batchStructType);

    // Extract column pointers and number of elements.
    Value numElements;
    for (const auto &indexedOperand : llvm::enumerate(adaptor.getOperands())) {
      // Extract pointer and number of elements from memref descriptor.
      MemRefDescriptor descriptor(indexedOperand.value());
      Value ptr = descriptor.alignedPtr(rewriter, loc);
      if (indexedOperand.index() == 0) {
        numElements = descriptor.size(rewriter, loc, 0);
      }

      // Insert pointer into batch struct.
      auto idx = static_cast<int64_t>(indexedOperand.index()) + 1;
      batchStruct = createInsertValueOp(rewriter, loc, batchStruct, ptr, {idx});
    }

    // Insert number of elements.
    batchStruct =
        createInsertValueOp(rewriter, loc, batchStruct, numElements, {0});

    // Replace original op.
    rewriter.replaceOp(op, {batchStruct});
  }
};

struct ConstantTupleLowering : public OpConversionPattern<ConstantTupleOp> {
  ConstantTupleLowering(TypeConverter &typeConverter, MLIRContext *context,
                        PatternBenefit benefit = 1)
      : OpConversionPattern(typeConverter, context, benefit) {}

  LogicalResult match(ConstantTupleOp op) const override { return success(); }

  void rewrite(ConstantTupleOp op, OpAdaptor adaptor,
               ConversionPatternRewriter &rewriter) const override {
    Location loc = op->getLoc();

    // Convert tuple type.
    assert(op->getNumResults() == 1);
    Type tupleType = op.tuple().getType();
    Type structType = typeConverter->convertType(tupleType);

    // Undef.
    Value structValue = rewriter.create<UndefOp>(loc, structType);

    // Insert values.
    ArrayAttr values = op.values();
    assert(values);
    for (int i = 0; i < static_cast<int>(values.size()); i++) {
      // Create constant value op.
      Attribute field = values[i];
      Type fieldType = field.getType();
      auto valueOp = rewriter.create<LLVM::ConstantOp>(loc, fieldType, field);

      // Insert into struct.
      structValue =
          createInsertValueOp(rewriter, loc, structValue, valueOp, {i});
    }

    rewriter.replaceOp(op, structValue);
  }
};

/// Applies of 1-to-1 conversion of the given PrintTupleOp to a PrintOp.
struct PrintTupleOpLowering : public OpConversionPattern<PrintTupleOp> {
  PrintTupleOpLowering(TypeConverter &typeConverter, MLIRContext *context,
                       PatternBenefit benefit = 1)
      : OpConversionPattern(typeConverter, context, benefit) {}

  LogicalResult match(PrintTupleOp op) const override { return success(); }

  void rewrite(PrintTupleOp op, OpAdaptor adaptor,
               ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<PrintOp>(op, adaptor.tuple());
  }
};

struct PrintOpLowering : public OpConversionPattern<PrintOp> {
  PrintOpLowering(TypeConverter &typeConverter, MLIRContext *context,
                  PatternBenefit benefit = 1)
      : OpConversionPattern(typeConverter, context, benefit) {}

  LogicalResult match(PrintOp op) const override { return success(); }

  void rewrite(PrintOp op, OpAdaptor adaptor,
               ConversionPatternRewriter &rewriter) const override {
    Location loc = op->getLoc();
    Type i32 = rewriter.getI32Type();

    auto structType = adaptor.element().getType().dyn_cast<LLVMStructType>();
    assert(structType && "Only struct types supported for now");

    // Assemble format string in the form `(%lli, %lg, ...)`.
    std::string format("(");
    llvm::raw_string_ostream formatStream(format);
    llvm::interleaveComma(structType.getBody(), formatStream, [&](Type type) {
      // We extend lower-precision values later, so use the maximum width for
      // each type category.
      StringRef specifier;
      assert(type.isSignlessIntOrFloat());
      if (type.isSignlessInteger()) {
        specifier = "%lli";
      } else {
        specifier = "%lg";
      }
      formatStream << specifier;
    });
    format += ")\n\0"s;

    // Insert format string as global.
    auto module = op->getParentOfType<ModuleOp>();
    StringRef tupleName =
        (structType.isIdentified() ? structType.getName() : "anonymous_tuple");
    Value formatSpec = getOrCreateGlobalString(
        rewriter, Twine("frmt_spec.") + tupleName, format, module,
        /*makeUnique=*/!structType.isIdentified());

    // Assemble arguments to printf.
    SmallVector<Value, 8> values = {formatSpec};
    ArrayRef<Type> fieldTypes = structType.getBody();
    for (int i = 0; i < static_cast<int>(fieldTypes.size()); i++) {
      // Extract from struct.
      Type fieldType = fieldTypes[i];
      Value value = createExtractValueOp(rewriter, loc, fieldType,
                                         adaptor.element(), {i});

      // Extend.
      Value extValue;
      if (fieldType.isSignlessInteger()) {
        Type i64 = rewriter.getI64Type();
        extValue = rewriter.create<ZExtOp>(loc, i64, value);
      } else {
        Type f64 = rewriter.getF64Type();
        extValue = rewriter.create<FPExtOp>(loc, f64, value);
      }

      values.push_back(extValue);
    }

    // Generate call to printf.
    FlatSymbolRefAttr printfRef = lookupOrInsertPrintf(rewriter, module);
    rewriter.create<LLVM::CallOp>(loc, i32, printfRef, values);
    rewriter.eraseOp(op);
  }
};

//===----------------------------------------------------------------------===//
// ConstantStreamOp.
//===----------------------------------------------------------------------===//

/// Builds IR that resets the current index to 0. Possible result:
///
/// %0 = llvm.mlir.constant(0 : i32) : i32
/// %1 = llvm.insertvalue %0, %arg0[0 : index] :
///          !llvm.struct<"iterators.constant_stream_state", (i32)>
static Value buildOpenBody(ConstantStreamOp op, RewriterBase &rewriter,
                           Value initialState,
                           ArrayRef<IteratorInfo> upstreamInfos) {
  Location loc = op.getLoc();

  // Insert constant zero into state.
  Type i32 = rewriter.getI32Type();
  Attribute zeroAttr = rewriter.getI32IntegerAttr(0);
  Value zeroValue = rewriter.create<LLVM::ConstantOp>(loc, i32, zeroAttr);
  Value updatedState =
      createInsertValueOp(rewriter, loc, initialState, zeroValue, {0});

  return updatedState;
}

/// Creates a constant global array with the constant stream data provided in
/// the $value attribute of the given op.
static GlobalOp buildGlobalData(ConstantStreamOp op, RewriterBase &rewriter,
                                Type elementType) {
  Location loc = op->getLoc();

  // Find unique global name.
  auto module = op->getParentOfType<ModuleOp>();
  llvm::SmallString<64> candidateName;
  int64_t uniqueNumber = 0;
  while (true) {
    (Twine("iterators.constant_stream_data.") + Twine(uniqueNumber))
        .toStringRef(candidateName);
    if (!module.lookupSymbol(candidateName)) {
      break;
    }
    uniqueNumber++;
  }
  StringAttr nameAttr = rewriter.getStringAttr(candidateName);

  // Create global op.
  ArrayAttr valueAttr = op.value();
  LLVMArrayType globalType = LLVMArrayType::get(elementType, valueAttr.size());
  OpBuilder::InsertionGuard insertGuard(rewriter);
  rewriter.setInsertionPointToStart(module.getBody());
  auto globalArray =
      rewriter.create<GlobalOp>(loc, globalType, /*isConstant=*/true,
                                Linkage::Internal, nameAttr, Attribute());

  // Create initializer for global. Since arrays of arrays cannot be passed
  // to GlobalOp as attribute, we need to write an initializer that inserts
  // the data from the $value attribute one by one into the global array.
  rewriter.createBlock(&globalArray.getInitializer());
  Value initValue = rewriter.create<UndefOp>(loc, globalType);

  for (auto &elementAttr :
       llvm::enumerate(valueAttr.getAsValueRange<ArrayAttr>())) {
    Value structValue = rewriter.create<UndefOp>(loc, elementType);
    for (auto &fieldAttr : llvm::enumerate(elementAttr.value())) {
      auto value = rewriter.create<LLVM::ConstantOp>(
          loc, fieldAttr.value().getType(), fieldAttr.value());
      structValue =
          createInsertValueOp(rewriter, loc, structValue, value,
                              {static_cast<int64_t>(fieldAttr.index())});
    }
    initValue =
        createInsertValueOp(rewriter, loc, initValue, structValue,
                            {static_cast<int64_t>(elementAttr.index())});
  }

  rewriter.create<LLVM::ReturnOp>(loc, initValue);

  return globalArray;
}

/// Builds IR that produces the element at the current index and increments that
/// index. Also creates a global constant with the data. Pseudo-code:
///
/// if currentIndex > max: return {}
/// return data[currentIndex++]
///
/// Possible result:
///
/// llvm.mlir.global internal constant @iterators.constant_stream_data.0() :
///     !llvm.array<4 x !element_type> {
///   %0 = llvm.mlir.undef : !llvm.array<4 x !element_type>
///   // ...
///   llvm.return %n : !llvm.array<4 x !element_type>
/// }
/// // ...
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.constant_stream_state", (i32)>
/// %c4_i32 = arith.constant 4 : i32
/// %1 = arith.cmpi slt, %0, %c4_i32 : i32
/// %2:2 = scf.if %1 -> (!llvm.struct<"iterators.constant_stream_state", (i32)>,
///                      !element_type) {
///   %c1_i32 = arith.constant 1 : i32
///   %3 = arith.addi %0, %c1_i32 : i32
///   %4 = llvm.insertvalue %3, %arg0[0 : index] :
///            !llvm.struct<"iterators.constant_stream_state", (i32)>
///   %5 = llvm.mlir.addressof @iterators.constant_stream_data.0 :
///            !llvm.ptr<array<4 x !element_type>>
///   %c0_i32 = arith.constant 0 : i32
///   %6 = llvm.getelementptr %5[%c0_i32, %0] :
///            (!llvm.ptr<array<4 x !element_type>>, i32, i32)
///                -> !llvm.ptr<!element_type>
///   %7 = llvm.load %6 : !llvm.ptr<!element_type>
///   scf.yield %4, %7 :
///       !llvm.struct<"iterators.constant_stream_state", (i32)>, !element_type
/// } else {
///   %3 = llvm.mlir.undef : !element_type
///   scf.yield %arg0, %3 :
///       !llvm.struct<"iterators.constant_stream_state", (i32)>, !element_type
/// }
static llvm::SmallVector<Value, 4>
buildNextBody(ConstantStreamOp op, RewriterBase &rewriter, Value initialState,
              ArrayRef<IteratorInfo> upstreamInfos, Type elementType) {
  Type i32 = rewriter.getI32Type();
  Location loc = op->getLoc();

  // Extract current index.
  Value currentIndex =
      createExtractValueOp(rewriter, loc, i32, initialState, {0});

  // Test if we have reached the end of the range.
  int64_t numElements = op.value().size();
  Value lastIndex =
      rewriter.create<arith::ConstantIntOp>(loc, /*value=*/numElements,
                                            /*width=*/32);
  ArithBuilder ab(rewriter, op.getLoc());
  Value hasNext = ab.slt(currentIndex, lastIndex);
  auto ifOp = rewriter.create<scf::IfOp>(
      loc, TypeRange{initialState.getType(), elementType}, hasNext,
      /*thenBuilder=*/
      [&](OpBuilder &builder, Location loc) {
        // Increment index and update state.
        Value one = builder.create<arith::ConstantIntOp>(loc, /*value=*/1,
                                                         /*width=*/32);
        ArithBuilder ab(builder, loc);
        Value updatedCurrentIndex = ab.add(currentIndex, one);
        Value updatedState = createInsertValueOp(rewriter, loc, initialState,
                                                 updatedCurrentIndex, {0});

        // Load element from global data at current index.
        GlobalOp globalArray = buildGlobalData(op, rewriter, elementType);
        Value globalPtr = rewriter.create<AddressOfOp>(loc, globalArray);
        Value zero = rewriter.create<arith::ConstantIntOp>(loc, /*value=*/0,
                                                           /*width=*/32);
        Value gep =
            rewriter.create<GEPOp>(loc, LLVMPointerType::get(elementType),
                                   globalPtr, ValueRange{zero, currentIndex});
        Value nextElement = rewriter.create<LoadOp>(loc, gep);

        builder.create<scf::YieldOp>(loc,
                                     ValueRange{updatedState, nextElement});
      },
      /*elseBuilder=*/
      [&](OpBuilder &builder, Location loc) {
        // Don't modify state; return undef element.
        Value nextElement = rewriter.create<UndefOp>(loc, elementType);
        builder.create<scf::YieldOp>(loc,
                                     ValueRange{initialState, nextElement});
      });

  Value finalState = ifOp->getResult(0);
  Value nextElement = ifOp.getResult(1);
  return {finalState, hasNext, nextElement};
}

/// Forwards the initial state. The ConstantStreamOp doesn't do anything on
/// Close.
static Value buildCloseBody(ConstantStreamOp op, RewriterBase &rewriter,
                            Value initialState,
                            ArrayRef<IteratorInfo> upstreamInfos) {
  return initialState;
}

/// Builds IR that creates an initial iterator state consisting of an
/// (uninitialized) current index. Possible result:
///
/// %0 = llvm.mlir.constant(0 : i32) : i32
/// %1 = llvm.insertvalue %0, %arg0[0 : index] :
///          !llvm.struct<"iterators.constant_stream_state", (i32)>
/// return %1 : !llvm.struct<"iterators.constant_stream_state", (i32)>
static Value buildStateCreation(ConstantStreamOp op,
                                ConstantStreamOp::Adaptor /*adaptor*/,
                                RewriterBase &rewriter,
                                LLVM::LLVMStructType stateType) {
  return rewriter.create<UndefOp>(op.getLoc(), stateType);
}

//===----------------------------------------------------------------------===//
// FilterOp.
//===----------------------------------------------------------------------===//

/// Builds IR that opens the nested upstream iterator. Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.filter_state", !nested_state>
/// %1 = call @iterators.upstream.open.0(%0) :
///          (!nested_state) -> !nested_state
/// %2 = llvm.insertvalue %1, %arg0[0 : index] :
///          !llvm.struct<"iterators.filter_state", (!nested_state)>
static Value buildOpenBody(FilterOp op, RewriterBase &rewriter,
                           Value initialState,
                           ArrayRef<IteratorInfo> upstreamInfos) {
  Location loc = op.getLoc();
  Type upstreamStateType = upstreamInfos[0].stateType;

  // Extract upstream state.
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Call Open on upstream.
  SymbolRefAttr openFunc = upstreamInfos[0].openFunc;
  auto openCallOp = rewriter.create<func::CallOp>(
      loc, openFunc, upstreamStateType, initialUpstreamState);

  // Update upstream state.
  Value updatedUpstreamState = openCallOp->getResult(0);
  Value updatedState = createInsertValueOp(rewriter, loc, initialState,
                                           updatedUpstreamState, {0});

  return updatedState;
}

/// Builds IR that consumes all elements of the upstream iterator and returns
/// a stream of those that pass the given precicate. Pseudo-code:
///
/// while (nextTuple = upstream->Next()):
///     if precicate(nextTuple):
///         return nextTuple
/// return {}
///
/// Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.filter_state", (!nested_state)>
/// %1:3 = scf.while (%arg1 = %0) :
///            (!nested_state) -> (!nested_state, i1, !element_type) {
///   %3:3 = func.call @iterators.upstream.next.0(%arg1) :
///              (!nested_state) -> (!nested_state, i1, !element_type)
///   %4 = scf.if %3#1 -> (i1) {
///     %7 = func.call @predicate(%3#2) : (!element_type) -> i1
///     scf.yield %7 : i1
///   } else {
///     scf.yield %3#1 : i1
///   }
///   %true = arith.constant true
///   %5 = arith.xori %4, %true : i1
///   %6 = arith.andi %3#1, %5 : i1
///   scf.condition(%6) %3#0, %3#1, %3#2 : !nested_state, i1, !element_type
/// } do {
/// ^bb0(%arg1: !nested_state, %arg2: i1, %arg3: !element_type):
///   scf.yield %arg1 : !nested_state
/// }
/// %2 = llvm.insertvalue %1#0, %arg0[0 : index] :
///          !llvm.struct<"iterators.filter_state", (!nested_state)>
static llvm::SmallVector<Value, 4>
buildNextBody(FilterOp op, RewriterBase &rewriter, Value initialState,
              ArrayRef<IteratorInfo> upstreamInfos, Type elementType) {
  Location loc = op.getLoc();

  // Extract upstream state.
  Type upstreamStateType = upstreamInfos[0].stateType;
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Main while loop.
  Type i1 = rewriter.getI1Type();
  SmallVector<Type> nextResultTypes = {upstreamStateType, i1, elementType};
  scf::WhileOp whileOp = scf::createWhileOp(
      rewriter, loc, nextResultTypes, initialUpstreamState,
      /*beforeBuilder=*/
      [&](OpBuilder &builder, Location loc, Block::BlockArgListType args) {
        Value upstreamState = args[0];
        SymbolRefAttr nextFunc = upstreamInfos[0].nextFunc;
        auto nextCall = builder.create<func::CallOp>(
            loc, nextFunc, nextResultTypes, upstreamState);
        Value hasNext = nextCall->getResult(1);
        Value nextElement = nextCall->getResult(2);

        // If we got an element, apply predicate.
        auto ifOp = rewriter.create<scf::IfOp>(
            loc, i1, hasNext,
            /*ifBuilder=*/
            [&](OpBuilder &builder, Location loc) {
              // Call predicate.
              auto predicateCall = builder.create<func::CallOp>(
                  loc, i1, op.predicateRef(), ValueRange{nextElement});
              Value isMatch = predicateCall->getResult(0);
              builder.create<scf::YieldOp>(loc, isMatch);
            },
            /*elseBuilder=*/
            [&](OpBuilder &builder, Location loc) {
              // Note: hasNext is false in this branch.
              builder.create<scf::YieldOp>(loc, hasNext);
            });
        Value hasMatchingNext = ifOp->getResult(0);

        // Build condition: continue loop if (1) we did get an element from
        // upstream (i.e., hasNext) and (2) that element did not match,i.e.,
        // !hasMatchingNext = xor(hasMatchingNext, true).
        Value constTrue =
            builder.create<arith::ConstantIntOp>(loc, /*value=*/1, /*width=*/1);
        Value hasNoMatchingNext =
            builder.create<arith::XOrIOp>(loc, hasMatchingNext, constTrue);
        Value loopCondition =
            builder.create<arith::AndIOp>(loc, hasNext, hasNoMatchingNext);

        builder.create<scf::ConditionOp>(loc, loopCondition,
                                         nextCall->getResults());
      },
      /*afterBuilder=*/
      [&](OpBuilder &builder, Location loc, Block::BlockArgListType args) {
        Value upstreamState = args[0];
        builder.create<scf::YieldOp>(loc, upstreamState);
      });

  // Update state.
  Value finalUpstreamState = whileOp->getResult(0);
  Value finalState =
      createInsertValueOp(rewriter, loc, initialState, finalUpstreamState, {0});
  Value hasNext = whileOp->getResult(1);
  Value nextElement = whileOp->getResult(2);

  return {finalState, hasNext, nextElement};
}

/// Builds IR that closes the nested upstream iterator. Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.filter_state", (!nested_state)>
/// %1 = call @iterators.upstream.close.0(%0) :
///          (!nested_state) -> !nested_state
/// %2 = llvm.insertvalue %1, %arg0[0 : index] :
///          !llvm.struct<"iterators.filter_state", (!nested_state)>
static Value buildCloseBody(FilterOp op, RewriterBase &rewriter,
                            Value initialState,
                            ArrayRef<IteratorInfo> upstreamInfos) {
  Location loc = op.getLoc();
  Type upstreamStateType = upstreamInfos[0].stateType;

  // Extract upstream state.
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Call Close on upstream.
  SymbolRefAttr closeFunc = upstreamInfos[0].closeFunc;
  auto closeCallOp = rewriter.create<func::CallOp>(
      loc, closeFunc, upstreamStateType, initialUpstreamState);

  // Update upstream state.
  Value updatedUpstreamState = closeCallOp->getResult(0);
  return createInsertValueOp(rewriter, loc, initialState, updatedUpstreamState,
                             {0})
      .getResult();
}

/// Builds IR that initializes the iterator state with the state of the upstream
/// iterator. Possible output:
///
/// %0 = ...
/// %1 = llvm.mlir.undef : !llvm.struct<"iterators.filter_state",
///                                     (!nested_state)>
/// %2 = llvm.insertvalue %0, %1[0 : index] :
///          !llvm.struct<"iterators.filter_state", (!nested_state)>
static Value buildStateCreation(FilterOp op, FilterOp::Adaptor adaptor,
                                RewriterBase &rewriter,
                                LLVM::LLVMStructType stateType) {
  Location loc = op.getLoc();
  Value undefState = rewriter.create<UndefOp>(loc, stateType);
  Value upstreamState = adaptor.input();
  return createInsertValueOp(rewriter, loc, undefState, upstreamState, {0});
}

//===----------------------------------------------------------------------===//
// MapOp.
//===----------------------------------------------------------------------===//

/// Builds IR that opens the nested upstream iterator. Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.map_state", !nested_state>
/// %1 = call @iterators.upstream.open.0(%0) :
///          (!nested_state) -> !nested_state
/// %2 = llvm.insertvalue %1, %arg0[0 : index] :
///          !llvm.struct<"iterators.map_state", (!nested_state)>
static Value buildOpenBody(MapOp op, RewriterBase &rewriter, Value initialState,
                           ArrayRef<IteratorInfo> upstreamInfos) {
  Location loc = op.getLoc();
  Type upstreamStateType = upstreamInfos[0].stateType;

  // Extract upstream state.
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Call Open on upstream.
  SymbolRefAttr openFunc = upstreamInfos[0].openFunc;
  auto openCallOp = rewriter.create<func::CallOp>(
      loc, openFunc, upstreamStateType, initialUpstreamState);

  // Update upstream state.
  Value updatedUpstreamState = openCallOp->getResult(0);
  Value updatedState = createInsertValueOp(rewriter, loc, initialState,
                                           updatedUpstreamState, {0});

  return updatedState;
}

/// Builds IR that consumes all elements of the upstream iterator and returns
/// a stream where each original element is mapped to/transformed into a new
/// element using the given map function. Pseudo-code:
///
/// if (nextTuple = upstream->Next()):
///     return mapFunc(nextTuple)
/// return {}
///
/// Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.map_state", (!nested_state)>
/// %1:3 = scf.while (%arg1 = %0) :
///            (!nested_state) -> (!nested_state, i1, !element_type) {
///   %3:3 = func.call @iterators.upstream.next.0(%arg1) :
///              (!nested_state) -> (!nested_state, i1, !element_type)
///   %4 = scf.if %3#1 -> (i1) {
///     %7 = func.call @predicate(%3#2) : (!element_type) -> i1
///     scf.yield %7 : i1
///   } else {
///     scf.yield %3#1 : i1
///   }
///   %true = arith.constant true
///   %5 = arith.xori %4, %true : i1
///   %6 = arith.andi %3#1, %5 : i1
///   scf.condition(%6) %3#0, %3#1, %3#2 : !nested_state, i1, !element_type
/// } do {
/// ^bb0(%arg1: !nested_state, %arg2: i1, %arg3: !element_type):
///   scf.yield %arg1 : !nested_state
/// }
/// %2 = llvm.insertvalue %1#0, %arg0[0 : index] :
///          !llvm.struct<"iterators.map_state", (!nested_state)>
static llvm::SmallVector<Value, 4>
buildNextBody(MapOp op, RewriterBase &rewriter, Value initialState,
              ArrayRef<IteratorInfo> upstreamInfos, Type elementType) {
  Location loc = op.getLoc();

  // Extract upstream state.
  Type upstreamStateType = upstreamInfos[0].stateType;
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Extract input element type.
  StreamType inputStreamType = op.input().getType().dyn_cast<StreamType>();
  assert(inputStreamType);
  Type inputElementType = inputStreamType.getElementType();

  // Call next.
  Type i1 = rewriter.getI1Type();
  SmallVector<Type> nextResultTypes = {upstreamStateType, i1, inputElementType};
  SymbolRefAttr nextFunc = upstreamInfos[0].nextFunc;
  auto nextCall = rewriter.create<func::CallOp>(loc, nextFunc, nextResultTypes,
                                                initialUpstreamState);
  Value hasNext = nextCall->getResult(1);
  Value nextElement = nextCall->getResult(2);

  // If we got an element, apply map function.
  auto ifOp = rewriter.create<scf::IfOp>(
      loc, elementType, hasNext,
      /*ifBuilder=*/
      [&](OpBuilder &builder, Location loc) {
        // Apply map function.
        auto mapCall = builder.create<func::CallOp>(
            loc, elementType, op.mapFuncRef(), ValueRange{nextElement});
        Value mappedElement = mapCall->getResult(0);
        builder.create<scf::YieldOp>(loc, mappedElement);
      },
      /*elseBuilder=*/
      [&](OpBuilder &builder, Location loc) {
        // Return undefined value.
        Value undef = builder.create<LLVM::UndefOp>(loc, elementType);
        builder.create<scf::YieldOp>(loc, undef);
      });
  Value mappedElement = ifOp.getResult(0);

  // Update state.
  Value finalUpstreamState = nextCall.getResult(0);
  Value finalState =
      createInsertValueOp(rewriter, loc, initialState, finalUpstreamState, {0});

  return {finalState, hasNext, mappedElement};
}

/// Builds IR that closes the nested upstream iterator. Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.map_state", (!nested_state)>
/// %1 = call @iterators.upstream.close.0(%0) :
///          (!nested_state) -> !nested_state
/// %2 = llvm.insertvalue %1, %arg0[0 : index] :
///          !llvm.struct<"iterators.map_state", (!nested_state)>
static Value buildCloseBody(MapOp op, RewriterBase &rewriter,
                            Value initialState,
                            ArrayRef<IteratorInfo> upstreamInfos) {
  Location loc = op.getLoc();
  Type upstreamStateType = upstreamInfos[0].stateType;

  // Extract upstream state.
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Call Close on upstream.
  SymbolRefAttr closeFunc = upstreamInfos[0].closeFunc;
  auto closeCallOp = rewriter.create<func::CallOp>(
      loc, closeFunc, upstreamStateType, initialUpstreamState);

  // Update upstream state.
  Value updatedUpstreamState = closeCallOp->getResult(0);
  return createInsertValueOp(rewriter, loc, initialState, updatedUpstreamState,
                             {0})
      .getResult();
}

/// Builds IR that initializes the iterator state with the state of the upstream
/// iterator. Possible output:
///
/// %0 = ...
/// %1 = llvm.mlir.undef : !llvm.struct<"iterators.map_state", (!nested_state)>
/// %2 = llvm.insertvalue %0, %1[0 : index] :
///          !llvm.struct<"iterators.filter_state", (!nested_state)>
static Value buildStateCreation(MapOp op, MapOp::Adaptor adaptor,
                                RewriterBase &rewriter,
                                LLVM::LLVMStructType stateType) {
  Location loc = op.getLoc();
  Value undefState = rewriter.create<UndefOp>(loc, stateType);
  Value upstreamState = adaptor.input();
  return createInsertValueOp(rewriter, loc, undefState, upstreamState, {0});
}

//===----------------------------------------------------------------------===//
// ReduceOp.
//===----------------------------------------------------------------------===//

/// Builds IR that opens the nested upstream iterator. Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.reduce_state", !nested_state>
/// %1 = call @iterators.upstream.open.0(%0) :
///          (!nested_state) -> !nested_state
/// %2 = llvm.insertvalue %1, %arg0[0 : index] :
///          !llvm.struct<"iterators.reduce_state", (!nested_state)>
static Value buildOpenBody(ReduceOp op, RewriterBase &rewriter,
                           Value initialState,
                           ArrayRef<IteratorInfo> upstreamInfos) {
  Location loc = op.getLoc();
  Type upstreamStateType = upstreamInfos[0].stateType;

  // Extract upstream state.
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Call Open on upstream.
  SymbolRefAttr openFunc = upstreamInfos[0].openFunc;
  auto openCallOp = rewriter.create<func::CallOp>(
      loc, openFunc, upstreamStateType, initialUpstreamState);

  // Update upstream state.
  Value updatedUpstreamState = openCallOp->getResult(0);
  Value updatedState = createInsertValueOp(rewriter, loc, initialState,
                                           updatedUpstreamState, {0});

  return updatedState;
}

/// Builds IR that consumes all elements of the upstream iterator and combines
/// them into a single one using the given reduce function. Pseudo-code:
///
/// accumulator = upstream->Next()
/// if !accumulator: return {}
/// while (nextTuple = upstream->Next()):
///     accumulator = reduce(accumulator, nextTuple)
/// return accumulator
///
/// Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.reduce_state", (!nested_state)>
/// %1:3 = call @iterators.upstream.next.0(%0) :
///            (!nested_state) -> (!nested_state, i1, !element_type)
/// %2:3 = scf.if %1#1 -> (!nested_state, i1, !element_type) {
///   %4:3 = scf.while (%arg1 = %1#0, %arg2 = %1#2) :
///              (!nested_state, !element_type) ->
///                 (!nested_state, !element_type, !element_type) {
///     %5:3 = call @iterators.upstream.next.0(%arg1) :
///                (!nested_state) -> (!nested_state, i1, !element_type)
///     scf.condition(%5#1) %5#0, %5#2, %arg2 :
///         !nested_state, !element_type, !element_type
///   } do {
///   ^bb0(%arg1: !nested_state, %arg2: !element_type, %arg3: !element_type):
///     %5 = call @reduce_func(%arg2, %arg3) :
///              (!element_type, !element_type) -> !element_type
///     scf.yield %arg1, %8 : !nested_state, !element_type
///   }
///   %true = arith.constant true
///   scf.yield %4#0, %true, %4#2 : !nested_state, i1, !element_type
/// } else {
///   scf.yield %1#0, %1#1, %1#2 : !nested_state, i1, !element_type
/// }
/// %3 = llvm.insertvalue %2#0, %arg0[0 : index] :
///          !llvm.struct<"iterators.reduce_state", (!nested_state)>
static llvm::SmallVector<Value, 4>
buildNextBody(ReduceOp op, RewriterBase &rewriter, Value initialState,
              ArrayRef<IteratorInfo> upstreamInfos, Type elementType) {
  Location loc = op.getLoc();

  // Extract upstream state.
  Type upstreamStateType = upstreamInfos[0].stateType;
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Get first result from upstream.
  Type i1 = rewriter.getI1Type();
  SmallVector<Type> nextResultTypes = {upstreamStateType, i1, elementType};
  SymbolRefAttr nextFunc = upstreamInfos[0].nextFunc;
  auto firstNextCall = rewriter.create<func::CallOp>(
      loc, nextFunc, nextResultTypes, initialUpstreamState);

  // Check for empty upstream.
  Value firstHasNext = firstNextCall->getResult(1);
  auto ifOp = rewriter.create<scf::IfOp>(
      loc, nextResultTypes, firstHasNext,
      /*ifBuilder=*/
      [&](OpBuilder &builder, Location loc) {
        // Create while loop.
        Value firstCallUpstreamState = firstNextCall->getResult(0);
        Value firstCallElement = firstNextCall->getResult(2);
        SmallVector<Value> whileInputs = {firstCallUpstreamState,
                                          firstCallElement};
        SmallVector<Type> whileResultTypes = {
            upstreamStateType, // Updated upstream state.
            elementType,       // Accumulator.
            elementType        // Element from last next call.
        };
        scf::WhileOp whileOp = scf::createWhileOp(
            builder, loc, whileResultTypes, whileInputs,
            /*beforeBuilder=*/
            [&](OpBuilder &builder, Location loc,
                Block::BlockArgListType args) {
              Value upstreamState = args[0];
              Value accumulator = args[1];
              auto nextCall = builder.create<func::CallOp>(
                  loc, nextFunc, nextResultTypes, upstreamState);

              Value updatedUpstreamState = nextCall->getResult(0);
              Value hasNext = nextCall->getResult(1);
              Value maybeNextElement = nextCall->getResult(2);
              builder.create<scf::ConditionOp>(loc, hasNext,
                                               ValueRange{updatedUpstreamState,
                                                          accumulator,
                                                          maybeNextElement});
            },
            /*afterBuilder=*/
            [&](OpBuilder &builder, Location loc,
                Block::BlockArgListType args) {
              Value upstreamState = args[0];
              Value accumulator = args[1];
              Value nextElement = args[2];

              // Call reduce function.
              auto reduceCall = builder.create<func::CallOp>(
                  loc, elementType, op.reduceFuncRef(),
                  ValueRange{accumulator, nextElement});
              Value newAccumulator = reduceCall->getResult(0);

              builder.create<scf::YieldOp>(
                  loc, ValueRange{upstreamState, newAccumulator});
            });

        // The "then" branch of ifOp returns the result of whileOp.
        Value constTrue =
            builder.create<arith::ConstantIntOp>(loc, /*value=*/1, /*width=*/1);
        Value updatedUpstreamState = whileOp->getResult(0);
        Value accumulator = whileOp->getResult(1);
        builder.create<scf::YieldOp>(
            loc, ValueRange{updatedUpstreamState, constTrue, accumulator});
      },
      /*elseBuilder=*/
      [&](OpBuilder &builder, Location loc) {
        // This branch is taken when the first call to upstream's next does
        // not return anything. In this case, that "end-of-stream" signal is
        // the result of the reduction and the upstream state is final, so we
        // just forward the results of the call to next.
        builder.create<scf::YieldOp>(loc, firstNextCall->getResults());
      });

  // Update state.
  Value finalUpstreamState = ifOp->getResult(0);
  Value finalState =
      createInsertValueOp(rewriter, loc, initialState, finalUpstreamState, {0});
  Value hasNext = ifOp->getResult(1);
  Value nextElement = ifOp->getResult(2);

  return {finalState, hasNext, nextElement};
}

/// Builds IR that closes the nested upstream iterator. Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.reduce_state", (!nested_state)>
/// %1 = call @iterators.upstream.close.0(%0) :
///          (!nested_state) -> !nested_state
/// %2 = llvm.insertvalue %1, %arg0[0 : index] :
///          !llvm.struct<"iterators.reduce_state", (!nested_state)>
static Value buildCloseBody(ReduceOp op, RewriterBase &rewriter,
                            Value initialState,
                            ArrayRef<IteratorInfo> upstreamInfos) {
  Location loc = op.getLoc();
  Type upstreamStateType = upstreamInfos[0].stateType;

  // Extract upstream state.
  Value initialUpstreamState =
      createExtractValueOp(rewriter, loc, upstreamStateType, initialState, {0});

  // Call Close on upstream.
  SymbolRefAttr closeFunc = upstreamInfos[0].closeFunc;
  auto closeCallOp = rewriter.create<func::CallOp>(
      loc, closeFunc, upstreamStateType, initialUpstreamState);

  // Update upstream state.
  Value updatedUpstreamState = closeCallOp->getResult(0);
  return createInsertValueOp(rewriter, loc, initialState, updatedUpstreamState,
                             {0})
      .getResult();
}

/// Builds IR that initializes the iterator state with the state of the upstream
/// iterator. Possible output:
///
/// %0 = ...
/// %1 = llvm.mlir.undef : !llvm.struct<"iterators.reduce_state",
///                                     (!nested_state)>
/// %2 = llvm.insertvalue %0, %1[0 : index] :
///          !llvm.struct<"iterators.reduce_state", (!nested_state)>
static Value buildStateCreation(ReduceOp op, ReduceOp::Adaptor adaptor,
                                RewriterBase &rewriter,
                                LLVM::LLVMStructType stateType) {
  Location loc = op.getLoc();
  Value undefState = rewriter.create<UndefOp>(loc, stateType);
  Value upstreamState = adaptor.input();
  return createInsertValueOp(rewriter, loc, undefState, upstreamState, {0});
}

//===----------------------------------------------------------------------===//
// ScanColumnarBatchOp.
//===----------------------------------------------------------------------===//

/// Builds IR that (re) sets the current index to zero. Possible output:
///
/// %0 = llvm.mlir.constant(0 : i64) : i64
/// %1 = llvm.insertvalue %0, %arg0[0 : index] :
///          !llvm.struct<"iterators.scan_columnar_batch_state",
///                       (i64, !columnnar_batch_type)>
static Value buildOpenBody(ScanColumnarBatchOp op, RewriterBase &rewriter,
                           Value initialState,
                           ArrayRef<IteratorInfo> upstreamInfos) {
  Location loc = op.getLoc();

  // Insert constant zero into state.
  Type i64 = rewriter.getI64Type();
  Attribute zeroAttr = rewriter.getI64IntegerAttr(0);
  Value zeroValue = rewriter.create<LLVM::ConstantOp>(loc, i64, zeroAttr);
  Value updatedState =
      createInsertValueOp(rewriter, loc, initialState, zeroValue, {0});

  return updatedState;
}

/// Builds IR that assembles an element from the values at the current index and
/// increments that index. Pseudo-code: Pseudo-code:
///
/// tuple = (tensor[current_index] for tensor in input)
/// current_index++
/// return tuple
///
/// Possible output:
///
/// %0 = llvm.extractvalue %arg0[0 : index] :
///          !llvm.struct<"iterators.scan_columnar_batch_state",
///                       (i64, !columnar_batch_type)>
/// %1 = llvm.extractvalue %arg0[1 : index] :
///          !llvm.struct<"iterators.scan_columnar_batch_state",
///                       (i64, !columnar_batch_type)>
/// %2 = llvm.extractvalue %1[0 : index] : !llvm.!columnar_batch_type
/// %3 = arith.cmpi slt, %0, %2 : i64
/// %4:2 = scf.if %3 -> (!llvm.struct<"iterators.scan_columnar_batch_state",
///                                   (i64, !columnar_batch_type)>,
///                      !element_type) {
///   %c1_i64 = arith.constant 1 : i64
///   %5 = arith.addi %0, %c1_i64 : i64
///   %6 = llvm.insertvalue %5, %arg0[0 : index] :
///            !llvm.struct<"iterators.scan_columnar_batch_state",
///                         (i64, !columnar_batch_type)>
///   %7 = llvm.mlir.undef : !element_type
///   %8 = llvm.extractvalue %1[1 : index] : !llvm.!columnar_batch_type
///   %9 = llvm.getelementptr %8[%0] :
///            (!llvm.ptr<!column_type_0>, i64) -> !llvm.ptr<!column_type_0>
///   %10 = llvm.load %9 : !llvm.ptr<!column_type_0>
///   %11 = llvm.insertvalue %10, %7[0 : index] : !element_type
///   scf.yield %6, %11 :
///       !llvm.struct<"iterators.scan_columnar_batch_state",
///                    (i64, !columnar_batch_type)>,
///       !element_type
/// } else {
///   %5 = llvm.mlir.undef : !element_type
///   scf.yield %arg0, %5 :
///       !llvm.struct<"iterators.scan_columnar_batch_state",
///                    (i64, !columnar_batch_type)>,
///       !element_type
/// }
static llvm::SmallVector<Value, 4>
buildNextBody(ScanColumnarBatchOp op, RewriterBase &rewriter,
              Value initialState, ArrayRef<IteratorInfo> upstreamInfos,
              Type elementType) {
  Type i64 = rewriter.getI64Type();
  Location loc = op->getLoc();
  auto elementStructType = elementType.cast<LLVMStructType>();

  // Extract current index.
  Value currentIndex =
      createExtractValueOp(rewriter, loc, i64, initialState, {0});

  // Extract input batch.
  auto stateType = initialState.getType().cast<LLVMStructType>();
  Type convertedInputBatchType = stateType.getBody()[1];
  Value inputBatch = createExtractValueOp(
      rewriter, loc, convertedInputBatchType, initialState, {1});

  // Test if we have reached the end of the range.
  Value lastIndex = createExtractValueOp(rewriter, loc, i64, inputBatch, {0});

  ArithBuilder ab(rewriter, op.getLoc());
  Value hasNext = ab.slt(currentIndex, lastIndex);
  auto ifOp = rewriter.create<scf::IfOp>(
      loc, TypeRange{initialState.getType(), elementType}, hasNext,
      /*thenBuilder=*/
      [&](OpBuilder &builder, Location loc) {
        // Increment index and update state.
        Value one = builder.create<arith::ConstantIntOp>(loc, /*value=*/1,
                                                         /*width=*/64);
        ArithBuilder ab(builder, loc);
        Value updatedCurrentIndex = ab.add(currentIndex, one);
        Value updatedState = createInsertValueOp(rewriter, loc, initialState,
                                                 updatedCurrentIndex, {0});

        // Assemble field values from values at current index of column
        // pointers.
        Value nextElement = rewriter.create<UndefOp>(loc, elementType);
        for (const auto &indexedFieldType :
             llvm::enumerate(elementStructType.getBody())) {
          auto fieldIndex = static_cast<int64_t>(indexedFieldType.index());
          Type fieldType = indexedFieldType.value();
          Type columnPointerType = LLVMPointerType::get(fieldType);

          // Extract column pointer.
          Value columnPtr = createExtractValueOp(
              rewriter, loc, columnPointerType, inputBatch, {fieldIndex + 1});

          // Get element pointer.
          Value gep = rewriter.create<GEPOp>(loc, columnPointerType, columnPtr,
                                             ValueRange{currentIndex});

          // Load.
          Value fieldValue = rewriter.create<LoadOp>(loc, gep);

          // Insert into next element struct.
          nextElement = createInsertValueOp(rewriter, loc, nextElement,
                                            fieldValue, {fieldIndex});
        }

        builder.create<scf::YieldOp>(loc,
                                     ValueRange{updatedState, nextElement});
      },
      /*elseBuilder=*/
      [&](OpBuilder &builder, Location loc) {
        // Don't modify state; return undef element.
        Value nextElement = rewriter.create<UndefOp>(loc, elementType);
        builder.create<scf::YieldOp>(loc,
                                     ValueRange{initialState, nextElement});
      });

  Value finalState = ifOp->getResult(0);
  Value nextElement = ifOp.getResult(1);
  return {finalState, hasNext, nextElement};
}

/// Builds IR that does nothing. The ScanColumnarBatchOp does not need to do
/// anything on close.
static Value buildCloseBody(ScanColumnarBatchOp /*op*/,
                            RewriterBase & /*rewriter*/, Value initialState,
                            ArrayRef<IteratorInfo> /*upstreamInfos*/) {
  return initialState;
}

/// Builds IR that initializes the iterator state with the input columnar batch
/// and an undefined current index. Possible output:
///
/// %0 = ...
/// %1 = llvm.mlir.undef : !llvm.struct<"iterators.scan_columnar_batch_state",
///                                     (i64, !columnar_batch_type)>
/// %2 = llvm.insertvalue %0, %1[1 : index] :
///          !llvm.struct<"iterators.scan_columnar_batch_state",
///                       (i64, !columnar_batch_type)>
static Value buildStateCreation(ScanColumnarBatchOp op,
                                ScanColumnarBatchOp::Adaptor adaptor,
                                RewriterBase &rewriter,
                                LLVM::LLVMStructType stateType) {
  Location loc = op.getLoc();

  // Insert input into iterator state.
  Value iteratorState = rewriter.create<UndefOp>(loc, stateType);
  Value batch = adaptor.batch();
  iteratorState = createInsertValueOp(rewriter, loc, iteratorState, batch, {1});

  return iteratorState;
}

//===----------------------------------------------------------------------===//
// Helpers for creating Open/Next/Close functions and state creation.
//===----------------------------------------------------------------------===//

using OpenNextCloseBodyBuilder =
    llvm::function_ref<llvm::SmallVector<Value, 4>(RewriterBase &, Value)>;

/// Creates a new Open/Next/Close function at the parent module of originalOp
/// with the given types and name, initializes the function body with a first
/// block, and fills that block with the given builder. Since these functions
/// are only used by the iterators in this module, they are created with private
/// visibility.
static FuncOp
buildOpenNextCloseInParentModule(Operation *originalOp, RewriterBase &rewriter,
                                 Type inputType, TypeRange returnTypes,
                                 SymbolRefAttr funcNameAttr,
                                 OpenNextCloseBodyBuilder bodyBuilder) {
  Location loc = originalOp->getLoc();

  StringRef funcName = funcNameAttr.getLeafReference();
  ModuleOp module = originalOp->getParentOfType<ModuleOp>();
  assert(module);
  MLIRContext *context = rewriter.getContext();

  // Create function op.
  OpBuilder::InsertionGuard guard(rewriter);
  rewriter.setInsertionPointToStart(module.getBody());

  auto visibility = StringAttr::get(context, "private");
  auto funcType = FunctionType::get(context, inputType, returnTypes);
  FuncOp funcOp = rewriter.create<FuncOp>(loc, funcName, funcType, visibility);
  funcOp.setPrivate();

  // Create initial block.
  Block *block =
      rewriter.createBlock(&funcOp.getBody(), funcOp.begin(), inputType, loc);
  rewriter.setInsertionPointToStart(block);

  // Build body.
  Value initialState = block->getArgument(0);
  llvm::SmallVector<Value, 4> returnValues =
      bodyBuilder(rewriter, initialState);
  rewriter.create<func::ReturnOp>(loc, returnValues);

  return funcOp;
}

/// Type-switching proxy for builders of the body of Open functions.
static Value buildOpenBody(Operation *op, RewriterBase &rewriter,
                           Value initialState,
                           ArrayRef<IteratorInfo> upstreamInfos) {
  return llvm::TypeSwitch<Operation *, Value>(op)
      .Case<
          // clang-format off
          ConstantStreamOp,
          FilterOp,
          MapOp,
<<<<<<< HEAD
          ReduceOp
=======
          ReduceOp,
          ScanColumnarBatchOp
>>>>>>> sql_iterators_runtime
          // clang-format on
          >([&](auto op) {
        return buildOpenBody(op, rewriter, initialState, upstreamInfos);
      });
}

/// Type-switching proxy for builders of the body of Next functions.
static llvm::SmallVector<Value, 4>
buildNextBody(Operation *op, RewriterBase &rewriter, Value initialState,
              ArrayRef<IteratorInfo> upstreamInfos, Type elementType) {
  return llvm::TypeSwitch<Operation *, llvm::SmallVector<Value, 4>>(op)
      .Case<
          // clang-format off
          ConstantStreamOp,
          FilterOp,
          MapOp,
<<<<<<< HEAD
          ReduceOp
=======
          ReduceOp,
          ScanColumnarBatchOp
>>>>>>> sql_iterators_runtime
          // clang-format on
          >([&](auto op) {
        return buildNextBody(op, rewriter, initialState, upstreamInfos,
                             elementType);
      });
}

/// Type-switching proxy for builders of the body of Close functions.
static Value buildCloseBody(Operation *op, RewriterBase &rewriter,
                            Value initialState,
                            ArrayRef<IteratorInfo> upstreamInfos) {
  return llvm::TypeSwitch<Operation *, Value>(op)
      .Case<
          // clang-format off
          ConstantStreamOp,
          FilterOp,
          MapOp,
<<<<<<< HEAD
          ReduceOp
=======
          ReduceOp,
          ScanColumnarBatchOp
>>>>>>> sql_iterators_runtime
          // clang-format on
          >([&](auto op) {
        return buildCloseBody(op, rewriter, initialState, upstreamInfos);
      });
}

/// Type-switching proxy for builders of iterator state creation.
static Value buildStateCreation(IteratorOpInterface op, RewriterBase &rewriter,
                                LLVM::LLVMStructType stateType,
                                ValueRange operands) {
  return llvm::TypeSwitch<Operation *, Value>(op)
      .Case<
          // clang-format off
          ConstantStreamOp,
          FilterOp,
          MapOp,
<<<<<<< HEAD
          ReduceOp
=======
          ReduceOp,
          ScanColumnarBatchOp
>>>>>>> sql_iterators_runtime
          // clang-format on
          >([&](auto op) {
        using OpAdaptor = typename decltype(op)::Adaptor;
        OpAdaptor adaptor(operands, op->getAttrDictionary());
        return buildStateCreation(op, adaptor, rewriter, stateType);
      });
}

/// Creates an Open function for originalOp given the provided opInfo. This
/// function only does plumbing; the actual work is done by
/// `buildOpenNextCloseInParentModule` and `buildOpenBody`.
static FuncOp
buildOpenFuncInParentModule(Operation *originalOp, RewriterBase &rewriter,
                            const IteratorInfo &opInfo,
                            ArrayRef<IteratorInfo> upstreamInfos) {
  Type inputType = opInfo.stateType;
  Type returnType = opInfo.stateType;
  SymbolRefAttr funcName = opInfo.openFunc;

  return buildOpenNextCloseInParentModule(
      originalOp, rewriter, inputType, returnType, funcName,
      [&](RewriterBase &rewriter,
          Value initialState) -> llvm::SmallVector<Value, 4> {
        return {
            buildOpenBody(originalOp, rewriter, initialState, upstreamInfos)};
      });
}

/// Creates a Next function for originalOp given the provided opInfo. This
/// function only does plumbing; the actual work is done by
/// `buildOpenNextCloseInParentModule` and `buildNextBody`.
static FuncOp
buildNextFuncInParentModule(Operation *originalOp, RewriterBase &rewriter,
                            const IteratorInfo &opInfo,
                            ArrayRef<IteratorInfo> upstreamInfos) {
  // Compute element type.
  assert(originalOp->getNumResults() == 1);
  StreamType streamType =
      originalOp->getResult(0).getType().dyn_cast<StreamType>();
  assert(streamType);
  Type elementType = streamType.getElementType();

  // Build function.
  Type i1 = rewriter.getI1Type();
  Type inputType = opInfo.stateType;
  SymbolRefAttr funcName = opInfo.nextFunc;

  return buildOpenNextCloseInParentModule(
      originalOp, rewriter, inputType, {opInfo.stateType, i1, elementType},
      funcName, [&](RewriterBase &rewriter, Value initialState) {
        return buildNextBody(originalOp, rewriter, initialState, upstreamInfos,
                             elementType);
      });
}

/// Creates a Close function for originalOp given the provided opInfo. This
/// function only does plumbing; the actual work is done by
/// `buildOpenNextCloseInParentModule` and `buildCloseBody`.
static FuncOp
buildCloseFuncInParentModule(Operation *originalOp, RewriterBase &rewriter,
                             const IteratorInfo &opInfo,
                             ArrayRef<IteratorInfo> upstreamInfos) {
  Type inputType = opInfo.stateType;
  Type returnType = opInfo.stateType;
  SymbolRefAttr funcName = opInfo.closeFunc;

  return buildOpenNextCloseInParentModule(
      originalOp, rewriter, inputType, returnType, funcName,
      [&](RewriterBase &rewriter,
          Value initialState) -> llvm::SmallVector<Value, 4> {
        return {
            buildCloseBody(originalOp, rewriter, initialState, upstreamInfos)};
      });
}

//===----------------------------------------------------------------------===//
// Generic conversion of Iterator ops.
//===----------------------------------------------------------------------===//

/// Converts the given iterator op to LLVM using the converted operands from
/// the upstream iterator. This consists of (1) creating the initial iterator
/// state based on the initial states of the upstream iterators and (2) building
/// the op-specific Open/Next/Close functions.
static Value convert(IteratorOpInterface op, ValueRange operands,
                     RewriterBase &rewriter,
                     const IteratorAnalysis &iteratorAnalysis) {
  // IteratorInfo for this op.
  IteratorInfo opInfo = iteratorAnalysis.getExpectedIteratorInfo(op);

  // Assemble IteratorInfo for all the upstream iterators (i.e. all the defs).
  llvm::SmallVector<IteratorInfo, 8> upstreamInfos;
  for (Value operand : op->getOperands()) {
    IteratorInfo upstreamInfo;

    // Get info about operand *iff* it is defined by an iterator op; otherwise,
    // leave IteratorInfo empty.
    if (operand.getDefiningOp())
      if (auto definingOp =
              dyn_cast<IteratorOpInterface>(operand.getDefiningOp()))
        upstreamInfo = iteratorAnalysis.getExpectedIteratorInfo(definingOp);

    upstreamInfos.push_back(upstreamInfo);
  }

  // Build Open/Next/Close functions.
  buildOpenFuncInParentModule(op, rewriter, opInfo, upstreamInfos);
  buildNextFuncInParentModule(op, rewriter, opInfo, upstreamInfos);
  buildCloseFuncInParentModule(op, rewriter, opInfo, upstreamInfos);

  // Create initial state.
  LLVMStructType stateType = opInfo.stateType;
  return buildStateCreation(op, rewriter, stateType, operands);
}

/// Converts the given sink to LLVM using the converted operands from the root
/// iterator. The current sink consumes the root iterator and prints each
/// element it produces. Pseudo code:
///
/// rootIterator->Open()
/// while (nextTuple = rootIterator->Next())
///   print(nextTuple)
/// rootIterator->Close()
///
/// Possible result:
///
/// %2 = ... // initialize state of root iterator
/// %3 = call @iterators.reduce.open.1(%2) :
///          (!root_state_type) -> !root_state_type
/// %4:3 = scf.while (%arg0 = %3) :
///            (!root_state_type) -> (!root_state_type, i1, !element_type) {
///   %6:3 = call @iterators.reduce.next.1(%arg0) :
///              (!root_state_type) -> (!root_state_type, i1, !element_type)
///   scf.condition(%6#1) %6#0, %6#1, %6#2 : !root_state_type, i1, !element_type
/// } do {
/// ^bb0(%arg0: !root_state_type, %arg1: i1, %arg2: !element_type):
///   "iterators.print"(%arg1) : (!element_type) -> ()
///   scf.yield %arg0 : !root_state_type
/// }
/// %5 = call @iterators.reduce.close.1(%4#0) :
///          (!root_state_type) -> !root_state_type
static Value convert(SinkOp op, ValueRange operands, RewriterBase &rewriter,
                     const IteratorAnalysis &iteratorAnalysis) {
  Location loc = op->getLoc();

  // Look up IteratorInfo about root iterator.
  assert(operands.size() == 1);
  Operation *definingOp = op->getOperand(0).getDefiningOp();
  IteratorInfo opInfo = iteratorAnalysis.getExpectedIteratorInfo(definingOp);

  Type stateType = opInfo.stateType;
  SymbolRefAttr openFunc = opInfo.openFunc;
  SymbolRefAttr nextFunc = opInfo.nextFunc;
  SymbolRefAttr closeFunc = opInfo.closeFunc;

  // Open root iterator. ------------------------------------------------------
  Value initialState = operands[0];
  auto openCallOp =
      rewriter.create<func::CallOp>(loc, openFunc, stateType, initialState);
  Value openedUpstreamState = openCallOp->getResult(0);

  // Consume root iterator in while loop --------------------------------------
  // Input and return types.
  auto streamType = op->getOperand(0).getType().dyn_cast<StreamType>();
  assert(streamType);
  Type elementType = streamType.getElementType();
  Type i1 = rewriter.getI1Type();
  SmallVector<Type> nextResultTypes = {stateType, i1, elementType};
  SmallVector<Type> whileResultTypes = {stateType, elementType};
  SmallVector<Location> whileResultLocs = {loc, loc};

  scf::WhileOp whileOp = scf::createWhileOp(
      rewriter, loc, whileResultTypes, openedUpstreamState,
      /*beforeBuilder=*/
      [&](OpBuilder &builder, Location loc, Block::BlockArgListType args) {
        Value currentState = args[0];
        func::CallOp nextCallOp = builder.create<func::CallOp>(
            loc, nextFunc, nextResultTypes, currentState);

        Value updatedState = nextCallOp->getResult(0);
        Value hasNext = nextCallOp->getResult(1);
        Value nextElement = nextCallOp->getResult(2);
        builder.create<scf::ConditionOp>(loc, hasNext,
                                         ValueRange{updatedState, nextElement});
      },
      /*afterBuilder=*/
      [&](OpBuilder &builder, Location loc, Block::BlockArgListType args) {
        Value currentState = args[0];
        Value nextElement = args[1];

        // Print next element.
        builder.create<PrintOp>(loc, nextElement);

        // Forward iterator state to "before" region.
        builder.create<scf::YieldOp>(loc, currentState);
      });

  Value consumedState = whileOp.getResult(0);

  // Close root iterator. -----------------------------------------------------
  rewriter.create<func::CallOp>(loc, closeFunc, stateType, consumedState);

  return Value();
}

/// Converts the given op to LLVM using the converted operands from the upstream
/// iterator. This function is essentially a switch between conversion functions
/// for sink and non-sink iterator ops.
static Value convertIteratorOp(Operation *op, ValueRange operands,
                               RewriterBase &rewriter,
                               const IteratorAnalysis &iteratorAnalysis) {
  return TypeSwitch<Operation *, Value>(op)
      .Case<IteratorOpInterface>([&](auto op) {
        return convert(op, operands, rewriter, iteratorAnalysis);
      })
      .Case<SinkOp>([&](auto op) {
        return convert(op, operands, rewriter, iteratorAnalysis);
      });
}

//===----------------------------------------------------------------------===//
// Pass driver
//===----------------------------------------------------------------------===//

/// Converts all iterator ops of a module to LLVM. The lowering converts each
/// connected component of iterators to logic that co-executes all iterators in
/// that component. Currently, these connected components have to be shaped as a
/// tree. Roughly speaking, the lowering result consists of three things:
///
/// 1. Nested iterator state types that allow iterators to give up and resume
///    execution by passing control flow between them.
/// 2. Operations that create the initial states.
/// 3. Functions and control flow that executes the logic of one connected
///    component while updating the states.
///
/// Each iterator state is represented as an `!llvm.struct<...>` that constists
/// of an iterator-specific part plus the state of all upstream iterators, i.e.,
/// of all iterators in the transitive use-def chain of the operands of the
/// current iterator, which is inlined into its own state.
///
/// The lowering replaces the original iterator ops with ops that create the
/// initial operator states (which is usually short) plus the logic of the sink
/// operator, which drives the computation of the entire connected components.
/// The logic of the iterators is represented by functions that are added to
/// the current module, and which are called initially by the sink logic and
/// then transitively by the iterators in the connected component.
///
/// More precisely, the computations of each non-sink iterator are expressed as
/// the three functions, `Open`, `Next`, and `Close`, which operate on the
/// iterator's state and which continuously pass control flow between the logic
/// of to and from other iterators:
///
/// * `Open` initializes the computations, typically calling `Open` on the
///   nested states of the current iterator;
/// * `Next` produces the next element in the stream or signals "end of stream",
///   making zero, one, or more calls to `Next` on any of the nested states as
///   required by the logic of the current iterator; and
/// * `Close` cleans up the state if necessary, typically calling `Close` on the
///   nested states of the current iterator.
///
/// The three functions take the current iterator state as an input and return
/// the updated state. (Subsequent bufferization within LLVM presumably converts
/// this to in-place updates.) `Next` also returns the next element in the
/// stream, plus a Boolean that signals whether the element is valid or the end
/// of the stream was reached.
///
/// Iterator states inherently require to contain the states of the all
/// iterators they transitively consume from: Since producing an element for
/// the result stream of an iterator eventually requires to consume elements
/// from the operand streams and the consumption of those elements updates the
/// state of the iterators that produce them, the states of these iterators
/// need to be updated and hence known. If these updates should not be side
/// effects, they have to be reflected in the updated state of the current
/// iterator.
///
/// This means that the lowering of any particular iterator op depends on the
/// transitive use-def chain of its operands. However, it is sufficient to know
/// the state *types*, and, since these states are only forwarded to functions
/// from the upstream iterators (i.e., the iterators that produce the operand
/// streams), they are treated as blackboxes.
///
/// The lowering is done using a custom walker. The conversion happens in two
/// steps:
///
/// 1. The `IteratorAnalysis` computes the nested state of each iterator op
///    and pre-assigns the names of the Open/Next/Close functions of each
///    iterator op. The former provides all information needed from the use-def
///    chain and hences removes need to traverse it for the conversion of each
///    iterator op; the latter establishes the interfaces between each iterator
///    and its downstreams (i.e., consumers).
/// 2. The custom walker traverses the iterator ops in use-def order, converting
///    each iterator in an op-specific way providing the converted operands
///    (which it has walked before) to the conversion logic.
static void convertIteratorOps(ModuleOp module, TypeConverter &typeConverter) {
  IRRewriter rewriter(module.getContext());
  IteratorAnalysis analysis(module, typeConverter);
  BlockAndValueMapping mapping;

  // Collect all iterator ops in a worklist. Within each block, the iterator
  // ops are seen by the walker in sequential order, so each iterator is added
  // to the worklist *after* all of its upstream iterators.
  SmallVector<Operation *, 16> workList;
  module->walk<WalkOrder::PreOrder>([&](Operation *op) {
    TypeSwitch<Operation *, void>(op).Case<IteratorOpInterface, SinkOp>(
        [&](Operation *op) { workList.push_back(op); });
  });

  // Convert iterator ops in worklist order.
  for (Operation *op : workList) {
    rewriter.setInsertionPoint(op);

    // Look up converted operands. The worklist order guarantees that they
    // exist. Use type converter if it isn't produced by an iterator op.
    SmallVector<Value> mappedOperands;
    for (Value operand : op->getOperands()) {
      // Try mapping produced by analysis. This works for operands produced by
      // iterator ops.
      Value mappedOperand = mapping.lookupOrNull(operand);

      // In the other cases (i.e., non-iterator operands), insert unrealized
      // conversion cast to provide conversion with an operand of the converted
      // type similar to the standard dialect conversion.
      if (!mappedOperand) {
        Location loc = op->getLoc();
        Type convertedType = typeConverter.convertType(operand.getType());
        if (convertedType != operand.getType()) {
          mappedOperand = rewriter
                              .create<UnrealizedConversionCastOp>(
                                  loc, convertedType, operand)
                              .getResult(0);
        }
      }

      mappedOperands.push_back(mappedOperand);
    }

    // Convert this op.
    Value converted = convertIteratorOp(op, mappedOperands, rewriter, analysis);
    if (converted)
      mapping.map(op->getResult(0), converted);
    else
      assert(isa<SinkOp>(op) && "Only sink ops convert to a null Value");
  }

  // Delete the original, now-converted iterator ops.
  for (auto it = workList.rbegin(); it != workList.rend(); it++)
    rewriter.eraseOp(*it);
}

void mlir::iterators::populateIteratorsToLLVMConversionPatterns(
    RewritePatternSet &patterns, TypeConverter &typeConverter) {
  patterns.add<ColumnarBatchFromMemrefsLowering, ConstantTupleLowering,
               PrintTupleOpLowering, PrintOpLowering>(typeConverter,
                                                      patterns.getContext());
}

void ConvertIteratorsToLLVMPass::runOnOperation() {
  auto module = getOperation();
<<<<<<< HEAD
  IteratorsTypeConverter typeConverter;
=======
  LLVMTypeConverter llvmTypeConverter(&getContext());
  IteratorsTypeConverter typeConverter(llvmTypeConverter);
>>>>>>> sql_iterators_runtime

  // Convert iterator ops with custom walker.
  convertIteratorOps(module, typeConverter);

  // Convert the remaining ops of this dialect using dialect conversion.
  ConversionTarget target(getContext());
  target.addLegalDialect<arith::ArithmeticDialect,
                         bufferization::BufferizationDialect, LLVMDialect,
                         scf::SCFDialect>();
  target.addLegalOp<ModuleOp, UnrealizedConversionCastOp>();
  RewritePatternSet patterns(&getContext());
<<<<<<< HEAD
=======

>>>>>>> sql_iterators_runtime
  populateIteratorsToLLVMConversionPatterns(patterns, typeConverter);

  // Add patterns that converts function signature and calls.
  populateFunctionOpInterfaceTypeConversionPattern<FuncOp>(patterns,
                                                           typeConverter);
  populateCallOpTypeConversionPattern(patterns, typeConverter);

  // Force application of that pattern if signature is not legal yet.
  target.addDynamicallyLegalOp<func::FuncOp>([&](func::FuncOp op) {
    return typeConverter.isSignatureLegal(op.getFunctionType());
  });
  target.addDynamicallyLegalOp<func::ReturnOp>([&](func::ReturnOp op) {
    return typeConverter.isLegal(op.getOperandTypes());
  });
  target.addDynamicallyLegalOp<func::CallOp>([&](func::CallOp op) {
    return typeConverter.isSignatureLegal(op.getCalleeType());
  });

  // Use UnrealizedConversionCast as materializations, which have to be cleaned
  // up by later passes.
  auto addUnrealizedCast = [](OpBuilder &builder, Type type, ValueRange inputs,
                              Location loc) {
    auto cast = builder.create<UnrealizedConversionCastOp>(loc, type, inputs);
    return Optional<Value>(cast.getResult(0));
  };
  typeConverter.addSourceMaterialization(addUnrealizedCast);
  typeConverter.addTargetMaterialization(addUnrealizedCast);

  if (failed(applyFullConversion(module, target, std::move(patterns))))
    signalPassFailure();
}

std::unique_ptr<OperationPass<ModuleOp>>
mlir::createConvertIteratorsToLLVMPass() {
  return std::make_unique<ConvertIteratorsToLLVMPass>();
}
