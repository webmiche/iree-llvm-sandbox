# Prototype SQL dialect(s)

## Getting the code:

Get the code by cloning out github repository. Make sure to take the proper
branch (`zenodo_artifact`).

```Bash
git clone https://github.com/webmiche/iree-llvm-sandbox/tree/zenodo_artifact
cd iree-llvm-sandbos
```

## Preamble:

For this README, we refer to the directory where the sandbox was cloned into as
*/path/to*. Notice that you need
`*/path/to*/iree-llvm-sandbox/build/tools/sandbox/python_packages/` and
`*/path/to*/iree-llvm-sandbox/experimental/sql` in your `PYTHONPATH` in order to
run our Python-based parts. Furthermore, please build the project first by
following the instructions in `../../README.md`. Notice that you need to run
`configure.py` with the flag `--iterators`.

## Generating the execution time plots:

To get the exact times, change the `CYCLES_PER_SECOND` in
`experimental/sql/tools/load.c` and `experimental/sql/duckdb_run/run_duckdb.cpp`
to the speed of your processor.

### Generating the Data:

The data needed to benchmark both the execution of DuckDB and our approach is
the lineitem table in scale factors 1, 2, 4, 8, and 16 generated as integer data
(available [here](https://gitlab.inf.ethz.ch/OU-SYSTEMS/projects/cvm/dbgen), if
you do not have access to the repository, feel free to reach out to me using
`michel.web97@gmail.com`). The tables need to be placed in the directory
`experimental/sql/tables` in both a `.tbl` and a `.csv` version.

Notice both `experimental/sql/tools/gen_factors.sh`, a shell script to generate
the tables and `experimental/sql/tools/convert_tbl.py`, a python script to
convert `.tbl` to `.csv` files. Notice that you might have to change the paths
in the later.

### Generating the DuckDB data:

Compile the runner, linking it against the DuckDB library files. Then adjust the
library loading variable and execute the binary.

```Bash
clang++ experimental/sql/duckdb_run/run_duckdb.cpp experimental/sql/duckdb_run/src/libduckdb.so -O3
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:*/path/to*/iree-llvm-sandbox/experimental/sql/duckdb_run/src
./a.out > experimental/sql/data/data_duckdb_single.csv
```

### Generating the data for our approach:

To run our approach, we first compile the input query through our compilation
stack printing it to `tmp.mlir`.

```Bash
./experimental/sql/tools/rel_opt.py -f ibis -p ibis-to-alg,projection-pushdown,alg-to-ssa,ssa-to-impl,fuse-proj-into-scan,impl-to-iterators -t llvm experimental/sql/TPCH/q6.ibis > tmp.mlir
```

Then, translate this to proper LLVMIR using the built `mlir-translate`:

```Bash
./build/bin/mlir-translate --mlir-to-llvmir tmp.mlir > tmp.ll
```

The resulting LLVMIR file can be run using `experimental/sql/tools/load.c`.
Notice that you need a higher version than clang 15 as the built mlir-translate
uses opaque pointers. Additionally, you will need the -O3 flag.

```Bash
clang-15 experimental/tools/load.c experimental/tools/printf2.c tmp.ll -O3
```

Then run the compiled ELF-file:

```Bash
./a.out > experimental/sql/data/exec_sf.py
```

### Generating the plot

To generate the plot, change the directory to `experimental/sql/plots` and run
the script:

```Bash
cd experimental/sql/plots
python3 exec_sf.py
```

Your generated plot is `experimental/sql/plots/exec_sf.pdf`.

## Generating the conceptual plots:

After changing the `PYTHONPATH` as mentioned above, run the following to get the
list of features used in all the queries:

```Bash
python3 experimental/sql/TPCH/get_features.py > experimental/sql/data/ibis_nodes.csv
python3 experimental/sql/TPCH/get_accessed_cols.py > experimental/sql/data/partial_loading.txt
```

Similarly to above, change the directory to `experimental/sql/plots` and run the
script to generate the plot:

```Bash
cd experimental/sql/plots
python3 ibis_nodes.py
python3 partial_loading.py
```

Your generated plots are `experimental/sql/plots/ibis_nodes.pdf` and
`experimental/sql/plors/partial_loading.pdf` respectively.
