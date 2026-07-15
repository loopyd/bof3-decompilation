#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "bof3/defines.h"

/*
 * Cross-module function declarations.
 *
 * Organized by address range / suspected subsystem. Move to a confirmed
 * subsystem barrel once the owning target and behavior are proven.
 */

/* ---- core (0x8014xxxx) ---- */
extern void func_8014b020(void);
extern void func_8014b0f0(void);
extern void func_8014fc00(s32);

/* ---- game (0x8017xxxx–0x8019xxxx) ---- */
extern void func_80174668(s32);
extern s32  func_80174700(s32);
extern void func_801748e4(void);
extern void func_801753c4(s32);
extern s32  func_801753ec(void);
extern s32  func_80178138(s32, void*, s32);
extern s32  func_80178218(s32, void*);
extern void func_80178660(void);
extern void func_801790a8(s32, s32);
extern void func_801790c8(s32);
extern void func_8017af0c(s32);
extern void func_8017e07c(void);
extern void func_8017e0b4(void);
extern s32  func_8017ed3c(s32, s32, s32, void*);
extern void func_8017ed7c(s32);
extern void func_8017ee0c(void);
extern void func_8017ee1c(void);
extern void func_8017eebc(s32);
extern void func_80196f78(void);

#endif
