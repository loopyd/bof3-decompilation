#include "bof3/bof3.h"
#include <rand.h>

/* @source 0x1F800044 @kind unknown */
extern u8 *scenarioScena0900Work;

/* @source 0x801F76D0
 * @behavior Re-seeds all 0x20 records of the 0x28-byte-stride effect bank at
 * 0x800E5000 selected by scratchpad work byte 6 (bank stride 0x500). It forms
 * the bank base once from that byte, then walks the records and branches on a
 * fresh read of the same byte. When that read is zero the arm stores the
 * position words at 0x18 and 0x1C as 0x120000 + (rand() & 0x1FF) * 0x100 and
 * 0x2E8000 + (rand() & 0x1FF) * 0x100, the word at 0x20 as
 * 0x08400000 - ((word 0x18 - 0x120000) << 7), the velocity words at 0x08 and
 * 0x0C as (word 0x18 - 0x130000) >> 4 and (word 0x1C - 0x2F8000) >> 4, the
 * 0x10 velocity as -((rand() & 0xFF) << 5), and then subtracts 0x800 from the
 * 0x08 velocity. When that read is non-zero the arm uses the base pair 0x78000
 * and 0x318000, forms the 0x20 word from the 0x1C position instead, takes the
 * velocities as (word 0x18 - 0x88000) >> 4 and (word 0x1C - 0x328000) >> 4,
 * and overwrites the 0x0C velocity with -0x800 after the same random 0x10
 * velocity. Both arms then store the claim byte 1 at offset 0, the countdown
 * byte 0x18 at offset 2, zero at offset 1, the fade byte 0x60 at offset 3 and
 * `rand() & 7` at offset 4. It takes no arguments and returns nothing; its
 * 0x48-byte frame saves $ra and $s0-$s4, and the only memory it touches is the
 * scratchpad work pointer cell and the selected record bank.
 *
 * Name evidence: this target's exact caller
 * initializeRecordBankArmByte9AndAdvanceStateScenarioScena0900_801F73A0
 * (0x801F73A0) runs the lead-byte clear
 * clearSelectedRecordBankLeadBytesScenarioScena0900_801F7574 (0x801F7574) and
 * then this address, and the exact sibling
 * advanceAndRenderActiveRecordsInSelectedBankScenarioScena0900_801F75BC
 * (0x801F75BC) records that this helper fills the position words at 0x18, 0x1C
 * and 0x20 and the velocity words at 0x08, 0x0C and 0x10 of the same 0x28-byte
 * records in the same 0x800E5000 bank, which is what the two arms below write.
 * The countdown and fade roles of bytes 2 and 3 come from that sibling's own
 * per-frame integration, and byte 4 is the pattern byte it reseeds from
 * `rand() & 7`. Its only in-image reference is the jal at 0x801F73A8 inside
 * that caller; no data word addresses it.
 *
 * Frame evidence: the payload's 0x48-byte frame keeps 0x30 bytes below the
 * saved $ra/$s0-$s4 group, 0x20 of which no instruction in the body addresses;
 * with no missing saved register, spill or call argument, the reservation is a
 * declaration the original carried that emitted no code. The `scratch` local
 * below reconstructs exactly that 0x20-byte slot and takes no use, since using
 * it would change liveness. Its name and type are a reconstruction choice, not
 * recovered source, and are flagged for review; dropping it leaves a 0x28-byte
 * frame with the body otherwise identical.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedEffectRecordsInSelectedBankScenarioScena0900_801F76D0(void) {
  u8 *record;
  u8 i;
  /* Reserved frame bytes: see the frame evidence above. No statement reads or
   * writes this object, so removing it only shortens the frame. */
  u8 scratch[0x20];

  record = PSX_PTR(u8, 0x800E5000u) + scenarioScena0900Work[6] * 0x500;
  i = 0;
  do {
    if (scenarioScena0900Work[6] == 0) {
      *(s32 *)(record + 0x18) = 0x120000 + (rand() & 0x1FF) * 0x100;
      *(s32 *)(record + 0x1C) = 0x2E8000 + (rand() & 0x1FF) * 0x100;
      *(s32 *)(record + 0x20) =
          0x08400000 - ((*(s32 *)(record + 0x18) - 0x120000) << 7);
      *(s32 *)(record + 0x08) = (*(s32 *)(record + 0x18) - 0x130000) >> 4;
      *(s32 *)(record + 0x0C) = (*(s32 *)(record + 0x1C) - 0x2F8000) >> 4;
      *(s32 *)(record + 0x10) = -(rand() & 0xFF) << 5;
      *(s32 *)(record + 0x08) -= 0x800;
    } else {
      *(s32 *)(record + 0x18) = 0x78000 + (rand() & 0x1FF) * 0x100;
      *(s32 *)(record + 0x1C) = 0x318000 + (rand() & 0x1FF) * 0x100;
      *(s32 *)(record + 0x20) =
          0x08400000 - ((*(s32 *)(record + 0x1C) - 0x318000) << 7);
      *(s32 *)(record + 0x08) = (*(s32 *)(record + 0x18) - 0x88000) >> 4;
      *(s32 *)(record + 0x0C) = (*(s32 *)(record + 0x1C) - 0x328000) >> 4;
      *(s32 *)(record + 0x10) = -(rand() & 0xFF) << 5;
      *(s32 *)(record + 0x0C) = -0x800;
    }
    record[0] = 1;
    record[2] = 0x18;
    record[1] = 0;
    record[3] = 0x60;
    record[4] = rand() & 7;
    record += 0x28;
    i++;
  } while (i < 0x20);
}
