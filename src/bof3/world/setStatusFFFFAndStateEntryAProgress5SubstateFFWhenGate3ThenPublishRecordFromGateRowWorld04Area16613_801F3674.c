#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_801448EB; /* @source 0x801448EB @kind unknown */
extern u8 D_801448EC; /* @source 0x801448EC @kind unknown */
extern u8 D_801448ED; /* @source 0x801448ED @kind unknown */
extern u8 *D_80148208; /* @source 0x80148208 @kind unknown */
extern u8 D_801F3940[]; /* @source 0x801F3940 @kind table */
extern u8 D_801F3941[]; /* @source 0x801F3941 @kind table */

void func_8015C088(void);

/* @source 0x801F3674
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path (`ori v0,0,0xFFFF`, `sh`), and only while the shared
 *           gate byte at 0x801490C7 equals 3 (`lb`, so the cell is s8) runs the
 *           shared front-end startup helper func_8015C088 and then writes the
 *           shared state-entry tuple: 0x801448EB = 0x0A, 0x801448EC = 5 and
 *           0x801448ED = 0xFF, each from its own `addiu`. It then copies the two
 *           bytes of the row the gate byte selects from the payload-resident
 *           stride-2 row table at 0x801F3940 into the words at +0x18 and +0x1C
 *           of the record pointer held at D_80148208; the second copy takes the
 *           0x801F3941 byte of the same row. The gate byte is read again for
 *           that second row, which the fixed-address accessor at that use site
 *           reproduces. The gate load is hoisted above the status store, `bne`
 *           jumps past the tuple stores, so gate 3 is the arm that falls
 *           through, and the 0x18-byte frame only keeps $ra across the helper
 *           call. Takes no arguments and returns nothing.
 *
 *           Name evidence: 0x801448EB is the state-entry byte of the shared
 *           tuple the setStatusFFFF handlers write, with 0x801448EC as the
 *           progress byte and 0x801448ED as its companion byte, 0x801490C7 is
 *           the shared gate byte those handlers branch on, and 0x8015C088 is
 *           the shared front-end startup helper the world family runs.
 *           0x801F3940 and 0x801F3941 are the two byte columns of a five-row,
 *           stride-2 table in this payload's own textbin tail, whose low bytes
 *           read 4, 5, 7, 9 and 9 under zero high bytes; that is the same
 *           gate-row role the sibling source
 *           setStatusFFFFAndPublishRecordWords18And1CFromGateRowWorld04Area19213_801F2CFC
 *           gives the 0x801F380C pair in its overlay. The payload names this
 *           body only from a table word: 0x801F38F8 holds 0x801F3674 as entry 1
 *           of the handler run 0x801F38F4, which the payload publishes in its own
 *           textbin tail at 0x801F3904, and no in-image `jal` reaches it, so its
 *           roles stay structural.
 *
 *           Shape: statement order, operand forms, offset spelling and
 *           declaration style are transcribed from the already-exact sibling
 *           setStatusFFFFAndStateEntryAProgress5SubstateFFWhenGate3ThenPublishRecordFromGateRowWorld04Area16813_801F4764,
 *           whose 0xA4-byte span at 0x801F4764 is word-for-word identical to
 *           this one except the two `lbu` row-table immediates: 162 of 164 bytes
 *           are equal and the two differing words are the 0x801F3940 and
 *           0x801F3941 row loads against the sibling's 0x801F4F94 and 0x801F4F95.
 *           No byte-identical twin exists, because a read of every payload under
 *           out/binaries found both row immediates only in this overlay, so the
 *           row addresses are written here from this span's own bytes and no
 *           clone adapter applies. The shape claims above were measured before
 *           the first gate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntryAProgress5SubstateFFWhenGate3ThenPublishRecordFromGateRowWorld04Area16613_801F3674(
    void) {
  u8 *record;

  D_801490A8 = 0xFFFF;
  if (D_801490C7 == 3) {
    func_8015C088();
    D_801448EB = 0xA;
    D_801448EC = 0x5;
    D_801448ED = 0xFF;
  }
  record = D_80148208;
  *(u32 *)(record + 0x18) = D_801F3940[D_801490C7 * 2];
  *(u32 *)(record + 0x1C) = D_801F3941[PSX_REF(s8, 0x801490C7u) * 2];
}
