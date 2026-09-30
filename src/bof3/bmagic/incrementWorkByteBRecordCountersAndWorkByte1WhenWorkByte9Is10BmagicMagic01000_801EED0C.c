#include "bof3/bof3.h"

/*
 * Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The cell is read once per access, so the original re-materializes its address
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) for each access; the cell is therefore
 * reached through this target-map symbol rather than through a constant
 * address. Splat's generated undefined_syms_auto.txt cannot bind it (nothing in
 * this overlay's remaining asm references 0x1F800044), so the target map owns
 * the address.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Counter table selected by the work object's byte 0xB: 12-byte records whose
 * leading bytes are the counters this handler advances. The original expresses
 * the three increments as field accesses of the selected record, so the record
 * base carries the byte-9 scale and the +0/+1/+2 field offsets fold into the
 * relocation (which is why the second counter's relocation is the base symbol
 * plus 0x1 and the third's is the distinct reviewed symbol 0x80145BD8). Splat
 * binds the address through the generated undefined_syms_auto.txt (the
 * still-unlifted func_801EEE20 reads the same table).
 */
typedef struct BmagicMagic01000CounterRecord {
  u8 counter_00;
  u8 counter_01;
  u8 counter_02;
  u8 pad_03[9];
} BmagicMagic01000CounterRecord;

extern BmagicMagic01000CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */

/* @source 0x801EED0C
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * per access and, for each of the record's three counter bytes, re-reads the
 * work object's byte 0xB, scales it by the 12-byte record stride and increments
 * the selected 0x80145BD6 record's bytes 0x0/0x1/0x2 with a byte-width add
 * (lbu, addiu $v0,$v0,1, sb). It then increments the work object's byte at
 * offset 0x9 and, when that truncated byte reaches 0x10 (andi $v0,$v0,0xFF
 * against a materialized 0x10), increments the work object's byte at offset
 * 0x1. Nothing is returned and no other state is read or written. The original
 * reserves a 0x10 frame even though no instruction outside the prologue and
 * epilogue touches it: the record-typed access reproduces that reservation
 * (measured on this selector: adding an explicit 16-byte local to this same
 * body raises the frame to 0x20 and moves the first difference to the frame
 * adjustment), and the frame adjustment lands in the byte-9 compare's delay
 * slot exactly as in the original. The 276 bytes are also byte-identical to the
 * unlifted func_801EED2C of emi/bmagic/magic117/00, which owns its own source,
 * map row and boundary and was neither lifted nor claimed from this lane.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBRecordCountersAndWorkByte1WhenWorkByte9Is10BmagicMagic01000_801EED0C(void) {
  D_80145BD6[g_battle_work[0xB]].counter_00++;
  D_80145BD6[g_battle_work[0xB]].counter_01++;
  D_80145BD6[g_battle_work[0xB]].counter_02++;
  if (++g_battle_work[9] == 0x10) {
    g_battle_work[1]++;
  }
}
