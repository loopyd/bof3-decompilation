#include "bof3/ui/game00_internal.h"

/* Per-entity spawn fields inside the 0x98-byte spawn-record array and the
 * spawn-gate byte table. Each address is its own canonical label, so the
 * compiler emits the original per-access `lui %hi` / `addu $at,$at,index` /
 * `store %lo($at)` triple instead of one shared base. */
extern u8 D_80146900[];
extern u8 D_80146904[];
extern u8 D_80146908[];
extern u8 D_8014690C[];
extern u8 D_80146910[];
extern u8 D_8014691C[];
extern u8 D_80144FC4[];

extern void func_801A4AA8(void);
extern void func_8014DD3C(u16 entry);
extern s16  func_8015477C(s32 x, s32 y);
extern void func_801A4990(const volatile u8* data);
extern void func_801A4BC0(s16 x, s16 y, u32 size);
extern void func_8014D6B8(u32 flag);

#define ENTITY_SLOT_COUNT 30
#define ENTITY_SLOT_SIZE  0x98

/* Scratchpad spawn counter and packed entry word, read through the reviewed
 * D_1F800000 symbol so the original lui/addiu pair is reproduced. The counter
 * is tested and indexed through its signed halfword view; only the final
 * increment reads it unsigned. */
#define ENTITY_COUNT_S16 (((s16*)D_1F800000)[0])
#define ENTITY_COUNT_U16 (((u16*)D_1F800000)[0])
#define ENTITY_ENTRY_DATA (((u16*)D_1F800000)[1])

/* Persistent per-entity spawn field of the 0x98-byte record array at
 * 0x80146888. Each field is addressed through its own canonical symbol so the
 * compiler emits the original per-store address triple and re-reads the
 * counter for every one of these writes. */
#define ENTITY_SLOT_FIELD(type, symbol)                                        \
  PSX_REF(type, (u8*)symbol + (u32)ENTITY_COUNT_S16 * ENTITY_SLOT_SIZE)

/* Spawn-gate byte table addressed through its canonical label; same access as
 * GAME_SPAWN_GATE_BYTE, spelled through the symbol so the address triple
 * matches the original. */
#define SPAWN_GATE_BYTE(index)                                                 \
  PSX_REF(volatile u8, (u8*)D_80144FC4 + (u32)(index))

/* @behavior initializes the next free entity slot from one packed spawn
 * record, applies its saved-state gate, and advances the entity count.
 * @source 0x801A7330
 * @status partial
 * @match 78.37
 * @residual live audit 192/245 instructions; 980 original bytes versus 976
 * current, so the range is one instruction short. Every other instruction
 * class is emitted in exactly the original's quantity; the single missing
 * instruction is a delay-slot `nop`. The original fills the slot after the
 * spawn-counter reload that precedes the 0x80146904 store with the high half
 * of the work-pointer reload, while this source leaves that slot empty and
 * reloads the pointer after the store, so the branch and jump displacements
 * that follow diverge as well. Register naming differs too (a2 versus the
 * original's a1/a0 for the reloaded pointer). Residual class: scheduling and
 * allocation. The six per-entity field labels and the spawn-gate table label
 * are canonical Splat labels whose roles are not yet reviewed. The spawn
 * record stays a volatile view because the original reloads every spawn byte,
 * including spawn[0], spawn[3] and spawn[0xC] twice.
 */
void func_801A7330(const volatile u8* spawn) {
  struct GameWorkArea* work;
  s16                  slot;
  s16                  state;
  s16                  dist;
  s32                  coord_x;
  s32                  coord_y;
  s32                  result;
  u8                   check;

  slot = ENTITY_COUNT_S16;
  if (slot >= ENTITY_SLOT_COUNT) {
    return;
  }

  work = (struct GameWorkArea*)((u8*)D_80146888 + (u32)slot * ENTITY_SLOT_SIZE);
  g_game_work = work;
  GAME_WORK_AREA_PTR = work;
  ENTITY_ENTRY_DATA = (u16)(((u32)spawn[1] << 8) | spawn[2]);

  func_801A4AA8();
  func_8014DD3C(ENTITY_ENTRY_DATA);

  g_game_work->flags_00 = 1;

  coord_x = (s32)spawn[5] << 16;
  if (spawn[4] != 0) {
    coord_x |= 0x8000;
  }
  work = g_game_work;
  work->coord_x_34 = coord_x;
  ENTITY_SLOT_FIELD(s32, D_80146908) = coord_x;

  coord_y = (s32)spawn[7] << 16;
  if (spawn[6] != 0) {
    coord_y |= 0x8000;
  }
  work->coord_y_38 = coord_y;
  ENTITY_SLOT_FIELD(s32, D_8014690C) = coord_y;

  ENTITY_SLOT_FIELD(s32, D_80146910) = 0;

  state = 6;
  dist = func_8015477C(work->coord_x_34, work->coord_y_38);

  work = g_game_work;
  work->unk_01 = state;
  work->counter_3E = dist;
  g_game_work->flags_02 = 0;
  g_game_work->route_index_08 = spawn[0] & 0xF;
  g_game_work->unk_06 = spawn[0] >> 4;

  ENTITY_SLOT_FIELD(u8, D_80146900) = 0;
  ENTITY_SLOT_FIELD(u8, D_8014691C) = 0x7F;

  func_801A4990(spawn + 0xB);

  if ((spawn[3] & 0x80) != 0) {
    g_game_work->flags_00 |= 0x20;
  }

  g_game_work->unk_5D = 0;
  g_game_work->unk_5F = 0;
  g_game_work->unk_5E = 0;
  g_game_work->flags_5C = spawn[3] & 0xF;
  g_game_work->unk_29 = state;

  ENTITY_SLOT_FIELD(u16, D_80146904) = 0;

  work = g_game_work;
  work->unk_18 = (s32)(u16)(((u32)spawn[9] << 8) | spawn[0xA]);
  work->field_05 = spawn[8];
  g_game_work->field_0B = spawn[0xC];
  g_game_work->unk_2A = (u8)((g_game_work->unk_07 >> 4) & 1);

  check = g_game_work->field_05;
  if (((SPAWN_GATE_BYTE(check >> 3) >> (check & 7)) & 1) != 0) {
    if ((spawn[0xC] & 1) != 0) {
      g_game_work->flags_00 = 0;
      return;
    }
    if ((u16)g_game_work->coord_x_34 == 0 &&
        (u16)g_game_work->coord_y_38 == 0) {
      func_801A4BC0(FIELD_REF(s16, g_game_work, 0x36u),
                    FIELD_REF(s16, g_game_work, 0x3Au), 0x10);
    }
    result = 1;
  } else {
    result = 0;
  }

  func_8014D6B8(result);

  ENTITY_COUNT_U16 = ENTITY_COUNT_U16 + 1;
}
