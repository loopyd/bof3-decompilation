#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 *D_8014637C; /* @source 0x8014637C @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801F0240
 * @behavior Decrements scratchpad work byte 9 modulo 256. On zero, copies
 * byte 8 and words 0x34, 0x38 and 0x3C from the record at 0x8014637C,
 * sets work words 0xC and 0x10 to 0x8000 and zero, and passes the work
 * pointer to 0x801DD350. Adds words 0xC and 0x10 to words 0x34 and 0x38,
 * and adds 0x800000 to word 0x3C. Calls 0x8017A138 with the differences
 * between the record at 0x801EB4E0 and work words 0x34 and 0x38; clears
 * byte 9, stores the returned heading in word 0xC and advances byte 2.
 * Entry 0 of the work-byte-2 table at 0x801F1288. Takes no arguments and
 * returns nothing. Record identities and other field roles remain unknown.
 * @status partial
 * @match 54.43
 * @residual 43 of 79 instructions; 296 bytes versus 316. The source-record
 * pointer is reused across word stores where the original reloads it.
 * Post-helper register allocation and heading-load order also differ.
 */
void decrementWorkByte9ThenSeedFrom8014637CStepPositionSetHeadingToward801EB4E0AndAdvanceWorkByte2BmagicMagic12603_801F0240(void) {
    u8 *work;
    u8 *position;
    u8 *heading_work;
    s32 heading;

    if (--D_1F800044[9] == 0) {
        D_1F800044[8] = D_8014637C[8];
        work = D_1F800044;
        *(u32 *)(work + 0x34) = *(u32 *)(D_8014637C + 0x34);
        *(u32 *)(work + 0x38) = *(u32 *)(D_8014637C + 0x38);
        *(u32 *)(work + 0x3C) = *(u32 *)(D_8014637C + 0x3C);
        *(s32 *)(work + 0xC) = 0x8000;
        *(s32 *)(work + 0x10) = 0;
        func_801DD350(work);
        position = D_1F800044;
        *(s32 *)(position + 0x34) += *(s32 *)(position + 0xC);
        *(s32 *)(position + 0x38) += *(s32 *)(position + 0x10);
        *(s32 *)(position + 0x3C) += 0x800000;
        heading = func_8017A138(
            *(s32 *)(D_801EB4E0 + 0x34) - *(s32 *)(position + 0x34),
            *(s32 *)(D_801EB4E0 + 0x38) - *(s32 *)(position + 0x38));
        heading_work = D_1F800044;
        heading_work[9] = 0;
        *(s32 *)(heading_work + 0xC) = heading;
        D_1F800044[2]++;
    }
}
