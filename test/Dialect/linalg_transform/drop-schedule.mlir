// RUN: mlir-proto-opt -linalg-drop-schedule %s | FileCheck %s

func @matmul_tensors(
  %arg0: tensor<128x128xf32>, %arg1: tensor<128x128xf32>, %arg2: tensor<128x128xf32> { linalg.inplaceable = true})
    -> tensor<128x128xf32> {
  %0 = linalg.matmul  ins(%arg0, %arg1: tensor<128x128xf32>, tensor<128x128xf32>)
                     outs(%arg2: tensor<128x128xf32>)
    -> tensor<128x128xf32>
  return %0 : tensor<128x128xf32>
}

// CHECK-NOT: pdl.pattern
pdl.pattern @pdl_target : benefit(1) {
  %args = operands
  %results = types
  %0 = operation "linalg.matmul"(%args : !pdl.range<value>) -> (%results : !pdl.range<type>)
  %1 = pdl.attribute @matmul_tensors
  apply_native_constraint "nestedInFunc"(%0, %1 : !pdl.operation, !pdl.attribute)
  // TODO: we don't want this, but it is the required terminator for pdl.pattern
  rewrite %0 with "iree_linalg_transform.apply"
}

// CHECK-NOT: iree_linalg_transform.sequence
iree_linalg_transform.sequence {
  %0 = match @pdl_target
  tile %0 {sizes = [4, 4, 4], pad = false}
}
