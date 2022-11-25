# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

from dataclasses import dataclass
from xdsl.ir import Operation, MLContext, Region, Block, Attribute
from typing import List, Type, Optional, Tuple
from xdsl.dialects.builtin import ArrayAttr, StringAttr, ModuleOp, IntegerAttr, IntegerType, TupleType, UnitAttr
from xdsl.dialects.llvm import LLVMStructType, LLVMExtractValue, LLVMInsertValue, LLVMMLIRUndef
from xdsl.dialects.func import FuncOp, Return
from xdsl.dialects.arith import Addi, Constant, Cmpi, Muli, Subi, AndI

from xdsl.pattern_rewriter import RewritePattern, GreedyRewritePatternApplier, PatternRewriteWalker, PatternRewriter, op_type_rewrite_pattern

import dialects.rel_impl as RelImpl
import dialects.stream as Stream
from decimal import Decimal
from datetime import datetime, timezone
from time import mktime


def convert_datatype(type_: RelImpl.DataType) -> Attribute:
  if isinstance(type_, RelImpl.Boolean):
    return IntegerType.from_width(1)
  if isinstance(type_, RelImpl.Int32):
    return IntegerType.from_width(32)
  if isinstance(type_, RelImpl.Int64):
    return IntegerType.from_width(64)
  if isinstance(type_, RelImpl.Decimal):
    return IntegerType.from_width(64)
  if isinstance(type_, RelImpl.Timestamp):
    return IntegerType.from_width(64)
  if isinstance(type_, RelImpl.String):
    # TODO: This is a shortcut to represent strings in some way. Adjust this
    # to a) non-fixed length strings or b) dynamically fixed size strings.
    return LLVMStructType([
        StringAttr.from_str(""),
        ArrayAttr.from_list([IntegerType.from_width(8)] * 8)
    ])
  if isinstance(type_, RelImpl.Nullable):
    return convert_datatype(type_.type)
  raise Exception(f"type conversion not yet implemented for {type(type_)}")


def convert_bag(bag: RelImpl.Bag) -> Stream.Stream:
  types = [convert_datatype(s.elt_type) for s in bag.schema.data]
  return Stream.Stream.get(TupleType.from_type_list(types))


def convert_tuple(tuple: RelImpl.Tuple) -> LLVMStructType:
  types = [convert_datatype(s.elt_type) for s in tuple.schema.data]
  return TupleType.from_type_list(types)


def find_index_in_schema(col_name: str, tuple: RelImpl.Tuple):
  for i, curr_elem in zip(range(len(tuple.schema.data)), tuple.schema.data):
    if curr_elem.elt_name.data == col_name:
      return i
  raise Exception(f"name not found in tuple schema: " + col_name)


@dataclass
class StreamRewriter(RewritePattern):
  pass


#===------------------------------------------------------------------------===#
# Expressions
#===------------------------------------------------------------------------===#


@dataclass
class AndRewriter(StreamRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.And, rewriter: PatternRewriter):
    rewriter.replace_matched_op(AndI.get(op.lhs, op.rhs))


@dataclass
class LiteralRewriter(StreamRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Literal, rewriter: PatternRewriter):
    type = op.result.typ
    if isinstance(type, RelImpl.Nullable):
      type = type.type
    if isinstance(type, RelImpl.Int32) or isinstance(type, RelImpl.Int64):
      rewriter.replace_matched_op(
          Constant.from_int_constant(op.value.value, op.value.typ))
    elif isinstance(type, RelImpl.Decimal):
      rewriter.replace_matched_op(
          Constant.from_int_constant(int(Decimal(op.value.data) * Decimal(100)),
                                     64))
    elif isinstance(type, RelImpl.Timestamp):
      epoch = datetime.utcfromtimestamp(0)
      rewriter.replace_matched_op(
          Constant.from_int_constant(
              int((datetime.strptime(op.value.data, "%Y-%m-%d") -
                   epoch).total_seconds()), 64))
    else:
      raise Exception(
          f"lowering of literals with type {type(type)} not yet implemented")


@dataclass
class IndexByNameRewriter(StreamRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.IndexByName,
                        rewriter: PatternRewriter):
    # TODO: handle tuple with multiple elements
    if len(op.result.uses) == 0:
      rewriter.erase_matched_op()
    else:
      rewriter.replace_matched_op(Stream.UnpackOp.get(op.tuple))


@dataclass
class ToTupleConverter(StreamRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Operator, rewriter: PatternRewriter):
    if len(op.regions) == 0:
      return
    block = op.regions[0].blocks[0]
    rewriter.modify_block_argument_type(block.args[0],
                                        convert_tuple(block.args[0].typ))


@dataclass
class CompareRewriter(StreamRewriter):

  def convert_comparator(self, comparator: str) -> int:
    if comparator == "=":
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
class YieldValueRewriter(StreamRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.YieldValue,
                        rewriter: PatternRewriter):
    rewriter.replace_matched_op(Stream.YieldOp.get(op.op.op))


@dataclass
class YieldTupleRewriter(StreamRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.YieldTuple,
                        rewriter: PatternRewriter):
    rewriter.replace_matched_op(
        [t := Stream.PackOp.get(op.ops),
         Stream.YieldOp.get([t])])


@dataclass
class BinOpRewriter(StreamRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.BinOp, rewriter: PatternRewriter):
    # TODO: Decimals might change precision here. Reflect that somehow.
    if op.operator.data == "+":
      rewriter.replace_matched_op(
          Addi.build(operands=[op.lhs, op.rhs], result_types=[op.lhs.typ]))
      return
    if op.operator.data == "*":
      rewriter.replace_matched_op(
          Muli.build(operands=[op.lhs, op.rhs], result_types=[op.lhs.typ]))
      return
    if op.operator.data == "-":
      rewriter.replace_matched_op(Subi.get(op.lhs, op.rhs))
      return
    raise Exception(f"BinOp conversion not yet implemented for " +
                    op.operator.data)


#===------------------------------------------------------------------------===#
# Operators
#===------------------------------------------------------------------------===#


@dataclass
class FullTableScanRewriter(StreamRewriter):

  table_mapping: dict[str, Stream.Stream]

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.FullTableScanOp,
                        rewriter: PatternRewriter):

    op.result.replace_by(self.table_mapping[op.table_name.data])
    rewriter.erase_matched_op()


@dataclass
class ProjectRewriter(StreamRewriter):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: RelImpl.Project, rewriter: PatternRewriter):
    rewriter.replace_matched_op(
        Stream.MapOp.get(
            op.input,
            rewriter.move_region_contents_to_new_regions(op.projection),
            convert_bag(op.result.typ)))


def get_stream_and_name_list(
    op: FuncOp) -> Tuple[list[str], list[Stream.Stream]]:
  """
  Scans over all the operations of a top-level function op and returns a list of
  the table_names and a list of the batch types these table loads correspond to.
  In the case of a table without a `cols` attribute, all the types in the table
  are part of the ColumnarBatch, whereas a table with `cols` only has the types
  of columns to be loaded.
  """
  streams = []
  names = []
  for o in op.body.ops:
    if isinstance(
        o,
        RelImpl.FullTableScanOp) and not names.__contains__(o.table_name.data):
      curr_stream = Stream.Stream.get(
          TupleType([
              ArrayAttr.from_list([
                  convert_datatype(e.elt_type) for e in o.result.typ.schema.data
              ])
          ]))
      streams.append(curr_stream)
      if o.cols:
        names.append(o.table_name.data + "," +
                     ",".join([s.data for s in o.cols.data]))
      else:
        names.append(o.table_name.data)

  return streams, names


def impl_to_stream(ctx: MLContext, query: ModuleOp):

  streams, names = get_stream_and_name_list(query)

  table_mapping = {}

  # Wrapping everything into a main function
  body_block = Block.from_arg_types(streams)
  body_block.add_ops(
      [query.body.blocks[0].detach_op(o) for o in query.body.blocks[0].ops])
  query.body.detach_block(0)
  f = FuncOp.from_region("main", streams,
                         [convert_bag(body_block.ops[-1].results[0].typ)],
                         Region.from_block_list([body_block]))
  f.attributes['llvm.emit_c_interface'] = UnitAttr([])
  query.body.add_block(Block.from_ops([f]))

  # Populating a mapping from table names to BlockArguments
  for n, b in zip(names, f.body.blocks[0].args):
    table_mapping[n] = b

  # Adding the return
  query.body.blocks[0].ops[0].body.blocks[0].add_op(
      Return.get(query.body.blocks[0].ops[0].body.blocks[0].ops[-1]))

  index_walker = PatternRewriteWalker(GreedyRewritePatternApplier(
      [IndexByNameRewriter(), ToTupleConverter()]),
                                      walk_regions_first=False,
                                      apply_recursively=False,
                                      walk_reverse=False)
  index_walker.rewrite_module(query)
  walker = PatternRewriteWalker(GreedyRewritePatternApplier([
      LiteralRewriter(),
      CompareRewriter(),
      BinOpRewriter(),
      YieldValueRewriter(),
      AndRewriter(),
      ProjectRewriter(),
      FullTableScanRewriter(table_mapping),
      YieldTupleRewriter()
  ]),
                                walk_regions_first=False,
                                apply_recursively=False,
                                walk_reverse=False)
  walker.rewrite_module(query)
