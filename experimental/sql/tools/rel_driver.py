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
from src.fuse_proj_table import fuse_proj_table
from tools.IteratorsMLIRConverter import IteratorsMLIRConverter

from xdsl.ir import MLContext
from xdsl.printer import Printer

from decimal import Decimal

import time


def compile(query, mlir_ctx):
  ctx = MLContext()
  mod = ibis_to_xdsl(ctx, query)
  ibis_to_alg(ctx, mod)
  projection_pushdown(ctx, mod)
  alg_to_ssa(ctx, mod)
  ssa_to_impl(ctx, mod)
  fuse_proj_table(ctx, mod)
  data = impl_to_iterators(ctx, mod)

  converter = IteratorsMLIRConverter(ctx)
  return data, converter.convert_module_with_ctx(mod, mlir_ctx)


def run(query, df: pd.DataFrame):

  with Context() as mlir_ctx:
    it.register_dialect()
    start = time.time()
    data, mlir_module = compile(query, mlir_ctx)
    print(time.time() - start)

    arg = ctypes.pointer(to_partial_columnar_batch_descriptor(df, data[0]))
    start = time.time()
    ie.run(mlir_module, [arg])
    print(time.time() - start)


t = ibis.table([("ORDERKEY", "int64"), ("PARTKE", "int64"),
                ("SUPPKEY", "int64"), ("LINENUMBER", "int64"),
                ("QUANTITY", "int64"), ("EXTENDEDPRICE", "decimal(32, 2)"),
                ("DISCOUNT", "decimal(32, 2)"), ("TAX", "decimal(32, 2)"),
                ("RETURNFLAG", "string"), ("LINESTATUS", "string"),
                ("SHIPDATE", "timestamp"), ("COMMITDATE", "timestamp"),
                ("RECEIPTDATE", "timestamp"), ("SHIPINSTRUCT", "string"),
                ("SHIPMODE", "string"), ("COMMENT", "string")], 'lineitem')

t2 = ibis.table([("im", "int64")], 'u')
res = ibis.table([("revenue", "int64")], 'line')

p1 = ibis.literal('1994-01-01', "timestamp")
p1h = ibis.literal('1995-01-01', "timestamp")
p2 = 0.06
p3 = np.int64(24)

filtered = t.filter(
    (t['SHIPDATE'] >= p1) & (t['SHIPDATE'] < p1h) &
    (t['DISCOUNT'] >= ibis.literal(Decimal("0.05"), "decimal(6, 2)")) &
    (t['DISCOUNT'] <= ibis.literal(Decimal("0.07"), "decimal(6, 2)")) &
    (t['QUANTITY'] < p3))

multiply = filtered.projection(
    (filtered['EXTENDEDPRICE'] * filtered['DISCOUNT']).name('im'))

query = multiply.aggregate(multiply.im.sum().name('revenue'))

lineitem = pd.read_table('/home/michel/MasterThesis/dbgen/lineitem.tbl',
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

run(query, lineitem)
