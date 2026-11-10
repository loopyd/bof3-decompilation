#include "bof3/bof3.h"

/* Base of the shared main-RAM flag bank the two EXE-side helpers below index:
 * every call site in this body materializes it as `lui $a0,0x8014` /
 * `addiu $a0,$a0,0x4F28`, so the bank is addressed as a fixed cell and the
 * index arrives in $a1. */
extern u8 D_80144F28;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801F346C
 * @behavior Overlay rendezvous step entered by the loader: sets flag 0x6E of
 *           the shared main-RAM flag bank D_80144F28 through the EXE-side flag
 *           setter at 0x8015B580, then scans flags 0x6D through 0x70 of that
 *           same bank with the EXE-side flag tester at 0x8015B5D4 and returns 0
 *           as soon as any of them reads clear, leaving the rendezvous flag
 *           0x71 untouched. Only when all four probe set does it set flag 0x71
 *           through the same setter and return 0. Every path returns 0, so the
 *           step reports "not yet complete" for as long as one participant flag
 *           is missing. It reads no other state; the 0x18-byte frame only keeps
 *           $ra at 0x14 and the loop index in $s0 at 0x10 across the calls.
 *
 * Name evidence: the leading setter call takes the `addiu $a1,$zero,0x6E`
 * argument pair; the scan bound is the `addiu $s0,$zero,0x6D` loop start with
 * the signed `slti $v0,$s0,0x71` test and the `addiu $a1,$zero,0x71` setter
 * call on the exhaust path; the bank is the repeating `lui/addiu $a0,
 * %hi/%lo(D_80144F28)` pair, and the two helpers are the `jal 0x8015B580` /
 * `jal 0x8015B5D4` targets whose roles the already-lifted siblings in this
 * tree record (setter and tester of one bit of a byte flag bank).
 *
 * Shape: the payload holds no `jal` and no pointer word to 0x801F346C, so the
 * overlay loader is its only entry and the roles stay structural. A scan of
 * every payload under out/binaries/emi found no byte-identical 0x70-byte twin
 * (one hit, this span itself), so the clone lever does not apply. The three
 * same-shape siblings in other overlays - world00/area034/13 at 0x801F2DC4,
 * world01/area058/13 at 0x801F2C04 and world01/area046/13 at 0x801F2DA8 - are
 * word-for-word this body apart from their leading stored flag (0x6D, 0x6F and
 * 0x70), which is shape evidence only and not clone eligibility; this span's
 * own literal 0x6E comes from its bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setFlag6eThenSetFlag71WhenFlags6dTo70SetWorld01Area05613_801F346C(void) {
  s32 i;

  func_8015B580((u32)&D_80144F28, 0x6E);
  for (i = 0x6D; i < 0x71; i++) {
    if (func_8015B5D4((u32)&D_80144F28, i) == 0) {
      return 0;
    }
  }
  func_8015B580((u32)&D_80144F28, 0x71);
  return 0;
}
