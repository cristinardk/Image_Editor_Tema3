; ModuleID = 'probe2.c831b4d3939636d9-cgu.0'
source_filename = "probe2.c831b4d3939636d9-cgu.0"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; probe2::probe
; Function Attrs: nonlazybind uwtable
define void @_ZN6probe25probe17h73b07802d477f8deE() unnamed_addr #0 {
start:
  ret void
}

attributes #0 = { nonlazybind uwtable "probe-stack"="inline-asm" "target-cpu"="x86-64" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 8, !"PIC Level", i32 2}
!1 = !{i32 2, !"RtLibUseGOT", i32 1}
!2 = !{!"rustc version 1.78.0 (9b00956e5 2024-04-29)"}
