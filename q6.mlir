module attributes {llvm.data_layout = ""} {
  llvm.func @printf(!llvm.ptr<i8>, ...) -> i32
  llvm.mlir.global internal constant @frmt_spec.anonymous_tuple("(%lli)\0A\00")
  llvm.func @iterators.reduce.close.0(%arg0: !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>) -> !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    %1 = llvm.call @iterators.map.close.0(%0) : (!llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>) -> !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    %2 = llvm.insertvalue %1, %arg0[0 : index] : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    llvm.return %2 : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
  }
  llvm.func @iterators.reduce.next.0(%arg0: !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>) -> !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    %1 = llvm.call @iterators.map.next.0(%0) : (!llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>) -> !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %2 = llvm.extractvalue %1[0] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %3 = llvm.extractvalue %1[1] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %4 = llvm.extractvalue %1[2] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    llvm.cond_br %3, ^bb1, ^bb5
  ^bb1:  // pred: ^bb0
    llvm.br ^bb2(%2, %4 : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, !llvm.struct<(i64)>)
  ^bb2(%5: !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, %6: !llvm.struct<(i64)>):  // 2 preds: ^bb1, ^bb3
    %7 = llvm.call @iterators.map.next.0(%5) : (!llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>) -> !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %8 = llvm.extractvalue %7[0] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %9 = llvm.extractvalue %7[1] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %10 = llvm.extractvalue %7[2] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    llvm.cond_br %9, ^bb3(%8, %6, %10 : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, !llvm.struct<(i64)>, !llvm.struct<(i64)>), ^bb4
  ^bb3(%11: !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, %12: !llvm.struct<(i64)>, %13: !llvm.struct<(i64)>):  // pred: ^bb2
    %14 = llvm.call @sum_struct(%12, %13) : (!llvm.struct<(i64)>, !llvm.struct<(i64)>) -> !llvm.struct<(i64)>
    llvm.br ^bb2(%11, %14 : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, !llvm.struct<(i64)>)
  ^bb4:  // pred: ^bb2
    %15 = llvm.mlir.constant(true) : i1
    llvm.br ^bb6(%8, %15, %6 : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, !llvm.struct<(i64)>)
  ^bb5:  // pred: ^bb0
    llvm.br ^bb6(%2, %3, %4 : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, !llvm.struct<(i64)>)
  ^bb6(%16: !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, %17: i1, %18: !llvm.struct<(i64)>):  // 2 preds: ^bb4, ^bb5
    llvm.br ^bb7
  ^bb7:  // pred: ^bb6
    %19 = llvm.insertvalue %16, %arg0[0 : index] : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    %20 = llvm.mlir.undef : !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
    %21 = llvm.insertvalue %19, %20[0] : !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
    %22 = llvm.insertvalue %17, %21[1] : !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
    %23 = llvm.insertvalue %18, %22[2] : !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
    llvm.return %23 : !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
  }
  llvm.func @iterators.reduce.open.0(%arg0: !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>) -> !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    %1 = llvm.call @iterators.map.open.0(%0) : (!llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>) -> !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    %2 = llvm.insertvalue %1, %arg0[0 : index] : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    llvm.return %2 : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
  }
  llvm.func @iterators.map.close.0(%arg0: !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>) -> !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    %1 = llvm.call @iterators.filter.close.0(%0) : (!llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>) -> !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    %2 = llvm.insertvalue %1, %arg0[0 : index] : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    llvm.return %2 : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
  }
  llvm.func @iterators.map.next.0(%arg0: !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>) -> !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    %1 = llvm.call @iterators.filter.next.0(%0) : (!llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>) -> !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %2 = llvm.extractvalue %1[0] : !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %3 = llvm.extractvalue %1[1] : !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %4 = llvm.extractvalue %1[2] : !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    llvm.cond_br %3, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    %5 = llvm.call @m0(%4) : (!llvm.struct<(i64, i64, i64, i64)>) -> !llvm.struct<(i64)>
    llvm.br ^bb3(%5 : !llvm.struct<(i64)>)
  ^bb2:  // pred: ^bb0
    %6 = llvm.mlir.undef : !llvm.struct<(i64)>
    llvm.br ^bb3(%6 : !llvm.struct<(i64)>)
  ^bb3(%7: !llvm.struct<(i64)>):  // 2 preds: ^bb1, ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    %8 = llvm.insertvalue %2, %arg0[0 : index] : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    %9 = llvm.mlir.undef : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %10 = llvm.insertvalue %8, %9[0] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %11 = llvm.insertvalue %3, %10[1] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    %12 = llvm.insertvalue %7, %11[2] : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
    llvm.return %12 : !llvm.struct<(struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>, i1, struct<(i64)>)>
  }
  llvm.func @iterators.map.open.0(%arg0: !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>) -> !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    %1 = llvm.call @iterators.filter.open.0(%0) : (!llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>) -> !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    %2 = llvm.insertvalue %1, %arg0[0 : index] : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    llvm.return %2 : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
  }
  llvm.func @iterators.filter.close.0(%arg0: !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>) -> !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    %1 = llvm.call @iterators.scan_columnar_batch.close.0(%0) : (!llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>) -> !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
    %2 = llvm.insertvalue %1, %arg0[0 : index] : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    llvm.return %2 : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
  }
  llvm.func @iterators.filter.next.0(%arg0: !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>) -> !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    llvm.br ^bb1(%0 : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)
  ^bb1(%1: !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>):  // 2 preds: ^bb0, ^bb6
    %2 = llvm.call @iterators.scan_columnar_batch.next.0(%1) : (!llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>) -> !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %3 = llvm.extractvalue %2[0] : !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %4 = llvm.extractvalue %2[1] : !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %5 = llvm.extractvalue %2[2] : !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    llvm.cond_br %4, ^bb2, ^bb3
  ^bb2:  // pred: ^bb1
    %6 = llvm.call @s0(%5) : (!llvm.struct<(i64, i64, i64, i64)>) -> i1
    llvm.br ^bb4(%6 : i1)
  ^bb3:  // pred: ^bb1
    llvm.br ^bb4(%4 : i1)
  ^bb4(%7: i1):  // 2 preds: ^bb2, ^bb3
    llvm.br ^bb5
  ^bb5:  // pred: ^bb4
    %8 = llvm.mlir.constant(true) : i1
    %9 = llvm.xor %7, %8  : i1
    %10 = llvm.and %4, %9  : i1
    llvm.cond_br %10, ^bb6(%3, %4, %5 : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, !llvm.struct<(i64, i64, i64, i64)>), ^bb7
  ^bb6(%11: !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, %12: i1, %13: !llvm.struct<(i64, i64, i64, i64)>):  // pred: ^bb5
    llvm.br ^bb1(%11 : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)
  ^bb7:  // pred: ^bb5
    %14 = llvm.insertvalue %3, %arg0[0 : index] : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    %15 = llvm.mlir.undef : !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %16 = llvm.insertvalue %14, %15[0] : !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %17 = llvm.insertvalue %4, %16[1] : !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %18 = llvm.insertvalue %5, %17[2] : !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    llvm.return %18 : !llvm.struct<(struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>, i1, struct<(i64, i64, i64, i64)>)>
  }
  llvm.func @iterators.filter.open.0(%arg0: !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>) -> !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    %1 = llvm.call @iterators.scan_columnar_batch.open.0(%0) : (!llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>) -> !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
    %2 = llvm.insertvalue %1, %arg0[0 : index] : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    llvm.return %2 : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
  }
  llvm.func @iterators.scan_columnar_batch.close.0(%arg0: !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>) -> !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)> attributes {sym_visibility = "private"} {
    llvm.return %arg0 : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
  }
  llvm.func @iterators.scan_columnar_batch.next.0(%arg0: !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>) -> !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
    %1 = llvm.extractvalue %arg0[1 : index] : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
    %2 = llvm.extractvalue %1[0 : index] : !llvm.struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>
    %3 = llvm.icmp "slt" %0, %2 : i64
    llvm.cond_br %3, ^bb1, ^bb2
  ^bb1:  // pred: ^bb0
    %4 = llvm.mlir.constant(1 : i64) : i64
    %5 = llvm.add %0, %4  : i64
    %6 = llvm.insertvalue %5, %arg0[0 : index] : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
    %7 = llvm.mlir.undef : !llvm.struct<(i64, i64, i64, i64)>
    %8 = llvm.extractvalue %1[1 : index] : !llvm.struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>
    %9 = llvm.getelementptr %8[%0] : (!llvm.ptr<i64>, i64) -> !llvm.ptr<i64>
    %10 = llvm.load %9 : !llvm.ptr<i64>
    %11 = llvm.insertvalue %10, %7[0 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %12 = llvm.extractvalue %1[2 : index] : !llvm.struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>
    %13 = llvm.getelementptr %12[%0] : (!llvm.ptr<i64>, i64) -> !llvm.ptr<i64>
    %14 = llvm.load %13 : !llvm.ptr<i64>
    %15 = llvm.insertvalue %14, %11[1 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %16 = llvm.extractvalue %1[3 : index] : !llvm.struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>
    %17 = llvm.getelementptr %16[%0] : (!llvm.ptr<i64>, i64) -> !llvm.ptr<i64>
    %18 = llvm.load %17 : !llvm.ptr<i64>
    %19 = llvm.insertvalue %18, %15[2 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %20 = llvm.extractvalue %1[4 : index] : !llvm.struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>
    %21 = llvm.getelementptr %20[%0] : (!llvm.ptr<i64>, i64) -> !llvm.ptr<i64>
    %22 = llvm.load %21 : !llvm.ptr<i64>
    %23 = llvm.insertvalue %22, %19[3 : index] : !llvm.struct<(i64, i64, i64, i64)>
    llvm.br ^bb3(%6, %23 : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, !llvm.struct<(i64, i64, i64, i64)>)
  ^bb2:  // pred: ^bb0
    %24 = llvm.mlir.undef : !llvm.struct<(i64, i64, i64, i64)>
    llvm.br ^bb3(%arg0, %24 : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, !llvm.struct<(i64, i64, i64, i64)>)
  ^bb3(%25: !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, %26: !llvm.struct<(i64, i64, i64, i64)>):  // 2 preds: ^bb1, ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    %27 = llvm.mlir.undef : !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %28 = llvm.insertvalue %25, %27[0] : !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %29 = llvm.insertvalue %3, %28[1] : !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    %30 = llvm.insertvalue %26, %29[2] : !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
    llvm.return %30 : !llvm.struct<(struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>, i1, struct<(i64, i64, i64, i64)>)>
  }
  llvm.func @iterators.scan_columnar_batch.open.0(%arg0: !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>) -> !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)> attributes {sym_visibility = "private"} {
    %0 = llvm.mlir.constant(0 : i64) : i64
    %1 = llvm.insertvalue %0, %arg0[0 : index] : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
    llvm.return %1 : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
  }
  llvm.func @query(%arg0: !llvm.struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>) attributes {llvm.emit_c_interface, sym_visibility = "private"} {
    %0 = llvm.mlir.undef : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
    %1 = llvm.insertvalue %arg0, %0[1 : index] : !llvm.struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>
    %2 = llvm.mlir.undef : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    %3 = llvm.insertvalue %1, %2[0 : index] : !llvm.struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>
    %4 = llvm.mlir.undef : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    %5 = llvm.insertvalue %3, %4[0 : index] : !llvm.struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>
    %6 = llvm.mlir.undef : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    %7 = llvm.insertvalue %5, %6[0 : index] : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    %8 = llvm.call @iterators.reduce.open.0(%7) : (!llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>) -> !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    llvm.br ^bb1(%8 : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>)
  ^bb1(%9: !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>):  // 2 preds: ^bb0, ^bb2
    %10 = llvm.call @iterators.reduce.next.0(%9) : (!llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>) -> !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
    %11 = llvm.extractvalue %10[0] : !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
    %12 = llvm.extractvalue %10[1] : !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
    %13 = llvm.extractvalue %10[2] : !llvm.struct<(struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, i1, struct<(i64)>)>
    llvm.cond_br %12, ^bb2(%11, %13 : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, !llvm.struct<(i64)>), ^bb3
  ^bb2(%14: !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>, %15: !llvm.struct<(i64)>):  // pred: ^bb1
    %16 = llvm.mlir.addressof @frmt_spec.anonymous_tuple : !llvm.ptr<array<8 x i8>>
    %17 = llvm.mlir.constant(0 : i64) : i64
    %18 = llvm.getelementptr %16[%17, %17] : (!llvm.ptr<array<8 x i8>>, i64, i64) -> !llvm.ptr<i8>
    %19 = llvm.extractvalue %15[0 : index] : !llvm.struct<(i64)>
    %20 = llvm.zext %19 : i64 to i64
    %21 = llvm.call @printf(%18, %20) : (!llvm.ptr<i8>, i64) -> i32
    llvm.br ^bb1(%14 : !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>)
  ^bb3:  // pred: ^bb1
    %22 = llvm.call @iterators.reduce.close.0(%11) : (!llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>) -> !llvm.struct<"iterators.reduce_state", (struct<"iterators.map_state", (struct<"iterators.filter_state", (struct<"iterators.scan_columnar_batch_state", (i64, struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>)>)>)>)>
    llvm.return
  }
  llvm.func @_mlir_ciface_query(%arg0: !llvm.struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>) attributes {llvm.emit_c_interface, sym_visibility = "public"} {
    llvm.call @query(%arg0) : (!llvm.struct<(i64, ptr<i64>, ptr<i64>, ptr<i64>, ptr<i64>)>) -> ()
    llvm.return
  }
  llvm.func @s0(%arg0: !llvm.struct<(i64, i64, i64, i64)>) -> i1 attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %1 = llvm.mlir.constant(757382400 : i64) : i64
    %2 = llvm.icmp "sge" %0, %1 : i64
    %3 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %4 = llvm.mlir.constant(788918400 : i64) : i64
    %5 = llvm.icmp "slt" %3, %4 : i64
    %6 = llvm.extractvalue %arg0[1 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %7 = llvm.mlir.constant(5 : i64) : i64
    %8 = llvm.icmp "sge" %6, %7 : i64
    %9 = llvm.extractvalue %arg0[1 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %10 = llvm.mlir.constant(7 : i64) : i64
    %11 = llvm.icmp "sle" %9, %10 : i64
    %12 = llvm.extractvalue %arg0[2 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %13 = llvm.mlir.constant(24 : i64) : i64
    %14 = llvm.icmp "slt" %12, %13 : i64
    %15 = llvm.and %14, %11  : i1
    %16 = llvm.and %15, %8  : i1
    %17 = llvm.and %16, %5  : i1
    %18 = llvm.and %17, %2  : i1
    llvm.return %18 : i1
  }
  llvm.func @m0(%arg0: !llvm.struct<(i64, i64, i64, i64)>) -> !llvm.struct<(i64)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[3 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %1 = llvm.extractvalue %arg0[1 : index] : !llvm.struct<(i64, i64, i64, i64)>
    %2 = llvm.mul %0, %1  : i64
    %3 = llvm.mlir.undef : !llvm.struct<(i64)>
    %4 = llvm.insertvalue %2, %3[0 : index] : !llvm.struct<(i64)>
    llvm.return %4 : !llvm.struct<(i64)>
  }
  llvm.func @sum_struct(%arg0: !llvm.struct<(i64)>, %arg1: !llvm.struct<(i64)>) -> !llvm.struct<(i64)> attributes {sym_visibility = "private"} {
    %0 = llvm.extractvalue %arg0[0 : index] : !llvm.struct<(i64)>
    %1 = llvm.extractvalue %arg1[0 : index] : !llvm.struct<(i64)>
    %2 = llvm.add %0, %1  : i64
    %3 = llvm.insertvalue %2, %arg0[0 : index] : !llvm.struct<(i64)>
    llvm.return %3 : !llvm.struct<(i64)>
  }
}
