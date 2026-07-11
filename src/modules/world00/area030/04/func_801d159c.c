#include "internal.h"

/* @behavior queues the fixed AREA030 labels, formats one countdown string into the
 * shared UI buffer, then draws the local footer text and status panel.
 * @source 0x801d159c FUN_801d159c
 */
void func_801d159c(s16 arg0, s16 arg1) {
  s32 value;
  s32 mode;

  func_801e0c80(5, 3);
  func_801e0dcc(0xd, 3, (s16)(arg0 + 0x18), (s16)(arg1 + 8));
  func_801e0c80(0, 3);
  func_801e0dcc(0xe, 3, arg0, arg1);
  func_801e0dcc(0xf, 3, (s16)(arg0 + 0x100), arg1);

  if (WORLD00_AREA030_GLOBAL_BYTE_3FC9 == 0u) {
    value = 0;
  } else {
    value = 0x3e - WORLD00_AREA030_GLOBAL_BYTE_4002;
  }
  if (value < 0) {
    value = 0;
  }

  func_8017e3f4((char*)WORLD00_AREA030_UI_CHAR_BUFFER, (char*)0x801d0c04u,
                (s8)value);

  if ((value < 0x32) || (WORLD00_AREA030_GLOBAL_BYTE_5E92 != 4u) ||
      ((WORLD00_AREA030_GLOBAL_WORD_3E6C & 4u) == 0u)) {
    mode = 0;
  } else {
    mode = 2;
  }

  func_8014ff0c((s16)(arg0 + 0xf0), (s16)(arg1 + 0x1a), mode,
                (const void*)WORLD00_AREA030_UI_CHAR_BUFFER);
  func_801d195c(arg0, arg1);
}
