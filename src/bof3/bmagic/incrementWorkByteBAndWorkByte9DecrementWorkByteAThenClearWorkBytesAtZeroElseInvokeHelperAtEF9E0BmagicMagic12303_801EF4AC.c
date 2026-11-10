#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, already bound at that
 * address by this target's map as g_battle_work, the name its exact lifts
 * clearWorkBytesBmagicMagic12303_801EFD04 and
 * decrementGlobalWorkByteBThenClearWorkBytesBmagicMagic12303_801EF550 use. The
 * cell is evaluated once per access group because each byte store goes through
 * the pointer the cell supplied, so the compiler re-materializes
 * `lui $v1,0x1F80` / `lw $v1,0x44($v1)` exactly three times, at 0x801EF4AC,
 * 0x801EF4CC and 0x801EF4E8. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the overlay's remaining assembly loads it, and every access here is
 * byte-addressed, so the local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void clearWorkBytesBmagicMagic12303_801EFD04(void);
extern void func_801EF9E0(void); /* @source 0x801EF9E0 @kind function */

/* @source 0x801EF4AC
 * @behavior Entry index 2 of the four-word handler run based at 0x801EFD5C that
 *           this target's exact dispatcher
 *           dispatchStateByte2Tablefd5cBmagicMagic12303_801EF108 indexes with
 *           work byte 2, which is why no jal word in this payload names
 *           0x801EF4AC. It increments the scratchpad work record's unsigned
 *           byte 0xB by one at byte width (lbu / addiu 1 / sb), re-reads the
 *           pointer cell at 0x1F800044 and increments unsigned byte 9 the same
 *           way, then re-reads the cell once more and subtracts one from
 *           unsigned byte 0xA, testing the stored byte truncated to eight bits
 *           (andi $v0,$v0,0xFF / bnez), so the truncated register rather than a
 *           reload supplies the compared value. When that countdown reaches
 *           zero it reads the shared battle work record pointer at 0x801EB4E0,
 *           decrements that record's byte 0xB by one at byte width and invokes
 *           this overlay's work-byte clear routine at 0x801EFD04, the store
 *           filling the call delay slot. While byte 0xA is still nonzero it
 *           instead invokes the overlay's record routine at 0x801EF9E0 with no
 *           argument set up. The first cell load precedes frame creation
 *           because the increment needs no stack; the 0x18-byte frame only
 *           keeps $ra at 0x10 across whichever call runs. Takes no arguments,
 *           returns nothing through the jr $ra at 0x801EF548 and reads or
 *           writes no other state. The roles of work bytes 9, 0xA and 0xB and
 *           of the routine at 0x801EF9E0 are not established, so all four stay
 *           offset- or address-anchored. The 164-byte span is not byte-identical
 *           to any other payload span; its nearest shape sibling is
 *           add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic03903_801EF6E0, whose
 *           `if (--work[0xA] == 0)` countdown arm carries the same store,
 *           truncated test and store-filling-delay-slot call order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAndWorkByte9DecrementWorkByteAThenClearWorkBytesAtZeroElseInvokeHelperAtEF9E0BmagicMagic12303_801EF4AC(void) {
  g_battle_work[0xB]++;
  g_battle_work[9]++;
  if (--g_battle_work[0xA] == 0) {
    D_801EB4E0[0xB]--;
    clearWorkBytesBmagicMagic12303_801EFD04();
  } else {
    func_801EF9E0();
  }
}
