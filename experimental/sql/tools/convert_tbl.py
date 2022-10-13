from os import path


def converttbldatatocsvformat(header, scale_factor):
  csv = open(f"./experimental/sql/tables/lineitem_{scale_factor}.csv", "w+")
  csv.write(header + "\n")
  tbl = open(f"./experimental/sql/tables/lineitem_{scale_factor}.tbl", "r")
  lines = tbl.readlines()
  for line in lines:
    length = len(line)
    line = line[:length - 2] + line[length - 1:]
    line = line.replace(",", "N")
    line = line.replace("|", ",")
    csv.write(line)
  tbl.close()
  csv.close()


header_list = [("ORDERKEY", "int64"), ("PARTKEY", "int64"),
               ("SUPPKEY", "int64"), ("LINENUMBER", "int64"),
               ("QUANTITY", "int64"), ("EXTENDEDPRICE", "decimal(32, 2)"),
               ("DISCOUNT", "decimal(32, 2)"), ("TAX", "decimal(32, 2)"),
               ("RETURNFLAG", "string"), ("LINESTATUS", "string"),
               ("SHIPDATE", "timestamp"), ("COMMITDATE", "timestamp"),
               ("RECEIPTDATE", "timestamp"), ("SHIPINSTRUCT", "string"),
               ("SHIPMODE", "string"), ("COMMENT", "string")]

header_list = [x[0] for x in header_list]
converttbldatatocsvformat(",".join(header_list), 1)
converttbldatatocsvformat(",".join(header_list), 2)
converttbldatatocsvformat(",".join(header_list), 4)
converttbldatatocsvformat(",".join(header_list), 8)
converttbldatatocsvformat(",".join(header_list), 16)
