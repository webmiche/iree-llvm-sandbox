import ctypes
import numpy as np
import pandas as pd

import ibis

import python.mlir_iterators.runtime.iterators_executor as ie
from python.mlir_iterators.runtime.pandas_to_iterators import to_columnar_batch_descriptor
from src.ibis_frontend import ibis_to_xdsl
from src.ibis_to_alg import ibis_to_alg
from src.alg_to_ssa import alg_to_ssa
from src.ssa_to_impl import ssa_to_impl
from src.impl_to_iterators import impl_to_iterators
from tools.IteratorsMLIRConverter import IteratorsMLIRConverter

from xdsl.ir import MLContext


def run(query, df: pd.DataFrame):

  arg = ctypes.pointer(to_columnar_batch_descriptor(df))
  ctx = MLContext()
  mod = ibis_to_xdsl(ctx, query)
  ibis_to_alg(ctx, mod)
  alg_to_ssa(ctx, mod)
  ssa_to_impl(ctx, mod)
  impl_to_iterators(ctx, mod)

  converter = IteratorsMLIRConverter(ctx)
  mlir_module = converter.convert_module(mod)
  mlir_string = str(mlir_module)

  ie.run(mlir_string, [arg])


run(
    ibis.table([("a", "int32"), ("b", "int64")], 'lineitem'),
    pd.DataFrame.from_records(
        np.array([(0, 3), (1, 4), (2, 5)], dtype=[('a', 'i4'), ('b', 'i8')])))
