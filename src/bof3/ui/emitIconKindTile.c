#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Graphics-type query of the running executable, owned by the SDK map at
 * 0x8017B2B4. This payload only calls it. */
extern s32 GetGraphType(void);

/* One 4-byte entry of the icon tile table D_801D8B54, which sits at payload
 * offset 0x7F54 inside the reviewed leading data run of this overlay: the
 * texture byte pair that the emitter stores at 0xC and 0xD of its sprite packet
 * and the two palette bytes it folds into the clut word at 0xE, where clutX is
 * the palette column stored scaled by 16 (the emitter shifts it back down by 4)
 * and clutY the palette row offset added to 0x1E0 before being shifted up by 6.
 */
typedef struct
{
    u8 u;
    u8 v;
    u8 clutX;
    u8 clutY;
} IconTile;

/* Fixed-address icon tile table of this payload, indexed here by the third
 * argument of the emitter, the icon kind byte. The generated undefined_syms_auto
 * row already binds this word, so no map row is added for it.
 * @source 0x801D8B54 @kind data
 */
extern IconTile D_801D8B54[];

/* @source 0x801D3518
 * @behavior Icon sprite emitter of the BATE overlay, called with two
 *           coordinates, an icon kind byte and a mode byte: it takes the entry
 *           of the four-byte-stride icon tile table D_801D8B54 that the kind
 *           byte indexes, then picks the draw-mode texture page from
 *           GetGraphType, keeping 0x2E when the first query answers 1 and when
 *           a second query answers 2 and falling back to 0x1E otherwise. It
 *           arms SetDrawMode at g_PrimCursor with dfe 0, dtd 1 and no texture
 *           window, appends that 0xC-byte draw-mode
 *           packet at ordering-table slot 1, and initialises the 0x14-byte SPRT
 *           packet at the same cursor through SetSprt. It then picks the three
 *           colour bytes of the sprite from the mode byte (0 makes all three
 *           0x80, 1 makes them 0x30, any other value gives red 0x40, green 0x40
 *           and blue 0x80), stores the two coordinates at 0x8 and 0xA, copies
 *           the texture byte pair of the tile entry to 0xC and 0xD, fixes the
 *           size at 0x28 by 0x30, assembles the clut word at 0xE as
 *           ((clutY + 0x1E0) << 6) | (clutX >> 4) from the last two bytes of
 *           that entry, and appends the sprite at slot 1 as a 0x14-byte
 *           primitive. Both appends are func_8014E5A0 and the other callees are
 *           PsyQ helpers. The payload names this span only from the two jal
 *           sites 0x801D36A0 inside
 *           drawSpriteKindSubstitute and 0x801D833C
 *           inside renderThreeIconPanel, which pass the
 *           two coordinates, the sprite kind byte and the drawing mode byte. Its
 *           0x30-byte frame keeps $s0 through $s4 and $ra across those calls.
 *           One payload cmp proved its 0x14C bytes equal three twins apart from
 *           the tile table's own low word and the two internal jump words:
 *           0x801D8FC0 in emi/etc/shop/00, whose lift emitShopIconSprite
 *           measured exact and supplied this statement order, 0x801D8F8C in
 *           emi/etc/commu02/08 and 0x801D44C0 in emi/etc/shisu/00, both still
 *           asm rows. The two differing jump words move by exactly the address
 *           delta between the spans (0x5AA8, 0x5A74 and 0x0FA8, equal for both
 *           words of each twin) and the one remaining word is this span's own
 *           table binding 0x801D8B54, so the twins are shape evidence rather
 *           than a clone source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitIconKindTile(s32 x, s32 y, s32 kind, s32 mode)
{
    SPRT *sprite;
    IconTile *tile;
    u32 colorMode;
    s32 tpage;

    tile = &D_801D8B54[kind & 0xFF];

    if (GetGraphType() == 1)
    {
        tpage = 0x2E;
    }
    else if (GetGraphType() == 2)
    {
        tpage = 0x2E;
    }
    else
    {
        tpage = 0x1E;
    }

    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    GpuAppendPrim(1, 0xC);

    sprite = (SPRT *)g_PrimCursor;
    SetSprt(sprite);

    colorMode = mode & 0xFF;
    if (colorMode == 0)
    {
        sprite->b0 = 0x80;
        sprite->g0 = 0x80;
        sprite->r0 = 0x80;
    }
    else if (colorMode == 1)
    {
        sprite->b0 = 0x30;
        sprite->g0 = 0x30;
        sprite->r0 = 0x30;
    }
    else
    {
        sprite->g0 = 0x40;
        sprite->r0 = 0x40;
        sprite->b0 = 0x80;
    }

    sprite->x0 = x;
    sprite->y0 = y;
    sprite->u0 = tile->u;
    sprite->v0 = tile->v;
    sprite->clut = ((tile->clutY + 0x1E0) << 6) | (tile->clutX >> 4);
    sprite->w = 0x28;
    sprite->h = 0x30;
    GpuAppendPrim(1, 0x14);
}
