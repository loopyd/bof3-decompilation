#include "bof3/scenario/scena09_state.h"

/* Shared route/world word every scenario overlay tests before it runs a
 * world-specific arm. */
/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;
/* @source 0x80143F03 @kind unknown */
extern u8 D_80143F03;
/* @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;
/* @source 0x80145F09 @kind unknown */
extern u8 D_80145F09;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;
/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FC91C
 * @behavior Field 2 of this overlay's five-word entry record at 0x801FE3CC
 * (the word stored at 0x801FE3D4). The game overlay reaches that record through
 * its handler-pointer table D_801C8454, whose entry the shared scenario id
 * selects, and calls its field 0 - 0x801F78D0, the overlay's per-frame state
 * dispatcher - with no arguments, so no in-image jal and no other image word of
 * this payload addresses 0x801FC91C and the record's remaining four fields are
 * called only from outside this overlay. It takes a 16.16 fixed-point position
 * as two words and returns 1
 * when it requested the next primary scene state, 0 otherwise. It reads the
 * shared route word D_80143F00 (0x80143F00, read with lui/lhu once per arm)
 * and keeps eight world arms in ascending arm order: 0x25, 0x27, 0x29, 0x2E,
 * 0x31, 0x3B, 0x77 and 0x52. Every arm but the last tests the shared flag word
 * D_8014686C (0x8014686C, reloaded with lui/lw before each test because the
 * wait call clobbers memory) through the two-stage wait helper func_8015B5D4
 * for its own wait ids, then tests a window of the two position words - either
 * as unsigned (u32)(word - base) <= span differences, or on the tile index the
 * arm shifts out of the word, or on one or two exact word values - and only
 * when every test of the arm passes does it run the shared front-end startup
 * helper func_8015C088, arm the shared sub-state byte D_80146875 and the
 * shared primary state byte D_80146874 and answer 1. Some arms arm a scenario
 * flag bit of that same shared word through func_8015B580 first (0x25, 0x15,
 * 0x21) and the 0x31 arm's third sub-arm sets bit 0x0E of the halfword
 * D_80146258 in place. Every arm clears the shared progress byte
 * g_ScenarioProgress (0x80146864) - arms 0x2E and 0x52's last sub-arm store a
 * non-zero value there instead - and most clear the three shared mode bytes
 * D_80146865, D_80146866 and D_80146867 with it. The last arm, 0x52, takes no
 * wait: it returns 0 while the shared byte D_80143F03 is set, then matches two
 * adjacent tile windows of the position, stores the shared byte D_80145E98
 * into scratchpad byte 0 and branches on it (values 4, 5 and 6 answer 0) and on
 * the party byte D_80145F09 (0x80145F09) to pick its sub-state. No arm matched
 * answers 0. It takes the position only, and its 0x20-byte frame keeps $ra plus
 * the two position words in $s0/$s1 across the calls. No byte-identical copy of
 * this 0x95C-byte span exists in any shipped payload under out/binaries, and a
 * scan that keeps only each word's opcode and register fields finds no member of
 * the family either, so the body was written from this span's own bytes: the
 * nearest evidence is the same shape shrunk to one arm in the scena12 work-byte
 * table entry func_801FC890, which is still an asm row.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the first draft measured 412/599 with $s0/$s1 exchanged and
 * every span of 0x100000 and above written at sixteen times its stored value
 * (0x380000 for 0x38000). Correcting the spans to the bytes' own values took it
 * to 577/599 at the original size; the offset of the 0x31 second sub-arm's
 * second test is -0x5C0000 (`lui $v1,0xFFA4` with no `ori`), not -0x51C000, and
 * arm 0x29's zone test and arm 0x31's wait pair are an `if`/`else if` chain whose
 * then-arm's range failures jump past the else, which is what put those two
 * spans at 597/599 until the chain was written. Three operand-order sweeps (the
 * span written on the left, the difference written on the left, and the unsigned
 * cast moved onto the compared word) all measured identically to the installed
 * spelling, so the operand order of these comparisons is not a lever.
 */
s32 requestPrimaryWorldZone(s32 x,
                                                                       s32 y) {
  if (D_80143F00 == 0x25) {
    if (func_8015B5D4(D_8014686C, 0x2E) != 0 &&
        func_8015B5D4(D_8014686C, 0x38) == 0 &&
        (u32)(x - 0x200000) <= 0x38000 && (u32)(y - 0x410000) <= 0x8000) {
      func_8015C088();
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0x0F;
      D_80146874 = 0x0F;
      return 1;
    }
  }
  if (D_80143F00 == 0x27) {
    if ((x == 0x310000 || x == 0x318000) &&
        (u32)(y - 0x270000) <= 0x28000) {
      func_8015C088();
      D_80146875 = 0x1E;
      g_ScenarioProgress = 0;
      D_80146874 = 0x0F;
      return 1;
    }
  }
  if (D_80143F00 == 0x29) {
    if (func_8015B5D4(D_8014686C, 0x38) != 0 &&
        func_8015B5D4(D_8014686C, 0x23) == 0) {
      if (x == 0x400000 || x <= 0x408000) {
        if ((u32)(y - 0x270000) <= 0x18000) {
          func_8015C088();
          D_80146875 = 0x64;
          g_ScenarioProgress = 0;
          D_80146874 = 0x0F;
          return 1;
        }
      }
      if ((u32)(x - 0x480000) <= 0x38000) {
        if (y == 0x250000 || y == 0x258000) {
          func_8015C088();
          D_80146875 = 0x64;
          g_ScenarioProgress = 0;
          D_80146874 = 0x0F;
          return 1;
        }
      }
      if (x == 0x40000 || x == 0x38000) {
        if ((u32)(y - 0x20000) <= 0x18000) {
          func_8015C088();
          D_80146875 = 0x64;
          g_ScenarioProgress = 0;
          D_80146874 = 0x0F;
          return 1;
        }
      }
    }
  }
  if (D_80143F00 == 0x2E) {
    if (func_8015B5D4(D_8014686C, 0x23) != 0 &&
        func_8015B5D4(D_8014686C, 0x24) == 0 &&
        (u32)(x - 0xF0000) <= 0x18000 &&
        (y == 0x220000 || y == 0x228000)) {
      func_8015C088();
      g_ScenarioProgress = 1;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 0x10;
      return 1;
    }
  }
  if (D_80143F00 == 0x31) {
    if (func_8015B5D4(D_8014686C, 0x02) == 0) {
      if ((u32)(x - 0x268000) <= 0x50000 &&
          (u32)(y - 0x2B0000) <= 0x48000) {
        func_8015B580(D_8014686C, 0x25);
        func_8015C088();
        g_ScenarioProgress = 0;
        D_80146865 = 0;
        D_80146866 = 0;
        D_80146867 = 0;
        D_80146875 = 0;
        D_80146874 = 2;
        return 1;
      }
    } else if (func_8015B5D4(D_8014686C, 0x04) == 0) {
      if ((u32)(x - 0x290000) <= 0x28000 &&
          (u32)(y - 0x5C0000) <= 0x8000) {
        func_8015C088();
        D_80146875 = 0x0A;
        g_ScenarioProgress = 0;
        D_80146865 = 0;
        D_80146866 = 0;
        D_80146867 = 0;
        D_80146874 = 2;
        return 1;
      }
    }
    if (func_8015B5D4(D_8014686C, 0x1E) != 0 &&
        func_8015B5D4(D_8014686C, 0x2E) == 0 &&
        (u32)(x - 0x4E0000) <= 0x8000 &&
        (u32)(y - 0x220000) <= 0x38000) {
      D_80146258 |= 0x0E;
      func_8015C088();
      g_ScenarioProgress = 0;
      D_80146875 = 0;
      D_80146874 = 0x0E;
      return 1;
    }
  }
  if (D_80143F00 == 0x3B) {
    if (func_8015B5D4(D_8014686C, 0x3A) != 0) {
      if (func_8015B5D4(D_8014686C, 0x22) == 0) {
        if ((u32)(x - 0x470000) <= 0x28000 &&
            (u32)(y - 0x90000) <= 0x18000) {
          func_8015C088();
          D_80146875 = 0x82;
          g_ScenarioProgress = 0;
          D_80146865 = 0;
          D_80146866 = 0;
          D_80146867 = 0;
          D_80146874 = 0x0F;
          return 1;
        }
      } else if (func_8015B5D4(D_8014686C, 0x23) == 0) {
        if ((u32)(x - 0x290000) <= 0x8000 &&
            (u32)(y - 0x110000) <= 0x78000) {
          func_8015C088();
          D_80146875 = 0x8C;
          g_ScenarioProgress = 0;
          D_80146865 = 0;
          D_80146866 = 0;
          D_80146867 = 0;
          D_80146874 = 0x0F;
          return 1;
        }
      }
    }
  }
  if (D_80143F00 == 0x77) {
    if (func_8015B5D4(D_8014686C, 0x3D) != 0 &&
        func_8015B5D4(D_8014686C, 0x15) == 0 &&
        (u32)(x - 0x420000) <= 0x28000 &&
        (u32)(y - 0x3C0000) <= 0x28000) {
      func_8015B580(D_8014686C, 0x15);
      func_8015C088();
      D_80146875 = 0x14;
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146874 = 8;
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x3D) != 0 &&
        func_8015B5D4(D_8014686C, 0x16) == 0 &&
        (u32)(x - 0x420000) <= 0x28000 &&
        (u32)(y - 0x410000) <= 0x28000) {
      func_8015C088();
      D_80146875 = 0x19;
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146874 = 8;
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x1E) == 0) {
      if ((u32)(x - 0xA0000) <= 0x8000 && (u32)(y - 0x20000) <= 0x10000) {
        func_8015C088();
        g_ScenarioProgress = 0;
        D_80146875 = 0;
        D_80146874 = 9;
        return 1;
      }
    } else if (func_8015B5D4(D_8014686C, 0x2C) == 0) {
      if ((u32)(x - 0x60000) <= 0x8000 && (u32)(y - 0x20000) <= 0x10000) {
        func_8015C088();
        D_80146875 = 0x1E;
        g_ScenarioProgress = 0;
        D_80146874 = 9;
        return 1;
      }
    }
  }
  if (D_80143F00 == 0x52) {
    u8 gate;

    if (D_80143F03 != 0) {
      return 0;
    }
    if (func_8015B5D4(D_8014686C, 0x20) != 0 &&
        func_8015B5D4(D_8014686C, 0x21) == 0 &&
        (u32)((y >> 16) - 0x40) < 5 && (u32)((x >> 16) - 0x22) < 4) {
      func_8015B580(D_8014686C, 0x21);
      func_8015C088();
      D_80146874 = 0x0B;
      D_80146875 = 0x1E;
      return 1;
    }
    if ((u32)((y >> 16) - 0x3C) < 5 && (u32)((x >> 16) - 0x22) < 4) {
      gate = D_80145E98;
      SPAD_REF(u8, 0) = gate;
      if (gate == 5 || gate == 6) {
        return 0;
      }
      if (gate == 4) {
        return 0;
      }
      if (D_80145F09 != 2) {
        if (func_8015B5D4(D_8014686C, 0x20) != 0 &&
            func_8015B5D4(D_8014686C, 0x21) == 0) {
          return 0;
        }
        g_ScenarioProgress = 0x10;
        func_8015C088();
        return 1;
      }
      if (func_8015B5D4(D_8014686C, 0x20) != 0) {
        return 0;
      }
      func_8015C088();
      D_80146874 = 0x0B;
      D_80146875 = 0;
      return 1;
    }
    return 0;
  }
  return 0;
}
