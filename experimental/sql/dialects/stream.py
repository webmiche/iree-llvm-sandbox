# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

from dataclasses import dataclass
from typing import Any, Union, List
from xdsl.ir import Block, Region, Operation, SSAValue, ParametrizedAttribute, Data, MLContext, Attribute
from xdsl.dialects.builtin import StringAttr, ArrayAttr, ArrayOfConstraint, IntegerAttr, IntegerType, TupleType, FlatSymbolRefAttr
from xdsl.irdl import AttributeDef, OperandDef, ResultDef, RegionDef, SingleBlockRegionDef, irdl_attr_definition, irdl_op_definition, ParameterDef, AnyAttr, VarOperandDef, builder

#===------------------------------------------------------------------------===#
# Data types
#===------------------------------------------------------------------------===#

#===------------------------------------------------------------------------===#
# Query types
#===------------------------------------------------------------------------===#


@irdl_attr_definition
class Stream(ParametrizedAttribute):
  """
  Stream of elements of the given type.
  """
  name = "stream.stream"

  type: ParameterDef[Attribute]

  @builder
  @staticmethod
  def get(elem_type: Attribute) -> 'Stream':
    return Stream([elem_type])  #type: ignore


#===------------------------------------------------------------------------===#
# Operations
#===------------------------------------------------------------------------===#


@irdl_op_definition
class MapOp(Operation):
  name = "stream.map"

  input = OperandDef(Stream)
  # TODO: add registers
  region = RegionDef()

  res = ResultDef(Stream)

  @builder
  @staticmethod
  def get(input: Operation, reg: Region, res_type: Attribute) -> 'MapOp':
    return MapOp.build(operands=[input], regions=[reg], result_types=[res_type])


@irdl_op_definition
class UnpackOp(Operation):
  name = "stream.unpack"

  input = OperandDef(TupleType)
  res = ResultDef(AnyAttr())

  @builder
  @staticmethod
  def get(input: SSAValue) -> 'UnpackOp':
    return UnpackOp.build(operands=[input], result_types=input.typ.types.data)


@irdl_op_definition
class PackOp(Operation):
  name = "stream.pack"

  inputs = VarOperandDef(AnyAttr())
  res = ResultDef(TupleType)

  @builder
  @staticmethod
  def get(inputs: List[SSAValue]) -> 'PackOp':
    return PackOp.build(
        operands=[inputs],
        result_types=[TupleType.from_type_list([e.typ for e in inputs])])


@irdl_op_definition
class YieldOp(Operation):
  name = "stream.yield"

  res = VarOperandDef(AnyAttr())

  @builder
  @staticmethod
  def get(ops: list[Operation]) -> 'YieldOp':
    return YieldOp.build(operands=[ops])


@dataclass
class Streamer:
  ctx: MLContext

  def __post_init__(self: 'Streamer'):
    self.ctx.register_attr(Stream)

    self.ctx.register_op(MapOp)

    self.ctx.register_op(UnpackOp)
    self.ctx.register_op(PackOp)
    self.ctx.register_op(YieldOp)
