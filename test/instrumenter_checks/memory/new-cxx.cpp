/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2016,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2024,
 * Forschungszentrum Jülich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

#include <new>

/* Forces the compiler to not optimize the allocation away
 * LLVM 19.1.0 IR with -O3:
 * define dso_local noalias noundef nonnull ptr @allocate_int()() local_unnamed_addr #0 !dbg !10 {
 * %1 = tail call noalias noundef nonnull dereferenceable(4) ptr @operator new(unsigned long)(i64 noundef 4) #5, !dbg !16
 * store i32 1, ptr %1, align 4, !dbg !16
 * ret ptr %1, !dbg !21
 * } */
int*
allocate_int( int argc )
{
    return new int( argc );
}

/* Forces the compiler to not optimize the deletion away
 * LLVM 19.1.0 IR with -O3:
 * define dso_local void @free_int(int*)(ptr noundef %0) local_unnamed_addr #2 !dbg !22 {
 *     #dbg_value(ptr %0, !26, !DIExpression(), !27)
 *   %2 = icmp eq ptr %0, null, !dbg !28
 *   br i1 %2, label %4, label %3, !dbg !28
 *
 * 3:
 *   tail call void @operator delete(void*, unsigned long)(ptr noundef nonnull %0, i64 noundef 4) #6, !dbg !28
 *   br label %4, !dbg !28
 *
 * 4:
 *   ret void, !dbg !29
 * } */
void
free_int( int* memory )
{
    delete memory;
}

int
main( int argc, char* argv[] )
{
    int* memory = allocate_int( argc );
    free_int( memory );
    return 0;
}
