#include "bof3/context.h"
#include "internal.h"

extern int sprintf(char* buffer, const char* format, ...);

extern u8          D_80145AD4[];
extern const char  D_80149990[];
extern const void* D_8017F470[];
extern const void* D_8017F4B8[];
extern const void* PTR_s_EXCEPTION_8017f504;
extern const void* PTR_s_INTERRRUPT_8017f508[];

/* @behavior displays the boot exception register dump and loops forever updating
 * the double-buffered debug screen.
 * @source 0x8014B3C4
 */
void func_8014B3C4(void) {
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

  func_8017EE1C();

  while (1) {
    exception_pc = (const u32*)exception_frame[0x22];
    VSync(2);
    y = 0x3c;

    PutDispEnv((DISPENV*)D_80143E68);
    PutDrawEnv((DRAWENV*)(D_80143E68 + 0x14));
    label_table = D_8017F470;
    DrawOTag((u_long*)(D_80143E68 + 0x8c));

    D_80143D44 = D_80143D44 ^ 1;
    D_80143E68 = &D_80143D48[D_80143D44 * 0x90];

    ClearOTagR((u_long*)(D_80143E68 + 0x70), 8);
    func_8014B020();

    for (i = 0; ((unsigned int)0x12) > i; i++) {
      func_80150098(0x14, y, 0, (void*)label_table[i]);
      sprintf((char*)D_80145AD4, D_80149990, saved_context[i + 2]);
      func_80150098(0x3c, y, 0, (void*)D_80145AD4);
      y += 8;
    }

    y = 0x3c;
    label_table = D_8017F4B8;

    for (i = 0x12; ((unsigned long)i) < 0x25; i++) {
      func_80150098(0xa0, y, 0, (void*)label_table[i - 0x12]);
      sprintf((char*)D_80145AD4, D_80149990, saved_context[i + 2]);
      func_80150098(0xc8, y, 0, (void*)D_80145AD4);
      y += 8;
    }

    func_80150098(0x14, 0x14, 0, (void*)PTR_s_EXCEPTION_8017f504);
    func_80150098(0x64, 0x14, 2,
                  *(void* const*)((const u8*)PTR_s_INTERRRUPT_8017f508 +
                                  (exception_frame[0x26] & 0x3c)));

    sprintf((char*)D_80145AD4, D_80149990, (u32)exception_pc);
    func_80150098(0x46, 0x24, 3, (void*)D_80145AD4);
    sprintf((char*)D_80145AD4, D_80149990, exception_pc[0]);
    func_80150098(0x8e, 0x24, 4, (void*)D_80145AD4);

    DrawSync(0);
    func_8014B0F0();
    D_80143E6C += 1;
  }
}
