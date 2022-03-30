# Copyright 2021 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import ibis
import os
import sys

import pandas as pd

from xdsl.ir import MLContext
from xdsl.printer import Printer
from src.ibis_frontend import ibis_to_xdsl
from src.ibis_to_rel import ibis_dialect_to_relational

connection = ibis.pandas.connect({"t": pd.DataFrame({"a": ["AS", "EU", "NA"]})})

table = connection.table('t')

#country_npa = countries['name', 'population', 'area_km2']
#country_names = country_npa['name']

#print(country_names.op().table)
#print(type(country_names.op().table))
#print(country_names.op().table.op().table)

#print(type(countries))
#print(type(countries['continent']))
#print(type(countries['continent'] == 'AS'))
#print(type(countries.filter(countries['continent'] == 'AS')))

query = table.filter(table['a'] == 'AS')

p = Printer()
ctx = MLContext()
xdsl_query = ibis_to_xdsl(ctx, query)
ibis_dialect_to_relational(ctx, xdsl_query)
p.print_op(xdsl_query)

#Note: Expr nodes have .op() to get to operation
#Note: countries['name'] is TableColumn --> access table via parent()
#Note: countries['name', 'population'] is Selection --> access table via table
