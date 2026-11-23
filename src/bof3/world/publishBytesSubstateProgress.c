#include "bof3/bof3.h"

/* Shared gate byte the overlay entry handlers branch on; read with lb, so
 * signed. */
extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */

/* Shared status halfword the entry handlers latch, stored with sh. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */

/* Target-local halfword table indexed by the gate byte: its two entries are
 * 0x35 and 0x36, the last four bytes of this payload. */
extern u16 D_801F5058[]; /* @source 0x801F5058 @kind table */

/* Shared scenario progress byte and the three shared bytes cleared beside it. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */
extern u8 D_80146865; /* @source 0x80146865 @kind unknown */
extern u8 D_80146866; /* @source 0x80146866 @kind unknown */
extern u8 D_80146867; /* @source 0x80146867 @kind unknown */

/* Shared primary state request cell and the secondary sub-state byte beside
 * it. */
extern u8 D_80146874; /* @source 0x80146874 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

/* @source 0x801F35B0
 * @behavior Entry handler that latches the halfword indexed by the shared gate
 *           byte into the shared status halfword, then acts on that gate byte.
 *           The gate byte 0x801490C7 is read once with lui/lb into one local;
 *           the table base 0x801F5058 is indexed by that local with sll by one
 *           and addu into $at, and the lhu result is stored with sh into the
 *           shared status halfword 0x801490A8. The same local then selects one
 *           of two arms: gate 0 stores 0xA into the shared sub-state byte
 *           0x80146875, zeroes the shared scenario progress byte 0x80146864 and
 *           the shared bytes 0x80146865, 0x80146866 and 0x80146867, in that
 *           order, and stores 6 into the shared primary state request byte
 *           0x80146874 last; gate 1 zeroes only 0x80146864; every other gate
 *           value leaves those cells untouched. Takes no arguments and returns
 *           nothing: the 0x8C-byte span is one leaf with no frame, a single
 *           jr $ra return path and no argument registers live at entry.
 *
 * Shape evidence: the switch topology with two gate arms and no default
 * statements is the one that measured exact in the same-shape sibling at
 * 0x801F3650, and the leading publish is the two-statement gate-read/table-lhu/
 * status-sh store of publishModeStatusFromTable449cAndProgress1Or0AWorld02Area08013_801F3C94.
 * The arm policies and the cell roles come from
 * setStatusFFFFAndRequestPrimaryState7Substate14WithProgress5AndStep6UnlessGateElseStep5World03Area14413_801F3D14,
 * which names 0x801490C7 as the shared gate byte, 0x801490A8 as the shared
 * status halfword, 0x80146874 as the primary state request cell and 0x80146875
 * as the secondary sub-state byte, and from the sources that call
 * 0x80146865/0x80146866/0x80146867 the shared bytes.
 *
 * Every immediate here is written from this span's own bytes: the table base
 * 0x801F5058, the arm constants 0xA and 6, and the five cells the arms clear.
 * A scan of the other 397 payloads under out/binaries finds this 140-byte span
 * zero times, so no byte-identical sibling exists and this body is transcribed
 * rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBytesSubstateProgress(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = D_801F5058[gate];
  switch (gate) {
  case 0:
    D_80146875 = 0xA;
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146874 = 6;
    break;
  case 1:
    g_ScenarioProgress = 0;
    break;
  }
}
