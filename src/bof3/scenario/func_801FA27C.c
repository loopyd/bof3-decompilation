#include "bof3/scenario/scena00_internal.h"

/* @behavior Runs the SCENA00 progress controller selected by the state byte
 * D_80146875, whose eleven states chain the front-end cues, effect-bank setup
 * and fixed-point ramps this scenario needs: state 0, while the gate word
 * D_80143C40 is clear, queues front-end mode 1 through func_80150224, arms
 * D_80143BB0 = 2 and advances to state 1; state 1 advances to state 2 once that
 * mode byte leaves 2; state 2 waits for scenario progress byte 3 and state 3
 * for progress byte 4, each queueing func_80150224(2)/func_80150224(3) and
 * arming D_80143BB0 = 2, with state 3 also loading the 0x130-frame countdown
 * D_80146876; state 4 counts that countdown down, and while it still runs
 * publishes the ramp ((0x130 - remaining) * 0x57943) >> 16 in D_8014932C
 * before advancing to state 5 when it reaches zero; state 5, on progress byte
 * 5, stores the object byte func_8019601C() in scratchpad 0x1F800000 and seeds
 * that object's 0x74-byte row at 0x80143FC8 (+0 = 1, +5 = 0x13, +9 = 0x60, the
 * word -0x34A at +0x64 and the s16 words D_801492DA/D_801492DC at +0x68/+0x6C)
 * unless that byte is 0xFF, then advances to state 6; state 6 repeats that
 * seeding on progress byte 8 (+9 = 0x20, -0x2AC at +0x64), arms the 0x200-frame
 * countdown and advances to state 7; state 7 counts that countdown down,
 * publishing (remaining * 0xD) >> 2 in D_8014932C while it runs, and advances
 * to state 8 at zero; state 8, on progress byte 0xB, runs func_8014ECAC(0) and
 * advances to state 9; state 9, while D_80143C40 is clear, clears the effect
 * bank D_8014832E, queues func_80150224(4), arms D_80143BB0 = 2 and queues the
 * front-end cue func_80161CD0(2, 0x64, 0x20) before advancing to state 10;
 * state 10, once D_80143BB0 leaves 2, seeds the front selector context
 * (0x18, 0x630000, 0xC0000, 3), requests primary state 5 in D_80146874 and
 * returns the state byte to 0.
 * @source 0x801FA27C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801FA27C(void) {
    switch (D_80146875) {
    case 0:
        if (D_80143C40 != 0u) {
            break;
        }
        func_80150224(1u);
        D_80143BB0 = 2u;
        D_80146875 = 1u;
        break;

    case 1:
        if (D_80143BB0 == 2u) {
            break;
        }
        D_80146875 = 2u;
        break;

    case 2:
        if (*(u8 *)&g_ScenarioProgress != 3u) {
            break;
        }
        func_80150224(2u);
        D_80143BB0 = 2u;
        D_80146875 = 3u;
        break;

    case 3:
        if (*(u8 *)&g_ScenarioProgress != 4u) {
            break;
        }
        func_80150224(3u);
        D_80143BB0 = 2u;
        D_80146876 = 0x130u;
        D_80146875 = 4u;
        break;

    case 4: {
        u16 *counter = (u16 *)&D_80146876;

        if (*counter != 0u) {
            u16 next = (u16)(*counter - 1u);

            *counter = next;
            D_8014932C = (u16)(((0x130u - (u32)next) * 0x57943u) >> 16);
            break;
        }

        D_80146875 = 5u;
        break;
    }

    case 5: {
        u8 *scratch;
        u8  object_index;

        if (*(u8 *)&g_ScenarioProgress != 5u) {
            break;
        }

        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xffu) {
            D_80143FC8[(u32)object_index * 0x74u] = 1u;
            D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
            *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) = -0x34a;
            *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
            *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
            D_80143FD1[(u32)*scratch * 0x74u] = 0x60u;
        }

        D_80146875 = 6u;
        break;
    }

    case 6: {
        u8 *scratch;
        u8  object_index;

        if (*(u8 *)&g_ScenarioProgress != 8u) {
            break;
        }

        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xffu) {
            D_80143FC8[(u32)object_index * 0x74u] = 1u;
            D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
            *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) = -0x2ac;
            *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
            *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
            D_80143FD1[(u32)*scratch * 0x74u] = 0x20u;
        }

        D_80146876 = 0x200u;
        D_80146875 = 7u;
        break;
    }

    case 7: {
        u16 *counter = (u16 *)&D_80146876;
        u32  count = (u32)*counter;

        if (count != 0u) {
            u16 next = (u16)(*counter - 1u);

            D_8014932C = (u16)((count * 0xdu) >> 2);
            *counter = next;
            break;
        }

        D_80146875 = 8u;
        break;
    }

    case 8:
        if (*(u8 *)&g_ScenarioProgress != 0xbu) {
            break;
        }
        func_8014ECAC(0u);
        D_80146875 = 9u;
        break;

    case 9:
        if (D_80143C40 != 0u) {
            break;
        }
        D_8014832E = 0u;
        func_80150224(4u);
        D_80143BB0 = 2u;
        func_80161CD0(2u, 0x64, 0x20);
        D_80146875 = 0xau;
        break;

    case 10:
        if (D_80143BB0 == 2u) {
            break;
        }
        func_8019FA28(0x18u, 0x630000u, 0xc0000u, 3u);
        D_80146874 = 5u;
        D_80146875 = 0u;
        break;
    }
}
