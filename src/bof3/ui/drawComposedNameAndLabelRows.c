#include "bof3/bof3.h"

/* Five-byte-stride name table in main RAM: the row drawers of the neighbouring
 * commu overlays hand the address of one entry to func_8014F800 as a label
 * pointer, so an entry is a five-character name. */
extern u8 D_801457E8[][5]; /* @source 0x801457E8 @kind table */

/* Shared text buffer in main RAM the commu overlays print and compose their
 * text runs in before submission. */
extern u8 D_80145AD4[]; /* @source 0x80145AD4 @kind data */

/* Name id bytes of the row this overlay draws. Byte 0x801DB0AD selects the name
 * text the composed run opens with, byte 0x801DB0AE a second name text that
 * joins it unless it holds the 0xFF none sentinel, and signed byte 0x801DB0B4
 * the text the tail row submits. */
extern u8 D_801DB0AD; /* @source 0x801DB0AD @kind data */
extern u8 D_801DB0AE; /* @source 0x801DB0AE @kind data */
extern s8 D_801DB0B4; /* @source 0x801DB0B4 @kind data */

/* Current commu task pointer cell in main RAM. Byte 5 of the task is the name
 * table row this run appends as its label. */
extern u8 *D_801F2948; /* @source 0x801F2948 @kind data */

/* @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* @source 0x801D5B2C
 * @behavior Composes the shared text run at 0x80145AD4 and submits two rows.
 *           It writes 0x2A into the run's first byte, takes the halfword offset
 *           the name id at 0x801DB0AD selects from the table on the fixed
 *           0x80010200 main-RAM address, and copies the text on the 0x80010000
 *           base at that offset byte by byte from run position 1, one byte per
 *           pass, except that a byte of 0x12 or 0x13 also copies the byte after
 *           it and advances two positions, stopping on the zero terminator. The
 *           name id at 0x801DB0AE repeats that walk into the same run unless it
 *           reads 0xFF, so the run holds one or two names. It then writes 0x2B
 *           and 0xFF into the run, appends the five bytes of the name table row
 *           that byte 5 of the current commu task at 0x801F2948 selects, and
 *           terminates the run. The run is submitted through func_8014F800 at
 *           (0x32, 0x1E) with colour 0 and mask 0xFF, and a second row is
 *           submitted at (0x32, 0x42) with the same colour and mask, its handle
 *           being the 0x80010000 main-RAM base plus the halfword offset that the
 *           signed byte at 0x801DB0B4 selects in the table at 0x800101C6. The
 *           function takes no arguments, returns no value and reads no other
 *           cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawComposedNameAndLabelRows(void) {
  s32 i;
  u8 count;
  u8 *src;

  D_80145AD4[0] = 0x2A;
  src = (u8 *)(0x80010000u + PSX_REF(u16, 0x80010200u + D_801DB0AD * 2));
  count = 1;
  while (*src != 0) {
    D_80145AD4[count] = *src;
    count += 1;
    if ((u32)(*src - 0x12) < 2) {
      src++;
      D_80145AD4[count] = *src;
      count += 1;
    }
    src++;
  }

  if (D_801DB0AE != 0xFF) {
    src = (u8 *)(0x80010000u + PSX_REF(u16, 0x80010200u + D_801DB0AE * 2));
    while (*src != 0) {
      D_80145AD4[count] = *src;
      count += 1;
      if ((u32)(*src - 0x12) < 2) {
        src++;
        D_80145AD4[count] = *src;
        count += 1;
      }
      src++;
    }
  }

  D_80145AD4[count] = 0x2B;
  count += 1;
  D_80145AD4[count] = 0xFF;
  count += 1;

  for (i = 0; i < 5; i++) {
    D_80145AD4[count] = D_801457E8[D_801F2948[5]][i];
    count++;
  }

  D_80145AD4[count] = 0;
  func_8014F800(0x32, 0x1E, 0, 0xFF, (u32)D_80145AD4);
  func_8014F800(0x32, 0x42, 0, 0xFF,
                0x80010000u + PSX_REF(u16, 0x800101C6u + D_801DB0B4 * 2));
}
