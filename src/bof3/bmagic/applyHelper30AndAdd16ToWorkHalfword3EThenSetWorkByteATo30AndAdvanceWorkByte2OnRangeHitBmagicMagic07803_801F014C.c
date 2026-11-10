#include "bof3/bof3.h"
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_800AF2B4(u8 *record, s16 amount); /* @source 0x800AF2B4 */
extern u32 func_800AF5FC(u8 *record, u32 size); /* @source 0x800AF5FC */
/* @source 0x801F014C
 * @behavior Passes the record in cell 0x801EB4E0 and 0x30 to helper
 *           0x800AF2B4, then adds 0x10 to scratchpad work halfword 0x3E.
 *           Reloads the record cell and tests it with 0x800AF5FC at size
 *           0x1C000. On a nonzero result, sets work byte 0xA to 0x1E and
 *           reloads the work pointer to increment byte 2. Takes no arguments
 *           and returns nothing; table word 0x801F08CC publishes this callback.
 *           Work-field roles remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyHelper30AndAdd16ToWorkHalfword3EThenSetWorkByteATo30AndAdvanceWorkByte2OnRangeHitBmagicMagic07803_801F014C(void) {
  u8 *work;
  u8 *record;
  u16 value;
  func_800AF2B4(D_801EB4E0, 0x30);
  work = g_battle_work;
  value = FIELD_REF(u16, work, 0x3E);
  record = D_801EB4E0;
  FIELD_REF(u16, work, 0x3E) = value + 0x10;
  if (func_800AF5FC(record, 0x1C000)) {
    g_battle_work[0xA] = 0x1E;
    g_battle_work[2]++;
  }
}
