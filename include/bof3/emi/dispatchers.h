#ifndef BOF3_EMI_DISPATCHERS_H
#define BOF3_EMI_DISPATCHERS_H

#include "bof3/bof3.h"

/* Overlay work-record dispatchers.
 *
 * Every EMI overlay carries the same step dispatcher: read a byte of the work
 * record published at the scratchpad pointer cell 0x1F800044, scale it by four and
 * call that entry of a local handler table. The table and the record byte differ
 * per target; the shape does not. Three hundred and fifty copies of the body exist
 * across the corpus, written in seven different source spellings during earlier
 * lifting passes. This macro is the single definition they all use now.
 *
 * The variant selects which work-record byte indexes the table. Both variants
 * compile to the recorded 17-instruction, 68-byte body:
 *
 *   lui v0,0x1f80 / lw v0,68(v0) / addiu sp,sp,-24 / sw ra,16(sp)
 *   lbu v0,<slot>(v0) / nop / sll v0,v0,2 / lui at,<table page>
 *   addu at,at,v0 / lw v0,<table offset>(at) / nop / jalr v0 / nop
 *   lw ra,16(sp) / addiu sp,sp,24 / jr ra / nop
 *
 * Put each emitted function's metadata block directly above its standalone
 * invocation; the metadata checker attributes one function per invocation.
 *
 *     /* @source 0x801F6250 ... *\/
 *     EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStepTableA, D_801F6250);
 */

/* Variant enumerators: the work-record byte that indexes the handler table. */
#define EMI_DISPATCH_WORK_BYTE_1 1
#define EMI_DISPATCH_WORK_BYTE_2 2
#define EMI_DISPATCH_WORK_BYTE_3 3
#define EMI_DISPATCH_WORK_BYTE_4 4

/* Emit one dispatcher. `variant` is one of the enumerators above, `name` is the
 * emitted function and `table` the local handler table it indexes. */
#define EMI_DEFINE_DISPATCHER(variant, name, table)                            \
  void name(void) {                                                            \
    table[SPAD_PTR_SLOT(u8, 0x44)[variant]]();                                 \
  }

#endif
