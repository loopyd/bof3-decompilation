#include "bof3/bof3.h"

/* Shared sub-state byte of the main executable that selects which of the two
 * steps below runs.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

/* Shared front-end mode byte: both steps run only while it reads other than 2,
 * and step 0 latches it to 2 when it finishes.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared selector byte written by the menu overlay (0 through 5 in the
 * emi/etc/shisu/00 payload); it picks the (entry, stream) name pair step 0
 * resolves.
 * @source 0x80144E5E @kind unknown
 */
extern u8 D_80144E5E;

/* Shared twelve-byte name buffer the resolved name is copied into.
 * @source 0x801490D8 @kind data
 */
extern u8 D_801490D8[];

/* Terminator byte directly after the twelve bytes of the name buffer above.
 * @source 0x801490E4 @kind unknown
 */
extern u8 D_801490E4;

/* Neighbouring shared byte cleared together with the terminator; its role is
 * not established here.
 * @source 0x80149100 @kind unknown
 */
extern u8 D_80149100;

/* Shared flag bank the step-1 arm sets bit 0x98 of.
 * @source 0x80144F28 @kind unknown
 */
extern u8 D_80144F28;

/* Two shared bytes the step-1 arm clears.
 * @source 0x801448EB @kind unknown
 */
extern u8 D_801448EB;

/* @source 0x801448ED @kind unknown */
extern u8 D_801448ED;

/* Local name helper of the shared EXE image, outside this payload: it answers a
 * pointer to the name selected by its two whole-argument word and byte.
 */
extern u8* func_80165D48(s32 arg0, u8 arg1);

/* Shared cue gate of the shared EXE image, outside this payload: four unsigned
 * bytes in, an unsigned byte answer this body discards.
 */
extern u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);

/* Shared action starter of the shared EXE image, outside this payload. */
extern void func_80150224(s32 arg0);

/* Shared sound cue dispatcher of the shared EXE image, outside this payload. */
extern void func_8015DF18(u16 arg0);

/* Shared flag setter of the shared EXE image, outside this payload: it takes
 * the flag bank and the bit index to set.
 */
extern void func_8015B580(void* arg0, u8 bit_index);

/* Shared release helper of the shared EXE image, outside this payload. */
extern void func_8015C058(void);

extern char* strncpy(char* dest, const char* src, int count);

/* @source 0x801F3CD0
 * @behavior Overlay step handler of the shared sub-state byte D_80146875,
 *           reached through the loader: no `jal` and no table word in this
 *           payload or in the main executable names it. Sub-state 0 runs only
 *           while the shared front-end mode byte D_80143BB0 reads other than 2:
 *           it reads the shared selector byte D_80144E5E once and dispatches it
 *           through the six-entry absolute jump table at 0x801F2C04, an arm per
 *           selector value 0 through 5, and each arm resolves the (entry,
 *           stream) pair its value carries - 0 with 0xE, 0 with 7, 0 with 8,
 *           2 with 0x15, 1 with 0x11 and 3 with 0x19 - through the shared name
 *           helper func_80165D48, copies that answer's twelve bytes into the
 *           shared name buffer D_801490D8 through strncpy, and passes the same
 *           pair to the shared cue gate func_801650B4 with 1 and 0. Selector
 *           values 6 and above take no arm and continue at the same block,
 *           which clears the name buffer's terminator byte D_801490E4 and the
 *           neighbouring shared byte D_80149100, starts action 0x49 through
 *           func_80150224, latches the front-end mode byte to 2 and advances
 *           the sub-state byte to 1. Sub-state 1 also runs only while that
 *           mode reads other than 2: it queues sound cue 0x106 through
 *           func_8015DF18, sets bit 0x98 of the shared flag bank D_80144F28
 *           through func_8015B580, runs the shared release helper func_8015C058
 *           and clears the two shared bytes D_801448EB and D_801448ED. Any
 *           other sub-state value returns without touching memory. The span
 *           takes no arguments, returns nothing, and its 0x18-byte frame
 *           keeps only $ra across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedEquipmentObjectNameFromSelectorStartAction49ThenQueueCue106OnFrontEndModeReleaseWorld03Area13013_801F3CD0(
    void) {
  u8 state;
  u8 selector;

  state = D_80146875;
  switch (state) {
    case 0:
      if (D_80143BB0 == 2) {
        return;
      }
      selector = D_80144E5E;
      switch (selector) {
        case 0:
          strncpy((char*)D_801490D8, (char*)func_80165D48(0, 0xE), 0xC);
          func_801650B4(0, 0xE, 1, 0);
          break;
        case 1:
          strncpy((char*)D_801490D8, (char*)func_80165D48(0, 7), 0xC);
          func_801650B4(0, 7, 1, 0);
          break;
        case 2:
          strncpy((char*)D_801490D8, (char*)func_80165D48(0, 8), 0xC);
          func_801650B4(0, 8, 1, 0);
          break;
        case 3:
          strncpy((char*)D_801490D8, (char*)func_80165D48(2, 0x15), 0xC);
          func_801650B4(2, 0x15, 1, 0);
          break;
        case 4:
          strncpy((char*)D_801490D8, (char*)func_80165D48(1, 0x11), 0xC);
          func_801650B4(1, 0x11, 1, 0);
          break;
        case 5:
          strncpy((char*)D_801490D8, (char*)func_80165D48(3, 0x19), 0xC);
          func_801650B4(3, 0x19, 1, 0);
          break;
      }
      D_801490E4 = 0;
      D_80149100 = 0;
      func_80150224(0x49);
      D_80143BB0 = 2;
      D_80146875 = 1;
      break;
    case 1:
      if (D_80143BB0 == 2) {
        return;
      }
      func_8015DF18(0x106);
      func_8015B580(&D_80144F28, 0x98);
      func_8015C058();
      D_801448EB = 0;
      D_801448ED = 0;
      break;
  }
}
