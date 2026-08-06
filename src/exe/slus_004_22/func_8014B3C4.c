#include "bof3/context.h"
#include "base/barrier.h"
#include "internal.h"

extern int sprintf(char* buffer, const char* format, ...);

typedef struct ExceptionQuad {
  u32 w[4];
} ExceptionQuad;

extern u8          D_80145AD4[];
extern const char  D_80149990[];
extern const void* D_8017F470[];
extern const void* PTR_s_EXCEPTION_8017f504;
extern const void* PTR_s_INTERRRUPT_8017f508[];

/* @behavior displays the boot exception register dump and loops forever updating
 * the double-buffered debug screen.
 * @source 0x8014B3C4
 */
void func_8014B3C4(void) {
  const u32*           exception_frame;
  const u32*           exception_pc;
  const ExceptionQuad* src;
  const void**         label_table;
  const char*          fmt;
  ExceptionQuad*       dst;
  u32                  cause;
  u32                  saved_context[48];
  unsigned int         i;
  s16                  x1;
  s32                  x2;
  s16                  y;

  exception_frame = **(const u32***)0x108;
  src = (const ExceptionQuad*)exception_frame;
  dst = (ExceptionQuad*)saved_context;

  do {
    *dst = *src;
    src++;
    dst++;
  } while (src != (const ExceptionQuad*)(exception_frame + 48));

  cause = exception_frame[0x26];
  /* MATCHING_AID barrier(): original stores cause (sw 208(sp)) before
   * loading exception_pc (lw s8,136(a2)); without this barrier GCC
   * schedules the load first. */
  barrier();
  exception_pc = (const u32*)exception_frame[0x22];
  ExitCriticalSection();

  while (1) {
    VSync(2);
    x1 = 0x14;
    y = 0x3c;
    i = 0;
    x2 = 0x3c << 16;

    PutDispEnv((DISPENV*)D_80143E68);
    PutDrawEnv((DRAWENV*)(D_80143E68 + 0x14));
    label_table = D_8017F470;
    DrawOTag((u_long*)(D_80143E68 + 0x8c));

    D_80143D44 = D_80143D44 ^ 1;
    D_80143E68 = &D_80143D48[D_80143D44 * 0x90];

    ClearOTagR((u_long*)(D_80143E68 + 0x70), 8);
    func_8014B020();

    for (; i < 0x12; i++) {
      func_80150098(x1, y, 0, (const u8*)label_table[i]);
      sprintf((char*)D_80145AD4, D_80149990, saved_context[i + 2]);
      func_80150098((s16)(x2 >> 16), y, 0, (const u8*)D_80145AD4);
      y += 8;
    }

    x1 = 0xa0;
    y = 0x3c;
    x2 = 0xc8 << 16;

    for (i = 0x12; i < 0x25; i++) {
      func_80150098(x1, y, 0, (const u8*)label_table[i]);
      sprintf((char*)D_80145AD4, D_80149990, saved_context[i + 2]);
      func_80150098((s16)(x2 >> 16), y, 0, (const u8*)D_80145AD4);
      y += 8;
    }

    func_80150098(0x14, 0x14, 0, (const u8*)PTR_s_EXCEPTION_8017f504);
    func_80150098(0x64, 0x14, 2,
                  *(void* const*)((const u8*)PTR_s_INTERRRUPT_8017f508 +
                                  (cause & 0x3c)));

    fmt = D_80149990;
    sprintf((char*)D_80145AD4, fmt, (u32)exception_pc);
    func_80150098(0x46, 0x24, 3, (const u8*)D_80145AD4);
    sprintf((char*)D_80145AD4, fmt, exception_pc[0]);
    func_80150098(0x8e, 0x24, 4, (const u8*)D_80145AD4);

    DrawSync(0);
    linkRenderOtPackets();
    D_80143E6C += 1;
  }
}
