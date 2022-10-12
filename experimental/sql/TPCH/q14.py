from utils import add_date
import ibis


def get_ibis_query(DATE="1995-09-01"):
  from tpc_h_tables import lineitem, part
  q = lineitem
  q = q.join(part, lineitem.l_partkey == part.p_partkey)
  q = q.filter([q.l_shipdate >= DATE, q.l_shipdate < add_date(DATE, dm=1)])

  revenue = q.l_extendedprice * (ibis.literal(1, "int64") - q.l_discount)
  promo_revenue = q.p_type.like("PROMO%").ifelse(revenue,
                                                 ibis.literal(0, "int64"))

  q = q.aggregate(promo_revenue=ibis.literal(100, "int64") *
                  promo_revenue.sum() / revenue.sum())
  return q
