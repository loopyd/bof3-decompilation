#ifndef BOF3_CONTEXT_01_GLOBALS_H
#define BOF3_CONTEXT_01_GLOBALS_H

/* global data register/memory macros */

#define BOF3_GAME_FRONT_EFFECT_BUSY            (*(volatile u16*)0x80143c40u)
#define BOF3_GAME_FRONT_PAD_STATE              (*(volatile u16*)0x80145aa8u)
#define BOF3_GAME_FRONT_STATE                  (*(volatile u16*)0x80143c10u)
#define BOF3_GAME_FRONT_TIMER                  (*(volatile u16*)0x80143c20u)
#define BOF3_GAME_FRONT_BANNER_SCROLL          (*(volatile u16*)0x80143c22u)
#define BOF3_GAME_FRONT_BANNER_ALPHA           (*(volatile u16*)0x80143c24u)
#define BOF3_GAME_FRONT_WINDOW_ALPHA_PRIMARY   (*(volatile u16*)0x80143c26u)
#define BOF3_GAME_FRONT_WINDOW_ALPHA_SECONDARY (*(volatile u16*)0x80143c28u)
#define BOF3_GAME_FRONT_FADE_PHASE             (*(volatile u8*)0x80143c31u)
#define BOF3_GAME_FRONT_WINDOW_PHASE           (*(volatile u8*)0x80143c32u)
#define BOF3_GAME_FRONT_INPUT_GATE             (*(volatile u8*)0x80143c33u)
#define BOF3_GAME_FRONT_SELECTION              (*(volatile u8*)0x80145029u)
#define BOF3_GAME_FRONT_PALETTE_STAGE_SERIAL   (*(volatile u8*)0x80145988u)
#endif
