#ifndef BOF3_BMAGIC_MAGIC04803_INTERNAL_H
#define BOF3_BMAGIC_MAGIC04803_INTERNAL_H

/* Callback table indexed by byte 1 of the work record published at
 * scratchpad pointer cell 0x1F800044.
 * @source 0x801EF818 @kind table
 */
extern void (*workByte1HandlerTable[])(void);

#endif
