#include "bof3/bof3.h"

void func_80196718(void *arg0);
extern s32 func_8019651C(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. The symbolic address folds into each load (lui $vN,0x1F80 /
 * lw $vN,68($vN)), so every separate read of the cell re-materialises the base
 * and keeps the original's five loads. The cell is deliberately not volatile
 * and no local is carried across a store group: a store through the loaded
 * pointer may alias the cell, which is what forces the reload the original
 * makes before each later group. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Local battler work-record arrays selected by the work object's index byte
 * 0xB: the party records hold 0x140-byte entries, the enemy records
 * 0x118-byte entries. The two record bases serve the selection at the top of
 * the body; the party record's comparison byte at +8 and its class byte at
 * +0x79 are addressed through their own symbols because the original body
 * materialises each of those two bases separately instead of reusing one
 * record pointer. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8 D_80145E98[]; /* @source 0x80145E98 @kind unknown */
extern u8 D_80145F09[]; /* @source 0x80145F09 @kind unknown */
extern u8 D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */

/* Party y-delta byte pair: the original picks the 0x801EB108 byte while the
 * record's byte +8 is below two and the 0x801EB109 byte otherwise, both
 * indexed by the record's class byte times two. */
extern u8 D_801EB108[]; /* @source 0x801EB108 @kind unknown */
extern u8 D_801EB109[]; /* @source 0x801EB109 @kind unknown */

/* Enemy class byte of the 0x118-byte-stride enemy entries, indexed by the
 * reduced battler index. The class row it selects lives in the absolute class
 * table at 0x800E40CB and is addressed as a literal, which is what makes the
 * original emit `lui at,0x800E` / `addu at,v0,at` / `lbu v0,0x40CB(at)` - the
 * same literal form the exact battle03 lifts use for this class table. */
extern u8 D_801EB710[]; /* @source 0x801EB710 @kind unknown */

/* @source 0x801EF564
 * @behavior Work-byte-2 state 0 handler of this overlay: entry 0 of the
 *           handler table at 0x801F0520, which the byte 2 dispatcher at
 *           0x801EF520 indexes with the scratchpad work object's state byte
 *           0x2. It re-reads the work-object pointer cell at 0x1F800044 and
 *           selects a battler work record from the object's own index byte
 *           0xB: indices below three take the 0x140-byte-stride party entries
 *           at 0x80145E90, the rest the 0x118-byte-stride enemy entries at
 *           0x801EB2E8. It submits the selected record through func_80196718
 *           and its positional effect through func_8019651C with the 0/0/0
 *           offsets and the 1 in this frame's $sp+0x10 slot, storing the
 *           returned effect value into the work object's byte 0xA. It then
 *           re-reads the cell once for the seeding group and stores the
 *           record's halfwords 0x2E and 0x30 less 0x20 and 0xC into the work
 *           object's own 0x2E and 0x30, re-reading the object's byte 0xB for
 *           the class delta that reduces the 0x30 seed: for party records the
 *           delta is the 0x801EB108 byte while the record's byte +8 is below
 *           two and the 0x801EB109 byte otherwise, both indexed by the
 *           record's class byte +0x79 times two; for enemy records the delta
 *           is the class byte of 0x801EB710 at the reduced index times 0x118,
 *           indexing the absolute class table at 0x800E40CB with a 0x88-byte
 *           stride. The subtraction is duplicated into each arm and the store
 *           to the work object's 0x30 is the single common tail, so the two
 *           class arms jump to the shared store with their own `subu` in the
 *           delay slot. It re-reads the cell twice more to clear the countdown
 *           byte 9 and advance work byte 2, which selects byte-2 table entry
 *           1 (0x801EF724). The 0x20-byte frame holds $ra at 0x1C, $s0 at
 *           0x18 and the fifth func_8019651C argument at 0x10; the cell load
 *           that opens the body is issued before the frame is created because
 *           nothing before it needs the stack. The roles of bytes 0xB, 0x2E,
 *           0x30 and the two record strides stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHalfwords2EAnd30FromSelectedBattlerRecordClassDeltaSubmitRecordStoreEffectByteAAdvanceWorkByte2BmagicMagic06903_801EF564(
    void) {
    u8 *work;
    u8 *record;
    u8 index;
    s32 enemy;
    u16 height;

    index = g_battle_work[0xB];
    if (index < 3u) {
        record = (u8 *)D_80145E90 + index * 0x140u;
    } else {
        record = D_801EB2E8 + index * 0x118u;
    }
    func_80196718(record);
    g_battle_work[0xA] = func_8019651C(record, 0, 0, 0, 1);
    work = g_battle_work;
    ((u16 *)work)[0x17] = ((u16 *)record)[0x17] - 0x20;
    height = ((u16 *)record)[0x18] - 0xC;
    ((u16 *)work)[0x18] = height;
    index = work[0xB];
    if (index < 3u) {
        if (D_80145E98[index * 0x140u] < 2u) {
            ((u16 *)work)[0x18] =
                height - D_801EB108[D_80145F09[index * 0x140u] * 2];
        } else {
            ((u16 *)work)[0x18] =
                height - D_801EB109[D_80145F09[index * 0x140u] * 2];
        }
    } else {
        enemy = index - 3;
        ((u16 *)work)[0x18] = height - PSX_REF(
            u8, 0x800E40CBu + D_801EB710[enemy * 0x118u] * 0x88u);
    }
    g_battle_work[9] = 0;
    g_battle_work[2]++;
}
