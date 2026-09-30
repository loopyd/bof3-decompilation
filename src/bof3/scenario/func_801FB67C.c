#include "bof3/scenario/scena00_internal.h"

/**
 * @source 0x801FB67C
 * @behavior Ticks the scenario sub-state byte D_80146875: while it is zero,
 * arms the 0x3C-frame countdown D_80146876 and moves the state byte to 2 when
 * the shared mode byte D_80146866 reads 0x71; in state 2 it decrements that
 * countdown and, when it reaches zero, seeds the front selector context
 * (1, 0xF0000, 0x210000, 1), queues frontend cue 0x206, clears the scenario
 * progress word and returns the state byte to 0 with D_80146874 = 0xA.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the two state tests form a dispatch
 * chain that sits above both out-of-line arm bodies and falls through to the
 * epilogue, which is the compare chain the compiler emits for a two-case
 * switch on the state byte. Written as `if (status == 0) ... else if
 * (status == 2)` it emits an inverted single test with the first arm inline
 * and matches 41/50 instructions (measured with bin/asm-diff).
 */
void func_801FB67C(void)
{
    u8 *state = &D_80146875;
    u8  status = *state;

    switch (status) {
    case 0:
        if (D_80146866 == 0x71u) {
            D_80146876 = 0x3Cu;
            *state = 2u;
        }
        break;
    case 2:
        if (D_80146876 != 0u) {
            D_80146876 = (u16)(D_80146876 - 1u);
        } else {
            func_8019FA28(1u, 0xF0000u, 0x210000u, 1u);
            game_queue_frontend_cue(0x206u);
            g_ScenarioProgress = 0u;
            *state = 0u;
            D_80146874 = 0xAu;
        }
        break;
    }
}
