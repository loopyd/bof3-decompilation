#include "internal.h"

/* @behavior seeds one local rotation triplet, applies it to four packed source
 * vertices, and writes the transformed positions back into the destination slot
 * relative to the current state origin.
 * @source 0x801f3708 FUN_801f3708
 */
void func_801f3708(void* arg0, const void* arg1, s16* arg2) {
  volatile u8*       dst;
  const volatile u8* src;
  MATRIX             matrix;
  SVECTOR            vertex;
  VECTOR             screen;
  long               flag;

  dst = (volatile u8*)arg0;
  src = (const volatile u8*)arg1;

  arg2[8] = rand() & 0xfc0;
  arg2[9] = rand() & 0xfc0;
  arg2[10] = rand() & 0xfc0;

  RotMatrix((SVECTOR*)((u8*)arg2 + 0x10), &matrix);
  matrix.t[0] = 0;
  matrix.t[1] = 0;
  matrix.t[2] = 0;
  SetTransMatrix(&matrix);
  SetRotMatrix(&matrix);

  vertex.pad = 0;

  vertex.vx = *(const volatile s16*)(src + 2);
  vertex.vy = *(const volatile s16*)(src + 4);
  vertex.vz = *(const volatile s16*)(src + 6);
  RotTrans(&vertex, &screen, &flag);
  *(volatile s16*)(dst + 2) = arg2[0] + screen.vx;
  *(volatile s16*)(dst + 4) = arg2[1] + screen.vy;
  *(volatile s16*)(dst + 6) = arg2[2] + screen.vz;

  vertex.vx = *(const volatile s16*)(src + 8);
  vertex.vy = *(const volatile s16*)(src + 10);
  vertex.vz = *(const volatile s16*)(src + 12);
  RotTrans(&vertex, &screen, &flag);
  *(volatile s16*)(dst + 8) = arg2[0] + screen.vx;
  *(volatile s16*)(dst + 10) = arg2[1] + screen.vy;
  *(volatile s16*)(dst + 12) = arg2[2] + screen.vz;

  vertex.vx = *(const volatile s16*)(src + 14);
  vertex.vy = *(const volatile s16*)(src + 16);
  vertex.vz = *(const volatile s16*)(src + 18);
  RotTrans(&vertex, &screen, &flag);
  *(volatile s16*)(dst + 14) = arg2[0] + screen.vx;
  *(volatile s16*)(dst + 16) = arg2[1] + screen.vy;
  *(volatile s16*)(dst + 18) = arg2[2] + screen.vz;

  vertex.vx = *(const volatile s16*)(src + 20);
  vertex.vy = *(const volatile s16*)(src + 22);
  vertex.vz = *(const volatile s16*)(src + 24);
  RotTrans(&vertex, &screen, &flag);
  *(volatile s16*)(dst + 20) = arg2[0] + screen.vx;
  *(volatile s16*)(dst + 22) = arg2[1] + screen.vy;
  *(volatile s16*)(dst + 24) = arg2[2] + screen.vz;
}
