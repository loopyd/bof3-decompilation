#include "bof3/bof3.h"

/* The shared distance helper: it takes the two signed coordinates the shared
 * work block holds at 0x80149308 and 0x8014930C and answers with the distance
 * this body shifts left 16 into each record's 0x3C cell. No payload of this
 * overlay defines it, so only its call is bound here.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* The source position of both UI work records: this body stores the first cell
 * at record offset 0x34 and the second at 0x38 of each record and passes the
 * pair to the distance helper. Both cells lie outside this overlay's payload,
 * so they belong to the shared image; the second is already bound by the shared
 * map, the first is unbound and carries only this origin tag.
 */
extern s32 D_80149308; /* @source 0x80149308 @kind data */
extern s32 D_8014930C; /* @source 0x8014930C @kind data */

/* The shared pointer cell both records publish at offset 0x54: the first record
 * takes the word itself, the second the same word plus 8, and each record's
 * 0x50 word then takes the field at +4 of that published pointer.
 */
extern u8 *D_801F501C; /* @source 0x801F501C @kind data */

/* The published pointer cell of each work record, the first record's at
 * 0x801DA09C and the second record's at 0x801DA114. The original stores the
 * published pointer and then re-reads the same cell for the field at +4, which
 * is the reload a volatile cell forces, so both cells are declared volatile.
 */
extern u8 *volatile D_801DA09C; /* @source 0x801DA09C @kind data */
extern u8 *volatile D_801DA114; /* @source 0x801DA114 @kind data */

/* The first UI work record, based at 0x801DA048. Its byte at offset 1 is the
 * state byte the overlay's step dispatch at 0x801D2C0C keys the handler run on,
 * and its channel bytes at 0x5D through 0x5F are the ones its drawing step
 * steps.
 */
extern u8 D_801DA049; /* @source 0x801DA049 @kind data */
extern u32 D_801DA07C; /* @source 0x801DA07C @kind data */
extern u32 D_801DA080; /* @source 0x801DA080 @kind data */
extern u32 D_801DA084; /* @source 0x801DA084 @kind data */
extern u32 D_801DA088; /* @source 0x801DA088 @kind data */
extern u8 D_801DA090; /* @source 0x801DA090 @kind data */
extern u32 D_801DA098; /* @source 0x801DA098 @kind data */
extern s8 D_801DA0A5; /* @source 0x801DA0A5 @kind data */
extern s8 D_801DA0A6; /* @source 0x801DA0A6 @kind data */
extern s8 D_801DA0A7; /* @source 0x801DA0A7 @kind data */
extern u32 D_801DA0AC; /* @source 0x801DA0AC @kind data */
extern u32 D_801DA0B0; /* @source 0x801DA0B0 @kind data */
extern u32 D_801DA0B4; /* @source 0x801DA0B4 @kind data */

/* The second UI work record, the same 0x78-byte layout based at 0x801DA0C0 and
 * the record the drawing step of this overlay takes as its argument.
 */
extern u8 D_801DA0C1; /* @source 0x801DA0C1 @kind data */
extern u32 D_801DA0F4; /* @source 0x801DA0F4 @kind data */
extern u32 D_801DA0F8; /* @source 0x801DA0F8 @kind data */
extern u32 D_801DA0FC; /* @source 0x801DA0FC @kind data */
extern u32 D_801DA100; /* @source 0x801DA100 @kind data */
extern u8 D_801DA108; /* @source 0x801DA108 @kind data */
extern u32 D_801DA110; /* @source 0x801DA110 @kind data */
extern s8 D_801DA11D; /* @source 0x801DA11D @kind data */
extern s8 D_801DA11E; /* @source 0x801DA11E @kind data */
extern s8 D_801DA11F; /* @source 0x801DA11F @kind data */
extern u32 D_801DA124; /* @source 0x801DA124 @kind data */
extern u32 D_801DA128; /* @source 0x801DA128 @kind data */
extern u32 D_801DA12C; /* @source 0x801DA12C @kind data */

/* @source 0x801D27BC
 * @behavior Initializes both 0x78-byte UI work records of the SHISU work block,
 *           the first at 0x801DA048 and the second at 0x801DA0C0, from the
 *           shared position. Each record publishes the shared pointer cell
 *           0x801F501C at offset 0x54 (the second record that word plus 8) and
 *           that pointer's field at +4 into offset 0x50, takes the two shared
 *           coordinates into offsets 0x34 and 0x38, seeds the 16.16 cell at
 *           offset 0x40 with 0x1000, zeroes the byte at offset 0x48, the words
 *           at 0x64, 0x68 and 0x6C and the three channel bytes at 0x5D through
 *           0x5F to -0x80, and closes by storing the answer of the shared
 *           distance helper, shifted left 16 and biased by 0xFE000000 in the
 *           first record and by 0x02000000 in the second, into offset 0x3C. The
 *           first record's byte at offset 1 is the state byte the overlay's step
 *           dispatch at 0x801D2C0C keys its handler run on. No jal or table word
 *           of this payload names this body, so it is an overlay entry called
 *           from outside the payload; its 0x28-byte frame keeps $ra at 0x24
 *           above five saved registers and it returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeShisuUiWorkRecordsShisu00_801D27BC(void)
{
    s32 *px;
    s32 *py;
    u8 **pp;
    s32 field;

    px = &D_80149308;
    py = &D_8014930C;
    pp = &D_801F501C;
    D_801DA09C = *pp;
    field = *(s32 *)(D_801DA09C + 4);
    D_801DA049 = 0;
    D_801DA090 = 0;
    D_801DA088 = 0x1000;
    D_801DA0AC = 0;
    D_801DA0B0 = 0;
    D_801DA0B4 = 0;
    D_801DA0A5 = -0x80;
    D_801DA0A6 = -0x80;
    D_801DA0A7 = -0x80;
    D_801DA07C = *px;
    D_801DA080 = *py;
    D_801DA098 = field;
    D_801DA084 = (func_8015477C(D_80149308, D_8014930C) << 16) + 0xFE000000;
    D_801DA114 = *pp + 8;
    field = *(s32 *)(D_801DA114 + 4);
    D_801DA0C1 = 0;
    D_801DA108 = 0;
    D_801DA100 = 0x1000;
    D_801DA124 = 0;
    D_801DA128 = 0;
    D_801DA12C = 0;
    D_801DA11D = -0x80;
    D_801DA11E = -0x80;
    D_801DA11F = -0x80;
    D_801DA0F4 = *px;
    D_801DA0F8 = *py;
    D_801DA110 = field;
    D_801DA0FC = (func_8015477C(D_80149308, D_8014930C) << 16) + 0x02000000;
}
