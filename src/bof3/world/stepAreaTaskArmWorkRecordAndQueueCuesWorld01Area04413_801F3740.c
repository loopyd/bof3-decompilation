#include "bof3/bof3.h"

extern u8 D_801448EB;
extern s8 D_801448EC;
extern u8 D_80144F28[];
extern u8 D_80146867;
extern u8 D_80147A61;
extern u8 D_80147AF9;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015DF18(u16 arg0);
void func_801F2DF4(void);
void func_801F318C(u32 index, u32 level);

/* @source 0x801F3740
 * @behavior Steps the shared area task whose state byte is D_801448EC, reading
 *           the mode byte D_80146867 once at entry into its low two bits and its
 *           bits 4-5. State 0 waits on event flag 0x17 of the D_80144F28 flag
 *           bank: level 0 arms record slot variant 7 and sets the mode byte's
 *           bit 0x20 over the kept low nibble, level 2 arms variant 3 and keeps
 *           the low nibble alone, and either success queues frontend cue 0x201
 *           and latches state 1 while every other level latches state 10. State
 *           1 first returns while the render gate byte D_80147AF9 is set, then
 *           ticks the animation records through func_801F2DF4 and falls into
 *           state 10's block, which arms variant 5 and sets the mode byte's low
 *           bits to 2 over the kept high nibble, or arms variant 1 and keeps the
 *           high nibble alone, each only when flag 0x17 answers its own polarity;
 *           each armed arm queues cue 0x20E while the state byte still reads 1
 *           and cue 0x201 otherwise, then latches state 0xB. The same block then
 *           advances a state outside 0xB to 0x14, queueing cue 0x202 first when
 *           the state byte reads 1. State 11 returns while D_80147A61 is set and
 *           otherwise ticks the records, queues cue 0x202 and latches 0x14.
 *           State 20 clears D_801448EB and the state byte and runs the shared
 *           release helper func_8015C058. State 19 and every other state that
 *           the table does not dispatch returns immediately.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepAreaTaskArmWorkRecordAndQueueCuesWorld01Area04413_801F3740(void) {
  u8 mode;
  u8 choice;
  u8 level;
  s8 *state;

  mode = D_80146867;
  choice = mode & 3;
  level = (u8)((mode & 0x30) >> 4);

  switch (D_801448EC) {
  case 0:
    if (level == 0) {
      if (func_8015B5D4((u32)D_80144F28, 0x17) == 0) {
        D_801448EC = 10;
        break;
      }
      func_801F318C(7, 1);
      func_8015DF18(0x201);
      {
        u8 *gate;
        u8 byte;

        gate = &D_80146867;
        byte = *gate;
        D_801448EC = 1;
        *gate = (byte & 0xF) | 0x20;
      }
      break;
    }
    if (level == 2) {
      if (func_8015B5D4((u32)D_80144F28, 0x17) != 0) {
        D_801448EC = 10;
        break;
      }
      func_801F318C(3, 1);
      func_8015DF18(0x201);
      {
        u8 *gate;
        u8 byte;

        gate = &D_80146867;
        byte = *gate;
        D_801448EC = 1;
        *gate = byte & 0xF;
      }
      break;
    }
    D_801448EC = 10;
    break;
  case 1:
    if (D_80147AF9 != 0) {
      break;
    }
    func_801F2DF4();
    /* fall through */
  case 10:
    if (choice == 0) {
      if (func_8015B5D4((u32)D_80144F28, 0x17) != 0) {
        func_801F318C(5, 0);
        {
          u8 *gate;
          u8 byte;

          gate = &D_80146867;
          byte = *gate;
          *gate = (byte & 0xF0) | 2;
        }
        if (D_801448EC == 1) {
          func_8015DF18(0x20E);
        } else {
          func_8015DF18(0x201);
        }
        D_801448EC = 0xB;
      }
    } else if (choice == 2) {
      if (func_8015B5D4((u32)D_80144F28, 0x17) == 0) {
        func_801F318C(1, 0);
        {
          u8 *gate;
          u8 byte;

          gate = &D_80146867;
          byte = *gate;
          *gate = byte & 0xF0;
        }
        if (D_801448EC == 1) {
          func_8015DF18(0x20E);
        } else {
          func_8015DF18(0x201);
        }
        D_801448EC = 0xB;
      }
    }
    state = &D_801448EC;
    if (*state != 0xB) {
      if (*state == 1) {
        func_8015DF18(0x202);
      }
      *state = 0x14;
    }
    break;
  case 11:
    if (D_80147A61 != 0) {
      break;
    }
    func_801F2DF4();
    func_8015DF18(0x202);
    D_801448EC = 0x14;
    break;
  case 19:
    break;
  case 20:
    D_801448EB = 0;
    D_801448EC = 0;
    func_8015C058();
    break;
  }
}
