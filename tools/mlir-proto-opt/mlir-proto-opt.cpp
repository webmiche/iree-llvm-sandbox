//===- mlir-opt.cpp - MLIR Optimizer Driver -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Main entry function for mlir-opt for when built as standalone binary.
// Reuse Registration.cpp as much as possible.
//
//===----------------------------------------------------------------------===//

#include "Registration.h"
#include "mlir/IR/Dialect.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"

using namespace mlir;

namespace mlir {
namespace test_ext {
void registerTestVectorMaskingUtils();
void registerTestVectorWarps();
} // namespace test_ext
} // namespace mlir

int main(int argc, char **argv) {
  llvm::InitLLVM y(argc, argv);
  registerAllPasses();
  registerOutsideOfDialectRegistry();

  mlir::test_ext::registerTestVectorMaskingUtils();
  mlir::test_ext::registerTestVectorWarps();

  // TODO: Find a way to register test passes here.

  DialectRegistry registry;
  registerIntoDialectRegistry(registry);

  return failed(MlirOptMain(argc, argv, "MLIR modular optimizer driver\n",
                            registry,
                            /*preloadDialectsInContext=*/true));
}
