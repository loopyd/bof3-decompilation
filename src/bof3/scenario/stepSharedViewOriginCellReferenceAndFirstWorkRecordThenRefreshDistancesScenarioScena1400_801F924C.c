#include "bof3/bof3.h"

/* Shared view-origin mirror of the main executable: the first coordinate of
 * the pair the shared view routine at 0x801C5DFC derives from the view-origin
 * words D_80143F7C/D_80143F80 and mirrors here. The scenario step at
 * 0x801FB204 rebases this cell together with D_80143F7C.
 * @source 0x80149308 @kind data
 */
extern s32 D_80149308;

/* Second coordinate of the shared view-origin mirror pair.
 * @source 0x8014930C @kind data
 */
extern s32 D_8014930C;

/* Shared scroll word. No payload other than this one and the world03/area149/13
 * step reads it; the main executable writes it at 0x80152880 and 0x80153FB0 and
 * its role is not established, so it keeps its address name.
 * @source 0x80149310 @kind data
 */
extern u32 D_80149310;

/* Shared reference halfword of the render-cell mapping: the cell coordinate the
 * mapped-position helper subtracts from the high half of a 16.16 position, with
 * its partner D_80149322.
 * @source 0x80149320 @kind data
 */
extern u16 D_80149320;

/* First coordinate of the shared view-origin pair of the main executable.
 * @source 0x80143F7C @kind data
 */
extern s32 D_80143F7C;

/* Halfword beside the committed view-scale word D_80143F84 that this step
 * refreshes with the distance of the view-origin mirror pair.
 * @source 0x80143F86 @kind data
 */
extern u16 D_80143F86;

/* First coordinate of the first shared 0x98-byte work record's position pair.
 * @source 0x80147A8C @kind data
 */
extern s32 D_80147A8C;

/* Second coordinate of that work-record position pair.
 * @source 0x80147A90 @kind data
 */
extern s32 D_80147A90;

/* Halfword this step refreshes with the distance of the moved work-record
 * position pair.
 * @source 0x80147A96 @kind data
 */
extern u16 D_80147A96;

/* Fixed-point distance helper of the main executable: it answers the s16
 * distance between the two 16.16 coordinates it is passed. Its prototype is the
 * one the family headers declare for the same address.
 * @source 0x8015477C
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* Shared view helper of the main executable: it takes a halfword view value, as
 * the scenario stage setup at 0x801F9BE4 and the view publishers of the world
 * overlays show. Only its address is established here.
 * @source 0x80154FD8
 */
extern void func_80154FD8(u32 arg0);

/* Shared overlay helper the neighbouring scenario steps run after they rebase
 * the view words; only its address is established.
 * @source 0x80154698
 */
extern void func_80154698(void);

/* @source 0x801F924C
 * @behavior Overlay view step of this scenario overlay. Three in-image jal sites
 *           reach it - 0x801F8F88, 0x801F8FCC and 0x801F90DC, all inside the
 *           state controller func_801F8CE4 - and each passes no argument and
 *           ignores the result, so the body takes none and returns nothing.
 *           While the shared scroll word D_80149310 reads 0x77FF it consumes
 *           that request by rewriting the word as 0x63FF and moves the view one
 *           whole step: it adds 0x140000, that is 20.0 in 16.16, to the shared
 *           view-origin mirror word D_80149308 and to the view-origin word
 *           D_80143F7C, adds 0x14, that is 20 cells, to the render-cell
 *           reference halfword D_80149320, and stores the s16 distance
 *           func_8015477C answers for the moved D_80149308/D_8014930C pair into
 *           the view halfword D_80143F86. It hands that same distance, widened,
 *           to the shared view helper func_80154FD8, runs func_80154698, and
 *           then moves the first shared work record by the same 0x140000 along
 *           x - adding it to D_80147A8C - and stores the distance of the moved
 *           record position pair D_80147A8C/D_80147A90 into D_80147A96. A scroll
 *           word other than 0x77FF leaves every cell untouched. The 0x18-byte
 *           frame keeps $ra and $s0 only, $s0 being the live 0x140000 step the
 *           three additions share; the second coordinate of each pair is loaded
 *           before its step is committed, and both committed steps land in the
 *           jal delay slots.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 164 bytes at offset 0x2668 of
 * out/binaries/emi/scenario/scena14/00.bin, that is the whole core of this body
 * from the 0x63FF store through both distance refreshes, measured byte-identical,
 * relocations included, to the 164 bytes at offset 0x8CC of
 * out/binaries/emi/world03/area149/13.bin, the state-0 arm of func_801F3460
 * there, so that arm supplies the statement order and the declaration style of
 * the core while the scroll gate, the frame and the epilogue are this span own.
 * The immediate 0x77FF appears in those two payloads only, so the span has no
 * byte-identical twin to clone; it opens with its own addiu sp and closes with
 * jr ra around three jal sites, so neither the data nor the carve lever applied.
 * The three stepped cells are held in pointer locals, which is why the original
 * loads each through its register and stores through it, while the first
 * distance call re-reads D_80149308 with its own lui/lw pair because a store
 * through an unknown pointer discards the known value, and the second call reads
 * its first argument through the record pointer, which reuses the value just
 * stored and lets that store land in the jal delay slot. The measured ladder was
 * the direct-cell form of the three steps at 36 of 53 instructions with an
 * absolute lui at store, the pointer-local form at 47 of 56 instructions and 224
 * bytes, and the record-pointer argument above at 53 of 53 and 212 bytes.
 */
void stepSharedViewOriginCellReferenceAndFirstWorkRecordThenRefreshDistancesScenarioScena1400_801F924C(
    void) {
  s16 distance;

  if (D_80149310 == 0x77ffu) {
    s32 *origin = &D_80149308;
    s32 *view = &D_80143F7C;
    s32 *record = &D_80147A8C;

    D_80149310 = 0x63ffu;
    *origin += 0x140000;
    D_80149320 = (u16)(D_80149320 + 0x14);
    *view += 0x140000;
    distance = func_8015477C(D_80149308, D_8014930C);
    D_80143F86 = (u16)distance;
    func_80154FD8(distance);
    func_80154698();
    *record += 0x140000;
    D_80147A96 = (u16)func_8015477C(*record, D_80147A90);
  }
}
