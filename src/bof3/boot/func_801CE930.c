#include "bof3/boot/logo_internal.h"

#include <libpress.h>

/*
 * @source 0x801CE930
 * @behavior starts the CAPCOM30.STR stream after initWorkAreaAndStartSubsystems
 * has published the work-area buffers: clears the D_801EB458 buffer index, the
 * D_801EB46C transfer flag and the D_801EB464 frame number, resets the DCT
 * decoder, installs func_801CECA4 as the MDEC output callback, arms the CD
 * stream for D_801EB448 through func_801CFC00, sets the D_801EB474 and
 * D_801EB475 stream flags, starts the disc read for the supplied LBA through
 * func_801D0320, and then repeats the three readiness probes (func_801CFEF4 for
 * modes 0xD and 2 on D_801EB474/D_801EB4AC and func_801D2180 for one
 * 0x1C8-byte frame) while any probe has reported not-ready; it then waits for
 * func_801D2558 to publish the VLC bitstream and frame-header pointers, records
 * the frame number of the header in D_801EB468, decodes the bitstream into the
 * CD buffer of the current index through DecDCTvlc, advances the decoder stream
 * position through func_801D2464 and always returns 0.
 * @status partial
 * @match 82.22
 * @residual the 0x1C8 frame size is rematerialized into $s0 at the loop preheader (0x801CE9C8) instead of staying in $s2 from the DecDCToutCallback delay slot (0x801CE974), so the $s2 save/restore is dropped (344 vs 360 bytes).
 * Residual class: loop-invariant constant live range / register allocation.
 * Tried and measured identical: assignment at declaration, `register`
 * qualifier, s32 local with an s32 prototype (no conversion), literal
 * argument, assignment inside the loop, and assignment both before and inside
 * the loop. One bounded 60s permuter run scored 260 (base 800) with only an
 * artificial `do {} while (0);` wrapper, which was rejected. The original
 * keeps the value live across the whole function, which under the canonical
 * -O2 profile requires a profile probe (`bin/flag-search`); that rung is
 * outside this lane's bounds.
 */
s32 func_801CE930(u_long disc_lba) {
  u_long* data;
  u_long* header;
  s32 pending;
  u_long size;

  D_801EB458 = 0;
  D_801EB46C = 0;
  D_801EB464 = 0;
  DecDCTReset(0);
  DecDCToutCallback(func_801CECA4);
  size = 0x1C8;
  func_801CFC00(D_801EB448, 0xF);
  pending = 0;
  func_801D23DC(1, 1, 0xFFFFFFF, 0, 0);
  D_801EB474 = 1;
  D_801EB475 = 1;
  func_801D0320(disc_lba);
  do {
    if (func_801CFEF4(0xD, &D_801EB474, 0) == 0) {
      pending++;
    }
    if (func_801CFEF4(2, &D_801EB4AC, 0) == 0) {
      pending++;
    }
    if (func_801D2180(size) == 0) {
      pending++;
    }
  } while (pending != 0);
  do {
  } while (func_801D2558(&data, &header) != 0);
  D_801EB468 = header[2];
  DecDCTvlc(data, D_801EB44C_BUFFERS[D_801EB458]);
  func_801D2464(data);
  return 0;
}
