import mlir_iterators
import mlir_iterators.ir
from mlir_iterators.ir import Module, Context, Location
from mlir_iterators.execution_engine import ExecutionEngine
from mlir_iterators.passmanager import PassManager
from mlir_iterators.dialects import iterators as it
import mlir_iterators.all_passes_registration

from sys import stdout

import argparse

arg_parse = argparse.ArgumentParser()
arg_parse.add_argument("input_file",
                       type=str,
                       nargs="?",
                       help="path to input file")


def main(args):

  input_str = open(args.input_file).read()

  with Context() as ctx, Location.unknown() as loc:
    it.register_dialect()
    mlir_module = Module.parse(input_str)

    PassManager.parse('convert-iterators-to-llvm').run(mlir_module)
    PassManager.parse('convert-func-to-llvm').run(mlir_module)
    print(mlir_module)

    shared_libs = ["build/lib/libruntime_utils.so"]

    exec_eng = ExecutionEngine(mlir_module, shared_libs=shared_libs)
    exec_eng.invoke("main")


if __name__ == "__main__":
  main(arg_parse.parse_args())
