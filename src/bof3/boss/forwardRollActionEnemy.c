#include "bof3/bof3.h"

extern int rand(void);

/* Shared main-executable readiness helper at 0x801E30F8, outside this overlay's
 * image and bound through this target's generated undefined-funcs map. The
 * adjacent byte-9 handler of this overlay at 0x800C1F0C calls the same entry
 * and discards its byte; this body is the one that tests it. */
u8 func_801E30F8(void); /* @source 0x801E30F8 */

/* Shared main-executable helper at 0x800A9820, outside this overlay's image and
 * bound through this target's generated undefined-funcs map. It consumes the
 * zero-extended battle byte as its only argument, which is how the battle lifts
 * of the shared image call it. */
void func_800A9820(u8 arg0); /* @source 0x800A9820 */

/* Action-slot allocator of the shared battle image at 0x801E590C, outside this
 * overlay's image and bound through this target's generated undefined-funcs map.
 * This label's own hook-mode dispatcher 0x800C2760 calls it with the same
 * two-word argument form; this body discards the returned slot byte. */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Scratch-state reset of the shared battle image at 0x801E567C, outside this
 * overlay's image and bound through this target's generated undefined-funcs
 * map. The sibling overlay lift at 0x800C227C calls the same entry, and this
 * payload's other jal to it sits in this body. */
void func_801E567C(void); /* @source 0x801E567C */

/* Battle byte of the shared main executable holding the current enemy work
 * record index: the hook-mode dispatcher 0x800C2760 of this same overlay scales
 * it by the 0x140-byte record stride of the band at 0x80145FAC. This body
 * forwards it unchanged as the only argument of 0x800A9820. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle flag halfword at 0x801462E8, the word the battle lifts of the
 * shared image raise bit 0x4 of and the boss overlay lift at 0x800C227C tests.
 * Its address is outside this payload, so the generated undefined-syms map
 * binds it and no target-map row is added. */
extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Shared battle phase flag byte at 0x801462ED, five above the flag halfword.
 * The hook-mode dispatcher 0x800C2760 and the flag raiser 0x800C2B98 of this
 * same overlay read and write the same byte. */
extern u8 D_801462ED; /* @source 0x801462ED @kind unknown */

/* @source 0x800C1F90
 * @behavior Random-action handler of this overlay, entry 1 of the handler run
 *           based at 0x800C3274: the byte-2 dispatcher 0x800C1EC8 indexes that
 *           run with the work record's byte 2, whose entry 0 is the adjacent
 *           0x800C1F0C, and the word 0x800C1F90 at 0x800C3278 is the only
 *           reference this payload makes to the body, so no `jal` reaches it.
 *           It first calls the shared readiness helper 0x801E30F8 with no
 *           arguments, masks the returned byte to eight bits and returns
 *           nothing further while that byte is zero. When the helper reports,
 *           it forwards the shared battle byte 0x80146384, the current enemy
 *           work record index, into the shared helper 0x800A9820, then rolls
 *           rand() and keeps the low two bits. On a zero roll it raises bit
 *           0x80 of the shared battle flag halfword 0x801462E8 and bit 0x10 of
 *           the shared phase flag byte 0x801462ED and submits effect 2 through
 *           the shared action-slot allocator 0x801E590C with slot class 0,
 *           discarding the returned slot index; the two raises are scheduled
 *           as a pair, both loads then both stores. Every reporting path then
 *           raises bit 0x4 of 0x801462E8 and calls the shared scratch-state
 *           reset 0x801E567C. The 0x18-byte frame only keeps the return
 *           address at 0x10 across the calls, the slot after each argument load
 *           and the slot after the last call stay nops, and the body takes no
 *           arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardRollActionEnemy(
    void) {
  if (func_801E30F8() == 0) {
    return;
  }
  func_800A9820(D_80146384);
  if ((rand() & 3) == 0) {
    D_801462E8 |= 0x80;
    D_801462ED |= 0x10;
    func_801E590C(0, 2);
  }
  D_801462E8 |= 0x4;
  func_801E567C();
}
