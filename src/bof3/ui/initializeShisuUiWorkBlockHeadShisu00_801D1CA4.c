#include "bof3/bof3.h"

/* The shared code mapper of the main executable: its first argument selects a
 * table pair, its second is the byte code to search for in that pair's first
 * list and its third selects the search mode, and it answers with the byte the
 * parallel list holds at the code's index, or zero when the code is absent. The
 * two lists the mode-0 walk advances live in shared work RAM at 0x801C893C and
 * 0x801C8950 and hold no content in the executable image, so the mapper's role
 * beyond that search stays unreviewed. No payload of this overlay defines it,
 * so only its call is bound here.
 * @source 0x8016628C
 */
extern s32 func_8016628C(s32 table, s32 code, s32 mode);

/* The four leading bytes of the SHISU UI work block, the block based at
 * 0x801DA028 that the state initializer at 0x801D1D1C publishes as D_801483BC:
 * this function is their only writer in the payload and no instruction of the
 * payload reads them individually, so they leave the overlay as the head of the
 * published block.
 */
extern u8 D_801DA028; /* @source 0x801DA028 @kind data */
extern u8 D_801DA029; /* @source 0x801DA029 @kind data */
extern u8 D_801DA02A; /* @source 0x801DA02A @kind data */
extern u8 D_801DA02B; /* @source 0x801DA02B @kind data */

/* @source 0x801D1CA4
 * @behavior Overlay entry of the SHISU (masters) overlay and its first code: no
 *           jal and no table word of the payload names it, so the loader runs it
 *           ahead of the state initializer at 0x801D1D1C. It asks the shared
 *           mapper func_8016628C, in mode 0 and for table 0, which byte the
 *           codes 0x4D, 0x23, 0x24 and 0x56 map to, and stores the four answers
 *           in that order into the leading bytes of the UI work block at
 *           0x801DA028 through 0x801DA02B. Each store follows its own call, with
 *           the next call's two constant arguments already set ahead of it, and
 *           the 0x18-byte frame only keeps $ra at 0x10. The function takes no
 *           arguments and leaves the fourth mapper answer in $v0 because no
 *           store clobbers it; no word of the payload reads that answer, so the
 *           return stays undeclared.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeShisuUiWorkBlockHeadShisu00_801D1CA4(void)
{
    D_801DA028 = func_8016628C(0, 0x4D, 0);
    D_801DA029 = func_8016628C(0, 0x23, 0);
    D_801DA02A = func_8016628C(0, 0x24, 0);
    D_801DA02B = func_8016628C(0, 0x56, 0);
}
