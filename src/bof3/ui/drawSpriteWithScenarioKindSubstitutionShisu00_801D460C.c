#include "bof3/bof3.h"

/* The shared main-RAM byte 0x80146870 is the first byte of the scenario state
 * record, declared as scenarioState at the same address in
 * include/bof3/ui/game00_internal.h, and lies outside this EMI payload, so this
 * target declares it locally at the original signed byte access width. */
extern s8 D_80146870; /* @source 0x80146870 @kind bss */

/* Sprite emitter of this EMI (unlifted): it stores the two coordinates as the
 * sprite halfwords at 0x8 and 0xA of the primitive at g_PrimCursor, scales the
 * kind byte by four, takes that entry of the four-byte-stride descriptor table
 * based at 0x801D9B78 for the texture coordinates and clut, sets a 0x28 by 0x30
 * size, picks the colour bytes from the mode byte (0 makes all three 0x80, 1
 * makes them 0x30, any other value gives 0x40 0x40 0x80) and selects the
 * drawing mode with GetGraphType. The widths below are the widths observed at
 * the call site, which widens every argument with andi 0xFFFF/0xFFFF/0xFF/0xFF,
 * not a recovered callee contract.
 * @source 0x801D44C0 */
extern void func_801D44C0(s32 x, s32 y, s32 kind, s32 mode);

/* @source 0x801D460C
 * @behavior Sprite wrapper of the SHISU overlay: when the kind byte equals 4
 *           and the shared scenario state byte 0x80146870, read with a signed
 *           byte load, is at least 8, it replaces that kind with 0xB. It then
 *           widens both unsigned 16-bit coordinates and both byte arguments and
 *           forwards them to the sprite emitter func_801D44C0 with no other
 *           work; its 0x18-byte frame only keeps $ra at 0x10 across that call.
 *           The span has exactly two references in this payload, the jal sites
 *           at 0x801D3844 inside func_801D3320 and at 0x801D7164 inside
 *           func_801D6090, and both pass the byte at +5 of a field record and a
 *           drawing mode byte. The same 0x54 bytes sit at 0x801D910C in
 *           emi/etc/shop/00 and at 0x801D3664 in emi/etc/bate/00 apart from the
 *           single jal word at +0x3C, whose target moves with each copy's own
 *           address delta to that payload's local emitter.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpriteWithScenarioKindSubstitutionShisu00_801D460C(u16 x, u16 y, u8 kind, u8 mode)
{
    if (kind == 4 && D_80146870 >= 8)
    {
        kind = 0xB;
    }
    func_801D44C0(x, y, kind, mode);
}
