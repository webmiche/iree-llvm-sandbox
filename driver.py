from mlir_iterators.ir import Context, Module, Location
from mlir_iterators.execution_engine import ExecutionEngine
from mlir_iterators.passmanager import PassManager
import numpy as np
import ctypes
from mlir_iterators.dialects import iterators as it
import mlir_iterators.all_passes_registration
#import mlir_iterators.dialects.memref as memref
from mlir_iterators.runtime import np_to_memref

with Context() as ctx, Location.unknown() as loc:

  mod = Module.parse("""
module attributes {llvm.data_layout = ""} {
func.func @store(%arg1 : memref<i64>) attributes { llvm.emit_c_interface} {
  %c0 = arith.constant 42 : i64
  memref.store %c0, %arg1[] : memref<i64>
  return
  }
}
      """)
  print(mod)

  PassManager.parse('convert-memref-to-llvm').run(mod)
  PassManager.parse('convert-func-to-llvm').run(mod)
  PassManager.parse('reconcile-unrealized-casts').run(mod)
  print(mod)

  shared_libs = ["build/lib/libruntime_utils.so"]

  output = np.array(1)

  output_memref = np_to_memref.get_ranked_memref_descriptor(output)

  output_ptr = ctypes.pointer(ctypes.pointer(output_memref))

  exec_eng = ExecutionEngine(mod, shared_libs=shared_libs)
  exec_eng.invoke("store", output_ptr)
  print(output)
