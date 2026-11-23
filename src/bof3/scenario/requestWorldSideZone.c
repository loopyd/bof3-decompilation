#include "bof3/bof3.h"

/* Shared world id halfword of the main-executable data segment: this body tests
 * it once per world gate, against 0x8D, 0x8E, 0x94, 0xAC and 0xBF in that order.
 * @source 0x80143F00 @kind unknown
 */
extern u16 D_80143F00;
/* Front-side byte of the same segment, selecting the arm set of the 0x8D and
 * 0x8E worlds and gating every other world on 0.
 * @source 0x80143F03 @kind unknown
 */
extern u8 D_80143F03;
/* Main-RAM flag bank base, whose address is handed to the flag tester for the
 * single 0x7A probe of the front-side-1 arm of world 0x8D.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;
/* Shared scenario flag word, handed to the main-executable flag tester
 * func_8015B5D4 with ids 0x15, 0x16, 0x17, 0x19, 0x1A, 0x1C, 0x1F, 8 and 9.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;
/* Shared primary state byte of the front-end state tuple.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;
/* Shared secondary sub-state byte of the same tuple.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

s32 func_8015B5D4(u32 flags, s32 id);
void func_8015C088(void);
void func_8015D404(u32 arg0, s32 arg1);
void func_80161BBC(u32 selection_id);

/* @source 0x801FB84C
 * @behavior Overlay front-side gate that requests one front-end state pair per
 * world and returns 1 only for the two arms that also leave the field towards a
 * new scene. It first tests the shared world id halfword D_80143F00 against
 * 0x8D and, under that world, splits on the front-side byte D_80143F03: front
 * side 0 takes the first arm whose flag probe on the shared flag word D_8014686C
 * reports the tested id clear through func_8015B5D4, trying ids 0x15, 0x16 and
 * 0x17 in order, and each of those three sub-arms needs the first argument to
 * stay at or below its own 16.16 bound (0x4F7FFF, 0x497FFF, 0x407FFF) and the
 * arithmetic high half of the second argument to lie in 0x7..0xE or 0x9..0xC; a
 * pass runs the shared front-end startup helper func_8015C088, stores 7 in the
 * shared primary state byte D_80146874 and 0, 1 or 2 in the shared secondary
 * sub-state byte D_80146875. Front side 1 needs the main-RAM flag bank
 * D_80144F28 to answer flag 0x7A set and then the same flag-word probe to report
 * id 0x19 or, failing that, 0x1A clear, with the first argument at or below
 * 0x397FFF or 0x2D7FFF and the high half of the second in 0x31..0x34, and stores
 * the pair 7/3 or 7/4. The world 0x8E gate splits the same way: front side 1
 * needs flag 0x1C clear, the second argument at or below 0x1D7FFF and the high
 * half of the first in 0x13..0x16 to store 7/5, while front side 3 needs flag
 * 0x1F clear, the first argument at or below 0x2C7FFF and the high half of the
 * second in 0x26..0x27 to run func_8015C088, then func_8015D404(0, 0) and
 * func_80161BBC(0x9F), and store 7/0xA. The world 0x94 gate needs front side 0,
 * flag 0x11 clear, the high half of the second argument exactly 0x1C and the
 * high half of the first in 0x12..0x14 to run func_8015C088, store 4/0xA and
 * return 1. The world 0xAC gate needs front side 0, flag 0x13 clear, the first
 * argument at or below 0xB7FFF and the high half of the second in 0x3F..0x40 to
 * store 6/0xA. The world 0xBF gate needs front side 0, the high half of the
 * first argument in 0x1D..0x21, the high half of the second in 0x14..0x15, flag
 * 8 set and flag 9 clear to run func_8015C088, store 2/0x1D and return 1. Every
 * other path returns 0. It takes its two arguments in $a0 and $a1, keeps them in
 * $s0 and $s1, and its 0x20-byte frame saves those two plus $ra; no in-image jal
 * or pointer word names the address. The whole 0x4E0-byte span is one function
 * with one prologue and one jr $ra, so no carve or data promotion applies, and a
 * byte scan of the 7161 extracted files found the span only in this target's own
 * SCENA14 slot 0 payload, so the clone lever is ineligible for want of any
 * byte-identical copy; the nearest same-family spans (SCENA13 0x801FCF4C and
 * SCENA10 0x801FF624, 227 and 223 equal bytes of 1248) carry different world and
 * flag sets and seed nothing.
 *
 * Representation notes. Both front-side tests inside a world gate are written as
 * independent `if` statements rather than as an `else if` chain. That is the form
 * that measured exact: with `else if (D_80143F03 == 1)` and `== 3` every
 * successful arm jumps straight to the join past the next gate and the body
 * measured 252/312 instructions in 1216 bytes, while the two separate tests leave
 * each successful arm on the following gate the way the original does, which
 * measured 312/312 in 1248 bytes. The store tails then group the way the original
 * groups them, one shared secondary-state store per flag chain. Nesting the range
 * tests instead of the flat `&&`, nesting the flag chains in an `else` block,
 * spelling the front-side-0 test as `!D_80143F03`, and passing the sub-state
 * through a local both per arm and per chain all measured 252/312 or worse (1180
 * to 1228 bytes) and are not retained. The final live gate measured every
 * instruction and all 1248 bytes matching.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestWorldSideZone(
    s32 arg0, s32 arg1) {
  if (D_80143F00 == 0x8D) {
    if (D_80143F03 == 0) {
      if (func_8015B5D4(D_8014686C, 0x15) == 0) {
        if (arg0 <= 0x4F7FFF && (u32)((arg1 >> 16) - 7) < 8u) {
          func_8015C088();
          D_80146874 = 7;
          D_80146875 = 0;
        }
      } else if (func_8015B5D4(D_8014686C, 0x16) == 0) {
        if (arg0 <= 0x497FFF && (u32)((arg1 >> 16) - 9) < 4u) {
          func_8015C088();
          D_80146874 = 7;
          D_80146875 = 1;
        }
      } else if (func_8015B5D4(D_8014686C, 0x17) == 0) {
        if (arg0 <= 0x407FFF && (u32)((arg1 >> 16) - 9) < 4u) {
          func_8015C088();
          D_80146874 = 7;
          D_80146875 = 2;
        }
      }
    }
    if (D_80143F03 == 1) {
      if (func_8015B5D4((u32)&D_80144F28, 0x7A) != 0) {
        if (func_8015B5D4(D_8014686C, 0x19) == 0) {
          if (arg0 <= 0x397FFF && (u32)((arg1 >> 16) - 0x31) < 4u) {
            func_8015C088();
            D_80146874 = 7;
            D_80146875 = 3;
          }
        } else if (func_8015B5D4(D_8014686C, 0x1A) == 0) {
          if (arg0 <= 0x2D7FFF && (u32)((arg1 >> 16) - 0x31) < 4u) {
            func_8015C088();
            D_80146874 = 7;
            D_80146875 = 4;
          }
        }
      }
    }
  }
  if (D_80143F00 == 0x8E) {
    if (D_80143F03 == 1) {
      if (func_8015B5D4(D_8014686C, 0x1C) == 0 && arg1 <= 0x1D7FFF &&
          (u32)((arg0 >> 16) - 0x13) < 4u) {
        func_8015C088();
        D_80146874 = 7;
        D_80146875 = 5;
      }
    }
    if (D_80143F03 == 3) {
      if (func_8015B5D4(D_8014686C, 0x1F) == 0 && arg0 <= 0x2C7FFF &&
          (u32)((arg1 >> 16) - 0x26) < 4u) {
        func_8015C088();
        func_8015D404(0, 0);
        func_80161BBC(0x9F);
        D_80146874 = 7;
        D_80146875 = 0xA;
      }
    }
  }
  if (D_80143F00 == 0x94 && D_80143F03 == 0 &&
      func_8015B5D4(D_8014686C, 0x11) == 0 && (arg1 >> 16) == 0x1C &&
      (u32)((arg0 >> 16) - 0x12) < 3u) {
    func_8015C088();
    D_80146874 = 4;
    D_80146875 = 0xA;
    return 1;
  }
  if (D_80143F00 == 0xAC && D_80143F03 == 0 &&
      func_8015B5D4(D_8014686C, 0x13) == 0 && arg0 <= 0xB7FFF &&
      (u32)((arg1 >> 16) - 0x3F) < 2u) {
    func_8015C088();
    D_80146874 = 6;
    D_80146875 = 0xA;
  }
  if (D_80143F00 == 0xBF && D_80143F03 == 0 &&
      (u32)((arg0 >> 16) - 0x1D) < 5u && (u32)((arg1 >> 16) - 0x14) < 2u &&
      func_8015B5D4(D_8014686C, 8) != 0 && func_8015B5D4(D_8014686C, 9) == 0) {
    func_8015C088();
    D_80146874 = 2;
    D_80146875 = 0x1D;
    return 1;
  }
  return 0;
}
