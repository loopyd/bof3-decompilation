#include "base/types.h"

/* Scratchpad work-object pointer cell at 0x1F800044, already bound by this
 * target's map. The body reads the cell once per group - the publish group, the
 * step-seed group and each of the three counter advances - and the original
 * re-materialises the cell address with its own lui / lw before every read, so
 * the cell is reached through the named pointer rather than a folded constant
 * base. The plain cell is kept rather than a volatile one: every read after the
 * first is separated from the previous read by a store through the loaded
 * pointer, so the reloads come from those stores. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared per-frame counter at 0x80143E6C. Only its low two bits are read here;
 * the sibling lifts of this family read the same word for frame parity. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Record pool at 0x801EFA8C - 48 records of stride 0x78 whose byte 0 carries
 * the in-use bit. This function fills the pointer word at +0x74 and the two
 * bytes at +0xA and +0xB of one record; the original reaches each of them as
 * `lui $at,%hi(ADDR)` / `addu $at,$at,index*0x78` / `sb|sw %lo(ADDR)($at)`, so
 * every addressed field keeps its own flat address inside this one base. */
extern u8 magic08103RecordFlags[]; /* @source 0x801EFA8C @kind data */

u8 reserveFirstFreeRecordBmagicMagic08103_801EFA20(void); /* @source 0x801EFA20 */

/* @source 0x801EF18C
 * @behavior Per-frame record spawner of this overlay. On the frame phases where
 *           the low two bits of the shared counter at 0x80143E6C are both set
 *           - every fourth frame - it claims the first free record of the pool
 *           at 0x801EFA8C through reserveFirstFreeRecordBmagicMagic08103_801EFA20,
 *           scales the returned byte index by the 0x78-byte record stride, and
 *           fills three fields of that record: the pointer word at +0x74
 *           receives the scratchpad work-object pointer read from the cell at
 *           0x1F800044, byte +0xB receives the work object's byte 0x9 read
 *           through the same pointer, and byte +0xA receives the ramp step
 *           derived from that byte. The step is 8 while work byte 0x9 is below
 *           100 and 8 - (byte 9 - 100) / 4 above it, so the value falls by one
 *           every four counts past 100. The guarded part ends by re-reading the
 *           cell and incrementing the work object's byte 0xB.
 *           Outside the guard every frame re-reads the cell, increments work
 *           byte 0x9 and tests the truncated result: once that byte reaches
 *           0x81 the cell is read again and the work object's state byte 0x2 is
 *           incremented, which advances this overlay's work byte 2 handler
 *           sequence. Frame 0x18 with $ra only, because of the call; it takes no
 *           arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void reserveRecordEveryFourthTickPublishWorkPointerCopyWorkByte9SeedByteAStepThenAdvanceWorkByteBAndByte9AndByte2AtLeast81BmagicMagic08103_801EF18C(void) {
    u8 *work;
    u32 index;
    u32 offset;
    u8 frame;
    u8 step;

    if ((D_80143E6C & 3) == 3) {
        index = reserveFirstFreeRecordBmagicMagic08103_801EFA20();
        offset = index * 0x78;
        work = g_battle_work;
        *(u32 *)&magic08103RecordFlags[offset + 0x74] = (u32)work;
        magic08103RecordFlags[offset + 0xB] = work[9];
        frame = g_battle_work[9];
        if (frame < 100) {
            step = 8;
        } else {
            step = 8 - (frame - 100) / 4;
        }
        magic08103RecordFlags[offset + 0xA] = step;
        g_battle_work[0xB]++;
    }
    if (++g_battle_work[9] >= 0x81) {
        g_battle_work[2]++;
    }
}
