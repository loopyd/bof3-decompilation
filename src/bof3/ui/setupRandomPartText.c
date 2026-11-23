#include "bof3/bof3.h"

extern int rand(void);

/* Six-byte work run this overlay stages its ids and its text in. */
extern u8 D_801DB080[]; /* @source 0x801DB080 @kind data */

/* @source 0x801D8968
 * @behavior Clears the six-byte work run at 0x801DB080 from its last byte
 *           down to the first, then appends text to it twice. Each pass draws
 *           one rand() value, masks it to six bits, takes that halfword offset
 *           from its own 64-entry table - the first from 0x800103E0 and the
 *           second from 0x80010460 - and walks the bytes of the text sitting
 *           on the fixed 0x80010000 main-RAM base plus that offset, storing
 *           each byte at the run position the running eight-bit count selects
 *           and stopping on the zero terminator, so the run holds at most six
 *           bytes of the two randomly picked parts. Both tables and the text
 *           pool lie in the fixed main-RAM area below the main executable's
 *           load address, so no map symbol covers them and no word of this
 *           payload names them. Nothing else is read and no value is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setupRandomPartText(void) {
  u8 count;
  s32 i;
  u8 *src;

  count = 0;
  src = D_801DB080;
  for (i = 5; i >= 0; i--) {
    src[i] = 0;
  }

  src = (u8 *)(0x80010000u + ((u16 *)0x800103E0)[rand() & 0x3F]);
  while (1) {
    if (*src == 0) {
      break;
    }
    D_801DB080[count] = *src;
    count++;
    src++;
  }

  src = (u8 *)(0x80010000u + ((u16 *)0x80010460)[rand() & 0x3F]);
  while (1) {
    if (*src == 0) {
      break;
    }
    D_801DB080[count] = *src;
    count++;
    src++;
  }
}
