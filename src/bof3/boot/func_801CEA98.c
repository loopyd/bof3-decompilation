#include "bof3/boot/logo_internal.h"

#include <libpress.h>

/*
 * @source 0x801CEA98
 * @behavior pumps one frame of the CAPCOM30.STR MDEC stream. It waits up to a
 * 0x100000-iteration timeout for func_801D2558 to publish the VLC bitstream and
 * frame-header pointers and reports 1 on timeout; then records the new frame
 * number in D_801EB468 and the previous one in D_801EB464, resets the LOGO
 * frame column RECT D_801EB45C to x 0, the y offset of the buffer selected by
 * D_801EB4A8, 0x18 wide and 0xF0 high, resets the DCT decoder, submits the CD
 * work buffer of the current D_801EB458 index, decodes the bitstream into the
 * buffer selected by the toggled index, advances the decoder stream position,
 * advances the frame column through func_801CEC88 when the previous frame
 * number left the 0xAD..0xFE window, then waits two vsyncs, flips the display
 * buffer and returns whether the previous frame number reached 0xDC.
 * @status exact
 * @match 100.00
 * @residual none
 * live comparison is instruction- and byte-exact.
 */
s32 func_801CEA98(void) {
  u_long* data;
  u_long* header;
  u_long new_frame;
  u_long last_frame;
  u_long timeout;
  u_long index;
  s32 y;
  s32 finished;

  y = -(D_801EB4A8 != 0) & 0xF0;
  timeout = 0x100000;
  while (func_801D2558(&data, &header) != 0) {
    if (--timeout == 0) {
      return 1;
    }
  }
  last_frame = D_801EB468;
  D_801EB464 = last_frame;
  new_frame = header[2];
  finished = (last_frame >= 0xDC);
  D_801EB45C.x = 0;
  D_801EB45C.y = y;
  D_801EB45C.w = 0x18;
  D_801EB45C.h = 0xF0;
  D_801EB468 = new_frame;
  DecDCTReset(1);
  DecDCTin(D_801EB44C_BUFFERS[D_801EB458], 1);
  func_801D6FCC(D_801EB444, 0xB40);
  index = (D_801EB458 < 1);
  D_801EB458 = index;
  DecDCTvlc(data, D_801EB44C_BUFFERS[index]);
  func_801D2464(data);
  if ((D_801EB464 - 0xAD) >= 0x52) {
    func_801CEC88();
  }
  VSync(2);
  func_801CE8DC();
  return finished;
}
