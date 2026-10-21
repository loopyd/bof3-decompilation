#include "bof3/boot/logo_internal.h"

#include <libpress.h>

/*
 * @source 0x801CE930
 * @behavior starts the CAPCOM30.STR stream after initWorkAreaAndStartSubsystems
 * has published the work-area buffers: clears the D_801EB458 buffer index, the
 * D_801EB46C transfer flag and the D_801EB464 frame number, resets the DCT
 * decoder, installs func_801CECA4 as the MDEC output callback, arms the CD
 * stream for D_801EB448 through func_801CFC00, sets the D_801EB474 and
 * D_801EB475 stream flags, converts the supplied LBA into the three BCD
 * minute/second/sector position bytes that func_801D0320 writes at
 * D_801EB4AC, and then repeats the three readiness probes (func_801CFEF4 for
 * modes 0xD and 2 on D_801EB474/D_801EB4AC and func_801D2180 for one
 * 0x1C8-byte frame) while any probe has reported not-ready; it then waits for
 * func_801D2558 to publish the VLC bitstream and frame-header pointers, records
 * the frame number of the header in D_801EB468, decodes the bitstream into the
 * CD buffer of the current index through DecDCTvlc, advances the decoder stream
 * position through func_801D2464 and always returns 0.
 * @status partial
 * @match 98.89
 * @residual The stream-start mode constant is loaded after the stream address instead of before it; both bodies are 360 bytes.
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
  do {
    size = 0x1C8;
  } while (0);
  func_801CFC00(D_801EB448, 0xF);
  pending = 0;
  func_801D23DC(1, 1, 0xFFFFFFF, 0, 0);
  D_801EB474 = 1;
  D_801EB475 = 1;
  func_801D0320(disc_lba, &D_801EB4AC);
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
