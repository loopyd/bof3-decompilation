#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Graphics-type query of the running executable, owned by the SDK map at
 * 0x8017B2B4. This payload only calls it. */
extern s32 GetGraphType(void);

/* 16x16 sprite setter of the shared libgpu: stamps the tag and the three-word
 * sprite code of the primitive it is handed. The SDK map this tree links
 * against names no entry at 0x8017AA08, so the callee stays address-anchored. */
/* @source 0x8017AA08 */
extern void func_8017AA08(SPRT_16* primitive);

/* One CLUT column byte per commu entry icon ordinal: the high nibble is the
 * palette column the icon's CLUT sits in. The run at 0x801D4CD4 holds twelve
 * bytes, 0xA0 for ordinals 0, 1 and 5 and 0x90 for the other nine. */
extern u8 D_801D4CD4[]; /* @source 0x801D4CD4 @kind data */

/* Representation notes, each measured against this body's 77 instructions at
 * 308 bytes: separate remainder and quotient byte locals scored 60 of 77 at
 * the original size when the remainder re-divided the ordinal and 74 of 77 at
 * four bytes under when it subtracted a product of the quotient, either with
 * an s32 quotient or with a byte one, and a compound subtraction from a byte
 * copy of the ordinal scored 66 and 74 of 77 at four bytes under. The inline
 * remainder and quotient written below scored 77 of 77 at the original size. */

/* @source 0x801D3EFC
 * @behavior Emits one 16x16 kind-icon sprite of the commu01/07 entry list at
 *           (arg0, arg1) into the shared primitive cursor. Installs a draw-mode
 *           primitive carrying the graph-type texture page (0x2E when the first
 *           query answers 1, 0x2E when a second query answers 2, 0x1E
 *           otherwise) with dfe 0, dtd 0 and no texture window, appends it as
 *           0xC bytes, stamps a 16x16 sprite through the libgpu setter at
 *           0x8017AA08, and fills the position (arg0, arg1), the atlas cell,
 *           the CLUT that the high nibble of D_801D4CD4's byte for arg2 sets
 *           over 0x7840, and the neutral gray 0x80 on all three colour bytes,
 *           closed by a 0x10-byte append under ordering-table slot 1. The atlas
 *           cell is the icon ordinal split into decimal digits: u0 is the
 *           remainder times 16 plus 0x50 less the quotient times 0x30, and v0
 *           is the quotient times 16 less 0x20, so ordinals 0 through 9 sit in
 *           the texel row at 0xE0 and 10 and 11 in the row below it. It reads
 *           only the CLUT byte the ordinal selects and writes only the cursor.
 *           Its single in-image caller func_801D3038 walks the stride-8 active
 *           records at 0x801455C8 and passes each record kind's icon ordinal,
 *           D_801D4C34[kind] for every kind but 5 and 0x02 plus
 *           D_801457A9[record] for kind 5, at (x + 0x22, y + 0x6C); no jal and
 *           no pointer word outside the payload names this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawCommuEntryKindIconSprite(s16 arg0, s16 arg1, u8 arg2) {
    s32      page;
    SPRT_16* primitive;
    u8       ordinal;

    if (GetGraphType() == 1) {
        page = 0x2E;
    } else if (GetGraphType() == 2) {
        page = 0x2E;
    } else {
        page = 0x1E;
    }

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, page, 0);
    GpuAppendPrim(1, 0xC);

    primitive = (SPRT_16*)g_PrimCursor;
    func_8017AA08(primitive);

    primitive->x0 = arg0;
    primitive->y0 = arg1;
    ordinal = arg2;
    primitive->u0 = (ordinal % 10) * 16 + 0x50 - (ordinal / 10) * 0x30;
    primitive->v0 = (ordinal / 10) * 16 - 0x20;
    primitive->clut = (D_801D4CD4[ordinal] >> 4) | 0x7840;
    primitive->r0 = 0x80;
    primitive->g0 = 0x80;
    primitive->b0 = 0x80;
    GpuAppendPrim(1, 0x10);
}
