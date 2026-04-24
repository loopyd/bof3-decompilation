#ifndef BOF3_ORIGINAL_SYMBOLS_H
#define BOF3_ORIGINAL_SYMBOLS_H

/*
 * Typed calls into original BOF3 code that is not lifted into this source tree
 * yet. Use these only for named functions whose Ghidra/default name encodes
 * the target address.
 */

#define BOF3_ORIGINAL_FUNC(return_type, address, args) \
  ((return_type(*) args)(address))

#define func_8014b020 BOF3_ORIGINAL_FUNC(void, 0x8014b020, (void))
#define func_8014b0f0 BOF3_ORIGINAL_FUNC(void, 0x8014b0f0, (void))
#define func_8014fc00 BOF3_ORIGINAL_FUNC(void, 0x8014fc00, (s32))
#define func_80174668 BOF3_ORIGINAL_FUNC(void, 0x80174668, (s32))
#define func_80174700 BOF3_ORIGINAL_FUNC(void, 0x80174700, (s32))
#define func_801748e4 BOF3_ORIGINAL_FUNC(void, 0x801748e4, (void))
#define func_801753c4 BOF3_ORIGINAL_FUNC(void, 0x801753c4, (s32))
#define func_801753ec BOF3_ORIGINAL_FUNC(s32, 0x801753ec, (void))
#define func_80178138 BOF3_ORIGINAL_FUNC(s32, 0x80178138, (s32, void*, s32))
#define func_80178218 BOF3_ORIGINAL_FUNC(s32, 0x80178218, (s32, void*))
#define func_80178660 BOF3_ORIGINAL_FUNC(void, 0x80178660, (void))
#define func_801790a8 BOF3_ORIGINAL_FUNC(void, 0x801790a8, (s32, s32))
#define func_801790c8 BOF3_ORIGINAL_FUNC(void, 0x801790c8, (s32))
#define func_8017af0c BOF3_ORIGINAL_FUNC(void, 0x8017af0c, (s32))
#define func_8017e07c BOF3_ORIGINAL_FUNC(void, 0x8017e07c, (void))
#define func_8017e0b4 BOF3_ORIGINAL_FUNC(void, 0x8017e0b4, (void))
#define func_8017ed3c \
  BOF3_ORIGINAL_FUNC(s32, 0x8017ed3c, (s32, s32, s32, void*))
#define func_8017ed7c BOF3_ORIGINAL_FUNC(void, 0x8017ed7c, (s32))
#define func_8017ee0c BOF3_ORIGINAL_FUNC(void, 0x8017ee0c, (void))
#define func_8017ee1c BOF3_ORIGINAL_FUNC(void, 0x8017ee1c, (void))
#define func_8017eebc BOF3_ORIGINAL_FUNC(void, 0x8017eebc, (s32))
#define func_801ce760 BOF3_ORIGINAL_FUNC(void, 0x801ce760, (void*, u_long))
#define func_801cea98 BOF3_ORIGINAL_FUNC(int, 0x801cea98, (void))
#define func_801cebfc BOF3_ORIGINAL_FUNC(void, 0x801cebfc, (void))

#endif
