from utils import add_date
import ibis


def get_ibis_query(NATION1="FRANCE", NATION2="GERMANY", DATE="1995-01-01"):
  from tpc_h_tables import supplier, lineitem, orders, customer, nation

  q = supplier
  q = q.join(lineitem, supplier.s_suppkey == lineitem.l_suppkey)
  q = q.join(orders, orders.o_orderkey == lineitem.l_orderkey)
  q = q.join(customer, customer.c_custkey == orders.o_custkey)
  n1 = nation
  n2 = nation.view()
  q = q.join(n1, supplier.s_nationkey == n1.n_nationkey)
  q = q.join(n2, customer.c_nationkey == n2.n_nationkey)

  q = q[n1.n_name.name("supp_nation"),
        n2.n_name.name("cust_nation"), lineitem.l_shipdate,
        lineitem.l_extendedprice, lineitem.l_discount,
        lineitem.l_shipdate.name("l_year"),  #.year().cast("string").name("l_year"),
        (lineitem.l_extendedprice * (ibis.literal(1, "int64") - lineitem.l_discount)).name("volume"),]

  q = q.filter([
      ((q.cust_nation == NATION1) & (q.supp_nation == NATION2)) |
      ((q.cust_nation == NATION2) & (q.supp_nation == NATION1)),
      q.l_shipdate.between(DATE, add_date(DATE, dy=2, dd=-1)),
  ])

  gq = q.group_by(["supp_nation", "cust_nation", "l_year"])
  q = gq.aggregate(revenue=q.volume.sum())
  q = q.sort_by(["supp_nation", "cust_nation", "l_year"])

  return q
