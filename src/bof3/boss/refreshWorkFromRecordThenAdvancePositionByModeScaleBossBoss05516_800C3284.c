#include "bof3/bof3.h"

/* Enemy work record pointer cell the main program publishes; the overlay work
 * record below is refreshed from it here, as the neighbouring battle overlay
 * steps do. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad pointer cell holding this overlay work record. Declared as the
 * absolute-address symbol so every use reloads the cell with the lui plus
 * offset load form the original shows. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Per-mode signed 16-bit pair table of the func_800C34D4 boundary, which holds
 * data rather than code: entry n is read at its first halfword here and at its
 * second halfword through D_800C34D6. */
extern s16 func_800C34D4[]; /* @source 0x800C34D4 @kind data */
extern s16 D_800C34D6[]; /* @source 0x800C34D6 @kind data */

extern s32 fixedPointToUnitsBossBoss05516_800C3254(s32 value, s32 arg1); /* @source 0x800C3254 */

/* @source 0x800C3284
 * @behavior Overlay step that refreshes the work record at scratchpad cell
 * 0x1F800044 from the enemy work record published at 0x801EB4E0: bytes 0x00,
 * 0x48, 0x27, 0x28, 0x2A, 0x5C, 0x5D, 0x5E and 0x5F, words 0x40 and 0x44,
 * and halfwords 0x2E, 0x30 and 0x32. When the refreshed byte 0x48 is set it
 * then advances the halfword accumulators 0x2E and 0x30 by the whole-unit
 * value of the 16.16 velocity words 0x40 and 0x44 scaled by the signed
 * halfword pair at 0x800C34D4 and 0x800C34D6 indexed by work byte 1; when
 * byte 0x48 is clear the same accumulators advance by that pair value alone.
 * The mode index is not bounded before the scaled table load. No word of this
 * payload names this function, its callee or its table base, so it is an
 * overlay step reached from outside the payload; its prologue and return path
 * alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284(void) {
  ((u8 *)g_battle_work)[0x00] = ((u8 *)D_801EB4E0)[0x00];

  ((u8 *)g_battle_work)[0x48] = ((u8 *)D_801EB4E0)[0x48];

  FIELD_REF(u32, g_battle_work, 0x40) = FIELD_REF(u32, ((u8 *)D_801EB4E0), 0x40);
  FIELD_REF(u32, g_battle_work, 0x44) = FIELD_REF(u32, ((u8 *)D_801EB4E0), 0x44);
  ((u8 *)g_battle_work)[0x27] = ((u8 *)D_801EB4E0)[0x27];

  ((u8 *)g_battle_work)[0x28] = ((u8 *)D_801EB4E0)[0x28];

  ((u8 *)g_battle_work)[0x2A] = ((u8 *)D_801EB4E0)[0x2A];

  FIELD_REF(u16, g_battle_work, 0x2E) = FIELD_REF(u16, ((u8 *)D_801EB4E0), 0x2E);
  FIELD_REF(u16, g_battle_work, 0x30) = FIELD_REF(u16, ((u8 *)D_801EB4E0), 0x30);
  FIELD_REF(u16, g_battle_work, 0x32) = FIELD_REF(u16, ((u8 *)D_801EB4E0), 0x32);
  ((u8 *)g_battle_work)[0x5C] = ((u8 *)D_801EB4E0)[0x5C];

  ((u8 *)g_battle_work)[0x5D] = ((u8 *)D_801EB4E0)[0x5D];

  ((u8 *)g_battle_work)[0x5E] = ((u8 *)D_801EB4E0)[0x5E];

  ((u8 *)g_battle_work)[0x5F] = ((u8 *)D_801EB4E0)[0x5F];

  if (((u8 *)g_battle_work)[0x48] != 0) {
    FIELD_REF(u16, g_battle_work, 0x2E) += fixedPointToUnitsBossBoss05516_800C3254(
        func_800C34D4[((u8 *)g_battle_work)[0x1] * 2] * FIELD_REF(u32, g_battle_work, 0x40),
        FIELD_REF(u32, g_battle_work, 0x40));
    FIELD_REF(u16, g_battle_work, 0x30) += fixedPointToUnitsBossBoss05516_800C3254(
        D_800C34D6[((u8 *)g_battle_work)[0x1] * 2] * FIELD_REF(u32, g_battle_work, 0x44),
        FIELD_REF(u32, g_battle_work, 0x44));
  } else {
    FIELD_REF(u16, g_battle_work, 0x2E) += fixedPointToUnitsBossBoss05516_800C3254(
        func_800C34D4[((u8 *)g_battle_work)[0x1] * 2] << 16, 0x10000);
    FIELD_REF(u16, g_battle_work, 0x30) += fixedPointToUnitsBossBoss05516_800C3254(
        D_800C34D6[((u8 *)g_battle_work)[0x1] * 2] << 16, 0x10000);
  }
}
