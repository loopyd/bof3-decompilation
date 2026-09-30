#include "bof3/scenario/scena00_internal.h"

/* @behavior Runs the third SCENA00 controller selected by the state byte
 * D_80146875, whose eighteen states drive the mid-scenario camera/effect
 * sequence. While the state byte sits in 4..6 and the shared frame word
 * D_80143E6C has its low nibble clear, it reseeds D_80146867 with 1 << (rand()
 * % 3) and, when that byte leaves 0/1, bumps the shared mode byte D_80146866 by
 * 0x10; state 0 raises flag 0x80 in the mode word D_80146258, runs
 * func_8015B580(D_8014686C, 0..3), arms D_80146876 = 0x90 and advances to state
 * 1; states 1 and 2 count D_80146876 down while publishing (count * 8) in
 * D_8014932C and, at zero, store 2 in D_80149333, clear D_8014932C and advance
 * to state 3; state 3, on progress byte 0xC, arms D_80146876 = 0x3C, clears the
 * mode byte and advances to state 4; state 4 counts that word down, and once it
 * expires stores state 5, increments the scenario progress byte and then, while
 * the mode byte is non-zero, decrements it and steps D_80147A96 by the signed
 * D_801FCA4C nibble entry (frames D_80143E6C & 0xF) times 4; state 5 first
 * re-arms the countdown and advances to state 6 on progress byte 0x11 and then
 * repeats that mode-byte decrement/D_80147A96 step; state 6 counts down and, at
 * zero, stores the object byte func_8019601C() in scratchpad 0x1F800000 and,
 * unless that byte is 0xFF, seeds that object's 0x74-byte row at 0x80143FC8
 * (+0 = 1, +5 = 0x1E, the D_80145EC4/D_80145EC8 word pair at +0x34/+0x38, the
 * func_8015477C distance of that pair scaled by 0x10000 at +0x3C and 0 at
 * +0x29), then reloads the countdown with 0x84, advances the progress byte and
 * requests state 7; state 7 counts down and, at exactly 0x20, runs
 * func_8014ECAC(8) and advances to state 8; state 8 counts down, clearing the
 * effect bank D_8014832E while the gate word D_80143C40 is clear, and at zero
 * sets progress byte 0x14 and reloads 0x40 frames before requesting state 10;
 * state 9, on progress byte 0x14, runs func_80164020(0, 1) and arms 0x25C
 * frames before requesting state 10; state 10 counts down and, at zero, runs
 * func_8015B580(D_8014686C, 4), seeds the front selector context
 * (0x18, 0x630000, 0x110000, 3) through func_8019FA28, stores 9/0xFF in
 * D_80143F1D/D_80143F1E and advances to state 11; state 11, while the gate word
 * D_80143C40 is clear, stores progress byte 1 and advances to state 12; state
 * 12, on progress byte 4 once func_80162D00 reports ready, queues the frontend
 * cue func_80161C20(3, 0x69, 8) and advances to state 13; state 13, on progress
 * byte 5, arms 0x20 frames and advances to state 14; state 14 counts down while
 * adding the signed D_801FCA4C nibble entry (frames D_80143E6C & 0xF) shifted
 * by 11 to D_801469EC and subtracting it from D_801469F0, advancing to state 15
 * at zero; state 15, on progress byte 7, passes the D_80145EC4/D_80145EC8 pair
 * with 1 to func_801BFE34, runs func_801C7A54(1) and func_8015B580(D_8014686C,
 * 5) before advancing to state 16; state 16 is not handled (its jump-table
 * slot is the switch default) and state 17 increments the scenario progress
 * byte, runs func_8015C058 and func_801BE1B0(6) and clears D_80146874 and
 * D_80146875.
 * @source 0x801FA7D8
 * @status partial
 * @match 94.64
 * @residual Instruction count and size agree with the original (485/485
 * instructions, 1940/1940 bytes) and 459/485 instructions match, but 26
 * instructions differ: eight are the stack-frame setup/teardown words (the
 * frame is 0x30 instead of 0x20) and eighteen are placed or allocated
 * differently in four regions
 * (case 4/5's shared D_80146866/D_80147A96 tail: the original materialises
 * `u8 *` and `u16 *` addresses where this source folds them, and the original
 * reads the D_801FCA4C step with `lbu` + a register sign extension where this
 * source folds it into a signed byte load; case 6's tail; the case 11/12 state
 * handoffs; case 14's ramp). Shape evidence: measured with the mission-local
 * normalised .text diff (same instruction multiset per case).
 *
 * Live gate state: bin/asm-diff and bin/byte-match both abort for this selector
 * with `reviewed .rodata placement for func_801FA7D8 does not match original
 * bytes`, so no diff or byte verdict is available while the generated jump
 * table differs from the original 0x48 bytes at 0x801F6C58. Two of its
 * eighteen entries are off, each by one instruction placed in a different
 * basic block: entry 5 (case 5, +4) because the D_80147A96 pointer is
 * materialised in the case-4 counter-load delay slot instead of the case-4
 * tail, and entry 9 (case 9, -4) because case 8's `li v0,20` is sunk into its
 * `beqz` delay slot. The same request minus the placement assertion reports
 * 422/485 (87.01%).
 *
 * Measured clean-C shapes that did not move the residual: s32/u32/u16/s16/u8/s8
 * step locals, inline and `(u8)`-cast step expressions, `shift` spelling, `u8 *`
 * / `u16 *` pointer locals for D_80146866 and D_80147A96, `u32 *` and `u8 *`
 * views of the record words, struct-typed record access, hoisted index locals,
 * argument locals for func_8015477C, both statement orders for the case 4/5
 * tail, and statement-order permutations of case 6's seeding block. The frame
 * residual is 16 bytes of reserved-but-unused stack: gcc reserves one outgoing
 * argument slot pair for the two D_80143FFC/D_80144000 memory arguments of the
 * func_8015477C call in case 6, and no clean-C argument spelling tried
 * (locals, casts, pointer views, prototype variants) removes it. Confirmed
 * since: the frame is reserved as `vars` with no stack access anywhere in the
 * object; literal arguments to that call give vars=0, and hoisting the byte
 * offset (`u32 off = (u32)*scratch * 0x74u;`) gives vars=0/frame 0x20 but adds
 * a callee-saved register and loses 20 instructions (480 total), so it is not
 * the original shape either.
 *
 * The case 9 `D_80146875 = 0xa` store belongs inside the progress test: the
 * original's `bne` at 0x801FAD00 targets the epilogue, so progress != 0x14
 * returns without storing. Moving the store inside the `if` reproduces that
 * control flow and gives case 9 the original's 15 instructions, but the live
 * aligned match then drops to 393/485 because case 8 stays one instruction
 * short (20 vs 21) and every later case shifts by 4 bytes; the unconditional
 * store is therefore retained here until case 8 is settled. The case 4/5 step
 * load also stays `lb`+`sll 2` for every spelling tried (s32/u32/u8 step
 * locals, inline cast, nested declaration); the original's
 * `lbu`+`sll 0x18`+`sra 0x16` folds the byte sign extension into the load, a
 * combine difference rather than a source shape, and it is the sole reason
 * case 5 has 30 instructions instead of 31.
 * Next untried rung: the opt-in compiler-profile probe (bin/flag-search, or
 * each installed compiler via --compiler ID) is the decisive evidence; the
 * permuter is the only other candidate. Neither is authorised for this
 * mission, and both are blocked until the jump-table placement verifies.
 */
void func_801FA7D8(void) {
    if ((u32)(D_80146875 - 4u) < 3u) {
        if ((D_80143E6C & 0xFu) == 0) {
            u8 effect = (u8)(1 << (rand() % 3));

            D_80146867 = effect;
            if (effect >= 2u) {
                D_80146866 = (u8)(D_80146866 + 0x10u);
            }
        }
    }

    switch (D_80146875) {
    case 0:
        D_80146258 |= 0x80u;
        func_8015B580(D_8014686C, 0);
        func_8015B580(D_8014686C, 1);
        func_8015B580(D_8014686C, 2);
        func_8015B580(D_8014686C, 3);
        D_80146876 = 0x90u;
        D_80146875 = 1u;
        break;

    case 1:
    case 2: {
        u16 *counter = (u16 *)&D_80146876;
        u32  count = (u32)*counter;

        if (count != 0u) {
            u16 next = (u16)(*counter - 1u);

            D_8014932C = (u16)((count * 0x80000u) >> 16);
            *counter = next;
            break;
        }

        D_80149333 = 2u;
        D_8014932C = 0u;
        D_80146875 = 3u;
        break;
    }

    case 3:
        if (*(u8 *)&g_ScenarioProgress == 0xcu) {
            D_80146876 = 0x3cu;
            D_80146875 = 4u;
            D_80146866 = 0u;
        }
        break;

    case 4: {
        u16 *counter = (u16 *)&D_80146876;
        u16 *world = (u16 *)&D_80147A96;
        s32  step;

        if (*counter != 0u) {
            *counter = (u16)(*counter - 1u);
        } else {
            D_80146875 = 5u;
            *(u8 *)&g_ScenarioProgress += 1;
        }

        if (D_80146866 == 0u) {
            break;
        }

        D_80146866 = (u8)(D_80146866 - 1u);
        step = D_801FCA4C[D_80143E6C & 0xFu];
        *world = (u16)(*world + (s32)(s8)step * 4);
        break;
    }

    case 5: {
        u16 *world = (u16 *)&D_80147A96;
        s32  step;

        if (*(u8 *)&g_ScenarioProgress == 0x11u) {
            D_80146876 = 0x3cu;
            D_80146875 = 6u;
        }

        if (D_80146866 == 0u) {
            break;
        }

        D_80146866 = (u8)(D_80146866 - 1u);
        step = D_801FCA4C[D_80143E6C & 0xFu];
        *world = (u16)(*world + (s32)(s8)step * 4);
        break;
    }

    case 6: {
        u16 *counter = (u16 *)&D_80146876;
        u8  *scratch;
        u8   object_index;

        if (*counter != 0u) {
            *counter = (u16)(*counter - 1u);
            break;
        }

        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xffu) {
            D_80143FC8[(u32)object_index * 0x74u] = 1u;
            D_80143FCD[(u32)*scratch * 0x74u] = 0x1eu;
            *(s32 *)(D_80143FFC + (u32)*scratch * 0x74u) = D_80145EC4;
            *(s32 *)(D_80144000 + (u32)*scratch * 0x74u) = D_80145EC8;
            *(s32 *)(D_80144004 + (u32)*scratch * 0x74u) =
                func_8015477C(*(s32 *)(D_80143FFC + (u32)*scratch * 0x74u),
                              *(s32 *)(D_80144000 + (u32)*scratch * 0x74u)) << 16;
            D_80143FF1[(u32)*scratch * 0x74u] = 0u;
        }

        *counter = 0x84u;
        D_80146875 = 7u;
        *(u8 *)&g_ScenarioProgress += 1;
        break;
    }

    case 7: {
        u16 *counter = (u16 *)&D_80146876;
        u16  next = (u16)(*counter - 1u);

        *counter = next;
        if (next != 0x20u) {
            break;
        }

        func_8014ECAC(8u);
        D_80146875 = 8u;
        break;
    }

    case 8: {
        u16 *counter = (u16 *)&D_80146876;

        if (*counter != 0u) {
            if (D_80143C40 == 0u) {
                D_8014832E = 0u;
            }
            *counter = (u16)(*counter - 1u);
            break;
        }

        *(u8 *)&g_ScenarioProgress = 0x14u;
        *counter = 0x40u;
        D_80146875 = 0xau;
        break;
    }

    case 9:
        if (*(u8 *)&g_ScenarioProgress == 0x14u) {
            func_80164020(0, 1);
            D_80146876 = 0x25cu;
        }
        D_80146875 = 0xau;
        break;

    case 10: {
        u16 *counter = (u16 *)&D_80146876;

        if (*counter != 0u) {
            *counter = (u16)(*counter - 1u);
            break;
        }

        func_8015B580(D_8014686C, 4);
        func_8019FA28(0x18u, 0x630000u, 0x110000u, 3u);
        D_80143F1D = 9u;
        D_80143F1E = 0xffu;
        D_80146875 = 0xbu;
        break;
    }

    case 11:
        if (D_80143C40 == 0u) {
            *(u8 *)&g_ScenarioProgress = 1u;
            D_80146875 = 0xcu;
        }
        break;

    case 12:
        if (*(u8 *)&g_ScenarioProgress == 4u && func_80162D00() != 0) {
            func_80161C20(3u, 0x69, 8);
            D_80146875 = 0xdu;
        }
        break;

    case 13:
        if (*(u8 *)&g_ScenarioProgress == 5u) {
            D_80146876 = 0x20u;
            D_80146875 = 0xeu;
        }
        break;

    case 14: {
        u16 *counter = (u16 *)&D_80146876;
        u16  remaining = *counter;

        if (remaining != 0u) {
            u32 index = remaining & 0xfu;

            D_801469EC += (s32)(s8)D_801FCA4C[index] << 11;
            *counter = (u16)(remaining - 1u);
            D_801469F0 -= (s32)(s8)D_801FCA4C[index] << 11;
            break;
        }

        D_80146875 = 0xfu;
        break;
    }

    case 15:
        if (*(u8 *)&g_ScenarioProgress != 7u) {
            break;
        }

        func_801BFE34(D_80145EC4, D_80145EC8, 1);
        func_801C7A54(1);
        func_8015B580(D_8014686C, 5);
        D_80146875 = 0x10u;
        break;

    case 17:
        *(u8 *)&g_ScenarioProgress += 1;
        func_8015C058();
        func_801BE1B0(6u);
        D_80146874 = 0u;
        D_80146875 = 0u;
        break;
    }
}
