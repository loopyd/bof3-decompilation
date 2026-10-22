#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC218
 * @behavior Initializes the 0x118-byte battle local panel record selected by
 *   `index` (0x801EB6A4 + index*0x118) from the 0x88-byte class template
 *   selected by `class_id` (0x800E4048 + class_id*0x88). It copies the
 *   template's leading bytes and bytes 0x18..0x1F, re-maps the remaining
 *   template fields into the record layout, republishes the record bytes
 *   0x4C..0x6B at 0x2C, stores the class id at 0x6C, and clears the record's
 *   trailing state (0x89, 0x8C..0x8F, 0x90..0x93, 0x94..0x9B, 0x9E..0xA1).
 *   Template reads use the fixed template addresses; every record field is
 *   addressed through its own symbol (see battle15_internal.h) because the
 *   original encodes those accesses as `lui %hi(SYM); addu at,at,record byte
 *   offset; op %lo(SYM)(at)`.
 * @status partial
 * @match 86.71
 * @residual 274/316 instructions, 1264/1260 bytes (+4). Two shapes carry this
 *   reading. (1) The 0x801EB730 clear is byte-addressed
 *   (`(u8 *)D_801EB730 + index * 0x118u`): D_801EB730 is declared as a
 *   0x118-byte record array, so the earlier spelling scaled by 0x118 twice and
 *   stored at 0x801EB730 + 0x118*0x118*index instead of the original's
 *   0x801EB730 + 0x118*index, which is record +0x8C of the same band.
 *   (2) The 8-byte template copy at record +0x24 (0x801EB6C8) is two
 *   independent 4-byte copies from template +0x74 and +0x78, which is what the
 *   original bytes encode. Together they moved the reading from 218/317 (1268
 *   bytes) to 274/316 (1264 bytes). What remains is one allocation-slot
 *   rotation: at +0x0004 the original copies the class byte into $t4 where
 *   this candidate uses $t5, and the same slot carries the record base ($a1
 *   vs $a2) and the template base ($a0 vs $a1); the original also keeps one
 *   template %hi/%lo pair live across an intervening store where the candidate
 *   re-materialises it, derives 0x801EB630 as -0x98 from the D_801EB6C8 base
 *   register, and orders the three template byte stores differently. Every
 *   block is otherwise the same instruction sequence. Measured with
 *   bin/harness lift sweep: local declaration order, explicit parameter
 *   copies, while/do-while/`!=` loop forms, per-site record views, inlining the
 *   0x801EB630 view, `i = 0` before the pointer setup and `u8 i = 0` with the
 *   for-init dropped all leave the rotation in place; the mirrored template
 *   halfword pairs must be loaded once into a local. An index-free record-band
 *   anchor was measured and rejected three ways: `rec = D_801EB6C8` for the
 *   two 4-byte copies with `band = rec - 0x98` for the flag base measured
 *   195/315 at 1252, and keeping either of those two halves alone measured
 *   197/315 at 1256 and 193/315 at 1256. The original derives 0x801EB630 as
 *   -0x98 from its live record+0x24 register, but every C spelling that ties
 *   the band to that pointer instead of to the D_801EB630 symbol rotates the
 *   entry copies (first difference moves to instruction 0, `move a2,a0` vs
 *   `move a3,a0`) and loses about 79 matched instructions. Repair round 4
 *   spelled the same -0x98 derivation as two expressions on the live
 *   D_801EB6C8 symbol, with no pointer local, and measured it four ways:
 *   248/315 at 1264 (flag base only), 205/315 at 1264 (pad loop only),
 *   204/315 at 1260 (both) and 231/315 at 1264 (D_801EB6C8 derived from the
 *   D_801EB630 constant instead). The 1260-byte shape reaches the original
 *   size but loses 70 matches, so the +4 bytes are an allocation cost rather
 *   than a representation gap and the band spelling is measured out, not
 *   untried. The untried rungs are the selector profile and permuter, which
 *   this lane is not authorized to run.
 *   Callers: two direct in-target `jal func_800AC218` sites, func_800ABCE0 at
 *   0x800ABEF0 and func_800AC01C at 0x800AC1AC. Both mask both arguments to
 *   0xFF (0x800ABEBC `andi s2,0xFF` with 0x800ABEC0 `andi a1,s0,0xFF`;
 *   0x800AC0B4 `andi s2,0xFF` with 0x800AC180 `andi s1,s4,0xFF`) and pass the
 *   record index in a0 and the class byte in a1. Both scale the record index by
 *   0x118 themselves and materialise the D_801EB630 band base with the same
 *   `lui %hi` + `addiu %lo` + `addu` sequence, which is the pair the residual
 *   above measures: func_800AC01C at 0x800AC068 and again at 0x800AC0CC, and
 *   func_800ABCE0 at 0x800ABD0C on the same base (the record region this
 *   function clears at +0x104 and +0x108), where it sets byte +0. That pair is
 *   ordinary codegen at both sites, not a representation error. func_800ABCE0
 *   also scales the class by 0x88 and reads `lhu %lo(D_800E405E)(at)`, which is
 *   template +0x16, storing it to D_801EB6BA, the record halfword +0x16 this
 *   function leaves untouched; that store names its base with `lui` alone and
 *   folds `%lo` into the displacement, so it is not part of the pair.
 *   Both sites then call func_800AD0A8 with the same a0 and a1. This
 *   corroborates the two-u8 ABI and the 0x88/0x118 bands and names no role, so
 *   the address-anchored name stays.
 *   No aids: no register pins, clobbers, barriers or empty asm.
 */
void func_800AC218(u8 index, u8 class_id) {
  u8 i;
  u8 *rec;
  u8 *tmpl;
  u8 *flags;
  u16 pair;

  rec = D_801EB6A4 + (u32)index * 0x118u;
  tmpl = PSX_PTR(u8, 0x800E4048u) + (u32)class_id * 0x88u;
  for (i = 0; i < 8u; i++) {
    rec[i] = tmpl[i];
  }

  ((u8 *)D_801EB6AC)[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E4050u + (u32)class_id * 0x88u);
  D_801EB6AD[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E4051u + (u32)class_id * 0x88u);
  D_801EB6AE[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E4052u + (u32)class_id * 0x88u);
  *(u16 *)(D_801EB6B0 + (u32)index * 0x118u) =
      PSX_REF(u16, 0x800E4056u + (u32)class_id * 0x88u);
  *(u16 *)(D_801EB6B4 + (u32)index * 0x118u) =
      PSX_REF(u16, 0x800E4058u + (u32)class_id * 0x88u);
  *(u16 *)(D_801EB6B6 + (u32)index * 0x118u) =
      PSX_REF(u16, 0x800E405Au + (u32)class_id * 0x88u);
  *(u16 *)(D_801EB6B8 + (u32)index * 0x118u) =
      PSX_REF(u16, 0x800E405Cu + (u32)class_id * 0x88u);

  rec = D_801EB6BC + (u32)index * 0x118u;
  tmpl = PSX_PTR(u8, 0x800E4048u) + (u32)class_id * 0x88u;
  for (i = 0; i < 8u; i++) {
    rec[i] = tmpl[0x18 + i];
  }

  *(u16 *)(D_801EB6B2 + (u32)index * 0x118u) =
      PSX_REF(u8, 0x800E40C9u + (u32)class_id * 0x88u);
  __builtin_memcpy(D_801EB6C8 + (u32)index * 0x118u,
                   PSX_PTR(u8, 0x800E4074u + (u32)class_id * 0x88u), 4);
  __builtin_memcpy(D_801EB6C8 + (u32)index * 0x118u + 4,
                   PSX_PTR(u8, 0x800E4078u + (u32)class_id * 0x88u), 4);

  flags = (u8 *)D_801EB630 + (u32)index * 0x118u;
  pair = PSX_REF(u16, 0x800E4068u + (u32)class_id * 0x88u);
  *(u16 *)(flags + 0xC0) = pair;
  *(u16 *)(D_801EB6C4 + (u32)index * 0x118u) = pair;
  pair = PSX_REF(u16, 0x800E406Au + (u32)class_id * 0x88u);
  *(u16 *)(flags + 0xC2) = pair;
  *(u16 *)(D_801EB6C6 + (u32)index * 0x118u) = pair;
  *(u16 *)(D_801EB6F4 + (u32)index * 0x118u) =
      PSX_REF(u16, 0x800E406Cu + (u32)class_id * 0x88u);
  *(u16 *)(D_801EB6F6 + (u32)index * 0x118u) =
      PSX_REF(u16, 0x800E406Eu + (u32)class_id * 0x88u);
  *(u16 *)(D_801EB6F8 + (u32)index * 0x118u) =
      PSX_REF(u16, 0x800E4070u + (u32)class_id * 0x88u);
  *(u16 *)(D_801EB6FA + (u32)index * 0x118u) =
      PSX_REF(u16, 0x800E4072u + (u32)class_id * 0x88u);

  D_801EB6FF[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C0u + (u32)class_id * 0x88u);
  D_801EB700[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C1u + (u32)class_id * 0x88u);
  D_801EB701[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C2u + (u32)class_id * 0x88u);
  D_801EB702[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C3u + (u32)class_id * 0x88u);
  D_801EB703[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C4u + (u32)class_id * 0x88u);
  D_801EB704[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C5u + (u32)class_id * 0x88u);
  D_801EB705[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C6u + (u32)class_id * 0x88u);
  D_801EB706[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C7u + (u32)class_id * 0x88u);
  D_801EB707[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40C8u + (u32)class_id * 0x88u);

  __builtin_memcpy(D_801EB6C8 + (u32)index * 0x118u + 8,
                   D_801EB6C8 + (u32)index * 0x118u + 0x28, 32);

  D_801EB720[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40CCu + (u32)class_id * 0x88u);
  D_801EB721[(u32)index * 0x118u] =
      PSX_REF(u8, 0x800E40CDu + (u32)class_id * 0x88u);
  D_801EB710[(u32)index * 0x118u] = class_id;
  D_801EB72D[(u32)index * 0x118u] = 0;
  *(u32 *)(flags + 0x104) = 0;
  *(u32 *)((u8 *)D_801EB730 + (u32)index * 0x118u) = 0;
  D_801EB742[(u32)index * 0x118u] = 0;
  D_801EB743[(u32)index * 0x118u] = 0;
  D_801EB744[(u32)index * 0x118u] = 0;
  D_801EB745[(u32)index * 0x118u] = 0;

  for (i = 0; i < 8u; i++) {
    *(u8 *)((u8 *)D_801EB630 + (u32)index * 0x118u + 0x108 + i) = 0;
  }
}
