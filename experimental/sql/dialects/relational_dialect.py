# Copyright 2021 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

from dataclasses import dataclass
from typing import Any
from xdsl.ir import Block, Region, Operation, SSAValue, ParametrizedAttribute, Data, MLContext, Attribute
from xdsl.dialects.builtin import StringAttr, ArrayAttr, ArrayOfConstraint
from xdsl.irdl import AttributeDef, OperandDef, ResultDef, RegionDef, SingleBlockRegionDef, irdl_attr_definition, irdl_op_definition, ParameterDef, AnyAttr, VarOperandDef, builder

# This file contains the relational dialect. It is currently
# only aimed at the simple query and mirrors a lot of the
# ibis dialect. This is subject to change, once the translation
# to the iterator dialect starts getting in shape.


@irdl_attr_definition
class DataType(ParametrizedAttribute):
  name: str = "rel.datatype"


@irdl_attr_definition
class int32(DataType):
  name: str = "rel.int32"


@irdl_attr_definition
class String(DataType):
  name: str = "rel.String"


@irdl_attr_definition
class float64(DataType):
  name: str = "rel.float64"


@irdl_attr_definition
class Bag(ParametrizedAttribute):
  name: str = "rel.bag"


@irdl_attr_definition
class Column(ParametrizedAttribute):
  name: str = "rel.column"


@irdl_op_definition
class Equals(Operation):
  name: str = "rel.equals"

  table = OperandDef(Bag())
  column = AttributeDef(StringAttr)
  comparator = OperandDef(AnyAttr())

  result = ResultDef(Column())

  @staticmethod
  @builder
  def get(table: Operation, column: StringAttr,
          comparator: Operation) -> 'Equals':
    return Equals.build(operands=[table, comparator],
                        attributes={"column": column},
                        result_types=[Column()])


@irdl_op_definition
class PandasTable(Operation):
  name: str = "rel.pandas_table"

  table_name = AttributeDef(StringAttr)
  schema = SingleBlockRegionDef()
  result = ResultDef(Bag())

  @staticmethod
  @builder
  def get(name: str, Schema: Region, result_type: Attribute) -> 'PandasTable':
    return PandasTable.build(
        attributes={"table_name": StringAttr.from_str(name)},
        regions=[Schema],
        result_types=[result_type])


@irdl_op_definition
class SchemaElement(Operation):
  name: str = "rel.schema_element"

  elt_name = AttributeDef(StringAttr)
  elt_type = AttributeDef(DataType())

  @staticmethod
  def get(name: str, type: DataType):
    return SchemaElement.build(attributes={
        "elt_name": StringAttr.from_str(name),
        "elt_type": type
    })


@irdl_op_definition
class Selection(Operation):
  name: str = "rel.selection"

  parent_ = OperandDef(Bag())
  predicates = SingleBlockRegionDef()

  result = ResultDef(Bag())

  @builder
  @staticmethod
  def get(table: Operation, predicates: Region) -> 'Selection':
    return Selection.build(operands=[table],
                           regions=[predicates],
                           result_types=[Bag()])


@irdl_op_definition
class Yield(Operation):
  name: str = "rel.Yield"

  ops = VarOperandDef(AnyAttr())

  @staticmethod
  @builder
  def get(ops: list[Operation]) -> 'Yield':
    return Yield.build(operands=[ops])


@dataclass
class Relational:
  ctx: MLContext

  def __post_init__(self: 'Relational'):
    self.ctx.register_attr(Bag)
    self.ctx.register_attr(DataType)
    self.ctx.register_attr(int32)
    self.ctx.register_attr(float64)
    self.ctx.register_attr(String)

    self.ctx.register_op(Selection)
    self.ctx.register_op(PandasTable)
    self.ctx.register_op(SchemaElement)
