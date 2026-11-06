#include "bof3/bof3.h"

extern u8 D_80144E90;
extern u8 g_ScenarioProgress;
extern s8 D_80146872;

void func_801C1DF0(u8 index);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F8000
 * @behavior Boots progress state 0 of this overlay: the address is entry 0 (the
 * first word at 0x801FE29C) of the overlay's 24-entry handler table that
 * dispatchProgressHandlerScenarioScena0100_801F7FC4 indexes with the shared
 * scenario progress byte D_80146872, so no in-image jal targets it. It runs the
 * active scenario's own handler 0 through func_801C1DF0(0), seeds the front
 * selector/context bundle func_8019FA28(9, 0x570000, 0x40000, 7) - selection
 * seed 9, the channel pair 0x570000/0x40000 and kind 7 - then clears the shared
 * main-RAM byte D_80144E90 and the scenario progress word at 0x80146864 (the
 * four bytes g_ScenarioProgress, D_80146865, D_80146866 and D_80146867) and
 * advances the progress byte to 1 for the next frame. Takes no arguments,
 * returns nothing, and the 0x18-byte frame only keeps $ra across the two calls.
 * It carries the same boot shape as the exact siblings bootScenarioState
 * (0x801F9178, emi/scenario/scena00/00) and bootPrimaryState (0x801F6CCC,
 * emi/scenario/scena16/00), which differ in their own selector-seed constants
 * and extra request stores, so neither is a clone candidate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void bootProgressStateScenarioScena0100_801F8000(void) {
  func_801C1DF0(0u);
  func_8019FA28(9u, 0x570000u, 0x40000u, 7u);
  D_80144E90 = 0u;
  *(u32 *)&g_ScenarioProgress = 0u;
  D_80146872 = 1;
}
