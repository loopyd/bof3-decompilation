#include "bof3/scenario/scena00_internal.h"

/* @behavior Runs the primary SCENA00 state controller selected by the state byte
 * D_80146875: state 0 clears the tick word D_80146876 and raises flag 0x80 in the
 * mode word D_80146258 when the gate word D_80143C40 is clear; state 1 ticks
 * D_80146876 up to 0x7C before arming effect bank 0x1F in D_8014832E,
 * D_80146867 = 1 and state 2; state 2 advances to state 3 when D_80146867 reads
 * 0x54; state 3 clears D_8014832E and requests primary mode 2 in D_80143BB0;
 * state 4 seeds the front selector context (4, 0x440000, 0x80000, 5); state 5
 * sets D_8014832E = 0x1F and advances to state 6; state 6, when the shared mode
 * byte D_80146866 reads 0x23, stores the object byte func_8019601C() in
 * scratchpad 0x1F800000 and seeds that object's 0x74-byte row at 0x80143FC8
 * (+0 = 1, +5 = 0x13, +9 = 0x60 and the s16 words D_801492D8 + 0x180,
 * D_801492DA, D_801492DC at +0x64/+0x68/+0x6C) unless the byte is 0xFF; state 7,
 * when D_80146867 reads 0x60, seeds the front selector context
 * (0x1F, 0x70000, 0x140000, 0) and requests primary state 3 by storing
 * D_80146874 = 3 and D_80146875 = 0.
 * @source 0x801F9BE4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F9BE4(void) {
  switch (D_80146875) {
    case 0:
      if (D_80143C40 != 0u) {
        break;
      }

      func_8016C0C0(0, 0);
      func_80150224(0x10u);
      D_80143BB0 = 2u;
      func_80161C20(6u, 0x5a, 8);
      D_80146876 = 0u;
      D_80146875 = 1u;
      D_80146258 |= 0x80u;
      break;

    case 1: {
      u16* counter = (u16*)&D_80146876;

      func_8016C0C0((s32)(s16)*counter, (s32)(s16)*counter);
      if (*counter < 0x7cu) {
        *counter = (u16)(*counter + 1u);
      }

      if (D_80143BB0 == 2u) {
        break;
      }

      func_8016C0C0(0x7f, 0x7f);
      D_8014832E = 0x1fu;
      func_8015C088();
      func_8015C100();
      func_8014ECAC(1u);
      D_80146867 = 1u;
      *counter = 0u;
      D_80146875 = 2u;
      break;
    }

    case 2:
      if (D_80146867 == 0x54u) {
        func_8014ECAC(0u);
        D_80146875 = 3u;
      }
      break;

    case 3:
      if (D_80143C40 == 0u) {
        D_8014832E = 0u;
        func_80150224(0x11u);
        D_80143BB0 = 2u;
        D_80146875 = 4u;
      }
      break;

    case 4:
      if (D_80143BB0 == 2u) {
        break;
      }

      func_8019FA28(4u, 0x440000u, 0x80000u, 5u);
      D_80146875 = 5u;
      break;

    case 5:
      if (D_80143F03 == 2u) {
        D_8014832E = 0x1fu;
        D_80146875 = 6u;
      }
      break;

    case 6: {
      u8* scratch;
      u8  object_index;

      if (D_80146866 != 0x23u) {
        break;
      }

      object_index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = object_index;

      if (object_index != 0xffu) {
        D_80143FC8[(u32)object_index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32*)(D_8014402C + (u32)*scratch * 0x74u) =
            (s32)((s16)D_801492D8 + 0x180);
        *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
        *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DC;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x60u;
      }

      D_80146875 = 7u;
      break;
    }

    case 7:
      if (D_80146867 == 0x60u) {
        func_8019FA28(0x1fu, 0x70000u, 0x140000u, 0u);
        D_80143F1D = 0xffu;
        func_80161CD0(6u, 0x5a, 0x40);
        D_80146874 = 3u;
        D_80146875 = 0u;
      }
      break;
  }
}
