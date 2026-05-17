#include "internal.h"

void func_801a4aa8(void);
void func_8014dd3c(u16);
s16 func_8015477c(s32 x, s32 y);
void func_801a4990(const u8* data);
void func_801a4bc0(s16 x, s16 y, u32 size);
void func_8014d6b8(u32 flag);

#define ENTITY_MAX         30
#define ENTITY_SLOT_SIZE   0x98

#define ENTITY_COUNTER     (*(volatile s16*)0x1F800000u)
#define ENTITY_ENTRY_DATA  (*(volatile u16*)0x1F800002u)
#define WORK_AREA_BASE     ((struct GameWorkArea* volatile)0x80146888u)
#define WORK_AREA_PTR      (*(volatile struct GameWorkArea**)0x80146884u)

#define GAME_STATE_BASE    0x80140000u

s32 func_801a7330(const u8* spawn_data)
{
    struct GameWorkArea* work;
    s32 coord_x;
    s32 coord_y;
    s32 result;
    s16 slot_index;
    s16 dist;
    u8 check_byte;
    u8 spawn_flags;

    slot_index = ENTITY_COUNTER;
    if (slot_index >= ENTITY_MAX) {
        return 0;
    }

    work = WORK_AREA_BASE + slot_index;
    SCRATCH_WORK = work;
    WORK_AREA_PTR = work;

    ENTITY_ENTRY_DATA = (u16)(spawn_data[1] << 8) | spawn_data[2];

    func_801a4aa8();
    func_8014dd3c(ENTITY_ENTRY_DATA);

    work->flags_00 = 1;

    coord_x = (s32)spawn_data[5] << 16;
    if (spawn_data[4] != 0) {
        coord_x |= 0x8000;
    }
    work->coord_x_34 = coord_x;
    ((volatile s32*)(GAME_STATE_BASE + (s32)slot_index * ENTITY_SLOT_SIZE + 0x6908u))[0] = coord_x;

    coord_y = (s32)spawn_data[7] << 16;
    if (spawn_data[6] != 0) {
        coord_y |= 0x8000;
    }
    work->coord_y_38 = coord_y;
    ((volatile s32*)(GAME_STATE_BASE + (s32)slot_index * ENTITY_SLOT_SIZE + 0x690Cu))[0] = coord_y;

    ((volatile s32*)(GAME_STATE_BASE + (s32)slot_index * ENTITY_SLOT_SIZE + 0x6910u))[0] = 0;

    dist = func_8015477c(work->coord_x_34, work->coord_y_38);
    work->unk_01 = 6;
    work->counter_3E = dist;
    work->flags_02 = 0;
    work->route_index_08 = spawn_data[0] & 0xF;
    work->unk_06 = spawn_data[0] >> 4;

    ((volatile u8*)(GAME_STATE_BASE + (s32)slot_index * ENTITY_SLOT_SIZE + 0x6900u))[0] = 0;
    ((volatile u8*)(GAME_STATE_BASE + (s32)slot_index * ENTITY_SLOT_SIZE + 0x691Cu))[0] = 0x7F;

    func_801a4990(spawn_data + 0xB);

    if (spawn_data[3] & 0x80) {
        work->flags_00 |= 0x20;
    }

    work->unk_5D = 0;
    work->unk_5F = 0;
    work->unk_5E = 0;
    work->flags_5C = spawn_data[3] & 0xF;
    work->unk_29 = 6;

    ((volatile s16*)(GAME_STATE_BASE + (s32)slot_index * ENTITY_SLOT_SIZE + 0x6904u))[0] = 0;

    work->unk_18 = (s32)((u16)(spawn_data[9] << 8) | spawn_data[0xA]);
    work->field_05 = spawn_data[8];
    work->field_0B = spawn_data[0xC];
    work->unk_2A = (work->unk_07 >> 4) & 1;

    check_byte = work->field_05;
    spawn_flags = spawn_data[0xC];

    if ((((volatile u8*)(0x80144FC4u + (u32)(check_byte >> 3)))[0] >> (check_byte & 7)) & 1) {
        if (spawn_flags & 1) {
            work->flags_00 = 0;
            return 0;
        }
        result = 1;
        if ((u16)work->coord_x_34 == 0 && (u16)work->coord_y_38 == 0) {
            func_801a4bc0((s16)work->coord_x_34, (s16)work->coord_y_38, 0x10);
            result = 1;
        }
    } else {
        result = 0;
    }

    func_8014d6b8(result);

    ENTITY_COUNTER = ENTITY_COUNTER + 1;
    return 0;
}
