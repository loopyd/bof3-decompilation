#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC704
 * @behavior Find the first of eight records whose leading byte matches id.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the class-table byte stays an explicit fixed address whose
 * scaled index is held in a named local, so the address reaches the assembler
 * as a full 32-bit constant displacement (`lbu $2,-2146549680($2)`). The
 * original expands it as `lui $at,%hi(X)` + `addu at,index,at` + `lbu %lo(X)($at)`.
 * A declared symbol reference (`D_800E4050[i].id`) instead emits
 * `addu at,at,index` (measured 19/20 instructions, 95.00%), and inlining the
 * scaled index in the address expression lets gcc materialize the constant
 * (`lui`+`ori` base) and hoist it out of the loop.
 */
u8 func_800AC704(u8 id)
{
  u8 i;
  u32 offset;

  i = 0;
  while (i < 8) {
    offset = (u32)i * 0x88u;
    if (PSX_REF(u8, 0x800E4050u + offset) == id) {
      return i;
    }
    i++;
  }
  return 0xFF;
}
