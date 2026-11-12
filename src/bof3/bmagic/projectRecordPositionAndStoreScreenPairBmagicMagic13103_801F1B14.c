#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */
extern void func_8017AA30(void *primitive); /* @source 0x8017AA30 @kind function */

/* @source 0x801F1B14
 * @behavior Projects the work record position through the primitive at the
 * cursor and stores the projected screen pair back into the record. Reads the
 * record pointer cell at 0x801F24E0 once ahead of its 0x20-byte frame and
 * builds an eight-byte vector inside that frame: the record word at 0x14 moved
 * down nine bits and biased by -0x4000 becomes the halfword at stack offset
 * 0x10, the record word at 0x18 the same way the halfword at 0x12, and the
 * signed record halfword at 0x1E halved toward zero and negated the halfword at
 * 0x14. It then loads the primitive cursor at 0x8014598C, calls the shared
 * helper at 0x8017AA30 with that pointer, and runs RotTransPers on the vector,
 * passing the frame slot at 0x10 twice and the halfword slot at 0x14 as the
 * flag word. A second read of the cell at 0x801F24E0 supplies the
 * destinations: the primitive halfwords at +8 and +0xA reach the record
 * halfwords at 0x20 and 0x22. Called with no arguments by the record state
 * handlers at 0x801F1138 and 0x801F14A0 when record bytes 0x0 and 0x2 are both
 * non-zero; reads no other state and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectRecordPositionAndStoreScreenPairBmagicMagic13103_801F1B14(void) {
    SVECTOR position;
    u8 *prim;
    s32 *words;
    u16 *halves;

    words = (s32 *)workRecord801F24E0;
    halves = (u16 *)workRecord801F24E0;
    position.vx = (s16)(words[5] >> 9) - 0x4000;
    position.vy = (s16)(words[6] >> 9) - 0x4000;
    position.vz = (s16)(-((s16)halves[0xF] / 2));

    prim = g_PrimCursor;
    func_8017AA30(prim);
    RotTransPers(&position, (long *)(prim + 8), (long *)&position, (long *)&position.vz);

    halves = (u16 *)workRecord801F24E0;
    halves[0x10] = *(u16 *)(prim + 8);
    halves[0x11] = *(u16 *)(prim + 0xA);
}
