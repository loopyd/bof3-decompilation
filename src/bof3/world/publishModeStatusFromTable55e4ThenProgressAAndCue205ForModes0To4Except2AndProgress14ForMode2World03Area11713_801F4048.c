#include "bof3/bof3.h"

/* Shared mode byte of the resident executable, outside this overlay image. The
 * body reads it with `lb` and scales it by two for the status table, so it is
 * signed here as this overlay's neighbouring mode handler declares it.
 * @source 0x801490C7 @kind data
 */
extern s8 D_801490C7;

/* Shared status halfword of the resident executable; the body publishes the
 * selected table entry into it with one `sh`.
 * @source 0x801490A8 @kind data
 */
extern u16 D_801490A8;

/* Mode-indexed halfword status table at the end of the payload's 0x801F4624
 * tail blob. The body adds the signed mode scaled by two to this base and reads
 * one `lhu`, so an element is a halfword: modes 0-4 read 0xFFFF and mode 5
 * reads 0x002B. Those six halfwords are the ones this base reaches; the bytes
 * at 0x801F55F0 (0x22/0x22/0x07/0x85) follow them and this body never reads
 * them.
 * @source 0x801F55E4 @kind table
 */
extern u16 D_801F55E4[];

/* Shared scenario progress byte the two progress arms store into.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared frontend cue dispatcher of the resident executable; the cue id arrives
 * zero-extended in the argument word, as this overlay's neighbouring mode
 * handler passes it.
 * @source 0x8015DF18
 */
extern void func_8015DF18(u16 cue);

/* @source 0x801F4048
 * @behavior Overlay mode handler: loads the shared mode byte 0x801490C7 with a
 *           signed byte read (`lb`), scales it by two (`sll $v0,$v1,1`) and
 *           indexes this overlay's halfword status table at 0x801F55E4 (`addu`
 *           into $at plus one `lhu`), publishing the selected halfword into the
 *           shared status halfword 0x801490A8 with `sh`. It then bounds the same
 *           signed mode with one unsigned `sltiu $v0,$v1,6` and dispatches
 *           through the six-word absolute jump table at 0x801F2C04, the
 *           payload's leading table data: modes 0, 1, 3 and 4 store 0x0A into
 *           the shared scenario progress byte g_ScenarioProgress 0x80146864 and
 *           request frontend cue 0x205 through the resident dispatcher
 *           0x8015DF18 (`addiu a0,zero,0x205` in the `jal` delay slot, then `j`
 *           to the epilogue), mode 2 stores 0x14 into that byte and falls into
 *           the epilogue, and mode 5 jumps straight to it. The table's halfwords
 *           read 0xFFFF for modes 0-4 and 0x002B for mode 5. Takes no arguments
 *           and returns nothing, and the 0x18-byte frame keeps only $ra at
 *           +0x10, which is why the mode load and the table read precede the
 *           frame reservation in the original schedule.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: statement order, operand forms and
 * declaration style are transcribed from two reviewed siblings. The status
 * publish (`mode = D_801490C7; D_801490A8 = <table>[mode];`) follows the
 * same-target sibling 0x801F3FF0, which reads the very run these six halfwords
 * end and pairs 0x0A progress with cue 0x205 the way this body's first arm does.
 * The switch form and the `.rodata` placement route follow the reviewed
 * `dispatchSharedModeByteTable2C04*` lifts of world02/area111 and
 * world04/area168 and area188, whose jump tables also sit at their payload's
 * 0x801F2C04. This body's own words are its table base 0x801F55E4, its case set
 * 0/1/3/4 versus 2 plus the empty mode 5 slot that widens the table to six
 * entries, and its constants 0x0A, 0x14 and 0x205, so the siblings are shape
 * evidence only.
 *
 * Clone evidence: the exact 0x84-byte span occurs once across all 396 shipped
 * payloads, and a masked scan that wildcards only the two payload-local table
 * base words (hi and lo of 0x801F55E4 and of 0x801F2C04) also finds exactly one
 * occurrence, so the clone lever has no byte-identical source and was declined.
 * The six words of the dispatch table at 0x801F2C04 are 0x801F4094 four times
 * plus 0x801F40B0 and 0x801F40BC, which is the body order below.
 */
void publishModeStatusFromTable55e4ThenProgressAAndCue205ForModes0To4Except2AndProgress14ForMode2World03Area11713_801F4048(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F55E4[mode];
  switch (mode) {
  case 0:
  case 1:
  case 3:
  case 4:
    g_ScenarioProgress = 0xA;
    func_8015DF18(0x205);
    break;
  case 2:
    g_ScenarioProgress = 0x14;
    break;
  case 5:
    break;
  }
}
