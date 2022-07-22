// RUN: mlir-proto-opt %s \
// RUN:   -convert-iterators-to-llvm -reconcile-unrealized-casts \
// RUN: | FileCheck --enable-var-scope %s

!element_type = !llvm.struct<(i32)>

// CHECK-LABEL: func private @iterators.scan_columnar_batch.close.{{[0-9]+}}(%{{.*}}: !llvm.struct<"iterators.scan_columnar_batch_state{{.*}}", ({{.*}})>) -> !llvm.struct<"iterators.scan_columnar_batch_state{{.*}}", ({{.*}})> {
// CHUCK-NEXT:    %[[V0:.*]] = llvm.extractvalue %[[arg0:.*]][0 : index] : !llvm.struct<"[[scanColumnarBatchStateName:iterators\.scan_columnar_batch_state.*]]", ([[nestedUpstreamStateType:.*]])>
// CHUCK-NEXT:    %[[V1:.*]] = call @iterators.{{[a-zA-Z]+}}.close.{{[0-9]+}}(%[[V0]]) : ([[upstreamStateType:.*]]) -> [[upstreamStateType]]
// CHUCK-NEXT:    %[[V2:.*]] = llvm.insertvalue %[[V1]], %[[arg0]][0 : index] : !llvm.struct<"[[scanColumnarBatchStateName]]", ([[nestedUpstreamStateType]])>
// CHUCK-NEXT:    return %[[V2]] : !llvm.struct<"[[scanColumnarBatchStateName]]", ([[nestedUpstreamStateType]])>
// CHUCK-NEXT:  }

// CHECK-LABEL: func private @iterators.scan_columnar_batch.next.{{[0-9]+}}(%{{.*}}: !llvm.struct<"iterators.scan_columnar_batch_state{{.*}}", ({{.*}})>) -> (!llvm.struct<"iterators.scan_columnar_batch_state{{.*}}", ({{.*}})>, i1, !llvm.struct<(i32)>)
// CHUCK-NEXT:    %[[V0:.*]] = llvm.extractvalue %[[arg0:.*]][0 : index] : !llvm.struct<"[[scanColumnarBatchStateName:iterators\.scan_columnar_batch_state.*]]", ([[nestedUpstreamStateType:.*]])>
// CHUCK-NEXT:    %[[V1:.*]]:3 = call @iterators.{{[a-zA-Z]+}}.next.{{[0-9]+}}(%[[V0]]) : ([[upstreamStateType:.*]]) -> ([[upstreamStateType]], i1, !llvm.struct<(i32)>)
// CHUCK-NEXT:    %[[V2:.*]]:3 = scf.if %[[V1]]#1 -> ([[upstreamStateType]], i1, !llvm.struct<(i32)>) {
// CHUCK-NEXT:      %[[V4:.*]]:3 = scf.while (%[[arg1:.*]] = %[[V1]]#0, %[[arg2:.*]] = %[[V1]]#2) : ([[upstreamStateType]], !llvm.struct<(i32)>) -> ([[upstreamStateType]], !llvm.struct<(i32)>, !llvm.struct<(i32)>) {
// CHUCK-NEXT:        %[[V5:.*]]:3 = func.call @iterators.{{[a-zA-Z]+}}.next.{{[0-9]+}}(%[[arg1]]) : ([[upstreamStateType]]) -> ([[upstreamStateType]], i1, !llvm.struct<(i32)>)
// CHUCK-NEXT:        scf.condition(%[[V5]]#1) %[[V5]]#0, %[[arg2]], %[[V5]]#2 : [[upstreamStateType]], !llvm.struct<(i32)>, !llvm.struct<(i32)>
// CHUCK-NEXT:      } do {
// CHUCK-NEXT:      ^[[bb0:.*]](%[[arg1:.*]]: [[upstreamStateType]], %[[arg2:.*]]: !llvm.struct<(i32)>, %[[arg3:.*]]: !llvm.struct<(i32)>):
// CHUCK-NEXT:        %[[V5:.*]] = func.call @sum_struct(%[[arg2]], %[[arg3]]) : (!llvm.struct<(i32)>, !llvm.struct<(i32)>) -> !llvm.struct<(i32)>
// CHUCK-NEXT:        scf.yield %[[arg1]], %[[V5]] : [[upstreamStateType]], !llvm.struct<(i32)>
// CHUCK-NEXT:      }
// CHUCK-NEXT:      %[[true:.*]] = arith.constant true
// CHUCK-NEXT:      scf.yield %[[V4]]#0, %[[true]], %[[V4]]#1 : [[upstreamStateType]], i1, !llvm.struct<(i32)>
// CHUCK-NEXT:    } else {
// CHUCK-NEXT:      scf.yield %[[V1]]#0, %[[V1]]#1, %[[V1]]#2 : [[upstreamStateType]], i1, !llvm.struct<(i32)>
// CHUCK-NEXT:    }
// CHUCK-NEXT:    %[[V3:.*]] = llvm.insertvalue %[[V2]]#0, %[[arg0]][0 : index] : !llvm.struct<"[[scanColumnarBatchStateName]]", ([[nestedUpstreamStateType]])>
// CHUCK-NEXT:    return %[[V3]], %[[V2]]#1, %[[V2]]#2 : !llvm.struct<"[[scanColumnarBatchStateName]]", ([[nestedUpstreamStateType]])>, i1, !llvm.struct<(i32)>
// CHUCK-NEXT:  }

// CHECK-LABEL: func private @iterators.scan_columnar_batch.open.{{[0-9]+}}(%{{.*}}: !llvm.struct<"iterators.scan_columnar_batch_state{{.*}}", ({{.*}})>) -> !llvm.struct<"iterators.scan_columnar_batch_state{{.*}}", ({{.*}})>
// CHUCK-NEXT:    %[[V0:.*]] = llvm.extractvalue %[[arg0:.*]][0 : index] : !llvm.struct<"[[scanColumnarBatchStateName:iterators\.scan_columnar_batch_state.*]]", ([[nestedUpstreamStateType:.*]])>
// CHUCK-NEXT:    %[[V1:.*]] = call @iterators.{{[a-zA-Z]+}}.open.{{[0-9]+}}(%[[V0]]) : ([[upstreamStateType:.*]]) -> [[upstreamStateType]]
// CHUCK-NEXT:    %[[V2:.*]] = llvm.insertvalue %[[V1]], %[[arg0]][0 : index] : !llvm.struct<"[[scanColumnarBatchStateName]]", ([[nestedUpstreamStateType]])>
// CHUCK-NEXT:    return %[[V2]] : !llvm.struct<"[[scanColumnarBatchStateName]]", ([[nestedUpstreamStateType]])>
// CHUCK-NEXT:  }

func.func @main(%batch : !iterators.columnar_batch<tuple<i32>>) {
  // CHECK-LABEL: func.func @main(%{{arg.*}}: !llvm.struct<(i64, ptr<i32>)>) {
  %stream = "iterators.scan_columnar_batch"(%batch)
    : (!iterators.columnar_batch<tuple<i32>>) -> !iterators.stream<!llvm.struct<(i32)>>
  // CHECK-NEXT:    %[[V1:.*]] = llvm.mlir.undef : !llvm.struct<"[[scanColumnarBatchStateName:iterators\.scan_columnar_batch_state.*]]", (i64, struct<(i64, ptr<i32>)>)>
  // CHECK-NEXT:    %[[V2:.*]] = llvm.insertvalue %[[arg:.*]], %[[V1]][1 : index] : !llvm.struct<"[[scanColumnarBatchStateName]]", (i64, struct<(i64, ptr<i32>)>)>
  return
  // CHECK-NEXT:   return
}
// CHECK-NEXT:   }
