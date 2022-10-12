from utils import add_date
import ibis


def get_ibis_query(SHIPMODE1="MAIL", SHIPMODE2="SHIP", DATE="1994-01-01"):
  from tpc_h_tables import orders, lineitem
  q = orders
  q = q.join(lineitem, orders.o_orderkey == lineitem.l_orderkey)

  q = q.filter([
      q.l_shipmode.isin([SHIPMODE1, SHIPMODE2]),
      q.l_commitdate < q.l_receiptdate,
      q.l_shipdate < q.l_commitdate,
      q.l_receiptdate >= DATE,
      q.l_receiptdate < add_date(DATE, dy=1),
  ])

  gq = q.group_by([q.l_shipmode])
  q = gq.aggregate(
      high_line_count=(q.o_orderpriority.case().when(
          "1-URGENT",
          ibis.literal(1,
                       "int64")).when("2-HIGH", ibis.literal(1, "int64")).else_(
                           ibis.literal(0, "int64")).end()).sum(),
      low_line_count=(q.o_orderpriority.case().when(
          "1-URGENT",
          ibis.literal(0,
                       "int64")).when("2-HIGH", ibis.literal(0, "int64")).else_(
                           ibis.literal(1, "int64")).end()).sum(),
  )
  q = q.sort_by(q.l_shipmode)

  return q
