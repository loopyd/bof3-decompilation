#include "bof3/bof3.h"

/* Shared main-RAM front-end flag bank: this body materializes it as
 * `lui $a0,0x8014` / `addiu $a0,$a0,0x4F28`, so the bank is passed by address
 * and the bit index arrives in $a1. */
extern u8 D_80144F28;
/* Front-end effect-bank byte the overlay arms with 0x1F and clears elsewhere
 * in its own state handlers. */
extern u8 D_8014832E;
/* Signed shared scenario state byte that dispatches this overlay's handler
 * table and that this body advances to 1. */
extern s8 D_80146872;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015B5A8(u32 arg0, s32 arg1);
void func_8015C088(void);
void func_801C16B8(void);
void func_801C1DF0(u8 index);

/* @source 0x801F7D50
 * @behavior Boots scenario state 0 of this overlay: the address is word 0
 * (0x801F7D50) of the overlay-local twelve-word scenario state-handler table
 * D_801FBF48 that dispatchScenarioStateHandlerScenarioScena1300_801F7D14
 * (0x801F7D14) indexes with the signed shared main-RAM scenario state byte
 * D_80146872, so no in-image `jal` targets it. It runs the shared front-end
 * startup helper func_8015C088, arms the front-end effect bank byte
 * D_8014832E with 0x1F, runs the shared field-record reinitializer
 * func_801C16B8, runs the active scenario's own handler 0 through
 * func_801C1DF0(0), sets bit 0x43 and then clears bit 0x42 of the shared
 * main-RAM flag bank D_80144F28 through the main-executable bit-set helper
 * func_8015B580 and bit-clear helper func_8015B5A8, and finishes by advancing
 * the shared scenario state byte D_80146872 to 1 for the next frame. It reads
 * no other state, takes no arguments and returns nothing.
 *
 * Name evidence: the body's only entry is the table word at 0x801FBF48 (the
 * twelve code words 0x801F7D50 through 0x801FB1B4 end immediately before the
 * data words at 0x801FBF78), its last store advances D_80146872 to 1, and the
 * body is an initialization sequence - front-end startup, field-record
 * reinitialization, scene handler 0, flag arming - which is the shape the
 * exact sibling bootProgressStateScenarioScena0100_801F8000 (0x801F8000,
 * emi/scenario/scena01/00) carries as entry 0 of that overlay's own handler
 * table: func_801C1DF0(0), then D_80146872 = 1. The shared-helper roles above
 * are recorded by the exact lifts setScenarioFlagBit1CThenRequestStateEntry2CProgress0Substate11ScenarioScena1300_801FB414
 * (0x801FB414, func_8015C088 and func_8015B580) and
 * reinitializeAllFieldRecordsAndKeepRecordZeroFlaggedScenarioScena0800_801FA32C
 * (0x801FA32C, func_801C16B8); func_801C1DF0 is the front-end scene init its
 * sibling names in the same call order.
 *
 * Shape: statement order, operand forms, offset spellings and declaration
 * style come from three exact siblings. The `extern u8` bank cell plus
 * `void func_8015B580(u32 arg0, s32 arg1)` setter/clearer pair and the
 * `(u32)&D_80144F28` argument spelling are those of the exact lift
 * setFlag6eThenSetFlag71WhenFlags6dTo70SetWorld01Area05613_801F346C
 * (0x801F346C, world01/area056/13), whose bytes show the same `lui`/`addiu`
 * bank materialization. The frame, the `sw $s0` save scheduled into the first
 * `jal` delay slot, the single bank address held in $s0 across both flag calls
 * and the closing `addiu $v0,$zero,0x1` / `sb` state advance are the shape of
 * bootProgressStateScenarioScena0100_801F8000, whose `func_801C1DF0(0u)` call
 * emits the same `addu $a0,$zero,$zero` delay slot. Every literal here - 0x1F,
 * 0x43 and 0x42 - is written from this span's own bytes, and this body adds
 * the front-end startup call, the effect-bank arm and the two flag calls the
 * siblings do not have.
 *
 * Clone declined: a scan of the 0x6C-byte body over every extracted payload
 * under out/binaries/ returned only this span's own offset 0x1150 in its own
 * payload, and a second scan with each `jal` target word masked so
 * relocation-different siblings could match returned nothing more, so no twin
 * exists and the siblings above are shape evidence rather than a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void bootScenarioStateScenarioScena1300_801F7D50(void) {
  func_8015C088();
  D_8014832E = 0x1F;
  func_801C16B8();
  func_801C1DF0(0u);
  func_8015B580((u32)&D_80144F28, 0x43);
  func_8015B5A8((u32)&D_80144F28, 0x42);
  D_80146872 = 1;
}
