from TPCH.utils import add_date
import ibis
from decimal import Decimal


def get_ibis_query(DATE="1994-01-01", DISCOUNT=0.06, QUANTITY=24):
  from TPCH.tpc_h_tables import lineitem
  q = lineitem
  discount_min = round(DISCOUNT - 0.01, 2)
  discount_max = round(DISCOUNT + 0.01, 2)
  q = q.filter([
      q.l_shipdate >= ibis.literal(DATE, "timestamp"),
      q.l_shipdate < ibis.literal(add_date(DATE, dy=1), "timestamp"),
      q.l_discount.between(ibis.literal(Decimal(discount_min), "decimal(32,2)"),
                           ibis.literal(Decimal(discount_max),
                                        "decimal(32,2)")),
      q.l_quantity < ibis.literal(QUANTITY, "int64")
  ])
  proj = q.projection(q.columns +
                      [(q.l_extendedprice * q.l_discount).name('revenue')])
  q = proj.aggregate(revenue=proj.revenue.sum())
  return q
