; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

%iterators.reduce_state = type { %iterators.map_state }
%iterators.map_state = type { %iterators.filter_state }
%iterators.filter_state = type { %iterators.scan_columnar_batch_state }
%iterators.scan_columnar_batch_state = type { i64, { i64, i64*, i64*, i64*, i64* } }

@frmt_spec.anonymous_tuple = internal constant [8 x i8] c"(%lli)\0A\00"

declare i8* @malloc(i64)

declare void @free(i8*)

declare i32 @printf2(i8*, ...)

define %iterators.reduce_state @iterators.reduce.close.0(%iterators.reduce_state %0) !dbg !3 {
  %2 = extractvalue %iterators.reduce_state %0, 0, !dbg !7
  %3 = call %iterators.map_state @iterators.map.close.0(%iterators.map_state %2), !dbg !9
  %4 = insertvalue %iterators.reduce_state %0, %iterators.map_state %3, 0, !dbg !10
  ret %iterators.reduce_state %4, !dbg !11
}

define { %iterators.reduce_state, i1, { i64 } } @iterators.reduce.next.0(%iterators.reduce_state %0) !dbg !12 {
  %2 = extractvalue %iterators.reduce_state %0, 0, !dbg !13
  %3 = call { %iterators.map_state, i1, { i64 } } @iterators.map.next.0(%iterators.map_state %2), !dbg !15
  %4 = extractvalue { %iterators.map_state, i1, { i64 } } %3, 0, !dbg !16
  %5 = extractvalue { %iterators.map_state, i1, { i64 } } %3, 1, !dbg !17
  %6 = extractvalue { %iterators.map_state, i1, { i64 } } %3, 2, !dbg !18
  br i1 %5, label %7, label %21, !dbg !19

7:                                                ; preds = %1
  br label %8, !dbg !20

8:                                                ; preds = %15, %7
  %9 = phi %iterators.map_state [ %16, %15 ], [ %4, %7 ]
  %10 = phi { i64 } [ %19, %15 ], [ %6, %7 ]
  %11 = call { %iterators.map_state, i1, { i64 } } @iterators.map.next.0(%iterators.map_state %9), !dbg !21
  %12 = extractvalue { %iterators.map_state, i1, { i64 } } %11, 0, !dbg !22
  %13 = extractvalue { %iterators.map_state, i1, { i64 } } %11, 1, !dbg !23
  %14 = extractvalue { %iterators.map_state, i1, { i64 } } %11, 2, !dbg !24
  br i1 %13, label %15, label %20, !dbg !25

15:                                               ; preds = %8
  %16 = phi %iterators.map_state [ %12, %8 ]
  %17 = phi { i64 } [ %10, %8 ]
  %18 = phi { i64 } [ %14, %8 ]
  %19 = call { i64 } @sum_struct({ i64 } %17, { i64 } %18), !dbg !26
  br label %8, !dbg !27

20:                                               ; preds = %8
  br label %22, !dbg !28

21:                                               ; preds = %1
  br label %22, !dbg !29

22:                                               ; preds = %20, %21
  %23 = phi %iterators.map_state [ %4, %21 ], [ %12, %20 ]
  %24 = phi i1 [ %5, %21 ], [ true, %20 ]
  %25 = phi { i64 } [ %6, %21 ], [ %10, %20 ]
  br label %26, !dbg !30

26:                                               ; preds = %22
  %27 = insertvalue %iterators.reduce_state %0, %iterators.map_state %23, 0, !dbg !31
  %28 = insertvalue { %iterators.reduce_state, i1, { i64 } } undef, %iterators.reduce_state %27, 0, !dbg !32
  %29 = insertvalue { %iterators.reduce_state, i1, { i64 } } %28, i1 %24, 1, !dbg !33
  %30 = insertvalue { %iterators.reduce_state, i1, { i64 } } %29, { i64 } %25, 2, !dbg !34
  ret { %iterators.reduce_state, i1, { i64 } } %30, !dbg !35
}

define %iterators.reduce_state @iterators.reduce.open.0(%iterators.reduce_state %0) !dbg !36 {
  %2 = extractvalue %iterators.reduce_state %0, 0, !dbg !37
  %3 = call %iterators.map_state @iterators.map.open.0(%iterators.map_state %2), !dbg !39
  %4 = insertvalue %iterators.reduce_state %0, %iterators.map_state %3, 0, !dbg !40
  ret %iterators.reduce_state %4, !dbg !41
}

define %iterators.map_state @iterators.map.close.0(%iterators.map_state %0) !dbg !42 {
  %2 = extractvalue %iterators.map_state %0, 0, !dbg !43
  %3 = call %iterators.filter_state @iterators.filter.close.0(%iterators.filter_state %2), !dbg !45
  %4 = insertvalue %iterators.map_state %0, %iterators.filter_state %3, 0, !dbg !46
  ret %iterators.map_state %4, !dbg !47
}

define { %iterators.map_state, i1, { i64 } } @iterators.map.next.0(%iterators.map_state %0) !dbg !48 {
  %2 = extractvalue %iterators.map_state %0, 0, !dbg !49
  %3 = call { %iterators.filter_state, i1, { i64, i64, i64, i64 } } @iterators.filter.next.0(%iterators.filter_state %2), !dbg !51
  %4 = extractvalue { %iterators.filter_state, i1, { i64, i64, i64, i64 } } %3, 0, !dbg !52
  %5 = extractvalue { %iterators.filter_state, i1, { i64, i64, i64, i64 } } %3, 1, !dbg !53
  %6 = extractvalue { %iterators.filter_state, i1, { i64, i64, i64, i64 } } %3, 2, !dbg !54
  br i1 %5, label %7, label %9, !dbg !55

7:                                                ; preds = %1
  %8 = call { i64 } @m0({ i64, i64, i64, i64 } %6), !dbg !56
  br label %10, !dbg !57

9:                                                ; preds = %1
  br label %10, !dbg !58

10:                                               ; preds = %7, %9
  %11 = phi { i64 } [ undef, %9 ], [ %8, %7 ]
  br label %12, !dbg !59

12:                                               ; preds = %10
  %13 = insertvalue %iterators.map_state %0, %iterators.filter_state %4, 0, !dbg !60
  %14 = insertvalue { %iterators.map_state, i1, { i64 } } undef, %iterators.map_state %13, 0, !dbg !61
  %15 = insertvalue { %iterators.map_state, i1, { i64 } } %14, i1 %5, 1, !dbg !62
  %16 = insertvalue { %iterators.map_state, i1, { i64 } } %15, { i64 } %11, 2, !dbg !63
  ret { %iterators.map_state, i1, { i64 } } %16, !dbg !64
}

define %iterators.map_state @iterators.map.open.0(%iterators.map_state %0) !dbg !65 {
  %2 = extractvalue %iterators.map_state %0, 0, !dbg !66
  %3 = call %iterators.filter_state @iterators.filter.open.0(%iterators.filter_state %2), !dbg !68
  %4 = insertvalue %iterators.map_state %0, %iterators.filter_state %3, 0, !dbg !69
  ret %iterators.map_state %4, !dbg !70
}

define %iterators.filter_state @iterators.filter.close.0(%iterators.filter_state %0) !dbg !71 {
  %2 = extractvalue %iterators.filter_state %0, 0, !dbg !72
  %3 = call %iterators.scan_columnar_batch_state @iterators.scan_columnar_batch.close.0(%iterators.scan_columnar_batch_state %2), !dbg !74
  %4 = insertvalue %iterators.filter_state %0, %iterators.scan_columnar_batch_state %3, 0, !dbg !75
  ret %iterators.filter_state %4, !dbg !76
}

define { %iterators.filter_state, i1, { i64, i64, i64, i64 } } @iterators.filter.next.0(%iterators.filter_state %0) !dbg !77 {
  %2 = extractvalue %iterators.filter_state %0, 0, !dbg !78
  br label %3, !dbg !80

3:                                                ; preds = %17, %1
  %4 = phi %iterators.scan_columnar_batch_state [ %18, %17 ], [ %2, %1 ]
  %5 = call { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } @iterators.scan_columnar_batch.next.0(%iterators.scan_columnar_batch_state %4), !dbg !81
  %6 = extractvalue { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } %5, 0, !dbg !82
  %7 = extractvalue { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } %5, 1, !dbg !83
  %8 = extractvalue { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } %5, 2, !dbg !84
  br i1 %7, label %9, label %11, !dbg !85

9:                                                ; preds = %3
  %10 = call i1 @s0({ i64, i64, i64, i64 } %8), !dbg !86
  br label %12, !dbg !87

11:                                               ; preds = %3
  br label %12, !dbg !88

12:                                               ; preds = %9, %11
  %13 = phi i1 [ %7, %11 ], [ %10, %9 ]
  br label %14, !dbg !89

14:                                               ; preds = %12
  %15 = xor i1 %13, true, !dbg !90
  %16 = and i1 %7, %15, !dbg !91
  br i1 %16, label %17, label %21, !dbg !92

17:                                               ; preds = %14
  %18 = phi %iterators.scan_columnar_batch_state [ %6, %14 ]
  %19 = phi i1 [ %7, %14 ]
  %20 = phi { i64, i64, i64, i64 } [ %8, %14 ]
  br label %3, !dbg !93

21:                                               ; preds = %14
  %22 = insertvalue %iterators.filter_state %0, %iterators.scan_columnar_batch_state %6, 0, !dbg !94
  %23 = insertvalue { %iterators.filter_state, i1, { i64, i64, i64, i64 } } undef, %iterators.filter_state %22, 0, !dbg !95
  %24 = insertvalue { %iterators.filter_state, i1, { i64, i64, i64, i64 } } %23, i1 %7, 1, !dbg !96
  %25 = insertvalue { %iterators.filter_state, i1, { i64, i64, i64, i64 } } %24, { i64, i64, i64, i64 } %8, 2, !dbg !97
  ret { %iterators.filter_state, i1, { i64, i64, i64, i64 } } %25, !dbg !98
}

define %iterators.filter_state @iterators.filter.open.0(%iterators.filter_state %0) !dbg !99 {
  %2 = extractvalue %iterators.filter_state %0, 0, !dbg !100
  %3 = call %iterators.scan_columnar_batch_state @iterators.scan_columnar_batch.open.0(%iterators.scan_columnar_batch_state %2), !dbg !102
  %4 = insertvalue %iterators.filter_state %0, %iterators.scan_columnar_batch_state %3, 0, !dbg !103
  ret %iterators.filter_state %4, !dbg !104
}

define %iterators.scan_columnar_batch_state @iterators.scan_columnar_batch.close.0(%iterators.scan_columnar_batch_state %0) !dbg !105 {
  ret %iterators.scan_columnar_batch_state %0, !dbg !106
}

define { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } @iterators.scan_columnar_batch.next.0(%iterators.scan_columnar_batch_state %0) !dbg !108 {
  %2 = extractvalue %iterators.scan_columnar_batch_state %0, 0, !dbg !109
  %3 = extractvalue %iterators.scan_columnar_batch_state %0, 1, !dbg !111
  %4 = extractvalue { i64, i64*, i64*, i64*, i64* } %3, 0, !dbg !112
  %5 = icmp slt i64 %2, %4, !dbg !113
  br i1 %5, label %6, label %25, !dbg !114

6:                                                ; preds = %1
  %7 = add i64 %2, 1, !dbg !115
  %8 = insertvalue %iterators.scan_columnar_batch_state %0, i64 %7, 0, !dbg !116
  %9 = extractvalue { i64, i64*, i64*, i64*, i64* } %3, 1, !dbg !117
  %10 = getelementptr i64, i64* %9, i64 %2, !dbg !118
  %11 = load i64, i64* %10, align 4, !dbg !119
  %12 = insertvalue { i64, i64, i64, i64 } undef, i64 %11, 0, !dbg !120
  %13 = extractvalue { i64, i64*, i64*, i64*, i64* } %3, 2, !dbg !121
  %14 = getelementptr i64, i64* %13, i64 %2, !dbg !122
  %15 = load i64, i64* %14, align 4, !dbg !123
  %16 = insertvalue { i64, i64, i64, i64 } %12, i64 %15, 1, !dbg !124
  %17 = extractvalue { i64, i64*, i64*, i64*, i64* } %3, 3, !dbg !125
  %18 = getelementptr i64, i64* %17, i64 %2, !dbg !126
  %19 = load i64, i64* %18, align 4, !dbg !127
  %20 = insertvalue { i64, i64, i64, i64 } %16, i64 %19, 2, !dbg !128
  %21 = extractvalue { i64, i64*, i64*, i64*, i64* } %3, 4, !dbg !129
  %22 = getelementptr i64, i64* %21, i64 %2, !dbg !130
  %23 = load i64, i64* %22, align 4, !dbg !131
  %24 = insertvalue { i64, i64, i64, i64 } %20, i64 %23, 3, !dbg !132
  br label %26, !dbg !133

25:                                               ; preds = %1
  br label %26, !dbg !134

26:                                               ; preds = %6, %25
  %27 = phi %iterators.scan_columnar_batch_state [ %0, %25 ], [ %8, %6 ]
  %28 = phi { i64, i64, i64, i64 } [ undef, %25 ], [ %24, %6 ]
  br label %29, !dbg !135

29:                                               ; preds = %26
  %30 = insertvalue { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } undef, %iterators.scan_columnar_batch_state %27, 0, !dbg !136
  %31 = insertvalue { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } %30, i1 %5, 1, !dbg !137
  %32 = insertvalue { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } %31, { i64, i64, i64, i64 } %28, 2, !dbg !138
  ret { %iterators.scan_columnar_batch_state, i1, { i64, i64, i64, i64 } } %32, !dbg !139
}

define %iterators.scan_columnar_batch_state @iterators.scan_columnar_batch.open.0(%iterators.scan_columnar_batch_state %0) !dbg !140 {
  %2 = insertvalue %iterators.scan_columnar_batch_state %0, i64 0, 0, !dbg !141
  ret %iterators.scan_columnar_batch_state %2, !dbg !143
}

define void @query({ i64, i64*, i64*, i64*, i64* } %0) !dbg !144 {
  %2 = insertvalue %iterators.scan_columnar_batch_state undef, { i64, i64*, i64*, i64*, i64* } %0, 1, !dbg !145
  %3 = insertvalue %iterators.filter_state undef, %iterators.scan_columnar_batch_state %2, 0, !dbg !147
  %4 = insertvalue %iterators.map_state undef, %iterators.filter_state %3, 0, !dbg !148
  %5 = insertvalue %iterators.reduce_state undef, %iterators.map_state %4, 0, !dbg !149
  %6 = call %iterators.reduce_state @iterators.reduce.open.0(%iterators.reduce_state %5), !dbg !150
  br label %7, !dbg !151

7:                                                ; preds = %13, %1
  %8 = phi %iterators.reduce_state [ %14, %13 ], [ %6, %1 ]
  %9 = call { %iterators.reduce_state, i1, { i64 } } @iterators.reduce.next.0(%iterators.reduce_state %8), !dbg !152
  %10 = extractvalue { %iterators.reduce_state, i1, { i64 } } %9, 0, !dbg !153
  %11 = extractvalue { %iterators.reduce_state, i1, { i64 } } %9, 1, !dbg !154
  %12 = extractvalue { %iterators.reduce_state, i1, { i64 } } %9, 2, !dbg !155
  br i1 %11, label %13, label %18, !dbg !156

13:                                               ; preds = %7
  %14 = phi %iterators.reduce_state [ %10, %7 ]
  %15 = phi { i64 } [ %12, %7 ]
  %16 = extractvalue { i64 } %15, 0, !dbg !157
  %17 = call i32 (i8*, ...) @printf2(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @frmt_spec.anonymous_tuple, i64 0, i64 0), i64 %16), !dbg !158
  br label %7, !dbg !159

18:                                               ; preds = %7
  %19 = call %iterators.reduce_state @iterators.reduce.close.0(%iterators.reduce_state %10), !dbg !160
  ret void, !dbg !161
}

define void @_mlir_ciface_query({ i64, i64*, i64*, i64*, i64* } %0) !dbg !162 {
  call void @query({ i64, i64*, i64*, i64*, i64* } %0), !dbg !163
  ret void, !dbg !165
}

define i1 @s0({ i64, i64, i64, i64 } %0) !dbg !166 {
  %2 = extractvalue { i64, i64, i64, i64 } %0, 0, !dbg !167
  %3 = icmp sge i64 %2, 757382400, !dbg !169
  %4 = extractvalue { i64, i64, i64, i64 } %0, 0, !dbg !170
  %5 = icmp slt i64 %4, 788918400, !dbg !171
  %6 = extractvalue { i64, i64, i64, i64 } %0, 1, !dbg !172
  %7 = icmp sge i64 %6, 5, !dbg !173
  %8 = extractvalue { i64, i64, i64, i64 } %0, 1, !dbg !174
  %9 = icmp sle i64 %8, 7, !dbg !175
  %10 = extractvalue { i64, i64, i64, i64 } %0, 2, !dbg !176
  %11 = icmp slt i64 %10, 24, !dbg !177
  %12 = and i1 %11, %9, !dbg !178
  %13 = and i1 %12, %7, !dbg !179
  %14 = and i1 %13, %5, !dbg !180
  %15 = and i1 %14, %3, !dbg !181
  ret i1 %15, !dbg !182
}

define { i64 } @m0({ i64, i64, i64, i64 } %0) !dbg !183 {
  %2 = extractvalue { i64, i64, i64, i64 } %0, 3, !dbg !184
  %3 = extractvalue { i64, i64, i64, i64 } %0, 1, !dbg !186
  %4 = mul i64 %2, %3, !dbg !187
  %5 = insertvalue { i64 } undef, i64 %4, 0, !dbg !188
  ret { i64 } %5, !dbg !189
}

define { i64 } @sum_struct({ i64 } %0, { i64 } %1) !dbg !190 {
  %3 = extractvalue { i64 } %0, 0, !dbg !191
  %4 = extractvalue { i64 } %1, 0, !dbg !193
  %5 = add i64 %3, %4, !dbg !194
  %6 = insertvalue { i64 } %0, i64 %5, 0, !dbg !195
  ret { i64 } %6, !dbg !196
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "mlir", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "LLVMDialectModule", directory: "/")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DISubprogram(name: "iterators.reduce.close.0", linkageName: "iterators.reduce.close.0", scope: null, file: !4, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!4 = !DIFile(filename: "q6.mlir", directory: "/home/michel/MasterThesis/iree-llvm-sandbox")
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 10, scope: !8)
!8 = !DILexicalBlockFile(scope: !3, file: !4, discriminator: 0)
!9 = !DILocation(line: 6, column: 10, scope: !8)
!10 = !DILocation(line: 7, column: 10, scope: !8)
!11 = !DILocation(line: 8, column: 5, scope: !8)
!12 = distinct !DISubprogram(name: "iterators.reduce.next.0", linkageName: "iterators.reduce.next.0", scope: null, file: !4, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!13 = !DILocation(line: 11, column: 10, scope: !14)
!14 = !DILexicalBlockFile(scope: !12, file: !4, discriminator: 0)
!15 = !DILocation(line: 12, column: 10, scope: !14)
!16 = !DILocation(line: 13, column: 10, scope: !14)
!17 = !DILocation(line: 14, column: 10, scope: !14)
!18 = !DILocation(line: 15, column: 10, scope: !14)
!19 = !DILocation(line: 16, column: 5, scope: !14)
!20 = !DILocation(line: 18, column: 5, scope: !14)
!21 = !DILocation(line: 20, column: 10, scope: !14)
!22 = !DILocation(line: 21, column: 10, scope: !14)
!23 = !DILocation(line: 22, column: 10, scope: !14)
!24 = !DILocation(line: 23, column: 11, scope: !14)
!25 = !DILocation(line: 24, column: 5, scope: !14)
!26 = !DILocation(line: 26, column: 11, scope: !14)
!27 = !DILocation(line: 27, column: 5, scope: !14)
!28 = !DILocation(line: 30, column: 5, scope: !14)
!29 = !DILocation(line: 32, column: 5, scope: !14)
!30 = !DILocation(line: 34, column: 5, scope: !14)
!31 = !DILocation(line: 36, column: 11, scope: !14)
!32 = !DILocation(line: 38, column: 11, scope: !14)
!33 = !DILocation(line: 39, column: 11, scope: !14)
!34 = !DILocation(line: 40, column: 11, scope: !14)
!35 = !DILocation(line: 41, column: 5, scope: !14)
!36 = distinct !DISubprogram(name: "iterators.reduce.open.0", linkageName: "iterators.reduce.open.0", scope: null, file: !4, line: 43, type: !5, scopeLine: 43, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!37 = !DILocation(line: 44, column: 10, scope: !38)
!38 = !DILexicalBlockFile(scope: !36, file: !4, discriminator: 0)
!39 = !DILocation(line: 45, column: 10, scope: !38)
!40 = !DILocation(line: 46, column: 10, scope: !38)
!41 = !DILocation(line: 47, column: 5, scope: !38)
!42 = distinct !DISubprogram(name: "iterators.map.close.0", linkageName: "iterators.map.close.0", scope: null, file: !4, line: 49, type: !5, scopeLine: 49, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!43 = !DILocation(line: 50, column: 10, scope: !44)
!44 = !DILexicalBlockFile(scope: !42, file: !4, discriminator: 0)
!45 = !DILocation(line: 51, column: 10, scope: !44)
!46 = !DILocation(line: 52, column: 10, scope: !44)
!47 = !DILocation(line: 53, column: 5, scope: !44)
!48 = distinct !DISubprogram(name: "iterators.map.next.0", linkageName: "iterators.map.next.0", scope: null, file: !4, line: 55, type: !5, scopeLine: 55, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!49 = !DILocation(line: 56, column: 10, scope: !50)
!50 = !DILexicalBlockFile(scope: !48, file: !4, discriminator: 0)
!51 = !DILocation(line: 57, column: 10, scope: !50)
!52 = !DILocation(line: 58, column: 10, scope: !50)
!53 = !DILocation(line: 59, column: 10, scope: !50)
!54 = !DILocation(line: 60, column: 10, scope: !50)
!55 = !DILocation(line: 61, column: 5, scope: !50)
!56 = !DILocation(line: 63, column: 10, scope: !50)
!57 = !DILocation(line: 64, column: 5, scope: !50)
!58 = !DILocation(line: 67, column: 5, scope: !50)
!59 = !DILocation(line: 69, column: 5, scope: !50)
!60 = !DILocation(line: 71, column: 10, scope: !50)
!61 = !DILocation(line: 73, column: 11, scope: !50)
!62 = !DILocation(line: 74, column: 11, scope: !50)
!63 = !DILocation(line: 75, column: 11, scope: !50)
!64 = !DILocation(line: 76, column: 5, scope: !50)
!65 = distinct !DISubprogram(name: "iterators.map.open.0", linkageName: "iterators.map.open.0", scope: null, file: !4, line: 78, type: !5, scopeLine: 78, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!66 = !DILocation(line: 79, column: 10, scope: !67)
!67 = !DILexicalBlockFile(scope: !65, file: !4, discriminator: 0)
!68 = !DILocation(line: 80, column: 10, scope: !67)
!69 = !DILocation(line: 81, column: 10, scope: !67)
!70 = !DILocation(line: 82, column: 5, scope: !67)
!71 = distinct !DISubprogram(name: "iterators.filter.close.0", linkageName: "iterators.filter.close.0", scope: null, file: !4, line: 84, type: !5, scopeLine: 84, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!72 = !DILocation(line: 85, column: 10, scope: !73)
!73 = !DILexicalBlockFile(scope: !71, file: !4, discriminator: 0)
!74 = !DILocation(line: 86, column: 10, scope: !73)
!75 = !DILocation(line: 87, column: 10, scope: !73)
!76 = !DILocation(line: 88, column: 5, scope: !73)
!77 = distinct !DISubprogram(name: "iterators.filter.next.0", linkageName: "iterators.filter.next.0", scope: null, file: !4, line: 90, type: !5, scopeLine: 90, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!78 = !DILocation(line: 91, column: 10, scope: !79)
!79 = !DILexicalBlockFile(scope: !77, file: !4, discriminator: 0)
!80 = !DILocation(line: 92, column: 5, scope: !79)
!81 = !DILocation(line: 94, column: 10, scope: !79)
!82 = !DILocation(line: 95, column: 10, scope: !79)
!83 = !DILocation(line: 96, column: 10, scope: !79)
!84 = !DILocation(line: 97, column: 10, scope: !79)
!85 = !DILocation(line: 98, column: 5, scope: !79)
!86 = !DILocation(line: 100, column: 10, scope: !79)
!87 = !DILocation(line: 101, column: 5, scope: !79)
!88 = !DILocation(line: 103, column: 5, scope: !79)
!89 = !DILocation(line: 105, column: 5, scope: !79)
!90 = !DILocation(line: 108, column: 10, scope: !79)
!91 = !DILocation(line: 109, column: 11, scope: !79)
!92 = !DILocation(line: 110, column: 5, scope: !79)
!93 = !DILocation(line: 112, column: 5, scope: !79)
!94 = !DILocation(line: 114, column: 11, scope: !79)
!95 = !DILocation(line: 116, column: 11, scope: !79)
!96 = !DILocation(line: 117, column: 11, scope: !79)
!97 = !DILocation(line: 118, column: 11, scope: !79)
!98 = !DILocation(line: 119, column: 5, scope: !79)
!99 = distinct !DISubprogram(name: "iterators.filter.open.0", linkageName: "iterators.filter.open.0", scope: null, file: !4, line: 121, type: !5, scopeLine: 121, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!100 = !DILocation(line: 122, column: 10, scope: !101)
!101 = !DILexicalBlockFile(scope: !99, file: !4, discriminator: 0)
!102 = !DILocation(line: 123, column: 10, scope: !101)
!103 = !DILocation(line: 124, column: 10, scope: !101)
!104 = !DILocation(line: 125, column: 5, scope: !101)
!105 = distinct !DISubprogram(name: "iterators.scan_columnar_batch.close.0", linkageName: "iterators.scan_columnar_batch.close.0", scope: null, file: !4, line: 127, type: !5, scopeLine: 127, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!106 = !DILocation(line: 128, column: 5, scope: !107)
!107 = !DILexicalBlockFile(scope: !105, file: !4, discriminator: 0)
!108 = distinct !DISubprogram(name: "iterators.scan_columnar_batch.next.0", linkageName: "iterators.scan_columnar_batch.next.0", scope: null, file: !4, line: 130, type: !5, scopeLine: 130, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!109 = !DILocation(line: 131, column: 10, scope: !110)
!110 = !DILexicalBlockFile(scope: !108, file: !4, discriminator: 0)
!111 = !DILocation(line: 132, column: 10, scope: !110)
!112 = !DILocation(line: 133, column: 10, scope: !110)
!113 = !DILocation(line: 134, column: 10, scope: !110)
!114 = !DILocation(line: 135, column: 5, scope: !110)
!115 = !DILocation(line: 138, column: 10, scope: !110)
!116 = !DILocation(line: 139, column: 10, scope: !110)
!117 = !DILocation(line: 141, column: 10, scope: !110)
!118 = !DILocation(line: 142, column: 10, scope: !110)
!119 = !DILocation(line: 143, column: 11, scope: !110)
!120 = !DILocation(line: 144, column: 11, scope: !110)
!121 = !DILocation(line: 145, column: 11, scope: !110)
!122 = !DILocation(line: 146, column: 11, scope: !110)
!123 = !DILocation(line: 147, column: 11, scope: !110)
!124 = !DILocation(line: 148, column: 11, scope: !110)
!125 = !DILocation(line: 149, column: 11, scope: !110)
!126 = !DILocation(line: 150, column: 11, scope: !110)
!127 = !DILocation(line: 151, column: 11, scope: !110)
!128 = !DILocation(line: 152, column: 11, scope: !110)
!129 = !DILocation(line: 153, column: 11, scope: !110)
!130 = !DILocation(line: 154, column: 11, scope: !110)
!131 = !DILocation(line: 155, column: 11, scope: !110)
!132 = !DILocation(line: 156, column: 11, scope: !110)
!133 = !DILocation(line: 157, column: 5, scope: !110)
!134 = !DILocation(line: 160, column: 5, scope: !110)
!135 = !DILocation(line: 162, column: 5, scope: !110)
!136 = !DILocation(line: 165, column: 11, scope: !110)
!137 = !DILocation(line: 166, column: 11, scope: !110)
!138 = !DILocation(line: 167, column: 11, scope: !110)
!139 = !DILocation(line: 168, column: 5, scope: !110)
!140 = distinct !DISubprogram(name: "iterators.scan_columnar_batch.open.0", linkageName: "iterators.scan_columnar_batch.open.0", scope: null, file: !4, line: 170, type: !5, scopeLine: 170, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!141 = !DILocation(line: 172, column: 10, scope: !142)
!142 = !DILexicalBlockFile(scope: !140, file: !4, discriminator: 0)
!143 = !DILocation(line: 173, column: 5, scope: !142)
!144 = distinct !DISubprogram(name: "query", linkageName: "query", scope: null, file: !4, line: 175, type: !5, scopeLine: 175, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!145 = !DILocation(line: 177, column: 10, scope: !146)
!146 = !DILexicalBlockFile(scope: !144, file: !4, discriminator: 0)
!147 = !DILocation(line: 179, column: 10, scope: !146)
!148 = !DILocation(line: 181, column: 10, scope: !146)
!149 = !DILocation(line: 183, column: 10, scope: !146)
!150 = !DILocation(line: 184, column: 10, scope: !146)
!151 = !DILocation(line: 185, column: 5, scope: !146)
!152 = !DILocation(line: 187, column: 11, scope: !146)
!153 = !DILocation(line: 188, column: 11, scope: !146)
!154 = !DILocation(line: 189, column: 11, scope: !146)
!155 = !DILocation(line: 190, column: 11, scope: !146)
!156 = !DILocation(line: 191, column: 5, scope: !146)
!157 = !DILocation(line: 196, column: 11, scope: !146)
!158 = !DILocation(line: 198, column: 11, scope: !146)
!159 = !DILocation(line: 199, column: 5, scope: !146)
!160 = !DILocation(line: 201, column: 11, scope: !146)
!161 = !DILocation(line: 202, column: 5, scope: !146)
!162 = distinct !DISubprogram(name: "_mlir_ciface_query", linkageName: "_mlir_ciface_query", scope: null, file: !4, line: 204, type: !5, scopeLine: 204, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!163 = !DILocation(line: 205, column: 5, scope: !164)
!164 = !DILexicalBlockFile(scope: !162, file: !4, discriminator: 0)
!165 = !DILocation(line: 206, column: 5, scope: !164)
!166 = distinct !DISubprogram(name: "s0", linkageName: "s0", scope: null, file: !4, line: 208, type: !5, scopeLine: 208, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!167 = !DILocation(line: 209, column: 10, scope: !168)
!168 = !DILexicalBlockFile(scope: !166, file: !4, discriminator: 0)
!169 = !DILocation(line: 211, column: 10, scope: !168)
!170 = !DILocation(line: 212, column: 10, scope: !168)
!171 = !DILocation(line: 214, column: 10, scope: !168)
!172 = !DILocation(line: 215, column: 10, scope: !168)
!173 = !DILocation(line: 217, column: 10, scope: !168)
!174 = !DILocation(line: 218, column: 10, scope: !168)
!175 = !DILocation(line: 220, column: 11, scope: !168)
!176 = !DILocation(line: 221, column: 11, scope: !168)
!177 = !DILocation(line: 223, column: 11, scope: !168)
!178 = !DILocation(line: 224, column: 11, scope: !168)
!179 = !DILocation(line: 225, column: 11, scope: !168)
!180 = !DILocation(line: 226, column: 11, scope: !168)
!181 = !DILocation(line: 227, column: 11, scope: !168)
!182 = !DILocation(line: 228, column: 5, scope: !168)
!183 = distinct !DISubprogram(name: "m0", linkageName: "m0", scope: null, file: !4, line: 230, type: !5, scopeLine: 230, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!184 = !DILocation(line: 231, column: 10, scope: !185)
!185 = !DILexicalBlockFile(scope: !183, file: !4, discriminator: 0)
!186 = !DILocation(line: 232, column: 10, scope: !185)
!187 = !DILocation(line: 233, column: 10, scope: !185)
!188 = !DILocation(line: 235, column: 10, scope: !185)
!189 = !DILocation(line: 236, column: 5, scope: !185)
!190 = distinct !DISubprogram(name: "sum_struct", linkageName: "sum_struct", scope: null, file: !4, line: 238, type: !5, scopeLine: 238, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !6)
!191 = !DILocation(line: 239, column: 10, scope: !192)
!192 = !DILexicalBlockFile(scope: !190, file: !4, discriminator: 0)
!193 = !DILocation(line: 240, column: 10, scope: !192)
!194 = !DILocation(line: 241, column: 10, scope: !192)
!195 = !DILocation(line: 242, column: 10, scope: !192)
!196 = !DILocation(line: 243, column: 5, scope: !192)
