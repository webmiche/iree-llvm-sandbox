import ibis
import os

import pandas
import numpy as np

#import sql_to_ibis

from decimal import Decimal, getcontext, setcontext

getcontext().prec = 2

table_names = [
    "customer", "lineitem", "nation", "orders", "part", "partsupp", "region",
    "supplier"
]


def dec_to_int(x: str) -> int:
  return int(Decimal(x) * Decimal(100).to_integral())


lineitem = pandas.read_table(
    '/home/michel/MasterThesis/TPC-H/TPC-H_Tools_v3.0.0/dbgen/lineitem.tbl',
    delimiter="|",
    names=[
        "ORDERKEY", "PARTKEY", "SUPPKEY", "LINENUMBER", "QUANTITY",
        "EXTENDEDPRICE", "DISCOUNT", "TAX", "RETURNFLAG", "LINESTATUS",
        "SHIPDATE", "COMMITDATE", "RECEIPTDATE", "SHIPINSTRUCT", "SHIPMODE",
        "COMMENT"
    ],
    dtype={
        'ORDERKEY': np.int64,
        "PARTKEY": np.int64,
        "SUPPKEY": np.int64,
        "LINENUMBER": np.int64,
        "QUANTITY": np.int64,
        "EXTENDEDPRICE": object,
        "DISCOUNT": object,
        "TAX": object,
        "RETURNFLAG": str,
        "LINESTATUS": str,
        "SHIPDATE": object,
        "COMMITDATE": object,
        "RECEIPTDATE": object,
        "SHIPINSTRUCT": str,
        "SHIPMODE": str,
        "COMMENT": str
    },
    infer_datetime_format=True,
    parse_dates=["SHIPDATE", "COMMITDATE", "RECEIPTDATE"],
    converters={
        "DISCOUNT": lambda x: dec_to_int(x),
        "EXTENDEDPRICE": lambda x: dec_to_int(x),
        "TAX": lambda x: dec_to_int(x)
    },
    index_col=False)

# set DSS_QUERY to ./queries/

connection = ibis.pandas.connect({"lineitem": lineitem})

t = connection.table("lineitem")

#sql_to_ibis.register_temp_table(t, "lineitem")
#print(sql_to_ibis.query('SELECT QUANTITY FROM lineitem').execute())

p1 = '1994-01-01'
p1h = '1995-01-01'
p2 = 0.06
p3 = 24

filtered = t.filter((t['SHIPDATE'] >= p1) & (t['SHIPDATE'] < p1h) &
                    (t['DISCOUNT'] >= 100 * (p2 - 0.01)) &
                    (t['DISCOUNT'] <= 100 * (p2 + 0.01)) & (t['QUANTITY'] < p3))

query = (filtered['EXTENDEDPRICE'] * filtered['DISCOUNT']).sum().name('revenue')
print(query)
print(query.execute() / 10000)
