#include "bof3/bof3.h"

/* @source 0x801F6D50
 * @behavior Empty per-frame progress handler and null record callback for the
 * scena19 overlay: the whole 8-byte body is the plain `jr $ra` epilogue with a
 * nop in the delay slot (03E00008 00000000), so it takes no arguments, reads no
 * state, writes no state and returns immediately with no value. Its address is
 * word index 4 of this overlay's seven-entry per-frame progress handler table at
 * 0x801F6DD0 (0x801F6C40, 0x801F6C54, 0x801F6C68, 0x801F6CA4, 0x801F6D50,
 * 0x801F6D68, 0x801F6DB4), which func_801F6C04 indexes with the signed shared
 * progress byte D_80146872 (`lb`/`sll $v0,$v0,2`/`lw %lo(D_801F6DD0)($at)`) and
 * then calls with no arguments, so while that byte reads 4 the frame performs no
 * handler work. The same word is simultaneously index 0 of the three-word
 * sub-run at 0x801F6DE0 (0x801F6D50, 0x801F6D68, 0x801F6DB4) that func_801F6D10
 * indexes with the unsigned byte at offset 0x7A of a record
 * (`lbu $v0,0x7A($a0)`) and calls as (record, D_8014686C), so a record whose byte
 * 0x7A reads 0 also dispatches to this empty body.
 * @status unverified
 * @match 0.00
 * @residual none
 */
void noopProgressHandler_scena19_801F6D50(void) {}
