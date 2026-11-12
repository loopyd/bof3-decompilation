#include "bof3/bof3.h"

/* Shared selector byte this overlay's front-end pattern code branches on; it is
 * read as the same absolute cell the resident front-end code uses.
 * @source 0x80144F5A @kind data
 */
extern u8 D_80144F5A;
/* Shared mode halfword, read both as the unsigned halfword the first pattern
 * test subtracts from and as the signed halfword the later pattern tests
 * compare, so it is declared as a byte array and each site casts the width it
 * needs. The identical signed halfword sits at 0x80145ECA.
 * @source 0x80145EC6 @kind data
 */
extern u8 D_80145EC6[];
/* @source 0x80145ECA @kind data */
extern u8 D_80145ECA[];
/* Third byte of the pattern this body matches; its role is not evidenced, so it
 * stays address-anchored like the world-overlay reader that keys a record on it.
 * @source 0x80145EDB @kind data
 */
extern u8 D_80145EDB;
/* Leading byte of the shared game-mode record the pattern tests read.
 * @source 0x80145E98 @kind data
 */
extern u8 D_80145E98;
/* Shared progress byte and the sequence of state bytes this body clears after
 * it, then the requested primary-state byte at the end of the run.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind data */
extern u8 D_80146865;
/* @source 0x80146866 @kind data */
extern u8 D_80146866;
/* @source 0x80146867 @kind data */
extern u8 D_80146867;
/* Shared sub-state byte and the shared primary-state byte after it.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;

/* Shared EXE-side front-end startup helper. */
void func_8015C088(void);

/* @source 0x801FC7E4
 * @behavior Front-end pattern trigger. It returns at once unless the shared
 *           selector byte D_80144F5A reads 0 or 3, and then runs the same
 *           action for each of three recorded four-cell patterns it matches:
 *           the unsigned halfword at 0x80145EC6 in 0x44..0x45 with the signed
 *           halfword at 0x80145ECA = 0x3F, D_80145EDB = 0x42 and D_80145E98 =
 *           1; the signed halfword at 0x80145EC6 = 0x46 with 0x80145ECA = 0x3E,
 *           D_80145EDB = 0x45 and D_80145E98 = 7; and 0x80145EC6 = 0x44 with
 *           0x80145ECA = 0x3E, D_80145EDB = 0x43 and D_80145E98 = 3. The
 *           action runs the shared front-end startup helper func_8015C088
 *           (0x8015C088), sets the shared progress byte g_ScenarioProgress
 *           (0x80146864) to 0x23, clears the shared bytes 0x80146865,
 *           0x80146866 and 0x80146867, clears the shared sub-state byte
 *           D_80146875 and then requests primary scene state 0x1A through
 *           D_80146874. The three tests run in that order and each ends at the
 *           same store run, so the third pattern re-reads 0x80145EC6 after the
 *           first two have already rejected 0x44. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra across the
 *           call, every shared-byte store materialises the 0x8014 page in $at
 *           on its own, and the constant 0x1A is materialised in $v0 straight
 *           after the call and survives the five clearing stores, so the store
 *           order 0x80146864, 0x80146865, 0x80146866, 0x80146867, D_80146875,
 *           D_80146874 is the source statement order. The three-test pattern
 *           walk is the one the exact same-target sibling
 *           queueCue20BAndRequestPrimaryState1ASubstate1EOnModeCellMatchScenarioScena0200_801FC9CC
 *           showed, and the six-statement store run is the one the exact
 *           same-target sibling
 *           clearProgressAndSharedStateBytesThenRequestPrimaryState19ScenarioScena0200_801FD5A8
 *           showed; this body writes its own literals. No image word names
 *           0x801FC7E4, so the overlay reaches this body only through the
 *           front-end handler table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runFrontEndStartupSetProgress23AndRequestPrimaryState1AOnModeCellMatchScenarioScena0200_801FC7E4(void) {
  if (D_80144F5A == 0 || D_80144F5A == 3) {
    if ((u16)(*(u16*)(D_80145EC6 + 0x0) - 0x44) < 2 &&
        *(s16*)(D_80145ECA + 0x0) == 0x3F &&
        D_80145EDB == 0x42 &&
        D_80145E98 == 1) {
      func_8015C088();
      g_ScenarioProgress = 0x23;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 0x1A;
    }
    if (*(s16*)(D_80145EC6 + 0x0) == 0x46 &&
        *(s16*)(D_80145ECA + 0x0) == 0x3E &&
        D_80145EDB == 0x45 &&
        D_80145E98 == 7) {
      func_8015C088();
      g_ScenarioProgress = 0x23;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 0x1A;
    }
    if (*(s16*)(D_80145EC6 + 0x0) == 0x44 &&
        *(s16*)(D_80145ECA + 0x0) == 0x3E &&
        D_80145EDB == 0x43 &&
        D_80145E98 == 3) {
      func_8015C088();
      g_ScenarioProgress = 0x23;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 0x1A;
    }
  }
}
