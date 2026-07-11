#include "bof3/context.h"
#include "internal.h"

void func_80150098(s16 x, s16 y, u32 arg2, void* arg3);
int  sprintf(char* buffer, const char* format, ...);

extern u8          DAT_80143d44;
extern u8          DAT_80143d48[];
extern u8*         DAT_80143e68;
extern s32         DAT_80143e6c;
extern u8          DAT_80145ad4[];
extern const char  DAT_80149990[];
extern const void* PTR_DAT_8017f470[];
extern const void* PTR_DAT_8017f4b8[];
extern const void* PTR_s_EXCEPTION_8017f504;
extern const void* PTR_s_INTERRRUPT_8017f508[];

/* @behavior displays the boot exception register dump and loops forever updating
 * the double-buffered debug screen.
 * @source 0x8014b3c4 FUN_8014b3c4
 */
void func_8014b3c4(void) {
  const u32*   exception_frame;
  const u32*   exception_pc;
  const u32*   src;
  const void** label_table;
  u32*         dst;
  u32          saved_context[48];
  unsigned int i;
  s16          y;

  exception_frame = **(const u32***)0x108;
  src = exception_frame;
  dst = saved_context;

  do {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = exception_frame[3];
    src += 4;
    dst += 4;
  } while (src != exception_frame + 48);

  func_8017ee1c();

  while (1) {
    exception_pc = (const u32*)exception_frame[0x22];
    VSync(2);
    y = 0x3c;

    PutDispEnv((DISPENV*)DAT_80143e68);
    PutDrawEnv((DRAWENV*)(DAT_80143e68 + 0x14));
    label_table = PTR_DAT_8017f470;
    DrawOTag((u_long*)(DAT_80143e68 + 0x8c));

    DAT_80143d44 = DAT_80143d44 ^ 1;
    DAT_80143e68 = &DAT_80143d48[DAT_80143d44 * 0x90];

    ClearOTagR((u_long*)(DAT_80143e68 + 0x70), 8);
    func_8014b020();

    for (i = 0; ((unsigned int)0x12) > i; i++) {
      func_80150098(0x14, y, 0, (void*)label_table[i]);
      sprintf((char*)DAT_80145ad4, DAT_80149990, saved_context[i + 2]);
      func_80150098(0x3c, y, 0, (void*)DAT_80145ad4);
      y += 8;
    }

    y = 0x3c;
    label_table = PTR_DAT_8017f4b8;

    for (i = 0x12; ((unsigned long)i) < 0x25; i++) {
      func_80150098(0xa0, y, 0, (void*)label_table[i - 0x12]);
      sprintf((char*)DAT_80145ad4, DAT_80149990, saved_context[i + 2]);
      func_80150098(0xc8, y, 0, (void*)DAT_80145ad4);
      y += 8;
    }

    func_80150098(0x14, 0x14, 0, (void*)PTR_s_EXCEPTION_8017f504);
    func_80150098(0x64, 0x14, 2,
                  *(void* const*)((const u8*)PTR_s_INTERRRUPT_8017f508 +
                                  (exception_frame[0x26] & 0x3c)));

    sprintf((char*)DAT_80145ad4, DAT_80149990, (u32)exception_pc);
    func_80150098(0x46, 0x24, 3, (void*)DAT_80145ad4);
    sprintf((char*)DAT_80145ad4, DAT_80149990, exception_pc[0]);
    func_80150098(0x8e, 0x24, 4, (void*)DAT_80145ad4);

    DrawSync(0);
    func_8014b0f0();
    DAT_80143e6c += 1;
  }
}
