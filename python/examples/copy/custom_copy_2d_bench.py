# RUN: %PYTHON %s 2>&1 | FileCheck %s

# This file contains small benchmarks with reasonably-sized problem/tiling sizes
# and codegen options.

from ..core.experts import *
from ..core.harness import *
from ..core.transforms import *
from ..core.utils import *

from .definitions import CopyProblem

from typing import List

fun_name = 'copy_1d'
op_name = 'linalg.copy'

################################################################################
### Compilation strategies.
################################################################################


# Problem size-specific transformation parameters: the tile size is the max
# divisible entry that fits within
def all_experts(fun_name: str, problem_sizes: List[int]):
  sizes1 = l1_2d_divisible_tile_sizes(problem_sizes)
  sizes_for_register_tiling = [ \
    ts if ts > 0 else s for (s, ts) in zip(problem_sizes, sizes1) \
  ]
  sizes2 = register_2d_divisible_tile_sizes(sizes_for_register_tiling)

  # Before bufferization, the IR only has a tensor.extract_slice /
  #   tensor.insert_slice pair.
  # Bufferization then properly introduces copy ops (implemented with
  # linalg.generic)
  # We want to make more these copies more efficient.
  # In the case of a single copy benchmark it is the one true thing to optimize.
  return [
    # Note: `\` char at the end of next line prevents formatter reflows, keep it.
    e.print_ir(after_all=False, at_begin=False, llvm=False) for e in [         \
      Tile(fun_name=fun_name,
            op_name=op_name,
            tile_sizes=sizes2)
      .then(Bufferize())
      .then(Vectorize(fun_name=fun_name, op_name=''))
      .then(LowerVectors())
      .then(LowerToLLVM())
    ]
  ]


################################################################################
### Problem instantiations.
################################################################################

keys = ['m', 'n']

copy_2D_perf_search_list = [ \
    [100, 32],  # sweet spot for prefetchers, seems to maximize L1 BW @ 295GB/s

    [ 50, 272],  # 10% L2 load
    [100, 272],  # 20% L2 load
    [150, 272],  # 30% L2 load
    [200, 272],  # 40% L2 load
    [250, 272],  # 50% L2 load
    [300, 272],  # 60% L2 load
    [350, 272],  # 70% L2 load
    [400, 272],  # 80% L2 load
    [450, 272],  # 90% L2 load
    [500, 272],  # 100% L2 load

    [5000, 272],  # 40% L3 load
    [10000, 272],  # 80% L3 load
    [15000, 272],  # 120% L3 load

    [30000, 272],  # DRAM (2.4x L3 load)
    [300000, 272],  # DRAM (24x L3 load)
                           ]

copy_2D_perf_relevant_sizes = [
    [int(112 / 2) * int(112 / 2), 32 * 4],  # approx. depthwise_conv_2d size
]


# CHECK-NOT: FAILURE
def main():
  n_iters = 100
  for problem_sizes in copy_2D_perf_search_list:
    test_harness(lambda s, t: CopyProblem(dims=keys), [[np.float32] * 2],
                 test_sizes(keys, [problem_sizes]),
                 all_experts(fun_name, problem_sizes),
                 n_iters=n_iters,
                 function_name=fun_name,
                 dump_ir_to_file='/tmp/abc.mlir',
                 dump_obj_to_file='/tmp/abc.o')


if __name__ == '__main__':
  main()
