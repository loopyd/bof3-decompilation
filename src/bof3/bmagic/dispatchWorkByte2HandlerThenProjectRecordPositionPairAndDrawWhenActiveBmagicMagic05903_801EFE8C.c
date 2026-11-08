#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map.
 * The original re-materializes `lui $vN,0x1F80` / `lw $vN,0x44($vN)` once per
 * access group: before the dispatch index, after the indirect call for the gate
 * read, and again before each of the four record accesses, because every call
 * in between may republish the cell. Declaration style follows the exact
 * same-target sibling
 * dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic05903_801EF224. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad phase halfword 0x1F80000E. The original forms its address as one
 * `lui $s0` plus `addiu $s0` pair and keeps it in a saved register across the
 * three sampler calls, because the turn store and both phase reads go through
 * that one base. */
extern u16 D_1F80000E; /* @source 0x1F80000E @kind data */

/* Scratchpad centre halfword 0x1F800000, the projected centre the samplers
 * scale. The original publishes it through a direct fixed-address store and
 * re-reads it twice, each read a fresh `lui $v1` plus `lh $v1` pair rather than
 * a live base, so it is bound as its own cell and not through the phase
 * pointer. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Work-byte-2 handler table of this state, based at 0x801F0684: entry 0 =
 * 0x801F0018, entry 1 = 0x801F0070, entry 2 = 0x801F00DC, entry 3 =
 * 0x801F012C. Both handler groups of this overlay share one ten-word
 * code-pointer run that starts at 0x801F066C: the first group is the state-0
 * handler 0x801EF224 followed by its four work-byte-2 handlers, the second is
 * this function at 0x801EFE8C followed by its four. The byte-1 dispatcher
 * dispatchStateByte1Table0680BmagicMagic05903_801EFE48 indexes 0x801F0680,
 * the word that holds this function, while this body indexes the word one
 * further on. The original adds the scaled index to the 0x801F0684 word, so
 * the extern is declared at that indexed entry and not at the run start. */
extern void (*D_801F0684[])(void); /* @source 0x801F0684 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, and the name is its own
 * D_<address> form, so the binding needs no map row. Both accesses here read a
 * word field of the shared battle work record. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared frame counter in main RAM, outside this overlay's image. The shared
 * map owns the address, and the exact bmagic siblings
 * stepWorkRecordAngleProjectPositionPairAndCountDownByte9OnFrameParityThen
 * ClearRecordAtZeroBmagicMagic10603_801EF418 and
 * advanceWorkByteAOnFrameParityInvokePointHelperCountdownWorkByte9Decrement
 * WorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4 read the same word as a
 * per-frame phase. Here its low four bits give the sixteen-step offset passed
 * to the first sampler call and its low six bits give the sixty-four-step
 * offset passed to the last one. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Shared phase samplers: both take a 12-bit angle and return the sample in the
 * fixed-point scale the products below shift, the same two helpers the exact
 * bmagic siblings call. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared main-executable helper called once the work object is active and its
 * position pair has been projected. Unlifted and unnamed, so it keeps its raw
 * name; this caller passes no arguments and reads no result. */
void func_800AF764(void);

/* This overlay's draw routine at 0x801F01A4: it reads the work record words
 * 0x34 and 0x38 this body projects into, links the sprite primitive and emits
 * its four rotated vertices from the scratchpad centre and phase cells.
 * Unlifted, so it keeps its raw name; it takes no arguments here. */
void func_801F01A4(void);

/* @source 0x801EFE8C
 * @behavior Step handler for the second state of the overlay work object
 *           published at the scratchpad pointer cell 0x1F800044. It reads that
 *           object's unsigned byte 0x2, scales it by four and calls the handler
 *           pointer at that index of the work byte 2 handler table based at
 *           0x801F0684. When the handler returns it re-reads the cell and, only
 *           when the record byte 0x0 and the byte 0x2 are both non-zero,
 *           publishes the record turn from byte 0x9 into the scratchpad phase
 *           halfword 0x1F80000E as ((byte 0x9 * 3) & 0x3F) << 6 and projects
 *           the record position pair. The first sampler call takes the shared
 *           frame counter low four bits shifted into the turn, and its result
 *           shifted left four and then right twelve is added to the record word
 *           0xC and stored as the scratchpad centre halfword 0x1F800000. The
 *           next sampler call takes the phase cell and its product with that
 *           centre, shifted right three, is added to the shared battle work
 *           record word 0x34 and stored in the record word 0x34; the third
 *           sampler call takes the phase cell again and its product with the
 *           centre, shifted right three, is added to the shared battle work
 *           record word 0x38 and stored in the record word 0x38. The last
 *           sampler call takes the frame counter low six bits shifted into the
 *           turn, and its result shifted left seven and then right three is
 *           added to the record word 0x38 just stored. It then calls the
 *           shared main-executable helper at 0x800AF764 and finally this
 *           overlay's draw routine at 0x801F01A4. Each record access re-reads
 *           the pointer cell instead of holding the pointer across the calls.
 *           The first cell load precedes frame creation; the 0x18-byte frame
 *           only keeps $ra and the phase cell address across the calls. Takes
 *           no arguments, returns nothing and reads or writes no other state.
 *           No jal in this payload reaches it: its address is the word at
 *           0x801F0680 inside the textbin row T_801F065C that the byte-1
 *           dispatcher dispatchStateByte1Table0680BmagicMagic05903_801EFE48
 *           indexes. The last turn offset is computed into its own local before
 *           the record word 0x38 store: the original issues that frame-counter
 *           load and mask between the multiply of the previous product and the
 *           store it belongs to, which is the order the separate local gives.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProjectRecordPositionPairAndDrawWhenActiveBmagicMagic05903_801EFE8C(
    void) {
  u8 *step_record;
  u8 *gate_record;
  s16 *phase;
  s32 sample;
  s32 angle;

  step_record = g_battle_work;
  D_801F0684[step_record[2]]();
  gate_record = g_battle_work;
  if (gate_record[0] != 0 && gate_record[2] != 0) {
    phase = (s16 *)&D_1F80000E;
    *phase = ((gate_record[9] * 3) & 0x3F) << 6;
    sample = func_801782FC((D_80143E6C & 0xF) << 6);
    D_1F800000 = *(s32 *)(g_battle_work + 0xC) + ((sample << 4) >> 12);
    sample = func_801783C8(*phase);
    *(s32 *)(g_battle_work + 0x34) =
        *(s32 *)(D_801EB4E0 + 0x34) + ((sample * D_1F800000) >> 3);
    sample = func_801782FC(*phase);
    angle = (D_80143E6C & 0x3F) << 6;
    *(s32 *)(g_battle_work + 0x38) =
        *(s32 *)(D_801EB4E0 + 0x38) + ((sample * D_1F800000) >> 3);
    *(s32 *)(g_battle_work + 0x38) += (func_801783C8(angle) << 7) >> 3;
    func_800AF764();
    func_801F01A4();
  }
}
