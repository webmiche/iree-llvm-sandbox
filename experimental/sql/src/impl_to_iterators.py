# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

from dataclasses import dataclass
from xdsl.ir import Operation, MLContext, Region, Block, Attribute
from typing import List, Type, Optional
from xdsl.dialects.builtin import ArrayAttr, StringAttr, ModuleOp, IntegerAttr, IntegerType, TupleType
from xdsl.dialects.llvm import LLVMStructType, LLVMExtractValue, LLVMInsertValue, LLVMAlloca
from xdsl.dialects.func import FuncOp, Return
from xdsl.dialects.arith import Addi, Constant, Cmpi

from xdsl.pattern_rewriter import RewritePattern, GreedyRewritePatternApplier, PatternRewriteWalker, PatternRewriter, op_type_rewrite_pattern

import dialects.rel_impl as RelImpl
import dialects.iterators as it

# This file contains the rewrite infrastructure to translate the relational
# implementation dialect to the iterators dialect. The current design has a
# parent class `RelImplRewriter` that contains functions used for several
# `Rewriter`s. All other `Rewriter`s inherit from that class.


@dataclass
class RelImplRewriter(RewritePattern):

  def convert_datatype(self, type_: RelImpl.DataType) -> Attribute:
    if isinstance(type_, RelImpl.Int32):
      return IntegerType.from_width(32)
    if isinstance(type_, RelImpl.Int64):
      # TODO: this is obviously broken, but the backend currently only support i32
      return IntegerType.from_width(32)
    raise Exception(f"type conversion not yet implemented for {type(type_)}")

  def convert_bag(self, bag: RelImpl.Bag) -> it.Stream:
    types = [self.convert_datatype(s.elt_type) for s in bag.schema.data]
    return it.Stream.get(LLVMStructType.from_type_list(types))

  def convert_tuple(self, tuple: RelImpl.Tuple) -> LLVMStructType:
    types = [self.convert_datatype(s.elt_type) for s in tuple.schema.data]
    return LLVMStructType.from_type_list(types)

  def find_index_in_schema(self, col_name: str, tuple: RelImpl.Tuple):
    for i, curr_elem in zip(range(len(tuple.schema.data)), tuple.schema.data):
      if curr_elem.elt_name.data == col_name:
        return i
    raise Exception(f"name not found in tuple schema: " + col_name)

  def add_sum_function(self, region: Region, elem_types: List[Attribute],
                       name: str):
    struct_type = LLVMStructType.from_type_list(elem_types)
    index_attr = IntegerAttr.from_index_int_value(0)
    sum_struct = FuncOp.from_region(
        name,
        [struct_type, struct_type],
        [struct_type],
        Region.from_block_list([
            Block.from_callable(
                [struct_type, struct_type],
                lambda ba1, ba2: [
                    l := LLVMExtractValue.
                    build(result_types=elem_types,
                          attributes=
                          {"position": ArrayAttr.from_list([index_attr])},
                          operands=[ba1]),
                    # This comment is needed to save formatting
                    r := LLVMExtractValue.build(
                        result_types=elem_types,
                        attributes=
                        {"position": ArrayAttr.from_list([index_attr])},
                        operands=[ba2]),
                    s := Addi.get(l, r),
                    res := LLVMInsertValue.build(
                        result_types=[struct_type],
                        operands=[ba1, s],
                        attributes=
                        {"position": ArrayAttr.from_list([index_attr])}),
                    Return.get(res)
                ])
        ]))
    region.blocks[0].add_op(sum_struct)
    return


#===------------------------------------------------------------------------===#
# Expressions
#===------------------------------------------------------------------------===#


@dataclass
class LiteralRewriter(RelImplRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Literal, rewriter: PatternRewriter):
    rewriter.replace_matched_op(
        Constant.from_int_constant(op.value.value, op.value.typ))


@dataclass
class IndexByNameRewriter(RelImplRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.IndexByName,
                        rewriter: PatternRewriter):
    rewriter.replace_matched_op(
        LLVMExtractValue.build(
            operands=[op.tuple],
            result_types=[self.convert_datatype(op.result.typ)],
            attributes={
                "position":
                    ArrayAttr.from_list([
                        IntegerAttr.from_index_int_value(
                            self.find_index_in_schema(op.col_name.data,
                                                      op.tuple.typ))
                    ])
            }))


@dataclass
class CompareRewriter(RelImplRewriter):

  def convert_comparator(self, comparator: str) -> int:
    if comparator == "==":
      return 0
    elif comparator == "!=":
      return 1
    elif comparator == "<":
      return 2
    elif comparator == "<=":
      return 3
    elif comparator == ">":
      return 4
    elif comparator == ">=":
      return 5
    raise Exception(f"comparator conversion not yet implemented for: " +
                    comparator)

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Compare, rewriter: PatternRewriter):
    rewriter.replace_matched_op(
        Cmpi.get(op.left, op.right,
                 self.convert_comparator(op.comparator.data)))


@dataclass
class YieldRewriter(RelImplRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Yield, rewriter: PatternRewriter):
    rewriter.replace_matched_op(Return.get(*op.ops))


@dataclass
class CreateTupleRewriter(RelImplRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.CreateTuple,
                        rewriter: PatternRewriter):
    c = Constant.from_int_constant(1, 32)
    alloca = LLVMAlloca.create(
        operands=[c.results[0]],
        attributes={
            "alignment": IntegerAttr.from_int_and_width(0, 32),
            "elem_type": IntegerType.from_width(32)
        },
        result_types=[self.convert_bag(op.parent_op().results[0].typ).types])
    new_ops = [c, alloca]
    for i, o in zip(range(len(op.ops)), op.ops):
      new_ops.append(
          LLVMInsertValue.build(
              operands=[new_ops[-1], o],
              attributes={
                  "position":
                      ArrayAttr.from_list([IntegerAttr.from_index_int_value(i)])
              },
              result_types=[
                  self.convert_bag(op.parent_op().results[0].typ).types
              ]))

    rewriter.replace_matched_op(new_ops)


@dataclass
class BinOpRewriter(RelImplRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.BinOp, rewriter: PatternRewriter):
    if op.operator.data == "+":
      rewriter.replace_matched_op(Addi.get(op.lhs, op.rhs))
      return
    raise Exception(f"BinOp conversion not yet implemented for " +
                    op.operator.data)


#===------------------------------------------------------------------------===#
# Operators
#===------------------------------------------------------------------------===#


@dataclass
class FullTableScanRewriter(RelImplRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.FullTableScanOp,
                        rewriter: PatternRewriter):
    # TODO: Change this once loading from input is supported
    rewriter.replace_matched_op(
        it.ConstantStreamOp.get([[IntegerAttr.from_int_and_width(0, 32)],
                                 [IntegerAttr.from_int_and_width(1, 32)],
                                 [IntegerAttr.from_int_and_width(2, 32)],
                                 [IntegerAttr.from_int_and_width(3, 32)]],
                                self.convert_bag(op.result.typ)))


@dataclass
class AggregateRewriter(RelImplRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Aggregate, rewriter: PatternRewriter):
    self.add_sum_function(op.parent_op().parent_region(),
                          self.convert_bag(op.result.typ).types.types.data,
                          "sum_struct")
    rewriter.replace_matched_op(
        it.ReduceOp.get(op.input.op, StringAttr.from_str("sum_struct"),
                        self.convert_bag(op.result.typ)))


@dataclass
class SelectRewriter(RelImplRewriter):

  count: int = 0

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Select, rewriter: PatternRewriter):
    rewriter.modify_block_argument_type(
        op.predicates.blocks[0].args[0],
        self.convert_tuple(op.predicates.blocks[0].args[0].typ))
    new_reg = rewriter.move_region_contents_to_new_regions(op.predicates)
    op.parent_op().parent_region().blocks[0].add_op(
        FuncOp.from_region("s" + str(self.count),
                           [new_reg.blocks[0].args[0].typ],
                           [IntegerType.from_width(1)], new_reg))
    rewriter.replace_matched_op(
        it.FilterOp.get(op.input.op, StringAttr.from_str("s" + str(self.count)),
                        self.convert_bag(op.result.typ)))
    self.count = self.count + 1


@dataclass
class ProjectRewriter(RelImplRewriter):

  count: int = 0

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Project, rewriter: PatternRewriter):
    rewriter.modify_block_argument_type(
        op.projection.blocks[0].args[0],
        self.convert_tuple(op.projection.blocks[0].args[0].typ))
    new_reg = rewriter.move_region_contents_to_new_regions(op.projection)
    op.parent_op().parent_region().blocks[0].add_op(
        FuncOp.from_region("m" + str(self.count),
                           [new_reg.blocks[0].args[0].typ],
                           [self.convert_bag(op.result.typ).types], new_reg))
    rewriter.replace_matched_op(
        it.MapOp.get(op.input.op, StringAttr.from_str("m" + str(self.count)),
                     self.convert_bag(op.result.typ)))
    self.count = self.count + 1


#===------------------------------------------------------------------------===#
# Conversion setup
#===------------------------------------------------------------------------===#


def impl_to_iterators(ctx: MLContext, query: ModuleOp):

  # Wrapping everything into a main function
  f = FuncOp.from_region("main", [], [],
                         Region.from_block_list([query.body.detach_block(0)]))
  query.body.add_block(Block.from_ops([f]))
  # Adding the sink
  query.body.blocks[0].ops[0].body.blocks[0].add_op(
      it.SinkOp.get(query.body.blocks[0].ops[0].body.blocks[0].ops[-1]))
  # Adding the return
  query.body.blocks[0].ops[0].body.blocks[0].add_op(Return.get())
  # IndexByNames and Yields need to be rewritten first, since both need access
  # to the rel_impl schemas to find the right position in the case of
  # IndexByName or to find the right result type in the case of  Yield
  # respectively.
  index_walker = PatternRewriteWalker(GreedyRewritePatternApplier(
      [IndexByNameRewriter(), CreateTupleRewriter()]),
                                      walk_regions_first=False,
                                      apply_recursively=False,
                                      walk_reverse=False)
  index_walker.rewrite_module(query)
  walker = PatternRewriteWalker(GreedyRewritePatternApplier([
      FullTableScanRewriter(),
      AggregateRewriter(),
      SelectRewriter(),
      LiteralRewriter(),
      CompareRewriter(),
      ProjectRewriter(),
      BinOpRewriter(),
      YieldRewriter()
  ]),
                                walk_regions_first=False,
                                apply_recursively=False,
                                walk_reverse=False)
  walker.rewrite_module(query)
