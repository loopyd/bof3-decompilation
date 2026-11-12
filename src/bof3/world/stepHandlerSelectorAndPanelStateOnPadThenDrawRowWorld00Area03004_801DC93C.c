#include "bof3/world/area03004_internal.h"

/* @source 0x801DC93C
 * @behavior AREA030 work-record step, entry 2 of the five-entry handler table
 * at 0x801E22D0 that func_801DC838 dispatches through byte 3 of the scratch
 * work record published at the scratchpad cursor 0x1F800044. It decodes the
 * shared pad state D_80145AA8 masked by 0xA000 through func_8014E640 and steps
 * the shared handler-selector byte D_80144286 over its 0..2 range: pad bit
 * 0x8000 decrements it, wrapping a negative byte to 2, and pad bit 0x2000
 * increments it, wrapping to 0 above 2, both with cue 0x100. Otherwise, while
 * a bit shared with the pressed mask D_80145AC4 is set, it submits
 * func_8015D49C with (0, 0, 0x14, 0x10), dispatches cue 0x106, steps the
 * shared state byte D_80144281 (3 or 4 to 5, 8 to 0xB, 0xE to 0xF), clears the
 * byte 0x8014428B and publishes 4 into the record dispatch byte 3. Otherwise,
 * while a bit shared with the confirm mask D_80145AC2 is set, it dispatches
 * cue 0x103 and steps that state byte per selector value: selector 0 turns 8
 * into 0xB and 0xE into 0xF, selector 1 turns 3 or 4 into 5 and 0xE into 0xF,
 * and selector 2 turns 3 or 4 into 5 and 8 into 0xB, each step with cue 0x102,
 * after which it increments the record dispatch byte 3. While that byte still
 * holds 2 it submits the main-executable VRAM halfword of the selector's low
 * two bits (0x800100A0, 0x800100C0 or 0x800100DE) offset by the 0x80010000
 * base to func_8014F800 as (0x1D, 0x14, 0, 0xFF, base + halfword) and submits
 * func_801647C4 with (selector * 48 + 0x58, 0x2E, 0).
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepHandlerSelectorAndPanelStateOnPadThenDrawRowWorld00Area03004_801DC93C(void) {
    u16* pad;
    u32 dir;
    u8* record;
    u8* selp;
    u8* state;

    pad = &D_80145AA8;
    dir = func_8014E640(*pad & 0xA000u);
    if (dir & 0x8000u) {
        func_8015DF18(0x100);
        selp = &D_80144286;
        *selp = *selp - 1;
        if ((s8)*selp < 0) {
            *selp = 2;
        }
    } else if (dir & 0x2000u) {
        func_8015DF18(0x100);
        selp = &D_80144286;
        *selp = *selp + 1;
        if (*selp >= 3) {
            *selp = 0;
        }
    } else if (*pad & D_80145AC4) {
        func_8015D49C(0, 0, 0x14, 0x10);
        func_8015DF18(0x106);
        state = &D_80144281;
        if (*state == 3 || *state == 4) {
            *state = 5;
        } else if (*state == 8) {
            *state = 0xB;
        } else if (*state == 0xE) {
            *state = 0xF;
        }
        D_8014428B = 0;
        record = D_1F800044;
        record[3] = 4;
    } else if (*pad & D_80145AC2) {
        func_8015DF18(0x103);
        if (D_80144286 == 0) {
            if (D_80144281 == 8) {
                func_8015DF18(0x102);
                D_80144281 = 0xB;
            } else if (D_80144281 == 0xE) {
                func_8015DF18(0x102);
                D_80144281 = 0xF;
            }
        } else if (D_80144286 == 1) {
            if (D_80144281 == 3 || D_80144281 == 4) {
                func_8015DF18(0x102);
                D_80144281 = 5;
            } else if (D_80144281 == 0xE) {
                func_8015DF18(0x102);
                D_80144281 = 0xF;
            }
        } else {
            if (D_80144281 == 3 || D_80144281 == 4) {
                func_8015DF18(0x102);
                D_80144281 = 5;
            } else if (D_80144281 == 8) {
                func_8015DF18(0x102);
                D_80144281 = 0xB;
            }
        }
        record = D_1F800044;
        record[3] = record[3] + 1;
    }

    if (D_1F800044[3] == 2) {
        switch (D_80144286 & 3) {
        case 0:
            func_8014F800(0x1Du, 0x14, 0, 0xFFu,
                          0x80010000u + (u32)D_800100A0);
            break;
        case 1:
            func_8014F800(0x1Du, 0x14, 0, 0xFFu,
                          0x80010000u + (u32)D_800100C0);
            break;
        case 2:
            func_8014F800(0x1Du, 0x14, 0, 0xFFu,
                          0x80010000u + (u32)D_800100DE);
            break;
        default:
            break;
        }
        func_801647C4((u16)(D_80144286 * 48u + 0x58u), 0x2Eu, 0);
    }
}
