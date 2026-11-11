#include "bof3/bof3.h"

/* Shared flag bank of the main executable, probed here at flag number 0x0C. The
 * cell lies outside this overlay's payload, so the generated undefined-syms map
 * already binds the name and no target-map row is needed. */
extern u8 D_80144ED0;

/* Resident map base pointer cell of the main executable: the byte-oriented base
 * the world overlays stamp map cells into. Also outside this payload and bound by
 * the generated undefined-syms map. */
extern u32 D_8014931C;

/* Reader of shared flag bank 0x80144ED0: returns nonzero when the flag number
 * passed in the second argument is set. A main-executable routine, bound here by
 * its generated undefined-funcs row. */
extern s32 func_8015B5D4(u32 bank, s32 index);

/* The overlay record draw routine this step finishes with. A later code span of
 * this same overlay. */
void func_801F314C(void);

/* @source 0x801F30AC
 * @behavior Byte-2 state handler entry 5, the last of the six-entry handler table
 *           based at 0x801F4624, which the overlay dispatcher at 0x801F2C1C
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044. It probes flag
 *           0x0C of the shared flag bank at 0x80144ED0 through the resident
 *           tester 0x8015B5D4. When that flag is set it reads the byte at
 *           0x80104000, scales the resident map base pointer word 0x8014931C by
 *           21 and by 22 times that byte, and stores 0xA1 into the byte at offset
 *           0x01 of each of the two resulting map cells; both stores re-read the
 *           0x80104000 byte and the base pointer word, because a store through
 *           the loaded base can alias either. It then stores 1 into the record
 *           state byte at offset 0x02, so the next dispatch takes handler entry 1
 *           at 0x801F2E08. Every path, taken or not, finishes by calling the
 *           overlay record draw routine at 0x801F314C. Takes no arguments and
 *           returns nothing; the 0x18-byte frame keeps $ra at 0x10, the branch
 *           delay slot takes the `lui` half of the 0x80104000 address load, and
 *           the call and return delay slots stay nops.
 *
 *           The 21-instruction stamp block from the 0x80104000 load to the second
 *           0xA1 store is byte-identical to the same block at 0x801F2CFC of this
 *           target's entry-0 handler 0x801F2C60 (a payload compare of the two
 *           0x54-byte spans found no differing word), which fixed the statement
 *           order, operand forms and offset spelling here. The World03/Area119
 *           twin 0x801F3298 of the same overlay family carries the same shape
 *           with its own literals (probe index 0x0F, strides 62 and 63, store
 *           offset 0x41, draw target 0x801F3324), so it is shape evidence only
 *           and not clone eligibility.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stampMapCellsA1AndSetWorkByte2To1WhenFlag0CSetThenDrawWorld03Area11713_801F30AC(
    void) {
  u8* stride;
  u8* work;

  if (func_8015B5D4((u32)&D_80144ED0, 0x0C) != 0) {
    stride = PSX_PTR(u8, 0x80104000u);
    *(u8*)(D_8014931C + stride[0] * 21 + 1) = 0xA1;
    *(u8*)(D_8014931C + stride[0] * 22 + 1) = 0xA1;
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[2] = 1;
  }
  func_801F314C();
}
