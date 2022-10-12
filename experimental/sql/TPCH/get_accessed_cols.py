import q1
import q2
import q3
import q4
import q5
import q6
import q7
import q8
import q9
import q10
import q11
import q12
import q13
import q14
import q15
import q16
import q17
import q18
import q19
import q20
import q21
import q22

from src.ibis_frontend import ibis_to_xdsl
from src.ibis_to_alg import ibis_to_alg
from src.alg_to_ssa import alg_to_ssa
from src.ssa_to_impl import ssa_to_impl
from src.projection_pushdown import projection_pushdown
from src.fuse_proj_into_scan import fuse_proj_into_scan

from xdsl.ir import MLContext
from xdsl.printer import Printer

import sys
from io import StringIO

from src.impl_to_iterators import get_batch_and_name_list


def get_list(ctx, query) -> list[list[str]]:
  """
  Return a list, where element i is the list of columns to load from the ith
  argument to main. If an entry is None, load the whole table.
  """

  batches, names = get_batch_and_name_list(query)

  ret_list = []

  for n in names:
    if "," in n:
      ret_list.append(n.split(",")[1:])
    else:
      ret_list.append(None)

  return ret_list


def compile(query):
  ctx = MLContext()
  mod = ibis_to_xdsl(ctx, query)
  ibis_to_alg(ctx, mod)
  projection_pushdown(ctx, mod)
  alg_to_ssa(ctx, mod)
  ssa_to_impl(ctx, mod)
  fuse_proj_into_scan(ctx, mod)
  lists = get_list(ctx, mod)
  for l in lists:
    for elem in l:
      print(elem)

  return mod


def get_tpc_queries():
  queries = []
  queries.append(q1.get_ibis_query())
  queries.append(q3.get_ibis_query())
  queries.append(q5.get_ibis_query())
  queries.append(q6.get_ibis_query())
  queries.append(q10.get_ibis_query())
  return queries


def run():
  for i, q in enumerate(get_tpc_queries()):
    print(i + 1)
    compile(q)


if __name__ == "__main__":
  run()
