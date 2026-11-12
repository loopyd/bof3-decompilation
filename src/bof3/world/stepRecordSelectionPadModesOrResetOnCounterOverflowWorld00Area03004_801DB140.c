#include "bof3/world/area03004_internal.h"

/* @source 0x801DB140
 * @behavior AREA030 work-record dispatch step, handler 2 of the local table
 * 0x801E2268 selected by scratch work-record byte 3: saves the selected-record
 * value byte D_801E31F8 into D_801E31FC. While the shared counter D_80144000
 * reads at most 0x3E7FFF it walks the two pad halfwords: pad bits 0xC000 of
 * D_80145AA4 select record value 2 with the fresh countdown 8 in scratch byte 9
 * and the byte at 0x801E320C + 5 published into D_8014421C, otherwise the
 * existing countdown is decremented and serviced by func_8014D978 or, once it
 * reads zero, record value 0 with the 0 request 7 is restored; pad bits 0x3000
 * clear record value and the signed record cell at 0x801468A4 while sliding
 * 0x801E320C + 6 in with request 0; pad bit 0x40 selects record value 1 with
 * 0x801E320C + 4 and request 7. Both 0x40 arms step the signed record cell down
 * by (0x801E320C + 3) + 2 and run selectRecordValueAndSlot, the second one
 * through setActiveSlotValueFromFrameMask, and every taken arm advances
 * D_80144216. Above that counter limit it resets the shared selection counter
 * D_80144199 to 1, the state bytes D_8014419E and D_8014403D to 4, the mode byte
 * to 7, clears D_80143FC9 and D_8014420D, sets handler 6 on the active record,
 * plays cue 0x203, restarts the frontend effect, queues cue (0x2A, 100, 8),
 * selects 0xA, advances scratch byte 3 to 3 and returns; every other path ends
 * with func_801DDD4C.
 * @status partial
 * @match 95.40
 * @residual Two blocks differ by register allocation and intra-block
 * scheduling only, at an exact instruction and byte count (261/261, 1044 B).
 * The Cross/Pad arm 0x801E31F8 = 2 block keeps the 0x801E320C table pointer in
 * v1 where the original keeps it in v0, and therefore keeps `li 2` after the
 * cursor load instead of in the 0xC000 test delay slot; the 0x3000 arm keeps
 * that pointer in a1 and sinks the 0x8014421C store below the 0x98 index math
 * where the original stores it straight after the signed byte load. Measured
 * and reverted: pointer-cast forms of the 0x80146888 + 0x1C cell (materialised
 * base, 218/261), all-scalar progress byte (232/261), all-materialised array
 * view (208/261), arm statement orders A1/A2/va (239-249/261), 0x3000 arm
 * orders and an index or value temporary (239-249/261).
 */
void stepRecordSelectionPadModesOrResetOnCounterOverflowWorld00Area03004_801DB140(void) {
  u8* work;

  D_801E31FC = D_801E31F8;
  if (D_80144000 > 0x3E7FFF) {
    D_80144199_BYTE = 1;
    D_8014419E = 4;
    D_8014403D = 4;
    modeByte = 7;
    D_80143FC9 = 0;
    D_8014420D = 0;
    D_80146888[D_801E3208].handler_01 = 6;
    func_8015DF18(0x203);
    func_8015D404(0, 1);
    func_80161C20(0x2A, 100, 8);
    func_8014D8D4(0xA);
    work = D_1F800044;
    work[3] = 3;
    return;
  }
  if (D_80145AA4 & 0xC000) {
    if (D_80145AA8 & 0xC000) {
      work = D_1F800044;
      work[9] = 8;
      D_801E31F8 = 2;
      D_8014421C = D_801E320C[5];
      func_8014D8D4(8);
      D_80144216_BYTE++;
    } else {
      work = D_1F800044;
      if (work[9] != 0) {
        work[9] = work[9] - 1;
        func_8014D978();
        D_80144216_BYTE++;
      } else {
        D_8014421C = 0;
        D_801E31F8 = 0;
        func_8014D8D4(7);
      }
    }
    if (D_80145AA8 & 0x40) {
      D_80146888[D_801E3208].unk_1C =
          (s8)(D_80146888[D_801E3208].unk_1C - (((u8*)D_801E320C)[3] + 2));
      func_8014D8D4(7);
      selectRecordValueAndSlot();
      D_80144216[0]++;
    } else if (D_80145AA4 & 0x40) {
      setActiveSlotValueFromFrameMask();
      selectRecordValueAndSlot();
      func_8014D978();
      D_80144216[0]++;
    }
  } else if (D_80145AA4 & 0x3000) {
    D_801E31F8 = 0;
    D_8014421C = D_801E320C[6];
    D_80146888[D_801E3208].unk_1C = 0;
    func_8014D8D4(0);
    D_80144216_BYTE--;
  } else if (D_80145AA4 & 0x40) {
    if (D_80145AA8 & 0x40) {
      D_80146888[D_801E3208].unk_1C =
          (s8)(D_80146888[D_801E3208].unk_1C - (((u8*)D_801E320C)[3] + 2));
    } else {
      setActiveSlotValueFromFrameMask();
    }
    D_801E31F8 = 1;
    D_8014421C = D_801E320C[4];
    func_8014D8D4(7);
    func_8014D978();
    D_80144216_BYTE++;
  } else {
    D_801E31F8 = 0;
    D_8014421C = 0;
    D_80146888[D_801E3208].unk_1C = 0;
    func_8014D8D4(7);
  }
  func_801DDD4C();
}
