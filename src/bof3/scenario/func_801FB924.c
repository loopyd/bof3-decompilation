#include "bof3/scenario/scena00_internal.h"

/**
 * @source 0x801FB924
 * @behavior Runs the SCENA00 turn/state controller selected by the state byte
 * D_80146875 while the shared mode byte D_80146866 is below 0x14 and the held
 * key word D_80149314 still reads 0x5000, rebasing D_80149322, D_8014930C,
 * D_80147A90, D_80147B28 and D_80143F80 and running func_80154698. State 0
 * decrements the countdown D_80146876 and, at zero, sets D_80146866 = 0x10,
 * arms D_8014832E = 0x1F, runs func_8014ECAC(1), queues frontend cue 0x203,
 * reloads the countdown with 0x3C and advances to state 3; states 1 and 2 are
 * empty; state 3 branches on the running countdown, advancing through
 * D_80146866 = 0x11 to state 4 and 0x384 frames; state 4 decrements the
 * countdown and, on a held face button or at zero, clears D_80146865 and the
 * progress byte g_ScenarioProgress, sets D_80146866 = 0x12, runs
 * applyFieldTurnInput, reloads 0x384 frames and advances to state 5; state 5
 * clears both turn accumulators D_80147BEC/D_80147BF0 while either is busy,
 * then either counts the held-button nibble D_80145AA8 >> 12 up through the
 * two-byte table D_801FCA64, publishing D_80146888 to scratchpad 0x1F800044
 * and its partner byte through D_80146890/func_8014D8D4, or advances the
 * progress byte until 0x15 rolls D_80146865 back, then arms D_80146866 = 0x13
 * with state 6 once D_80146865 reaches 0x1F or the countdown expires; state 6
 * re-points the scratchpad delta cell D_1F800010 at the current record's delta
 * pair, rolls both s16 deltas to +-0x40 turn steps, refreshes the D_80146888
 * scratchpad pointer and D_80146890 slot while D_80143E6C bit 1 is clear,
 * queues frontend cue 0x204 and, when D_80146866 reads 0x15, clears both turn
 * accumulators and advances to state 7; state 7 advances to state 8 when
 * D_80146866 reads 0x17; state 8 runs func_8014ECAC(0) and advances to state 9;
 * state 9, while the gate word D_80143C40 is clear, clears D_8014832E, masks
 * D_80146258 with 0xFFB7 and D_80145EB4 with 0xDF, seeds the front selector
 * context (9, 0x570000, 0x40000, 7) and runs func_801A78F8.
 * @status partial
 * @match 95.07
 * @residual Intra-block scheduler/allocator order in the D_8014930C/D_80147A90 rebasing group and in case 6: identical instruction multiset, identical registers, 17 of 345 instructions placed differently.
 *
 * Shape evidence (live bin/asm-diff, first=+0x0064, 2 hunks). Both residuals are
 * ordering-only: in the rebasing group the two pointer loads and their address
 * materialisations are permuted, and in case 6 the &D_80147BEC address is
 * materialised one slot earlier, the 1F800010 delta cell is re-read one slot
 * later, and the selected +-0x40 turn step lives in $v0 instead of $v1, so
 * `sw $v1,0x4($s0)` runs before the D_80143E6C test rather than in its bnez
 * delay slot. Every other instruction in the function, including the reviewed
 * 10-entry jump-table placement at 0x801F6D98, matches the original bytes.
 *
 * Measured clean-C attempts that did not move either hunk: if/else vs ternary
 * turn arms, inverted arms, preinitialised arms, a dedicated s32 step local,
 * s16 dx/dy locals, inline `((s32 *)&sym)[0]` expressions, pointer-declaration
 * permutations, late pointer assignments, mirrored comparisons and unsigned
 * constants; the ternary and step-local forms additionally regress the function
 * to 347/350 instructions. Next untried rung: the opt-in compiler-profile probe
 * (bin/flag-search) or the permuter, neither authorised for this mission.
  */
void func_801FB924(void)
{
    if ((D_80143E6C & 0x1F) == 0) {
        D_80146867 = rand() & 7;
    }

    if (D_80146866 < 0x14 && D_80149314 == 0x5000) {
        s32 *word_a = &D_8014930C;
        s32 *word_b = &D_80147A90;
        s32 *word_c = &D_80143F80;

        D_80149314 = 0x3000;
        D_80149322 += 0x20;
        *word_a += 0x200000;
        *word_b += 0x200000;
        D_80147B28 += 0x200000;
        *word_c += 0x200000;
        func_80154698();
    }

    switch (D_80146875) {
    case 0: {
        u16 *countdown = &D_80146876;

        *countdown -= 1;
        if (*countdown == 0) {
            D_80146866 = 0x10;
            D_8014832E = 0x1F;
            func_8014ECAC(1);
            game_queue_frontend_cue(0x203);
            *countdown = 0x3C;
            D_80146875 = 3;
        }
        break;
    }
    case 1:
    case 2:
        break;
    case 3: {
        u16 *countdown = &D_80146876;

        if (*countdown != 0) {
            *countdown -= 1;
        } else {
            D_80146866 = 0x11;
            D_80146875 = 4;
            *countdown = 0x384;
        }
        break;
    }
    case 4: {
        u16 *countdown = &D_80146876;

        *countdown -= 1;
        if ((D_80145AA8 & 0xF000) != 0 || *countdown == 0) {
            D_80146865 = 0;
            *(u8 *)&g_ScenarioProgress = 0;
            D_80146866 = 0x12;
            applyFieldTurnInput();
            *countdown = 0x384;
            D_80146875 = 5;
        }
        break;
    }
    case 5: {
        s32 *turn = &D_80147BEC;
        u16 *pad;

        if (*turn != 0) {
            *turn = 0;
            D_80147BF0 = 0;
        } else if (D_80147BF0 != 0) {
            *turn = 0;
            D_80147BF0 = 0;
        }

        pad = &D_80145AA8;

        if ((*pad & 0xF000) != 0) {
            u32 slot;

            *(u8 *)&g_ScenarioProgress = 0;
            D_80146865 = (u8)(D_80146865 + 1);
            applyFieldTurnInput();
            slot = (*pad >> 12) * 2;

            if (D_801FCA64[slot] != 0) {
                D_1F800044 = &D_80146888;
                D_80146890 = D_801FCA65[slot];
                func_8014D8D4(D_80146890);
            }
        } else {
            *(u8 *)&g_ScenarioProgress += 1;

            if (*(u8 *)&g_ScenarioProgress >= 0x15) {
                D_80146865 = 0;
                *(u8 *)&g_ScenarioProgress = 0;
            }
        }

        if (D_80146865 >= 0x1F) {
            D_80146866 = 0x13;
            D_80146875 = 6;
            D_80146876 = 0;
        }

        {
            u16 *countdown = &D_80146876;

            *countdown -= 1;

            if (*countdown == 0) {
                D_80146866 = 0x13;
                D_80146875 = 6;
            }
        }
        break;
    }
    case 6: {
        s16 *delta;
        s32 *turn = &D_80147BEC;

        D_1F800010 = (u16 *)(*(u8 **)((u8 *)D_8017F974[D_80143F00] + 0xC) + 0x12);
        D_1F800010[0] = (rand() & 7) - 3;
        D_1F800010[1] = (rand() & 7) - 3;

        delta = (s16 *)D_1F800010;
        if (delta[0] >= 0) {
            turn[0] = 0x40;
        } else {
            turn[0] = -0x40;
        }
        if (delta[1] >= 0) {
            turn[1] = 0x40;
        } else {
            turn[1] = -0x40;
        }

        if ((D_80143E6C & 0x3) == 0) {
            D_1F800044 = &D_80146888;
            D_80146890 = (u8)((D_80146890 + 1) & 3);
            func_8014D8D4(D_80146890);
        }

        game_queue_frontend_cue(0x204);

        if (D_80146866 == 0x15) {
            turn[0] = 0;
            turn[1] = 0;
            D_80146875 = 7;
        }
        break;
    }
    case 7:
        if (D_80146866 == 0x17) {
            D_80146875 = 8;
        }
        break;
    case 8:
        func_8014ECAC(0);
        D_80146875 = 9;
        break;
    case 9:
        if (D_80143C40 == 0) {
            u8 *pad_flag = &D_80145EB4;

            D_8014832E = 0;
            D_80146258 &= 0xFFB7;
            *pad_flag &= 0xDF;
            func_8019FA28(9, 0x570000, 0x40000, 7);
            func_801A78F8();
        }
        break;
    }
}
