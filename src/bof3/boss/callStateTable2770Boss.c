#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the payload's reviewed `textbin` row D_800C2720
 * spans 0x800C2720 through 0x800C279F. Its first eight words are a small record
 * (0x83028100, 0x83028504, 0x8100, 0x81018101, 0x81018101, 0x8101, 0xFFFF0600,
 * 0xFFFFFFFF) and the twenty-four words from 0x800C2740 to 0x800C279C are
 * in-image code pointers, this payload's functions interleaved with the shared
 * main-executable handlers at 0x801E...... The run is only referenced here. This
 * dispatcher indexes it from the interior word 0x800C2770, the thirteenth pointer
 * word and the twenty-first word of the row, so the run stays one reviewed row and
 * this declaration binds only that word. From there the twelve reachable entries
 * are 0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490, 0x800C1940, 0x801E4928,
 * 0x800C1A04, 0x800C1A4C, 0x800C1B9C, 0x800C1BF4, 0x800C1BFC and 0x800C1C04,
 * the last word of the row. */
extern void (*D_800C2770[])(void); /* @source 0x800C2770 @kind data */

/* @source 0x800C18FC
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that record's
 * unsigned byte at offset 2, scales it by four and calls the matching entry of
 * this overlay's handler run based at the interior word 0x800C2770 with no
 * arguments. The original does not bound the index before the scaled table load.
 * It reads no other state and writes none; the pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 * and the slots after the index byte and after the table load stay nops. The
 * epilogue leaves $v0 untouched, so this shim returns whatever the selected
 * handler returns. The payload holds no jal naming 0x800C18FC; the one word that
 * does name it is the table entry at 0x800C2758, so the overlay loader that steps
 * the pointer run is its caller. Same statement shape as this target's
 * callStateTable2740Boss, which was lifted exact; that
 * body differs from this span in the work-record byte the code reads and in the
 * table base of the scaled load, the two words at 0x800C190C and 0x800C1920.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callStateTable2770Boss, D_800C2770);
