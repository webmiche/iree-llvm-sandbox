import ctypes
import numpy as np
import pandas as pd

import ibis

from xdsl import _mlir_module as mlir
import mlir_iterators.runtime.iterators_executor as ie
from mlir_iterators.ir import Module, Context
from mlir_iterators.dialects import iterators as it
from mlir_iterators.runtime.pandas_to_iterators import to_partial_columnar_batch_descriptor
from src.ibis_frontend import ibis_to_xdsl
from src.ibis_to_alg import ibis_to_alg
from src.alg_to_ssa import alg_to_ssa
from src.ssa_to_impl import ssa_to_impl
from src.impl_to_iterators import impl_to_iterators
from src.projection_pushdown import projection_pushdown
from src.fuse_proj_into_scan import fuse_proj_into_scan
from tools.IteratorsMLIRConverter import IteratorsMLIRConverter

from xdsl.ir import MLContext
from xdsl.printer import Printer

from decimal import Decimal
import TPCH.q6 as q6

import time


def compile(query):
  ctx = MLContext()
  mod = ibis_to_xdsl(ctx, query)
  ibis_to_alg(ctx, mod)
  projection_pushdown(ctx, mod)
  alg_to_ssa(ctx, mod)
  ssa_to_impl(ctx, mod)
  fuse_proj_into_scan(ctx, mod)
  Printer().print_op(mod)
  data = impl_to_iterators(ctx, mod)

  converter = IteratorsMLIRConverter(ctx)
  return [data[0].split(',')[1:]], converter.convert_module(mod)


def run(query, df: pd.DataFrame):

  with Context() as mlir_ctx:
    it.register_dialect()
    start = time.time()
    data, mlir_module = compile(query)
    print("compilation time: " + str(time.time() - start))

    arg = ctypes.pointer(to_partial_columnar_batch_descriptor(df, data[0]))
    mlir_string = str(mlir_module)
    mod = Module.parse(mlir_string)
    start = time.time()
    ie.run(mod, [arg])
    print("runtime: " + str(time.time() - start))


lineitem = pd.read_table('/home/michel/MasterThesis/dbgen/lineitem_1.tbl',
                         delimiter="|",
                         names=[
                             "ORDERKEY", "PARTKEY", "SUPPKEY", "LINENUMBER",
                             "QUANTITY", "EXTENDEDPRICE", "DISCOUNT", "TAX",
                             "RETURNFLAG", "LINESTATUS", "SHIPDATE",
                             "COMMITDATE", "RECEIPTDATE", "SHIPINSTRUCT",
                             "SHIPMODE", "COMMENT"
                         ],
                         dtype={
                             'ORDERKEY': np.int64,
                             "PARTKEY": np.int64,
                             "SUPPKEY": np.int64,
                             "LINENUMBER": np.int64,
                             "QUANTITY": np.int64,
                             "EXTENDEDPRICE": np.int64,
                             "DISCOUNT": np.int64,
                             "TAX": np.int64,
                             "RETURNFLAG": str,
                             "LINESTATUS": str,
                             "SHIPDATE": np.int64,
                             "COMMITDATE": np.int64,
                             "RECEIPTDATE": np.int64,
                             "SHIPINSTRUCT": str,
                             "SHIPMODE": str,
                             "COMMENT": str
                         },
                         infer_datetime_format=True,
                         index_col=False)

run(q6.get_ibis_query(), lineitem)
