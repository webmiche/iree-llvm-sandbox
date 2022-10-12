# Prototype SQL dialect(s)

check proper PYHTONPATH!

use -t llvm on experimental/sql/TPCH/q6.ibis
use built mlir-translate --mlir-to-llvmir
pass result to clang-15 experimental/tools/load.c -O3

How to link against empty printf??
