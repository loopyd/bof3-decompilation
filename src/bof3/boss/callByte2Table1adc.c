#include "bof3/emi/dispatchers.h"

/*
 * Interior base of this overlay's handler run. The reviewed 0x800C1A70
 * `textbin` row covers 0x800C1A70 through the payload end at 0x800C1B00: its
 * leading 0x3C bytes hold the byte tables that precede the code, and the
 * code-pointer run starts at 0x800C1AAC (word 0 = 0x800C1848) and runs from
 * there to that end. This dispatcher indexes the run from its word 12,
 * 0x800C1ADC, so that base word is named here on its own while the reviewed row
 * stays whole; the byte-1 dispatcher of this target indexes word 0 of the same
 * run. Run word 16 holds the next code span of this overlay, 0x800C18E4, and
 * words 18 through 20 the inert handlers 0x800C199C, 0x800C19A4 and 0x800C19AC.
 * The generated undefined_syms_auto.txt row for D_800C1ADC already covers the
 * address, so this declaration needs no map row.
 */
extern void (*D_800C1ADC[])(void); /* @source 0x800C1ADC @kind data */

/* @source 0x800C18A0
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 before it creates
 * its frame, takes that record's unsigned byte at offset 0x02 as the index,
 * scales it by four and calls the matching entry of this overlay's handler run
 * based at 0x800C1ADC with no arguments. The original does not bound the index
 * before the scaled table load, and the slots after the index byte, after the
 * table load and in both branch delay slots stay nops. It writes no state and
 * leaves the handler's return register untouched, so this shim keeps whatever
 * the handler leaves in $v0; the 0x18-byte frame only keeps the return address
 * at 0x10 across the indirect call. The only payload word that names this
 * address is 0x800C1AC4, word 6 of the run based at 0x800C1AAC that
 * callByte1Table1aac dispatches, and no jal in
 * this payload names it, so that shim is its caller: it passes no arguments and
 * forwards none. Same 0x44-byte body as the exact sibling
 * callByte1Table1aac of this target, which
 * differs from this span only in the work-record byte it reads and the run word
 * it indexes, so this lift writes this span's own byte offset and table base.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callByte2Table1adc,
                      callByte2Table1adc,
                      D_800C1ADC);
