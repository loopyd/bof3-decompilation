#include "bof3/bof3.h"

/* Shared selector byte this overlay's front-end code branches on; it reads the
 * cell as the same absolute symbol the resident front-end code uses.
 * @source 0x80144F5A @kind data
 */
extern u8 D_80144F5A;
/* Shared mode halfword at 0x80145EC6, read both as the unsigned halfword the
 * first pattern test subtracts from and as the signed halfword the later tests
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
/* Shared sub-state byte and the shared primary-state byte after it.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;

/* Shared EXE-side cue dispatcher. */
void func_8015DF18(u32 arg0);

/* @source 0x801FC9CC
 * @behavior Front-end pattern trigger. It returns at once unless the shared
 *           selector byte D_80144F5A reads 0 or 3, and then dispatches cue 0x20B
 *           through func_8015DF18 and sets the shared sub-state byte D_80146875
 *           to 0x1E and the shared primary-state byte D_80146874 to 0x1A for
 *           each of three recorded four-cell patterns it matches: the unsigned
 *           halfword at 0x80145EC6 in 0x44..0x45 with the signed halfword at
 *           0x80145ECA = 0x3F, D_80145EDB = 0x42 and D_80145E98 = 1; the signed
 *           halfword at 0x80145EC6 = 0x46 with 0x80145ECA = 0x3E, D_80145EDB =
 *           0x45 and D_80145E98 = 7; and 0x80145EC6 = 0x44 with 0x80145ECA =
 *           0x3E, D_80145EDB = 0x43 and D_80145E98 = 3. The three tests run in
 *           that order and each ends at the same store pair, so the third
 *           pattern re-reads 0x80145EC6 after the first two have already
 *           rejected 0x44. Takes no arguments and returns nothing; the 0x18-byte
 *           frame only keeps $ra across the call and each shared-byte store
 *           materialises the 0x8014 page in $at on its own.
 *
 *           0x80144F5A is the shared selector byte the same overlay's
 *           publishWorkBaseAndSeedFrontEndRecordScenarioScena0200_801FCD14 reads
 *           as the 0x801FE26C table index. No image word names 0x801FC9CC; the
 *           only `jal` to it is at 0x801F7C18, inside func_801F7A9C, on the arm
 *           that func_8015B5D4(D_8014686C, 0x2E) selects, so the overlay reaches
 *           this body only through that handler.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue20BAndRequestPrimaryState1ASubstate1EOnModeCellMatchScenarioScena0200_801FC9CC(void) {
  if (D_80144F5A == 0 || D_80144F5A == 3) {
    if ((u16)(*(u16*)(D_80145EC6 + 0x0) - 0x44) < 2 &&
        *(s16*)(D_80145ECA + 0x0) == 0x3F &&
        D_80145EDB == 0x42 &&
        D_80145E98 == 1) {
      func_8015DF18(0x20B);
      D_80146875 = 0x1E;
      D_80146874 = 0x1A;
    }
    if (*(s16*)(D_80145EC6 + 0x0) == 0x46 &&
        *(s16*)(D_80145ECA + 0x0) == 0x3E &&
        D_80145EDB == 0x45 &&
        D_80145E98 == 7) {
      func_8015DF18(0x20B);
      D_80146875 = 0x1E;
      D_80146874 = 0x1A;
    }
    if (*(s16*)(D_80145EC6 + 0x0) == 0x44 &&
        *(s16*)(D_80145ECA + 0x0) == 0x3E &&
        D_80145EDB == 0x43 &&
        D_80145E98 == 3) {
      func_8015DF18(0x20B);
      D_80146875 = 0x1E;
      D_80146874 = 0x1A;
    }
  }
}
