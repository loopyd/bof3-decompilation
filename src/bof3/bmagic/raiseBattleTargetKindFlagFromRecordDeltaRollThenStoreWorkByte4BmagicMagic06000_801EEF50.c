#include "bof3/bof3.h"

extern int rand(void);
u32 testBattleTargetKindFlagBmagicMagic06000_801EF118(void);
void func_801EF0B8(void);

/* 0x118-byte local panel record selected by the battle target selector byte
 * less three. The kind byte at +0x00 is the same first field
 * testBattleTargetKindFlagBmagicMagic06000_801EF118 reads; this lift adds the
 * two fields it reaches, the byte at +0x03 and the unsigned halfword at +0x0C.
 * Both files bind the row shape they read instead of sharing a header, and the
 * original forms one row scaling per use rather than a held row pointer. */
typedef struct BmagicMagic06000LocalPanelRow {
  u8 kind;
  u8 unk_01[2];
  u8 unk_03;
  u8 unk_04[8];
  u16 unk_0C;
  u8 unk_0E[0x118 - 0x0E];
} BmagicMagic06000LocalPanelRow;

/* Local panel record array in main RAM, outside this overlay's image. Bound
 * through this target's generated undefined_syms_auto.txt, which already
 * carries 0x801EB6AC, so no target-map row is added. */
extern BmagicMagic06000LocalPanelRow D_801EB6AC[]; /* @source 0x801EB6AC @kind table */

/* Row offset 0x7A of the 0x140-byte work-record table based at 0x80145E90, the
 * rows the plchar lifts index as D_80145E90[i * 0x140] and the bmagic magic069
 * sibling seedWorkHalfwords2EAnd30FromSelectedBattlerRecordClassDeltaSubmitRecordStoreEffectByteAAdvanceWorkByte2BmagicMagic06903_801EF564
 * selects as party entries. The original scales the selector byte by 0x140 at
 * this access instead of forming the row base, so the field keeps its own
 * address-derived symbol, the flat-symbol family the exact sibling
 * drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174 uses
 * for row offsets 0x74 and 0x79. */
extern u8 D_80145F0A[]; /* @source 0x80145F0A @kind table */

/* Party work-record selection byte in main RAM, outside this overlay's image.
 * The bmagic magic151 sibling
 * selectWorkRecordClearFlag40ThenAdvanceWorkByte1AndClearWorkBytes2And3BmagicMagic15103_801EF2A0
 * reads the same byte. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Battle target selector byte in main RAM, outside this overlay's image. The
 * -3 here is this overlay's local panel record base offset, the same reduction
 * testBattleTargetKindFlagBmagicMagic06000_801EF118 and
 * drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174 take. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EEF50
 * @behavior Work-byte-1 handler of this overlay, index 0 of the five-entry code
 *           pointer run at 0x801EEC04 (0x801EECA4, 0x801EED68, 0x801EEE00,
 *           0x801EEE84 and 0x801EEF04, terminated by a zero word) that the
 *           dispatcher func_801EEC34 copies into its 0x30-byte frame and
 *           selects with the scratchpad work object byte at offset 0x1 scaled
 *           by four. Its only caller func_801EECA4 reaches it with a jal after
 *           queueing two action slots. It forms a signed delta between a party
 *           work record byte and the battle target panel record halfword: the
 *           byte at row offset 0x7A of the 0x140-byte work-record table based at
 *           0x80145E90, selected by the byte D_80146374 and reached as the flat
 *           symbol D_80145F0A plus the scaled index, minus the unsigned
 *           halfword at row offset 0x0C of the 0x118-byte local panel record at
 *           0x801EB6AC, selected by the battle target selector byte D_80146384
 *           less three. It turns that delta into a threshold on a sixteen-step
 *           grid with a ladder of independent ifs that overwrite one register
 *           in descending order: 0x10 by default, 0x0E below 15, 0x0C below 10,
 *           0x0A below 5, 8 below 0, 6 below -5, 4 below -10 and 2 below -15,
 *           the default landing in the first branch delay slot and each later
 *           test landing in the preceding branch delay slot. One rand() nibble
 *           is then compared signed-less against that threshold, and the
 *           comparison is kept in a saved register because the answer is used
 *           after the following call. The threshold stands on the left of the
 *           comparison, which is the operand order that leaves the masked
 *           nibble in the register rand() answers in: keeping the nibble in a
 *           second register instead puts it across the following call and costs
 *           a callee-saved register the original does not save. That call is
 *           testBattleTargetKindFlagBmagicMagic06000_801EF118, whose low byte
 *           gates the first arm: when the per-kind flag bit of the flag words
 *           at 0x80144FA0 is already set, the scratchpad work object byte at
 *           offset 0x4 takes 1. Otherwise the kept roll decides together with
 *           the panel record byte at row offset 0x03 of the same selector row,
 *           and only when both are nonzero does it raise that flag bit through
 *           func_801EF0B8 and then store 1; a failing roll or a zero record byte
 *           stores 0. Each store re-reads the pointer cell 0x1F800044 at its own
 *           site, so the constant 1 and the constant 0 are materialised
 *           separately instead of through one merged result. One signed local
 *           carries the delta through the ladder and then the roll answer, so
 *           the same saved register holds both, and the per-kind flag answer is
 *           truncated into a byte local before its test. The 0x20-byte
 *           frame keeps ra at 0x18, s1 at 0x14 and s0 at 0x10; nothing is
 *           returned and no other state is read or written. Statement order,
 *           operand forms and declaration style follow the sibling work-byte-1
 *           handlers of this target. No byte-identical twin of this span exists
 *           in any target tree; the nearest sibling, the threshold-ladder body
 *           inside func_801EED50 of emi/bmagic/magic068/03, is a 447-instruction
 *           body that reuses the ladder idiom with a three-step grid, so it
 *           supplied shape evidence only and was not cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseBattleTargetKindFlagFromRecordDeltaRollThenStoreWorkByte4BmagicMagic06000_801EEF50(void) {
  s32 value;
  s32 threshold;
  u8 flag;

  value = D_80145F0A[D_80146374 * 0x140] - D_801EB6AC[D_80146384 - 3].unk_0C;
  threshold = 0x10;
  if (value < 0xF) {
    threshold = 0xE;
  }
  if (value < 0xA) {
    threshold = 0xC;
  }
  if (value < 5) {
    threshold = 0xA;
  }
  if (value < 0) {
    threshold = 8;
  }
  if (value < -5) {
    threshold = 6;
  }
  if (value < -0xA) {
    threshold = 4;
  }
  if (value < -0xF) {
    threshold = 2;
  }
  value = threshold > (rand() & 0xF);
  flag = testBattleTargetKindFlagBmagicMagic06000_801EF118();
  if (flag != 0) {
    SPAD_PTR_SLOT(u8, 0x44u)[4] = 1;
  } else if (value && (D_801EB6AC[D_80146384 - 3].unk_03 != 0)) {
    func_801EF0B8();
    SPAD_PTR_SLOT(u8, 0x44u)[4] = 1;
  } else {
    SPAD_PTR_SLOT(u8, 0x44u)[4] = 0;
  }
}
