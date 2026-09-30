#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC218
 * @behavior Initializes the 0x118-byte battle local panel record selected by
 *   `index` (D_801EB6A4 + index*0x118) from the 0x88-byte class template
 *   selected by `class_id` (0x800E4048 + class_id*0x88). It copies the
 *   template's leading bytes, re-maps the remaining template fields into the
 *   record layout, republishes the record bytes 0x4C..0x6B at 0x2C, stores the
 *   class id at 0x6C, and clears the record's trailing state (0x89, 0x8C..0x8F,
 *   0x90..0x91, 0x94..0x9B, 0x9E..0xA1).
 * @status partial
 * @match 7.52
 * @residual First difference at +0x0000: the original opens with the two byte
 *   parameter copies (move a3,a0 / move t4,a1) and the loop-0 initializer third,
 *   while the current build hoists the initializer (move t4,zero) and the record
 *   base materialization first. The destination field accesses also materialize
 *   a pooled record base (lui/addiu + `sb $v0,0($base)`) where the original
 *   rematerializes per field (`lui $at,%hi(field); addu $at,$at,index;
 *   sb %lo(field)($at)`); both tried clean-C address shapes (record-base symbol +
 *   constant field offset, and scaled index + absolute field address) reproduce
 *   the pool/materialize form. The source-template side and the byte-count
 *   (1276 vs 1260) match; the record halfword stores at 0x4C/0x4E/0x104 use the
 *   0x801EB630 view.
 */
void func_800AC218(u8 index, u8 class_id) {
  u8 panel = index;
  u8 klass = class_id;
  u8 i;

  /* Record bytes 0x00..0x07 <- template bytes 0x00..0x07. */
  for (i = 0; i < 8u; i++) {
    PSX_REF(volatile u8, D_801EB6A4 + 0x00 + (u32)panel * 0x118u + i) =
        PSX_REF(u8, 0x800E4048u + (u32)klass * 0x88u + i);
  }

  /* Record bytes 0x08..0x0A and halfwords 0x0C..0x14. */
  PSX_REF(volatile u8, D_801EB6A4 + 0x08 + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E4050u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x09 + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E4051u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x0A + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E4052u + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x0C + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E4056u + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x10 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E4058u + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x12 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E405Au + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x14 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E405Cu + (u32)klass * 0x88u);

  /* Record bytes 0x18..0x1F <- template bytes 0x18..0x1F. */
  for (i = 0; i < 8u; i++) {
    PSX_REF(volatile u8, D_801EB6A4 + 0x18 + (u32)panel * 0x118u + i) =
        PSX_REF(u8, 0x800E4060u + (u32)klass * 0x88u + i);
  }

  /* Record halfword 0x0E holds the template byte 0x81 zero-extended. */
  PSX_REF(volatile u16, D_801EB6A4 + 0x0E + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C9u + (u32)klass * 0x88u);
  /* Record bytes 0x24..0x2B <- template bytes 0x2C..0x33. */
  __builtin_memcpy(PSX_PTR(u8, D_801EB6A4 + 0x24 + (u32)panel * 0x118u),
                   PSX_PTR(u8, 0x800E4074u + (u32)klass * 0x88u), 8);

  /* Template halfwords 0x20 and 0x22 are mirrored at 0x4C/0x4E. */
  PSX_REF(volatile u16, D_801EB6A4 + 0x4C + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E4068u + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x20 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E4068u + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x4E + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E406Au + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x22 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E406Au + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x50 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E406Cu + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x52 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E406Eu + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x54 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E4070u + (u32)klass * 0x88u);
  PSX_REF(volatile u16, D_801EB6A4 + 0x56 + (u32)panel * 0x118u) =
      PSX_REF(u16, 0x800E4072u + (u32)klass * 0x88u);

  /* Record bytes 0x5B..0x63 <- template bytes 0x78..0x80. */
  PSX_REF(volatile u8, D_801EB6A4 + 0x5B + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C0u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x5C + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C1u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x5D + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C2u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x5E + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C3u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x5F + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C4u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x60 + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C5u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x61 + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C6u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x62 + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C7u + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x63 + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40C8u + (u32)klass * 0x88u);

  /* Record bytes 0x2C..0x4B <- record bytes 0x4C..0x6B. */
  __builtin_memcpy(PSX_PTR(u8, D_801EB6A4 + 0x2C + (u32)panel * 0x118u),
                   PSX_PTR(u8, D_801EB6A4 + 0x4C + (u32)panel * 0x118u), 32);

  PSX_REF(volatile u8, D_801EB6A4 + 0x7C + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40CCu + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x7D + (u32)panel * 0x118u) =
      PSX_REF(u8, 0x800E40CDu + (u32)klass * 0x88u);
  PSX_REF(volatile u8, D_801EB6A4 + 0x6C + (u32)panel * 0x118u) = klass;
  PSX_REF(volatile u8, D_801EB6A4 + 0x89 + (u32)panel * 0x118u) = 0;
  PSX_REF(volatile u32, D_801EB6A4 + 0x90 + (u32)panel * 0x118u) = 0;
  PSX_REF(volatile u32, D_801EB6A4 + 0x8C + (u32)panel * 0x118u) = 0;
  PSX_REF(volatile u8, D_801EB6A4 + 0x9E + (u32)panel * 0x118u) = 0;
  PSX_REF(volatile u8, D_801EB6A4 + 0x9F + (u32)panel * 0x118u) = 0;
  PSX_REF(volatile u8, D_801EB6A4 + 0xA0 + (u32)panel * 0x118u) = 0;
  PSX_REF(volatile u8, D_801EB6A4 + 0xA1 + (u32)panel * 0x118u) = 0;

  /* Record bytes 0x94..0x9B are cleared. */
  for (i = 0; i < 8u; i++) {
    PSX_REF(volatile u8, D_801EB6A4 + 0x94 + (u32)panel * 0x118u + i) = 0;
  }
}
