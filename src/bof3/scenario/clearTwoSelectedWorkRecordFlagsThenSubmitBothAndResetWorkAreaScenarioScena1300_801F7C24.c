#include "bof3/bof3.h"

/* Shared work-record table at 0x80146888: records of 0x98 bytes in main RAM,
 * outside this overlay's image. This target's generated undefined_syms_auto.txt
 * binds the address, so no target-map row is needed. The declared record view
 * carries the layout the reviewed shared work-record definitions in this tree
 * document for that address: byte 0 is the record flags byte, whose zero value
 * marks an unused slot. */
typedef struct ScenarioScena1300WorkRecord {
  u8 flags_00;
  u8 pad_01[0x97];
} ScenarioScena1300WorkRecord; /* 0x98 bytes */

extern ScenarioScena1300WorkRecord D_80146888[]; /* @source 0x80146888 @kind data */

/* Scratchpad work-object pointer cell 0x1F800044, shared by this overlay's
 * dispatchers and handlers. The cell is spelled as the named symbol the other
 * exact lifts of this target index, so each of this body's four references
 * re-materialises the cell address with its own `lui`/`lw` pair, as the
 * original does around every record it selects. It binds at its own address
 * with no target-map row. */
extern u8* D_1F800044;

/* Shared main-executable routine this overlay submits a selected work record
 * through. It is out of payload and its own body is not lifted anywhere in the
 * tree, so the name stays address-anchored and the parameter is the published
 * record pointer this call site passes. */
extern void func_80196718(void* record); /* @source 0x80196718 @kind function */

/* Shared work-area reset at 0x80196070: the exact ui lift clearWorkFlags
 * records it clearing the work-area header bytes 0x00-0x04 and nothing else.
 * It is called with no argument set, so the declaration stays argument-free. */
extern void func_80196070(void); /* @source 0x80196070 @kind function */

/* @source 0x801F7C24
 * @behavior Work-record release handler of this overlay, entry 11 (the word at
 *           0x801FBF1C) of the twelve-word in-image pointer run D_801FBEF0 that
 *           dispatchWorkByte1HandlerScenarioScena1300_801F6EA4 indexes with the
 *           scratchpad work object's byte 1; that one data word is this body's
 *           only reference in the payload, which encodes no `jal` to it. It
 *           takes no arguments and returns nothing.
 *
 *           Each of its four record references reloads the pointer cell
 *           0x1F800044 with its own `lui $v0,0x1F80` / `lw $v0,0x44($v0)`, and
 *           the `sll`/`addu`/`sll`/`subu`/`sll` chain scales the selected
 *           object byte by 0x98 (`sll 2`, add the byte, `sll 2`, subtract the
 *           byte, `sll 3`), the same stride and base the exact same-target lift
 *           convergeWorkRecordPairYOverCountdownThenAdvanceWorkByte1ScenarioScena1300_801F7958
 *           indexes with bytes 3 and 4 of this same object. The first two
 *           references clear the selected record's flags byte 0 with `sb $zero`
 *           through the absolute base 0x80146888 (`lui $at,%hi` / `addu
 *           $at,$at,$v0` / `sb $zero,%lo($at)`), first for the record selected
 *           by object byte 3 and then for the one selected by byte 4. The base
 *           is materialised into $s0 once, at the third reference, and the last
 *           two references pass each selected record to the shared routine
 *           func_80196718 with `addu $a0,$a0,$s0` in the jal delay slot, byte 3
 *           first and byte 4 second. The body then calls the shared work-area
 *           reset func_80196070 with a nop delay slot and returns; it reads and
 *           writes no other state. The 0x18-byte frame keeps $ra at 0x14 and
 *           $s0 at 0x10, and the first cell load precedes that frame.
 *
 *           The record flags byte and the selected object bytes are the only
 *           roles this body establishes; what func_80196718 does with a
 *           submitted record is not established here, so the body is named for
 *           the two records it clears and submits and for the work-area reset
 *           that closes it. Statement order, operand forms, offset-local
 *           spelling and declaration style come from the exact sibling
 *           submitTwoSelectedWorkRecordsThenResetWorkAreaScenarioScena0300_801F7334,
 *           whose two record submits and closing work-area reset this body adds
 *           its two leading flag clears to. One payload scan over out/binaries
 *           found no payload holding this span's leading words, so no
 *           byte-identical twin exists anywhere in the tree and that sibling is
 *           shape evidence only. The strided record view is the lever that
 *           closed the frame: the same statements over a flat byte array
 *           measured 54 of 60 instructions behind a 0x28-byte frame, first
 *           difference `addiu sp,sp,-40` against the payload's `-24`, and the
 *           record view matched all 60 at the payload's own 0x18-byte frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearTwoSelectedWorkRecordFlagsThenSubmitBothAndResetWorkAreaScenarioScena1300_801F7C24(void) {
  D_80146888[D_1F800044[3]].flags_00 = 0;
  D_80146888[D_1F800044[4]].flags_00 = 0;
  func_80196718(&D_80146888[D_1F800044[3]]);
  func_80196718(&D_80146888[D_1F800044[4]]);
  func_80196070();
}
