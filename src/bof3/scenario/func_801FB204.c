#include "bof3/scenario/scena00_internal.h"

/* @behavior Runs the SCENA00 route-entry controller selected by the state byte
 * D_80146875: state 0 stores 10 in the scenario progress byte, runs
 * func_801BE1B0(1) and func_801C601C(1) and advances to state 1; state 1 waits
 * for progress byte 0xD, then passes the D_80145EC4/D_80145EC8 pair with 7 to
 * func_801BFE34 and runs func_801C7A54(7), advancing to state 2; state 2 is
 * idle; state 3 raises flag 0x180 in the mode word D_80146258, queues
 * func_80150224(0xE), arms D_80143BB0 = 2 and advances to state 4; state 4,
 * once D_80143BB0 leaves 2, stores progress byte 0x1E, arms the effect bank
 * D_8014832E = 0x1F, runs func_8014ECAC(3), rebases the D_80143F7C/D_80149308
 * pair to 0x2F0000 and the D_80143F80/D_8014930C pair to 0x1D0000, runs
 * func_80153DE8, commits D_80143F84 = 0x5800000, runs func_80154FD8(0x580) and
 * seeds D_801492D8 = -0x30A, D_801492DC = 0x180, D_8014932C = 0x480 and the
 * 0x1E-frame countdown D_80146876 before advancing to state 5; state 5, while
 * the gate word D_80143C40 is clear, decrements that countdown and, at zero,
 * reruns func_801C601C(2), recommits D_80143F84 = 0x5800000, runs
 * func_80154FD8 with the re-read high halfword of that committed word, stores the object byte func_8019601C() in
 * scratchpad 0x1F800000 and seeds that object's 0x74-byte row at 0x80143FC8
 * (+0 = 1, +5 = 0x13, word -0x2AA at +0x64, lh D_801492DA at +0x68, word 0x200
 * at +0x6C, +9 = 0x20) unless that byte is 0xFF, then reloads the countdown
 * with 0x20 and advances to state 6; state 6 counts that countdown down while
 * subtracting 0x24 from the fixed-point channel D_8014932C and, at zero, arms
 * state 7 before incrementing the scenario progress byte; state 7, on progress
 * byte 0x24, queues the frontend cue func_80161C20(4, 0x69, 8) and advances to
 * state 8; state 8, on progress byte 0x25, seeds the front selector context
 * through func_8019FA28(0x1F, 0x40000, 0x240000, 1) and clears D_80146874 and
 * D_80146875.
 * @source 0x801FB204
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801FB204(void) {
    switch (D_80146875) {
    case 0:
        *(u8 *)&g_ScenarioProgress = 0xau;
        func_801BE1B0(1u);
        func_801C601C(1u);
        D_80146875 = 1u;
        break;

    case 1:
        if (*(u8 *)&g_ScenarioProgress != 0xdu) {
            break;
        }
        func_801BFE34(D_80145EC4, D_80145EC8, 7);
        func_801C7A54(7);
        D_80146875 = 2u;
        break;

    case 2:
        break;

    case 3:
        D_80146258 |= 0x180u;
        func_80150224(0xeu);
        D_80143BB0 = 2u;
        D_80146875 = 4u;
        break;

    case 4:
        if (D_80143BB0 == 2u) {
            break;
        }
        *(u8 *)&g_ScenarioProgress = 0x1eu;
        D_8014832E = 0x1fu;
        func_8014ECAC(3u);
        D_80143F7C = 0x2f0000;
        D_80149308 = 0x2f0000u;
        D_80143F80 = 0x1d0000;
        D_8014930C = 0x1d0000;
        func_80153DE8();
        D_80143F84 = 0x5800000u;
        func_80154FD8(0x580u);
        /* Signed immediate, stored through the shared halfword world-x channel;
         * a u16-typed store would materialise 0xFCF6 instead of -0x30A. */
        *(s16 *)&D_801492D8 = -0x30a;
        D_801492DC = 0x180u;
        D_8014932C = 0x480u;
        D_80146876 = 0x1eu;
        D_80146875 = 5u;
        break;

    case 5: {
        u16 *counter;
        u8  *scratch;
        u8   object_index;

        if (D_80143C40 != 0u) {
            break;
        }

        counter = (u16 *)&D_80146876;

        if (--*counter != 0u) {
            break;
        }

        func_801C601C(2u);
        D_80143F84 = 0x5800000u;
        /* The original re-reads the committed word's high halfword (0x80143F86)
         * through this same-object byte view, which keeps the frame-scale read
         * after the store; a second symbol name let gcc hoist it ahead. */
        func_80154FD8(*(s16 *)((u8 *)&D_80143F84 + 2u));
        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xffu) {
            D_80143FC8[(u32)object_index * 0x74u] = 1u;
            D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
            *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) = -0x2aa;
            *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
            *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
            D_80143FD1[(u32)*scratch * 0x74u] = 0x20u;
        }

        *counter = 0x20u;
        D_80146875 = 6u;
        break;
    }

    case 6: {
        u16 *counter = (u16 *)&D_80146876;

        if (*counter != 0u) {
            u16 next = (u16)(*counter - 1u);

            *counter = next;
            D_8014932C = (u16)(D_8014932C - 0x24u);
            break;
        }

        D_80146875 = 7u;
        *(u8 *)&g_ScenarioProgress += 1;
        break;
    }

    case 7:
        if (*(u8 *)&g_ScenarioProgress != 0x24u) {
            break;
        }
        func_80161C20(4u, 0x69, 8);
        D_80146875 = 8u;
        break;

    case 8:
        if (*(u8 *)&g_ScenarioProgress != 0x25u) {
            break;
        }
        func_8019FA28(0x1fu, 0x40000u, 0x240000u, 1u);
        D_80146874 = 0u;
        D_80146875 = 0u;
        break;
    }
}
