#include "bof3/bof3.h"
#include "gpu/prim.h"

/* The four runtime-filled value-string tables of the shared main executable.
 * Every row is a glyph-code string of at most twelve cells plus its terminator,
 * and each table pads its rows to its own width: this body scales the code byte
 * by 18 for the default table, by 24, 22 and 20 for kinds 1, 2 and 3. The
 * neighbours in this tree prove the string role: the exact area03004 lift
 * func_801E1004 copies the 18-byte row D_801C8964[id + 0x38] up to its zero
 * byte into the UI text buffer, and drawAlternatingNameAndQueueIconStripWorld00Area03004_801D1F38
 * hands the 20-byte row D_801C9E7C[index * 20] to func_8014F800 as a 0xC-cell
 * text run. All four addresses resolve through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added.
 * @source 0x801C8964 @kind table
 */
extern u8 D_801C8964[];

/* @source 0x801C90DC @kind table */
extern u8 D_801C90DC[];

/* @source 0x801C98A4 @kind table */
extern u8 D_801C98A4[];

/* @source 0x801C9E7C @kind table */
extern u8 D_801C9E7C[];

/* The twelve-glyph 0x5C rule of this overlay's own text tail, the placeholder
 * the result panel writes over once it has a value to show. The exact sibling
 * drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174 draws
 * the same cell at (0x70, 0x98) and (0x70, 0xAC), the two rows this function
 * falls back to when the overlay work byte 0x4 is clear.
 * @source 0x801F0B8C @kind string
 */
extern u8 D_801F0B8C[];

/* Main-executable row width query: the low byte of its answer is the cell count
 * the shared text submitter func_8014F800 takes. The exact shop00 lift
 * drawShopEquipPanel calls it with one record pointer per slot, masks the answer
 * with 0xFF and passes it in that same argument, so the return stays word-wide
 * and the argument stays the row address it receives. */
u32 func_801AF20C(const void* arg0);

/* Text helper of this overlay, still an asm sibling: it walks the
 * NUL-terminated glyph-code string its first argument points at and queues one
 * textured quad per byte at the pen position given by the second argument (x)
 * and the third (y), taking the codes from the 25-column small font bank. */
void func_801EF590(u8* text, s32 x, s32 y);

/* @source 0x801EF470
 * @behavior Draws one value row of the battle result panel at pen column 0x70
 *           and the caller-supplied row y. It reads the scratchpad work-object
 *           pointer cell 0x1F800044 once, before the 0x28-byte frame exists, and
 *           its byte at offset 0x4 gates the whole body: the cell is
 *           dereferenced on both paths, so that load precedes the frame in the
 *           original.
 *           With that byte nonzero it selects one of the four value-string
 *           tables by the kind byte (second argument): 1 takes the 24-byte rows
 *           of D_801C90DC, 2 the 22-byte rows of D_801C98A4 and 3 the 20-byte
 *           rows of D_801C9E7C, while kind 0 shares the default arm that takes
 *           the 18-byte rows of D_801C8964; every larger kind lands there too.
 *           All four rows are scaled by the low byte of the first argument. It
 *           queries the selected row through func_801AF20C and submits it
 *           through the shared text submitter func_8014F800 as (0x70, y, 0,
 *           count & 0xFF, row), so the queried low byte is the submitted cell
 *           count and the row address is that call's fifth argument.
 *           With that byte clear it submits the twelve-glyph 0x5C rule
 *           D_801F0B8C at the same pen position through the local text helper
 *           func_801EF590, which is the panel value slot before a value exists.
 *           The caller func_801EF2DC passes the two bytes of one panel-record
 *           halfword as (low byte, high byte) and the value rows 0x98 and 0xAC.
 *           Frame 0x28 holding $ra at 0x20, $s1 at 0x1C and $s0 at 0x18, with
 *           the fifth call argument at 0x10; nothing is returned and no state is
 *           written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawKindSelectedValueTextOrRulePlaceholderBmagicMagic06000_801EF470(
    u8 code, u8 kind, s32 y) {
  u8* row;
  u32 count;
  u32 addr;

  if (SPAD_PTR_SLOT(u8, 0x44u)[4] != 0) {
    switch (kind) {
      case 0:
      default:
        addr = (u32)code << 3;
        addr += code;
        addr <<= 1;
        addr += (u32)D_801C8964;
        break;
      case 1:
        addr = (u32)code << 1;
        addr += code;
        addr <<= 3;
        addr += (u32)D_801C90DC;
        break;
      case 2:
        addr = (u32)code << 1;
        addr += code;
        addr <<= 2;
        addr -= code;
        addr <<= 1;
        addr += (u32)D_801C98A4;
        break;
      case 3:
        addr = (u32)code << 2;
        addr += code;
        addr <<= 2;
        addr += (u32)D_801C9E7C;
        break;
    }
    row = (u8*)addr;
    count = func_801AF20C(row);
    func_8014F800(0x70, (s16)y, 0, count & 0xFF, (u32)row);
  } else {
    func_801EF590(D_801F0B8C, 0x70, (s16)y);
  }
}
