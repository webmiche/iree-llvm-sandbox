# Copyright 2021 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

from dataclasses import dataclass
from xdsl.dialects.builtin import ArrayAttr, StringAttr, ModuleOp
from xdsl.ir import Operation, MLContext, Region, Block
from typing import List, Type, Optional

from xdsl.pattern_rewriter import RewritePattern, GreedyRewritePatternApplier, PatternRewriteWalker, PatternRewriter, op_type_rewrite_pattern
import dialects.ibis_dialect as id
import dialects.relational_dialect as rel


@dataclass
class TableRewriter(RewritePattern):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: id.PandasTable, rewriter: PatternRewriter):
    new_op = rel.PandasTable.build(
        regions=[rewriter.move_region_contents_to_new_regions(op.schema)],
        attributes={"table_name": op.table_name},
        result_types=[rel.Bag()])
    rewriter.replace_matched_op(new_op)


@dataclass
class SchemaElementRewriter(RewritePattern):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: id.SchemaElement, rewriter: PatternRewriter):
    new_op = rel.SchemaElement.build(attributes={
        "elt_name": op.elt_name,
        "elt_type": op.elt_type
    })
    rewriter.replace_matched_op(new_op)


@dataclass
class TableColumnRewriter(RewritePattern):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: id.TableColumn, rewriter: PatternRewriter):
    rewriter.inline_block_before_matched_op(op.table.blocks[0])
    rewriter.erase_matched_op()


@dataclass
class SelectionRewriter(RewritePattern):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: id.Selection, rewriter: PatternRewriter):
    rewriter.inline_block_before_matched_op(op.table.blocks[0])
    new_op = rel.Selection.get(
        op.parent_block().ops[0],
        rewriter.move_region_contents_to_new_regions(op.predicates))
    rewriter.replace_matched_op([new_op])


@dataclass
class EqualsRewriter(RewritePattern):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: id.Equals, rewriter: PatternRewriter):
    rewriter.inline_block_before_matched_op(op.right.blocks[0])
    new_op = rel.Equals.get(op.parent_op().operands[0],
                            op.left.blocks[0].ops[0].col_name,
                            op.parent_block().ops[0])
    rewriter.replace_matched_op([new_op])


@dataclass
class YieldRewriter(RewritePattern):

  @op_type_rewrite_pattern
  def match_and_rewrite(self, op: id.Yield, rewriter: PatternRewriter):
    rewriter.erase_matched_op()


def ibis_dialect_to_relational(ctx: MLContext, query: ModuleOp):
  walker = PatternRewriteWalker(GreedyRewritePatternApplier([
      TableRewriter(),
      SchemaElementRewriter(),
      SelectionRewriter(),
      EqualsRewriter(),
      YieldRewriter()
  ]),
                                walk_regions_first=False,
                                apply_recursively=True,
                                walk_reverse=True)
  walker.rewrite_module(query)
