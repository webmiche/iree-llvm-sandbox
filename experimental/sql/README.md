# Prototype SQL dialect(s)

check proper PYHTONPATH!

use -t llvm on experimental/sql/TPCH/q6.ibis
use built mlir-translate --mlir-to-llvmir
pass result to clang-15 experimental/tools/load.c experimental/tools/printf2.c -O3
./a.out 2> experimental/sql/data/exec_sf.py

python3 experimental/sql/TPCH/get_features.py > experimental/sql/data/ibis_nodes.csv
python3 experimental/sql/TPCH/get_accessed_cols.py > experimental/sql/data/partial_loading.txt
