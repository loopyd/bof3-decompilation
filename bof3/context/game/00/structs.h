#ifndef BOF3_CONTEXT_GAME_00_STRUCTS_H
#define BOF3_CONTEXT_GAME_00_STRUCTS_H

/* Work area struct accessed via scratchpad pointer (0x1F800044) */
struct GameWorkArea {
    u8  pad_00[0x02];
    u8  flags_02;
    u8  pad_03[0x05];
    u8  route_index_08;
    u8  pad_09[0x2B];
    s32 coord_x_34;
    s32 coord_y_38;
    u16 counter_3E;
    u8  pad_40[0x09];
    u32 unk_49;
    u8  pad_4D[0x23];
    u8  speed_70;
    u8  pad_71[0x03];
    u16 anim_state_74;
};

#endif
