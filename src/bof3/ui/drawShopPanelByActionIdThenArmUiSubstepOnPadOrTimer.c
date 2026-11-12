#include "bof3/ui/shop00_internal.h"

/* @source 0x801D543C
 * @behavior shop UI sub-step handler at table entry 0x0C of the sub-step
 *           handler table D_801E530C, which func_801D46A4 dispatches with the
 *           UI sub-step byte D_80148652 as index. It switches on the main-RAM
 *           panel action id D_8018E228 and selects one icon handle and one
 *           next sub-step per action, then arms that sub-step when the player
 *           presses the pad or when the action 8/10 phase timer runs out.
 *           Actions 1, 3, 4, 6, 7 and 0x0B, and every action outside 1
 *           through 0x0B, publish the icon handle through the shared tail:
 *           the shop panel strip through func_801DAB90 with the fixed panel
 *           arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank
 *           byte D_80144952, then the texture row 0x1C at row 0x16 through
 *           func_8014F800 with func_801502D0(handle + 0x85) as the fifth
 *           argument. Action 2 does the same with the handle 0x89 but first
 *           clears the overlay byte D_801E607C and the work-block byte
 *           D_8014865B and arms the sub-step 0xF. Actions 5 and 9 emit the
 *           panel strip, submit func_801502D0(0x97) or func_801502D0(0x95),
 *           draw the sliding value panel through func_801DDFB0 at (0x62,
 *           0x4C) with the overlay flag byte D_801E60F8, the main-RAM byte
 *           D_8018E260 and the variant constant 1, and then publish the panel
 *           record state: the entry count D_801E6078 with 3, the
 *           remaining-capacity byte D_801E60FC with 0xF, the work-block byte
 *           D_8014865B with 0, the selected-slot byte D_801E6074 with 0 and
 *           the three 0xFF slot bytes of the panel records at D_801E60E5,
 *           D_801E60C9 and D_801E60AD, before arming the sub-step 8. Actions 8
 *           and 0x0A emit the strip, submit func_801502D0(0x8F) or
 *           func_801502D0(0x96), draw the panel row through
 *           func_801D67B0(0x20, 0x30, 0), set the overlay bytes D_801E6080 and
 *           D_801E6110 and arm the sub-step 0x0B. Action 0x0B emits the strip,
 *           submits func_801502D0(0x8B), draws the panel row through
 *           func_801D67B0(0x20, 0x30, 0) and arms the sub-step 8. The armed
 *           sub-step stays 0x12 for the shared tail. A nonzero shared pad
 *           state D_80145AA8 arms it at once; a clear pad state arms it only
 *           for the action ids 8 and 0x0A, and only once the frame timer
 *           phaseTimer it counts down reaches zero. Every arm clears
 *           phaseTimer, plays the accept cue 0x104 through func_8015DF18
 *           unless the action id is 8 or 0x0A, and stores the selected
 *           sub-step into D_80148652. Takes no arguments and returns nothing.
 *
 * Name evidence: the payload word at D_801E533C holds this address, so the
 * entry is 0x0C of the sub-step handler table D_801E530C that
 * func_801D46A4 dispatches, and the body draws the shop panel state each arm
 * selects from the panel action id D_8018E228 before it arms the UI sub-step
 * byte D_80148652 from that same arm. The payload names this body from that
 * one table word and from no jal, and the table entry carries no prototype, so
 * every argument and callee role inside the arms stays structural.
 *
 * Shape: the eleven arms are written out in full because the original carries
 * each arm whole, and the two shared tails are the compiler's own merge of the
 * identical suffixes rather than labels in the source: actions 5 and 9 merge at
 * func_801502D0, actions 8 and 0x0A merge there too, and the earlier arm of each
 * pair keeps its jump. The switch's eleven-word jump table sits in the payload
 * header at 0x801D0C3C and is placed by this target's
 * [[matching.section_placements]] row for the .rodata section. The three panel
 * record slot bytes beyond D_801E60AD have no map row, so this target declares
 * them at their own addresses beside it.
 *
 * The icon handle local is the switch's only carried value and stays zero for
 * the arms that publish their own texture, which is what skips the shared
 * tail; the sub-step local starts at 0x12 and every arm that changes it writes
 * the whole value. The countdown reaches the timer through a pointer, as its
 * two countdown siblings do: the original materializes the phaseTimer address
 * once into one register and reads and writes through that base, while a direct
 * access to the volatile cell folds the symbol into both instructions and
 * measures 209 of 213 instructions. The two action-id tests keep the orders the
 * original shows, 0x0A before 8 in the countdown gate and 8 before 0x0A in the
 * cue gate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopPanelByActionIdThenArmUiSubstepOnPadOrTimer(void) {
  s32 iconHandle;
  u8 nextSubstep;
  volatile u8* timer;
  u8 value;

  iconHandle = 0;
  nextSubstep = 0x12;
  switch (D_8018E228) {
  case 1:
    iconHandle = 3;
    break;
  case 2:
    iconHandle = 4;
    D_801E607C = 0;
    D_8014865B = 0;
    nextSubstep = 0xF;
    break;
  case 3:
    iconHandle = 5;
    break;
  case 4:
    iconHandle = 6;
    break;
  case 5:
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    nextSubstep = 8;
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x97));
    func_801DDFB0(0x62, 0x4C, D_801E60F8, D_8018E260, 1);
    D_801E6078 = 3;
    D_801E60FC = 0xF;
    D_8014865B = 0;
    D_801E6074 = 0;
    D_801E60E5 = 0xFF;
    D_801E60C9 = 0xFF;
    D_801E60AD = 0xFF;
    break;
  case 6:
    iconHandle = 7;
    break;
  case 7:
    iconHandle = 8;
    break;
  case 8:
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    nextSubstep = 0xB;
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x8F));
    func_801D67B0(0x20, 0x30, 0);
    D_801E6080 = 1;
    D_801E6110 = 1;
    break;
  case 9:
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    nextSubstep = 8;
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x95));
    func_801DDFB0(0x62, 0x4C, D_801E60F8, D_8018E260, 1);
    D_801E6078 = 3;
    D_801E60FC = 0xF;
    D_8014865B = 0;
    D_801E6074 = 0;
    D_801E60E5 = 0xFF;
    D_801E60C9 = 0xFF;
    D_801E60AD = 0xFF;
    break;
  case 10:
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    nextSubstep = 0xB;
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x96));
    func_801D67B0(0x20, 0x30, 0);
    D_801E6080 = 1;
    D_801E6110 = 1;
    break;
  case 11:
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    nextSubstep = 8;
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0x8B));
    func_801D67B0(0x20, 0x30, 0);
    break;
  default:
    iconHandle = 9;
    break;
  }
  if (iconHandle != 0) {
    func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
    func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(iconHandle + 0x85));
  }
  if (D_80145AA8 == 0) {
    if (D_8018E228 != 10 && D_8018E228 != 8) {
      return;
    }
    timer = &phaseTimer;
    value = *timer - 1;
    *timer = value;
    if (value != 0) {
      return;
    }
  }
  phaseTimer = 0;
  if (D_8018E228 != 8 && D_8018E228 != 10) {
    func_8015DF18(0x104);
  }
  D_80148652 = nextSubstep;
}
