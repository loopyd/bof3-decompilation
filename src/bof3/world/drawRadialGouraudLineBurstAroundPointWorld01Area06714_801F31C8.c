#include "bof3/bof3.h"
#include "gpu/prim.h"

void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017AA94(void* arg0);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_801F3394(u8* point);

/* @source 0x801F31C8
 * @behavior Draws the eight-segment Gouraud line burst of one screen-space
 *           record. It first sets the 0x380/0x100 texture page draw mode at the
 *           current primitive cursor and links that 0xC-byte packet through the
 *           shared grid-cell linker func_80155A08 with the target-local word
 *           pair at 0x801F53A0/0x801F53A4 as its two coordinates. Then, for
 *           eight phase steps 0x200 apart counted from the record halfword at
 *           +0x6, it emits one 0x14-byte LINE_G2 per step through the same
 *           linker: the line starts at the record point (+0x0 and +0x2
 *           halfwords) and ends at that point plus the +0x4 reach halfword
 *           scaled by the fixed-point x helper func_801783C8 and y helper
 *           func_801782FC of the phase, each product shifted by (i & 1) + 0xC,
 *           and the packet takes 0x80 for the base red, green and blue bytes
 *           and zero for the tip ones. Afterwards it hands the record to the
 *           overlay's next draw stage func_801F3394 and restores the same
 *           texture page draw mode with its own 0xC-byte linked packet.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRadialGouraudLineBurstAroundPointWorld01Area06714_801F31C8(u8* point) {
  u8* line;
  u16 phase;
  u8 i;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x380, 0x100), 0);
  func_80155A08(PSX_REF(s32, 0x801F53A0u), PSX_REF(s32, 0x801F53A4u), 4, 0xC);

  i = 0;
  do {
    phase = FIELD_REF(u16, point, 0x6u) + (i << 9);
    line = g_PrimCursor;
    func_8017AA94(line);
    SetSemiTrans(line, 1);
    FIELD_REF(u16, line, 0x8u) = FIELD_REF(u16, point, 0x0u);
    FIELD_REF(u16, line, 0xAu) = FIELD_REF(u16, point, 0x2u);
    FIELD_REF(u16, line, 0x10u) =
        FIELD_REF(u16, point, 0x0u) +
        ((FIELD_REF(s16, point, 0x4u) * func_801783C8(phase)) >> ((i & 1) + 0xC));
    FIELD_REF(u16, line, 0x12u) =
        FIELD_REF(u16, point, 0x2u) +
        ((FIELD_REF(s16, point, 0x4u) * func_801782FC(phase)) >> ((i & 1) + 0xC));
    FIELD_REF(u8, line, 0x4u) = 0x80;
    FIELD_REF(u8, line, 0x5u) = 0x80;
    FIELD_REF(u8, line, 0x6u) = 0x80;
    FIELD_REF(u8, line, 0xCu) = 0;
    FIELD_REF(u8, line, 0xDu) = 0;
    FIELD_REF(u8, line, 0xEu) = 0;
    func_80155A08(PSX_REF(s32, 0x801F53A0u), PSX_REF(s32, 0x801F53A4u), 4, 0x14);
    i++;
  } while (i < 8);

  func_801F3394(point);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x380, 0x100), 0);
  func_80155A08(PSX_REF(s32, 0x801F53A0u), PSX_REF(s32, 0x801F53A4u), 4, 0xC);
}
