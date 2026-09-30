#include "bof3/scenario/scena00_internal.h"

/**
 * @source 0x801FB744
 * @behavior Ticks the SCENA00 route-entry controller selected by the state
 * byte D_80146875: state 0 waits until func_8015BEA0(0x2D, 10) returns 0, then
 * arms the 0x32-frame countdown D_80146876 and advances to state 1; state 1,
 * while bit 0 of the shared mode byte D_80146866 is set, disturbs the world
 * offsets D_80147C54 and D_80147C58 by (rand() & 7) - 3 scaled by 0x400 around
 * their 0x1A8000/0x158000 baselines, and while D_80146866 reads 3 either
 * counts D_80146876 down or, at zero, seeds the front selector context
 * (1, 0x1A0000, 0x160000, 1) and advances to state 2; state 2 waits until
 * func_8015BEA0(0xF, 10) returns 0, then stores 4 in D_80146866 and advances to
 * state 3; state 3, when D_80146866 reads 9, clears flag 0x80 in the mode word
 * D_80146258, seeds the front selector context (0x19, 0xB8000, 0x5A0000, 1),
 * requests primary state 0xB in D_80146874, reloads the 0x258-frame countdown
 * and clears the state byte.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The case 1 arm reaches the shared mode byte D_80146866 through one address
 * taken into a pointer local, so that address stays live in $s1 across both
 * rand() calls and the second comparison reloads it at zero offset; spelling
 * the global directly emits a folded address per read and shrinks the frame to
 * 0x18, matching only 93/120 instructions (measured with bin/asm-diff).
 */
void func_801FB744(void)
{
    u8 *state = &D_80146875;
    u8  status = *state;

    switch (status) {
    case 0:
        if (func_8015BEA0(0x2D, 10) == 0) {
            D_80146876 = 0x32u;
            *state = 1u;
        }
        break;
    case 1: {
        u8 *mode = &D_80146866;

        if (*mode & 1u) {
            D_80147C54 = (((rand() & 7) - 3) << 10) + 0x1A8000;
            D_80147C58 = (((rand() & 7) - 3) << 10) + 0x158000;
        }
        if (*mode == 3u) {
            if (D_80146876 != 0u) {
                D_80146876 = (u16)(D_80146876 - 1u);
            } else {
                func_8019FA28(1u, 0x1A0000u, 0x160000u, 1u);
                *state = 2u;
            }
        }
        break;
    }
    case 2:
        if (func_8015BEA0(0xF, 10) == 0) {
            *state = 3u;
            D_80146866 = 4u;
        }
        break;
    case 3:
        if (D_80146866 == 9u) {
            D_80146258 &= 0xFF7Fu;
            func_8019FA28(0x19u, 0xB8000u, 0x5A0000u, 1u);
            D_80146874 = 0xBu;
            D_80146876 = 0x258u;
            *state = 0u;
        }
        break;
    }
}
