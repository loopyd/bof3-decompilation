#include "bof3/battle/battle03_internal.h"

void func_800ACF80(void);

/* Byte +0x07 and byte +0x1A of the 0xA4-byte field records at D_80144968, plus
 * the shared battle-reset bytes 0x801462E3/0x801462E4 and the sub-state word
 * 0x80143B92 that config/targets/shared/symbols.txt already maps. The record
 * cells join the flat-symbol family this target reaches with
 * `lui $at,%hi(X)` + `addu at,at,index*0xA4` + `%lo(X)(at)`. */
extern u8  D_8014496F[]; /* @source 0x8014496F @kind data */
extern u8  D_80144982[]; /* @source 0x80144982 @kind data */
extern u16 D_80143B92;   /* @source 0x80143B92 @kind unknown */
extern u8  D_801462E3;   /* @source 0x801462E3 @kind unknown */
extern u8  D_801462E4;   /* @source 0x801462E4 @kind unknown */

/* @source 0x801D7350
 * @behavior Third handler of the D_801EAD20 state table: resets the eight enemy
 *   work records (func_800ACF80) and the queued-slot key bytes (func_801E5A38),
 *   then initializes the 0xA4-byte field record of each active local work
 *   record -- active meaning bit 0x4000 of its +0x80 halfword (D_80145F10), with
 *   the field record selected by byte +0x13C (D_80145FCC). Initializing sets
 *   halfword +0x14 and byte +0x18 to 1, advances the +0x1A counter while it
 *   stays below 5, clears bit 0x4000 of halfword +0x0C and bit 0x02 of byte
 *   +0x07, and rebuilds the record through func_80164A44. After the loop it
 *   copies the templates into the local work records (copyLocalTemplates),
 *   clears the pending bytes 0x80146329 and 0x801462E0..0x801462E4, and
 *   advances the shared sub-state word 0x80143B92 -- the same six clears that
 *   precede the D_80143B92 advance in emi/etc/game/00 bank2CompleteOperation.
 * @status partial
 * @match 95.92
 * @residual 94/98 instructions at 392 versus 388 bytes. The final record-address
 *   addition precedes the flag updates instead of filling the func_80164A44
 *   delay slot, leaving one extra nop and shifting two branch targets.
 */
void initBattlerFieldRecordsAndAdvanceFrontState(void) {
  u16* substate;
  u16  state;
  u8   index;

  func_800ACF80();
  /* Keep this initialization in its own operation scope so it remains before
   * the queued-slot reset during scheduling. */
  do {
    index = 0;
  } while (0);
  func_801E5A38();
  if (localBattlerCount != 0) {
    do {
      u32 work_offset;
      u32 record_offset;
      u8  counter;
      u16 flags;
      u8  mark;

      work_offset = (u32)index * 0x140;
      if ((*(volatile u16*)&D_80145F10[work_offset] & 0x4000u) != 0u) {
        record_offset = (u32)D_80145FCC[work_offset] * 0xa4;
        counter = D_80144982[record_offset];
        *(volatile u16*)&D_8014497C[record_offset] = 1;
        D_80144980[record_offset] = 1;
        if (counter < 5u) {
          D_80144982[record_offset] = counter + 1;
        }
        flags = *(volatile u16*)&D_80144974[record_offset];
        mark = D_8014496F[record_offset];
        *(volatile u16*)&D_80144974[record_offset] = flags & 0xBFFFu;
        D_8014496F[record_offset] = mark & 0xFBu;
        record_offset = (u32)((u8*)D_80144968 + record_offset);
        func_80164A44(record_offset);
      }
      index += 1u;
    } while (index < localBattlerCount);
  }
  copyLocalTemplates();
  substate = &D_80143B92;
  state = *substate;
  D_80146329 = 0;
  D_801462E0 = 0;
  D_801462E1[0] = 0;
  BATTLE_GLOBAL_BYTE_62E2 = 0;
  D_801462E3 = 0;
  D_801462E4 = 0;
  *substate = state + 1;
}
