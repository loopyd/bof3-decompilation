#include "bof3/bof3.h"

/* @source 0x8015C058 @kind function */
extern void func_8015C058(void);
/* @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 step);
/* @source 0x8015B580 @kind function */
extern void func_8015B580(void* block, u8 bit_index);
/* @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                         u8 context_kind);
/* @source 0x80143BB0 @kind data */
extern u8 D_80143BB0;
/* @source 0x801448EB @kind data */
extern u8 D_801448EB;
/* @source 0x801448EC @kind data */
extern s8 D_801448EC;

/* @source 0x801F30BC
 * @behavior Dispatches on signed progress byte 0x801448EC. For progress 0,
 *           waits until phase byte 0x80143BB0 is not 2, releases the shared
 *           front end, requests step 6 and sets bit 0x88 in block 0x80144F28.
 *           For progress 0xA, applies the same phase guard and release,
 *           sets bit 0x89 and requests selector context (0x79, 0x2A0000,
 *           0x1A0000, 7). Both handled paths clear state-entry byte
 *           0x801448EB and progress. Other progress values do nothing.
 *           Takes no arguments and returns nothing.
 *
 * Name evidence: the original switch, helper calls, constants and byte
 * stores establish the two flag-setting release paths. Same-target sources
 * identify the state-entry and progress cells; area027 declarations identify
 * the shared release, step, bit-set and selector-context helper interfaces.
 * The block argument stays relative to the progress pointer, as the original
 * adds 0x63C to its saved base. No payload call or pointer names this entry,
 * so its loader-facing interface remains structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void releaseFrontendAndSetFlag88Or89ByProgressWorld03Area12813_801F30BC(void) {
  s8* progress = &D_801448EC;

  switch (*progress) {
    case 0:
      if (D_80143BB0 == 2) {
        return;
      }
      func_8015C058();
      func_801BE1B0(6);
      func_8015B580(progress + 0x63C, 0x88);
      break;
    case 0xA:
      if (D_80143BB0 == 2) {
        return;
      }
      func_8015C058();
      func_8015B580(progress + 0x63C, 0x89);
      func_8019FA28(0x79, 0x2A0000, 0x1A0000, 7);
      break;
    default:
      return;
  }
  D_801448EB = 0;
  *progress = 0;
}
