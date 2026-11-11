#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

typedef void (*BmagicMagic07603StepHandler)(void);

typedef struct BmagicMagic07603StepHandlerTable {
    BmagicMagic07603StepHandler handlers[3];
} BmagicMagic07603StepHandlerTable;

/* The three code pointers at 0x801EEC1C sit inside the overlay's leading
 * pointer run at 0x801EEC04: entry 0 = 0x801EFA60, entry 1 = 0x801EFC30,
 * entry 2 = 0x801EFD60, the three step handlers this function dispatches.
 * This declaration covers only the three words the body copies. */
extern BmagicMagic07603StepHandlerTable D_801EEC1C; /* @source 0x801EEC1C @kind data */

/* Shared graphics-type query that selects the two draw-mode pages. */
s32 GetGraphType(void);

/* Shared primitive linker: links a primitive at the primitive cursor from the
 * work object's word pair 0x34/0x38 with the given kind and size class. */
void func_80155A08(s32 x, s32 y, s32 kind, s32 size);

/* Shared helper called while the work object is active, followed by this
 * overlay's own two draw passes. */
void func_800AF764(void);
void func_801EFEA8(void);
void drawTranslucentGouraudRingOfEightQuadsBetweenRadiiTwentyAndEightTintedByWorkByte9TimesSignedBytes5d5e5fBmagicMagic07603_801F00FC(void);

/* @source 0x801EF8EC
 * @behavior Entry 0 of the overlay's work-byte-1 table at 0x801F04A0 and one
 *           step handler: the frame copy of the three code pointers at
 *           0x801EEC1C (0x801EFA60, 0x801EFC30, 0x801EFD60) is indexed by
 *           unsigned work byte 2 of the scratchpad work object and that step
 *           handler is called with no arguments. It then opens a 12-byte
 *           draw-mode packet on the primitive cursor with dfe 0, dtd 1 and page
 *           0xA5 when the successive graphics-type queries return 1 or 2, page
 *           0x35 otherwise, and links it from work words 0x34 and 0x38 with
 *           kind 2. The work pointer is read again; when work bytes 0 and 2 are
 *           both nonzero it calls 0x800AF764 and then this overlay's draw
 *           passes 0x801EFEA8 and 0x801F00FC. Finally it opens and links a
 *           second draw-mode packet with page 0x25 for graphics types 1 or 2
 *           and 0x15 otherwise. Each graphics-type test keeps its own call.
 *           Takes no arguments and returns nothing; the work pointer is re-read
 *           after calls that can replace it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawBetweenGraphTypeDrawModesAroundHelpersWhenWorkBytes0And2SetBmagicMagic07603_801EF8EC(void) {
    BmagicMagic07603StepHandlerTable handlers;
    s32 mode;

    handlers = D_801EEC1C;
    handlers.handlers[g_battle_work[2]]();
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        mode = 0xA5;
    } else {
        mode = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                  0xC);
    if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
        func_800AF764();
        func_801EFEA8();
        drawTranslucentGouraudRingOfEightQuadsBetweenRadiiTwentyAndEightTintedByWorkByte9TimesSignedBytes5d5e5fBmagicMagic07603_801F00FC();
    }
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        mode = 0x25;
    } else {
        mode = 0x15;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                  0xC);
}
