#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with
 * `lb`, so it is declared signed here while every arm stores it with a plain
 * byte store. This target's own setSharedStateByteAndRequestPair4a2fThenSeed*
 * neighbour at 0x801F3724, gated exact, calls 0x801448EC the shared overlay
 * state byte; the reviewed dispatchSharedModeByteTable* family calls it the
 * shared mode byte and dispatches it through a jump table in the leading table
 * data, which is the name this body keeps. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared state-entry byte the mode-4 arm clears beside the mode byte. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* Shared front-end phase byte the mode-3 arm raises to 2 and the mode-4 arm
 * waits to leave. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared halfword countdown the mode-1 arm arms with 0x1E and the mode-2 arm
 * steps. It sits between the state-entry byte and the scratch word of the same
 * shared block. */
extern u16 D_801448EE; /* @source 0x801448EE @kind data */

/* Shared scenario progress byte the mode-1 gate waits for 1 on, which the
 * mode-2 arm increments once the countdown runs out, and which the mode-3 gate
 * waits for 0 on. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */

/* Front-end entry table of the main executable at 0x80143FC8: 0x74-byte rows
 * whose byte 0 is the occupancy flag, whose byte 5 is the handler selector and
 * whose +0x34/+0x38 words are the row position. This target's exact neighbour
 * at 0x801F3724 seeds the same four cells of the same table, and this
 * target's generated undefined-syms map binds the address. */
extern u8 D_80143FC8[]; /* @source 0x80143FC8 @kind data */

/* Shared front-end handler of the main executable. The mode-0 arm passes 0,
 * the selector the reviewed dispatchSharedModeByteTable* family passes to
 * advance one mode. The generated undefined-funcs map binds the address. */
extern void func_801C601C(u32 arg0); /* @source 0x801C601C */

/* Front-end record allocator of the main executable: returns the index of a
 * free row of the 0x74-byte table at 0x80143FC8, or 0xFF when no row is free.
 * The generated undefined-funcs map binds the address. */
extern u8 func_8019601C(void); /* @source 0x8019601C */

/* Shared selector request helper of the main executable. The mode-3 arm passes
 * the mode selector 3; the argument role stays structural. The generated
 * undefined-funcs map binds the address. */
extern void func_80150224(u32 arg0); /* @source 0x80150224 */

/* Shared overlay release helper of the main executable. The mode-4 arm runs it
 * before clearing the shared state pair. The generated undefined-funcs map
 * binds the address. */
extern void func_8015C058(void); /* @source 0x8015C058 */

/* Name evidence: 0x801448EC is the shared mode byte and 0x801F2C1C the leading
 * table-data base this body dispatches through, and this span carries the
 * reviewed dispatchSharedModeByteTable* prologue (`lb` of the signed byte,
 * `sltiu` bound, `sll`/`lui %hi(jtbl)`/`addu`/`lw`/`jr`) with this overlay's
 * own table address and five dense slots, so it joins that family as
 * Table2C1CWorld02Area10013. What the modes do names no narrower role: mode 0
 * advances the shared handler, mode 1 seeds one front-end row, modes 2, 3 and 4
 * step the countdown, the progress byte and the phase byte, so the name states
 * the dispatch it performs rather than a single effect.
 *
 * No caller inside this payload: a whole-payload scan finds neither a `jal`
 * encoding that reaches 0x801F38D8 nor the address word 0x801F38D8 at any
 * 4-byte offset, so the dispatcher that selects it is outside this payload and
 * the body takes no arguments.
 *
 * Shape: the six reviewed dispatchSharedModeByteTable2C04* siblings supply the
 * switch form, the signed-byte declaration, the countdown idiom and the
 * `.rodata` placement route, none of them as a clone because each carries a
 * different mode set. A word compare of this 0x194-byte span against all 396
 * payloads in out/binaries finds no twin: the best other window agrees on 24 of
 * 101 words, so the discriminating handler selector 0x9B and the mode set are
 * this payload's own and the body is written from this span's bytes. Within
 * this target, setSharedStateByteAndRequestPair4a2fThenSeedFrontEndRecord
 * HandlerB7AtFixedPositionWorld02Area10013_801F3724, gated exact, supplied the
 * row-seed statement forms, the 0x74 index scale and the `0xFF` allocator test;
 * world03/area148/13 supplied the halfword countdown statement, and
 * world02/area111/13 and world04/area168/13 the mode-advance arms. The store
 * order of the four row cells (byte 0, byte 5, +0x34, +0x38) is this span's
 * own, as is the order of the mode-2 mode store and progress increment.
 *
 * The jump table is the compiler's: this target's `[[matching.section_placements]]`
 * row for the `.rodata` section at 0x801F2C1C places it over the five words the
 * payload keeps there, which is the address the original's `lui %hi` carries.
 *
 * @source 0x801F38D8
 * @behavior Overlay front-end mode dispatch. It switches on the shared mode
 *           byte 0x801448EC through the five-slot table at 0x801F2C1C and steps
 *           one mode per call. Mode 0 runs the shared handler 0x801C601C with 0
 *           and advances to 1. Mode 1, once the scenario progress byte reads 1,
 *           claims a row through the allocator 0x8019601C and, unless the
 *           returned index reads 0xFF, seeds that 0x74-byte row of the table at
 *           0x80143FC8 with occupancy 1 at byte 0, handler selector 0x9B at
 *           byte 5 and the fixed position words 0x460000 at 0x34 and 0x340000
 *           at 0x38; it then arms the shared countdown 0x801448EE with 0x1E and
 *           advances to 2 either way. Mode 2 steps that countdown and, when it
 *           reaches zero, advances to 3 and increments the progress byte. Mode
 *           3, while the progress byte still reads 0, runs the shared request
 *           0x80150224 with selector 3, raises the phase byte 0x80143BB0 to 2
 *           and advances to 4. Mode 4, while the phase byte no longer reads 2,
 *           runs the shared release 0x8015C058 and clears the state-entry byte
 *           0x801448EB and the mode byte back to 0. Every other mode value
 *           returns untouched. The 0x18-byte frame saves $ra at 0x10 and takes
 *           no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSharedModeByteTable2C1CWorld02Area10013_801F38D8(void) {
  switch (D_801448EC) {
  case 0:
    func_801C601C(0);
    D_801448EC = 1;
    break;
  case 1:
    {
      u8* record;
      u8 index;

      if (g_ScenarioProgress != 1) {
        break;
      }
      index = func_8019601C();
      if (index != 0xFF) {
        record = &D_80143FC8[(u32)index * 0x74u];
        record[0] = 1;
        record[5] = 0x9B;
        *(u32*)(record + 0x34) = 0x460000;
        *(u32*)(record + 0x38) = 0x340000;
      }
      D_801448EE = 0x1E;
      D_801448EC = 2;
    }
    break;
  case 2:
    {
      u16* counter;
      u8* progress;

      counter = &D_801448EE;
      if (--*counter != 0) {
        break;
      }
      progress = &g_ScenarioProgress;
      D_801448EC = 3;
      *progress = *progress + 1;
    }
    break;
  case 3:
    if (g_ScenarioProgress != 0) {
      break;
    }
    func_80150224(3);
    D_80143BB0 = 2;
    D_801448EC = 4;
    break;
  case 4:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C058();
    D_801448EB = 0;
    D_801448EC = 0;
    break;
  }
}
