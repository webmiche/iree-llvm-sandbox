import ibis
import os

import pandas
import numpy as np

from src.ibis_frontend import ibis_to_xdsl
from src.ibis_to_alg import ibis_to_alg
from src.alg_to_ssa import alg_to_ssa
from src.ssa_to_impl import ssa_to_impl
from src.impl_to_iterators import impl_to_iterators
from xdsl.ir import MLContext
from xdsl.printer import Printer

from decimal import Decimal, getcontext, setcontext

getcontext().prec = 2

t = ibis.table([("ORDERKEY", "int64"), ("PARTKE", "int64"),
                ("SUPPKEY", "int64"), ("LINENUMBER", "int64"),
                ("QUANTITY", "int64"), ("EXTENDEDPRICE", "decimal(32, 2)"),
                ("DISCOUNT", "decimal(32, 2)"), ("TAX", "decimal(32, 2)"),
                ("RETURNFLAG", "string"), ("LINESTATUS", "string"),
                ("SHIPDATE", "timestamp"), ("COMMITDATE", "timestamp"),
                ("RECEIPTDATE", "timestamp"), ("SHIPINSTRUCT", "string"),
                ("SHIPMODE", "string"), ("COMMENT", "string")], 'lineitem')

#sql_to_ibis.register_temp_table(t, "lineitem")
#print(sql_to_ibis.query('SELECT QUANTITY FROM lineitem').execute())

t2 = ibis.table([("im", "int64")], 'u')
res = ibis.table([("revenue", "int64")], 'line')

p1 = ibis.literal('1994-01-01', "timestamp")
p1h = ibis.literal('1995-01-01', "timestamp")
p2 = 0.06
p3 = np.int64(24)

d = ibis.expr.datatypes.Decimal(2, 1)
filtered = t.filter(
    (t['SHIPDATE'] >= p1) & (t['SHIPDATE'] < p1h) &
    (t['DISCOUNT'] >= ibis.literal(Decimal("0.05"), "decimal(6, 2)")) &
    (t['DISCOUNT'] <= ibis.literal(Decimal("0.07"), "decimal(6, 2)")) &
    (t['QUANTITY'] < p3))

multiply = filtered.projection(
    (filtered['EXTENDEDPRICE'] * filtered['DISCOUNT']).name('im'))

query = multiply.aggregate(multiply.im.sum().name('revenue'))

ctx = MLContext
module = ibis_to_xdsl(ctx, query)

Printer().print_op(module)

ibis_to_alg(ctx, module)
Printer().print_op(module)

alg_to_ssa(ctx, module)
Printer().print_op(module)

ssa_to_impl(ctx, module)
Printer().print_op(module)

impl_to_iterators(ctx, module)
Printer().print_op(module)
