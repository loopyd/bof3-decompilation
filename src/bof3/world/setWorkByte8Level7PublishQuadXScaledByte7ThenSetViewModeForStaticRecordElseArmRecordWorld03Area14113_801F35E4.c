#include "bof3/bof3.h"

/* Scratchpad pointer cell holding the work record this step runs on. Spelled as
 * the named fixed address rather than through the constant-address accessor,
 * because the original re-emits `lui $vN,0x1F80` plus the folded
 * `lw $vN,68($vN)` pair before each of its three access groups; the accessor
 * form shares one materialised base across them. The `D_<ADDRESS>` spelling
 * binds with no map row, the same declaration the neighbouring lift at
 * 0x801F3588 uses.
 * @source 0x1F800044 @kind unknown
 */
extern u8* D_1F800044;

/* Pointer cell of the shared record byte +0x07 of which this step publishes.
 * The original re-reads the cell after the level store, for the arming call's
 * record argument, so the cell keeps its named spelling and every access
 * re-reads it; the exact World00 area000 lift at 0x801F2CD8 reads the same
 * cell.
 * @source 0x80148210 @kind unknown
 */
extern u8* D_80148210;

/* Static main-RAM work record. The original compares the scratch record
 * pointer against this address with `lui`/`addiu` and, on equality, reads the
 * level byte at the fixed address 0x80143F50, which is this record's byte
 * +0x08, so the body spells the record as a byte base. The address lies
 * outside this payload and no composed map names it, so it stays
 * address-anchored.
 * @source 0x80143F48 @kind unknown
 */
extern u8 D_80143F48[];

/* Shared main-executable helper that arms the record passed as its first
 * argument for the level byte passed as its second.
 * @source 0x801ABBA8
 */
extern void func_801ABBA8(u8* record, u8 level);

/* Shared main-executable view routine; its byte argument selects the view
 * mode.
 * @source 0x801C5DFC
 */
extern void func_801C5DFC(u8 mode);

/* @source 0x801F35E4
 * @behavior Overlay step handler reached from data rather than a call: the
 *           payload's handler run based at 0x801F4D38 holds this address at its
 *           fourth word, 0x801F4D44, so the body takes no arguments and
 *           returns nothing.
 *           It loads the scratch work record published at the scratchpad
 *           pointer cell 0x1F800044, forms the record's signed word +0x34
 *           minus 0x450000 and shifts that difference right by 15, and returns
 *           untouched when the result is zero. Otherwise it reloads the record
 *           pointer cell 0x80148210 and stores the absolute value of that
 *           difference into byte +0x07 of the published record, re-reads the
 *           scratchpad cell and stores the level 7 into the work record's byte
 *           +0x08, then re-reads the cell a third time and compares the record
 *           pointer with the static main-RAM record at 0x80143F48: on equality
 *           it calls the shared view routine 0x801C5DFC with that static
 *           record's byte +0x08, read at the fixed address 0x80143F50, and
 *           otherwise it calls the shared arming helper 0x801ABBA8 with the
 *           published record and the work record's byte +0x08, reading that
 *           byte through the third load. Every group re-materialises the
 *           scratchpad cell because a store through the previously loaded
 *           pointer may alias the cell itself. The 0x18-byte frame holds only
 *           $ra at 0x10, and the record pointer load precedes the frame
 *           creation.
 *
 * Naming evidence: byte +0x08 of the work record is the level index the exact
 *   neighbours store before handing the record on - level 5 at 0x801F48DC,
 *   level 1 at 0x801F49C8, level 7 here, the highest value the modulo-8
 *   stepper 0x801F2E18 reaches. func_801ABBA8 is the shared arming helper the
 *   exact World00 area000 lift (0x801F2CD8) and the World01 area048 lifts
 *   (0x801F48DC, 0x801F49C8) name, and func_801C5DFC is the shared view
 *   routine the exact World03 area144 lift (0x801F3C78) and the World01
 *   area048 lift name. Byte +0x07 of the record published at 0x80148210 is the
 *   same published byte the World01 area048 lifts write a scaled value into,
 *   here the scaled absolute of the record's +0x34 coordinate word remapped
 *   about 0x450000. The role of the 0x450000 offset itself is unproven, so the
 *   literal is kept as the original constant.
 *
 * Shape evidence: the exact World01 area048 lift at 0x801F48DC carries the same
 *   statement sequence - work byte +0x08 level store, published byte +0x07
 *   store, then a choice between the shared view routine and the shared arming
 *   helper - and supplied the operand forms, the offset-local spelling and the
 *   cell-reload style used here. Two things are this span's own: it reloads the
 *   scratchpad cell between the two stores (that lift stores both bytes from
 *   one load), and its published value comes from the work record's +0x34 word
 *   rather than from a shared mode byte. The static-record test is this span's:
 *   it is an equality on the scratch record pointer with the static record
 *   address, and the `if`/`else` polarity is the original's, since the original
 *   branches away with `bne` and leaves the view-routine arm as the
 *   fall-through that ends in a `j` to the shared epilogue.
 *
 * Clone declined: the discriminating immediate 0xFFBB0000 of this body appears
 *   in no other payload byte image under out/binaries (396 images scanned), and
 *   no `jal` in this payload targets 0x801F35E4; the only reference is the
 *   handler-run word at 0x801F4D44, so no byte-identical twin exists to clone.
 *
 * Surplus bytes: none - the row runs from 0x801F35E4 to the next boundary at
 *   0x801F3688, and the body's own `jr $ra`/delay pair ends it at 0x801F3688.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte8Level7PublishQuadXScaledByte7ThenSetViewModeForStaticRecordElseArmRecordWorld03Area14113_801F35E4(
    void) {
  u8* work;
  u8* tail;
  s32 scale;

  work = D_1F800044;
  scale = (FIELD_REF(s32, work, 0x34) - 0x450000) >> 15;
  if (scale != 0) {
    D_80148210[7] = scale < 0 ? -scale : scale;
    tail = D_1F800044;
    tail[8] = 7;
    tail = D_1F800044;
    if (tail == D_80143F48) {
      func_801C5DFC(D_80143F48[8]);
    } else {
      func_801ABBA8(D_80148210, tail[8]);
    }
  }
}
