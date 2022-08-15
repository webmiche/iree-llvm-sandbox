import ctypes

import pandas as pd
import numpy as np

from mlir_iterators.runtime.pandas_to_iterators import to_columnar_batch_descriptor
from mlir_iterators.dialects import iterators as it
from mlir_iterators.ir import Context, Module
from mlir_iterators.passmanager import PassManager
from mlir_iterators.execution_engine import ExecutionEngine
import mlir_iterators.all_passes_registration


# inputs should be a list of ColumnarBatchDescriptors
def run(f: str, inputs):
  with Context():
    it.register_dialect()
    mod = Module.parse(f)

    pm = PassManager.parse(
        'convert-iterators-to-llvm,convert-memref-to-llvm,convert-func-to-llvm,'
        'reconcile-unrealized-casts,convert-scf-to-cf,convert-cf-to-llvm')
    pm.run(mod)

    engine = ExecutionEngine(mod)
    engine.invoke('main', *inputs)


if __name__ == "__main__":
  data = np.array([(0, 3), (1, 4), (2, 5)], dtype=[('a', 'i4'), ('b', 'i8')])
  df = pd.DataFrame.from_records(data)
  arg = ctypes.pointer(to_columnar_batch_descriptor(df))

  run(
      """
  !tuple_type = tuple<i32,i64>
          !struct_type = !llvm.struct<(i32,i64)>
          func.func @main(%input: !iterators.columnar_batch<!tuple_type>)
              attributes { llvm.emit_c_interface } {
            %stream = "iterators.scan_columnar_batch"(%input)
              : (!iterators.columnar_batch<!tuple_type>)
                -> !iterators.stream<!struct_type>
            "iterators.sink"(%stream) : (!iterators.stream<!struct_type>) -> ()
            return
           }
           """, [arg])
