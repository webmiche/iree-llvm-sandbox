# Prototype SQL dialect(s)

## How to run the vertical prototype:

Make sure you have built the sandbox with iterators turned on (`--iterators` flag to `configure.py`).

Then install all requirements used for the project:

``` bash
pip install -r experimental/sql/requirements.txt
```

Inspect the testcase in ibis:

```bash
cat experimental/sql/test/frontend/sum.ibis
```

Inspect the testcase in the ibis dialect:

```bash
python3 experimental/sql/tools/rel_opt.py -f ibis experimental/sql/test_mlir/end_to_end_tests/sum.ibis
```

Inspect the testcase in the other representations. By removing the `--print-between-passes` flag and modifying the applied passes, you can get a specific representation:

```bash
python3 experimental/sql/tools/rel_opt.py -f ibis -p ibis-to-alg,alg-to-ssa,ssa-to-impl,impl-to-iterators -t mlir experimental/sql/test_mlir/end_to_end_tests/sum.ibis  --print-between-passes
```

Print the testcase in MLIR generic form to a temporary file:

```bash
python3 experimental/sql/tools/rel_opt.py -f ibis -p ibis-to-alg,alg-to-ssa,ssa-to-impl,impl-to-iterators -t mlir experimental/sql/test_mlir/end_to_end_tests/sum.ibis > tmp.mlir
```

Use the runner to reparse, lower, and run the testcase:

```bash
python3 experimental/sql/tools/rel_runner.py tmp.mlir
```
