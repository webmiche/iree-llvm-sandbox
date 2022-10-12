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

from src.ibis_frontend import ibis_to_xdsl, features

from xdsl.ir import MLContext


def compile(query):
  ctx = MLContext()
  mod = ibis_to_xdsl(ctx, query)
  for e in list(dict.fromkeys(features)):
    print(e)


def get_tpc_queries():
  queries = []
  queries.append(q1.get_ibis_query())
  queries.append(q2.get_ibis_query())
  queries.append(q3.get_ibis_query())
  queries.append(q4.get_ibis_query())
  queries.append(q5.get_ibis_query())
  queries.append(q6.get_ibis_query())
  queries.append(q7.get_ibis_query())
  queries.append(q8.get_ibis_query())
  queries.append(q9.get_ibis_query())
  queries.append(q10.get_ibis_query())
  queries.append(q11.get_ibis_query())
  queries.append(q12.get_ibis_query())
  queries.append(q13.get_ibis_query())
  queries.append(q14.get_ibis_query())
  queries.append(q15.get_ibis_query())
  queries.append(q16.get_ibis_query())
  queries.append(q17.get_ibis_query())
  queries.append(q18.get_ibis_query())
  queries.append(q19.get_ibis_query())
  queries.append(q20.get_ibis_query())
  queries.append(q21.get_ibis_query())
  queries.append(q22.get_ibis_query())
  return queries


def run():
  for i, q in enumerate(get_tpc_queries()):
    print(i + 1)
    compile(q)


if __name__ == "__main__":
  run()
