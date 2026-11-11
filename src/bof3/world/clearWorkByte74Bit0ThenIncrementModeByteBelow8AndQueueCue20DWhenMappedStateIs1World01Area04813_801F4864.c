#include "bof3/bof3.h"

/* Work-area record pointer cell of the shared main executable, immediately
 * before the D_80146888 work-area array. It is outside this overlay's image, so
 * this overlay family's neighbouring lifts read the same cell.
 * @source 0x80146884 @kind data
 */
extern u8* D_80146884;

/* Byte +0x79 of the first 0x140-stride local work record of the shared run at
 * 0x80145E90. The neighbouring overlays read the same byte as that record's
 * party id and as its kind; this body only loads it, with `lbu`, to index the
 * table below, so it stays a byte.
 * @source 0x80145F09 @kind data
 */
extern u8 D_80145F09;

/* Party-id to field-record index table of the shared main executable. The
 * World04 area191/area192 lifts index the 0xA4-byte field record array
 * 0x80144968 with `D_80181B10[code]`, so this reads one entry of that same
 * mapping.
 * @source 0x80181B10 @kind table
 */
extern u8 D_80181B10[];

/* Shared small mode byte of the main executable. The neighbouring handler at
 * 0x801F4B40 stores 0 into it, the handler at 0x801F48DC scales it by four into
 * the pointer table 0x80148210, and this body is the one that steps it, so it
 * is the mode the overlay's handlers count with. Its role beyond that count is
 * not established here, so the kind stays unknown.
 * @source 0x8014686B @kind unknown
 */
extern u8 D_8014686B;

/* Shared frontend cue dispatcher of the main executable at 0x8015DF18; the cue
 * id arrives zero-extended in the whole argument word.
 * @source 0x8015DF18
 */
extern void func_8015DF18(u16 cue);

/* @source 0x801F4864
 * @behavior Clears bit 0 of the byte at +0x74 of the work record published by
 *   the shared pointer cell 0x80146884 with an `andi 0xFE` read-modify-write.
 *   When the shared party-id byte 0x80145F09 indexes table 0x80181B10 to the
 *   value 1, it advances the shared mode byte 0x8014686B by one only while that
 *   byte reads below 8, then requests frontend cue 0x20D through the shared cue
 *   dispatcher 0x8015DF18; otherwise it only performs the read-modify-write.
 *   Takes no arguments and returns nothing. The 0x18-byte frame keeps only $ra
 *   at +0x10.
 *
 * Naming evidence: the body itself carries every fact the name states. The
 *   +0x74 bit-0 clear and the table-1 gate are the same two statements the
 *   immediately preceding handler 0x801F4804 performs, whose exact lift stands
 *   in this target; that handler calls the shared bit setter where this one
 *   steps the mode byte and posts a cue, which is the whole difference. The
 *   stepped byte is the one the neighbouring lifts at 0x801F4B40 and 0x801F48DC
 *   already read as the shared small mode byte, and the posted id is the 0x20D
 *   immediate the body loads into the cue dispatcher's argument word.
 *
 *   The caller is unnamed: no `jal` in any of the 396 shipped payloads targets
 *   this address, and no word of any payload equals 0x801F4864, so the body
 *   takes no arguments and its roles stay structural.
 *
 * Shape evidence: the preceding handler 0x801F4804, exact in this target, holds
 *   the first sixteen words of this span unchanged - the same hoisted record
 *   pointer load above the frame creation, the same +0x74 read, `andi 0xFE` and
 *   store back, the same table-1 gate - and supplies the statement order, the
 *   pointer-local spelling and the frame. This span writes its own gated block
 *   from its own bytes: the mode byte, the bound 8 and the cue id 0x20D. The
 *   pointer-local spelling of the stepped byte follows the exact slus_004_22
 *   lift emiCdReadyCallback, whose `s8* slot_p = &D_80146489; *slot_p += 1;`
 *   compiled to this same `lui`/`addiu` address materialisation with load and
 *   store at offset 0 of that register.
 *
 * Clone declined: one scan of the 396 extracted payloads under out/binaries for
 *   this 0x78-byte body aligned on its own seven-word prefix returned only this
 *   span's own payload offset 0x1C64 byte for byte; the nearest other hit is
 *   this target's own 0x801F4804 at 13 differing words, and no payload carries
 *   the gated block's `lbu 0(a0)`, `sltiu 8`, `addiu`, `sb` sequence.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte74Bit0ThenIncrementModeByteBelow8AndQueueCue20DWhenMappedStateIs1World01Area04813_801F4864(
    void) {
  u8* record;

  record = D_80146884;
  record[0x74] &= ~1u;
  if (D_80181B10[D_80145F09] == 1) {
    u8* mode;

    mode = &D_8014686B;
    if (*mode < 8) {
      (*mode)++;
    }
    func_8015DF18(0x20D);
  }
}
