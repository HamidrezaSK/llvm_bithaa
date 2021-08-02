; ModuleID = 'inputs/vector_subtraction_kernel.c'
source_filename = "inputs/vector_subtraction_kernel.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @vector_subtraction_kernel(i32* nocapture %0, i32* nocapture readonly %1, i32* nocapture readonly %2) local_unnamed_addr #0 {
  %4 = load i32, i32* %1, align 4, !tbaa !3
  %5 = load i32, i32* %2, align 4, !tbaa !3
  %6 = sub i32 %4, %5
  store i32 %6, i32* %0, align 4, !tbaa !3
  %7 = getelementptr inbounds i32, i32* %1, i64 1
  %8 = load i32, i32* %7, align 4, !tbaa !3
  %9 = getelementptr inbounds i32, i32* %2, i64 1
  %10 = load i32, i32* %9, align 4, !tbaa !3
  %11 = sub i32 %8, %10
  %12 = getelementptr inbounds i32, i32* %0, i64 1
  store i32 %11, i32* %12, align 4, !tbaa !3
  %13 = getelementptr inbounds i32, i32* %1, i64 2
  %14 = load i32, i32* %13, align 4, !tbaa !3
  %15 = getelementptr inbounds i32, i32* %2, i64 2
  %16 = load i32, i32* %15, align 4, !tbaa !3
  %17 = sub i32 %14, %16
  %18 = getelementptr inbounds i32, i32* %0, i64 2
  store i32 %17, i32* %18, align 4, !tbaa !3
  %19 = getelementptr inbounds i32, i32* %1, i64 3
  %20 = load i32, i32* %19, align 4, !tbaa !3
  %21 = getelementptr inbounds i32, i32* %2, i64 3
  %22 = load i32, i32* %21, align 4, !tbaa !3
  %23 = sub i32 %20, %22
  %24 = getelementptr inbounds i32, i32* %0, i64 3
  store i32 %23, i32* %24, align 4, !tbaa !3
  %25 = getelementptr inbounds i32, i32* %1, i64 4
  %26 = load i32, i32* %25, align 4, !tbaa !3
  %27 = getelementptr inbounds i32, i32* %2, i64 4
  %28 = load i32, i32* %27, align 4, !tbaa !3
  %29 = sub i32 %26, %28
  %30 = getelementptr inbounds i32, i32* %0, i64 4
  store i32 %29, i32* %30, align 4, !tbaa !3
  %31 = getelementptr inbounds i32, i32* %1, i64 5
  %32 = load i32, i32* %31, align 4, !tbaa !3
  %33 = getelementptr inbounds i32, i32* %2, i64 5
  %34 = load i32, i32* %33, align 4, !tbaa !3
  %35 = sub i32 %32, %34
                                                                                                                                                                1,1           Top
