#ifndef EMI_SCENA00_00_INTERNAL_H
#define EMI_SCENA00_00_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*Scena00RecordCallback)(void* record, u32 arg1);

/* Scenario record passed to dispatchRecordCallbackByByte7A; only the callback
 * selector byte at 0x7A is proven. */
typedef struct Scena00RecordDispatch {
  u8 pad_00[0x7A];
  u8 callback_index_7A; /* 0x7A */
} Scena00RecordDispatch;

/* 4-byte scale/depth cell of a work-object cursor: the cursor code copies the
 * whole word between cursors while the s16 screen depth the sprite emitter
 * consumes lives in its high halfword. */
typedef union Scena00CursorScale {
  s32 word;
  struct {
    u16 frac_00;
    s16 depth_02;
  } parts;
} Scena00CursorScale;

/* 12-byte cursor held at offset 0x34 of the scratch work object and of every
 * 0x98-byte work-area cell. */
typedef struct Scena00Cursor {
  s32 coord_x_00;
  s32 coord_y_04;
  Scena00CursorScale scale_08;
} Scena00Cursor;

/* 0x98-byte work-area cell based at 0x80146888; only the fields this overlay's
 * cursor-trail handler touches are modelled. */
typedef struct Scena00WorkCell {
  u8  flags_00;
  u8  state_01;
  u8  pad_02[4];    /* 0x02 - 0x05 */
  u8  index_06;     /* 0x06 - work-area cell index */
  u8  pad_07[0x2D]; /* 0x07 - 0x33 */
  Scena00Cursor cursor_34;
  u8  pad_40[0x58]; /* 0x40 - 0x97 */
} Scena00WorkCell;

/* Three signed tint channels copied straight into a sprite primitive. */
typedef struct Scena00Tint {
  s8 r;
  s8 g;
  s8 b;
} Scena00Tint;

/* Route work area based at 0x801448FC: the overlay's state-0 boot stores the
 * 16-bit front selection seed at offset 0 and, 0xC04 bytes into the same
 * object, clears the first slot-flag table. Only those two offsets are proven;
 * the interior stays unmodelled. The selector bundle bytes that share this
 * range (0x801448FF, 0x80144900, 0x80144904) are separate globals: their
 * stores address the absolute symbol rather than this base.
 */
typedef struct Scena00RouteWork {
  u16 selection_seed_00;           /* 0x801448FC */
  u8  unmodelled_02[0xC04 - 0x02]; /* 0x02 - 0xC03 */
  u8  slot_flags_a_C04[6][4];      /* 0x80145500 - 0x80145517 */
} Scena00RouteWork;

/* @source 0x1F800010 @kind unknown */
extern u16 * volatile D_1F800010;
/* Screen pair the overlay's trail handlers project into: the shared projector
 * func_801AFF04 writes it and the handlers copy both halfwords back into the
 * work object.
 */
/* @source 0x1F800034 @kind unknown */
extern u16 D_1F800034;
/* @source 0x1F800036 @kind unknown */
extern u16 D_1F800036;
/* @source 0x1F800000 @kind unknown */
extern u8 D_1F800000;
/* @source 0x1F800044 @kind unknown */
extern u8 *D_1F800044;
/* Primitive cursor shared with the battle/front-end lifts; owned by the shared
 * symbol map.
 */
/* @source 0x8014598C @kind unknown */
extern u8 *g_PrimCursor;
/* Shared scratch byte set to 2 when the middle controller's ramp countdown
 * expires; owned by the shared symbol map.
 */
/* @source 0x80149333 @kind unknown */
extern u8 D_80149333;
/* Shared frame counter owned by the shared symbol map. */
/* @source 0x80143E6C @kind unknown */
extern s32 D_80143E6C;
/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;
/* @source 0x80143F03 @kind unknown */
extern u8 D_80143F03;
/* @source 0x80145AA8 @kind unknown */
extern u16 D_80145AA8;
/* @source 0x80145EB4 @kind unknown */
extern u8 D_80145EB4;
/* Front-side gate byte; the route-start states require a value other than 2. */
/* @source 0x80145E91 @kind table */
extern u8 D_80145E91[];
/* Front-end word pair passed together to func_801BFE34 by the route states. */
/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x80147B28 @kind unknown */
extern s32 D_80147B28;
/* @source 0x80147BEC @kind unknown */
extern s32 D_80147BEC;
/* @source 0x80147BF0 @kind unknown */
extern s32 D_80147BF0;
/* Word pair the route-stage handler arms with 1/3 when the route word reads 25.
 * @source 0x80147BA0 @kind unknown
 */
extern u32 D_80147BA0;
/* @source 0x80147BA4 @kind unknown */
extern u32 D_80147BA4;
/* Mode/selector cells the route-stage handler arms when the shared mode byte
 * D_80146866 reads 3: the byte gets 7 and the word pair 1/4.
 * @source 0x80147C21 @kind unknown
 */
extern u8 D_80147C21;
/* @source 0x80147C38 @kind unknown */
extern u32 D_80147C38;
/* @source 0x80147C3C @kind unknown */
extern u32 D_80147C3C;
/* @source 0x80147C54 @kind unknown */
extern s32 D_80147C54;
/* @source 0x80147C58 @kind unknown */
extern s32 D_80147C58;
/* @source 0x80146864 @kind unknown */
extern u32 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8                  D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8                  D_80146866;
/* @source 0x80146888 @kind unknown */
extern u8                  D_80146888;
/* @source 0x80146890 @kind unknown */
extern u8                  D_80146890;
/* Word pair the route-stage handler arms with 2 when the route word reads 25.
 * @source 0x801468A0 @kind unknown
 */
extern u32                 D_801468A0;
/* @source 0x801468A4 @kind unknown */
extern u32                 D_801468A4;
/* @source 0x8014686C @kind unknown */
extern volatile u32        D_8014686C;
/* Signed main-RAM primary-state byte the overlay's primary-state dispatcher at
 * 0x801F9B98 indexes its secondary handler table with.
 * @source 0x80146874 @kind unknown
 */
extern s8                  D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8                  D_80146875;
/* @source 0x80146876 @kind unknown */
extern u16                 D_80146876;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;
/* Mid-scenario mode byte pair written by the front-selector handoff state.
 * Owned by the shared symbol map.
 */
/* @source 0x80143F1E @kind unknown */
extern u8 D_80143F1E;
/* @source 0x80143F1D @kind unknown */
extern u8 D_80143F1D;
/* @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;
/* @source 0x80143FC8 @kind unknown */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FD1 @kind unknown */
extern u8 D_80143FD1[];
/* Row word fields at +0x0C/+0x10/+0x18/+0x1C of the same 0x74-byte record
 * table; the route-stage handler at 0x801F92DC stores its 3/route/2/2 words
 * through these address-derived bases.
 * @source 0x80143FD4 @kind unknown
 */
extern u8 D_80143FD4[];
/* @source 0x80143FD8 @kind unknown */
extern u8 D_80143FD8[];
/* @source 0x80143FE0 @kind unknown */
extern u8 D_80143FE0[];
/* @source 0x80143FE4 @kind unknown */
extern u8 D_80143FE4[];
/* @source 0x8014402C @kind unknown */
extern u8 D_8014402C[];
/* @source 0x80144030 @kind unknown */
extern u8 D_80144030[];
/* @source 0x80144034 @kind unknown */
extern u8 D_80144034[];
/* @source 0x80143FF1 @kind unknown */
extern u8 D_80143FF1[];
/* @source 0x80143FFC @kind unknown */
extern u8 D_80143FFC[];
/* @source 0x80144000 @kind unknown */
extern u8 D_80144000[];
/* @source 0x80144004 @kind unknown */
extern u8 D_80144004[];
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* Signed main-RAM state byte the overlay's code-span dispatcher at 0x801F913C
 * indexes its handler table with.
 * @source 0x80146872 @kind unknown
 */
extern s8 D_80146872;
/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;
/* Set to 1 by the route-path-3 step once it has rolled the two fixed halfword
 * buffer tails; the battle/15 buffer swap sets the same byte.
 * @source 0x80145988 @kind unknown
 */
extern u8 D_80145988;
/* @source 0x801492D8 @kind unknown */
extern u16 D_801492D8;
/* @source 0x801492DA @kind unknown */
extern u16 D_801492DA;
/* @source 0x801492DC @kind unknown */
extern u16 D_801492DC;
/* @source 0x8014930C @kind unknown */
extern s32 D_8014930C;
/* @source 0x80149314 @kind unknown */
extern u32 D_80149314;
/* @source 0x80149322 @kind unknown */
extern u16 D_80149322;
/* @source 0x8014932C @kind unknown */
extern u16 D_8014932C;
/* @source 0x80147A90 @kind unknown */
extern s32 D_80147A90;
/* Held-object world offset stepped by the middle controller's sound-ramp
 * states, four frames at a time.
 */
/* @source 0x80147A96 @kind unknown */
extern u16 D_80147A96;
/* Shared 152-byte work-record table the scratch pad work pointer at 0x1F800044
 * addresses; the selected-record handler copies a record's word at 0x54 into
 * the current work object.
 */
/* @source 0x80147A58 @kind table */
extern u8 D_80147A58[];
/* Two fixed-point channel words the middle controller ramps in opposite
 * directions with the signed D_801FCA4C nibble table.
 */
/* @source 0x801469EC @kind unknown */
extern s32 D_801469EC;
/* @source 0x801469F0 @kind unknown */
extern s32 D_801469F0;
/* @source 0x80143F80 @kind unknown */
extern s32 D_80143F80;
/* @source 0x80143F7C @kind unknown */
extern s32 D_80143F7C;
/* Committed view-scale word; its high halfword 0x80143F86 is re-read when the
 * countdown reruns func_80154FD8.
 */
/* @source 0x80143F84 @kind unknown */
extern u32 D_80143F84;
/* @source 0x80149308 @kind unknown */
extern u32 D_80149308;
/* @source 0x8017F974 @kind unknown */
extern void *D_8017F974[];
/* Four route bytes (0x0A, 0x32, 0x14, 0x32) keyed by the front-side byte
 * D_80143F03.
 */
/* @source 0x801FCA5C @kind table */
extern u8 D_801FCA5C[];
/* Two signed one-byte cursor offsets of the second-level handler table's
 * parameter block, read as the pair (0x0C, 0xC8) by the work-object handler at
 * 0x801F878C.
 * @source 0x801FC9E8 @kind table
 */
extern u8 D_801FC9E8[];
/* Signed one-byte step table indexed by the low nibble of the shared frame
 * word D_80143E6C; the middle controller scales its entries by 4 and by 0x800.
 */
/* @source 0x801FCA4C @kind table */
extern u8 D_801FCA4C[];
/* Two parallel one-byte tables indexed by the held-button nibble. */
/* @source 0x801FCA64 @kind table */
extern u8 D_801FCA64[];
/* @source 0x801FCA65 @kind table */
extern u8 D_801FCA65[];
extern Scena00RecordCallback D_801FCA84[];
/* Three signed tint channels the work-object cursor-trail emitter copies into
 * every sprite primitive it appends.
 * @source 0x801F6C04 @kind table
 */
extern s8 g_WorkObjectTint[];
extern u8* D_80147AA8;
extern u8* D_80147BD8;
extern s8* volatile D_80147BDC;
extern s32 D_801FCA90[][12];
extern s32 D_801FD030[][12];
extern s32 D_801FD5D0[][12];
/* Local per-frame handler table whose entry is selected by the scratch work
 * object's state byte at offset 1; every entry is one of the overlay's own
 * handler entry points.
 */
/* @source 0x801FC980 @kind table */
extern void (*D_801FC980[])(void);
/* Level-2 per-frame handler table of this code span, selected by the dispatch
 * byte at offset 1 of the scratch work object published at 0x1F800044; entries
 * 0 through 13 are the overlay's handlers 0x801F7484 through 0x801F82B4.
 */
/* @source 0x801FC99C @kind table */
extern void (*D_801FC99C[])(void);
/* Tail of the per-frame handler table above, read from its entry 11 and
 * selected by the dispatch byte at offset 1 of the scratch work object
 * published at 0x1F800044; entries 0 through 9 are the overlay's own handlers
 * 0x801F7A5C through 0x801F82B4.
 */
/* @source 0x801FC9AC @kind table */
extern void (*D_801FC9AC[])(void);
/* Second-level per-frame handler table of the same code span, contiguous with
 * the table above and selected by the dispatch byte at offset 1 of the scratch
 * work object published at 0x1F800044; entries 0 through 4 are the overlay's
 * fade-step handlers 0x801F878C through 0x801F8AB8.
 * @source 0x801FC9D4 @kind table
 */
extern void (*D_801FC9D4[])(void);
/* Signed one-byte fade-step table indexed by the work object's countdown byte
 * at offset 0x09 shifted right by two; the state-1 handler of the second
 * handler table at 0x801FC9D4 adds the selected entry to the object's halfword
 * at offset 0x30.
 */
/* @source 0x801FC9EC @kind table */
extern u8 D_801FC9EC[];
/* Overlay-local dispatch table of this code span's state handlers, indexed by
 * the main-RAM state byte D_80146872; entries 0 through 14 are the handlers
 * running from 0x801F9178 to 0x801FB924, and the table ends immediately before
 * the byte table at 0x801FCA4C.
 * @source 0x801FCA10 @kind table
 */
extern void (*D_801FCA10[])(void);
/* Overlay-local secondary dispatch table of this code span's primary state
 * handlers, indexed by the main-RAM state byte D_80146874: it is the D_801FCA10
 * handler table read from its third entry, so its first entries are the empty
 * handlers 0x801F9BD4/0x801F9BDC followed by the primary controller 0x801F9BE4
 * and the middle controllers.
 * @source 0x801FCA1C @kind table
 */
extern void (*D_801FCA1C[])(void);

/* Route work area the state-0 boot seeds and clears; owned by this target. */
/* @source 0x801448FC @kind data */
extern Scena00RouteWork D_801448FC;
/* Front selector kind byte of the bundle stored in the work area above. */
/* @source 0x801448FF @kind data */
extern u8 D_801448FF;
/* Front selector channel word pair of the bundle stored above. */
/* @source 0x80144900 @kind data */
extern u32 D_80144900;
/* @source 0x80144904 @kind data */
extern u32 D_80144904;
/* Second per-slot flag table, right behind the work area's first one; the
 * same address is mapped by the sibling battle/15 target as 12 rows of four
 * bytes.
 */
/* @source 0x80145518 @kind table */
extern u8 D_80145518[][4];

/* Named table remapper over one front-side byte; the sibling front-end lift
 * declares the same prototype.
 */
u32  func_8014D8D4(u8 arg0);
void func_801A78F8(void);

void func_8015B580(u32 arg0, s32 arg1);
/* Fixed-point distance between the word pair passed as world coordinates. */
s16  func_8015477C(s32 arg0, s32 arg1);
void func_80164020(s32 arg0, s32 arg1);
s32  func_8015B5D4(u32 arg0, s32 arg1);
s32  func_8015BEA0(s32 arg0, s32 arg1);
void func_80150224(u32 arg0);
void func_80154698(void);
void func_8015C058(void);
void func_8015C088(void);
void func_8015C100(void);

/* EXE-side slot loader the route-path-3 step at 0x801F9868 arms before it
 * polls func_80162D00 and seeds the scenario object row (declared with the
 * same prototype by the sibling scenario target). */
void func_80161BBC(u32 slot_id);

/* @source 0x801C1DF0 - front-end scene init the overlay's state-0 boot runs
 * before it seeds its selector bundle (declared with the same prototype by the
 * sibling scenario targets).
 */
void func_801C1DF0(u32 arg0);

/* Route-state companion calls into the EXE-side front end. */
void func_801BFE34(u32 arg0, u32 arg1, s32 arg2);
void func_801C7A54(s32 arg0);
void func_801C601C(u32 arg0);
void func_80153DE8(void);
void func_80154FD8(u32 arg0);

/* PsyQ libc random source, called twice per scenario record. */
int rand(void);

/* @behavior queues one frontend cue/event id through the EXE-side dispatcher.
 * @source 0x8015DF18
 */
void game_queue_frontend_cue(u32 cue_id);

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8016C0C0(s32 arg0, s32 arg1);

u8   func_8019601C(void);
void func_80196070(void);

/* @source 0x801AFF04 - shared projector: projects the three-word point passed
 * as arg0 into the screen pair at arg1 (declared with the same prototype by
 * the world targets, whose trail lifts call it the same way).
 */
void func_801AFF04(const void* arg0, void* arg1);

/* @behavior copies the active front selector/context bundle into the entry-0
 * local runtime state.
 * @source 0x8019FA28
 */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x8017B2B4 - PsyQ LIBGPU graphics-type query; absent from the
 * shipped headers, and declared with the same prototype by the sibling world
 * targets.
 */
s32 GetGraphType(void);

/* @source 0x8014E5A0 - engine primitive-append helper (lifted in
 * exe/slus_004.22); takes the primitive size class and the allocation class.
 */
void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x8017AA80 - engine primitive stamper this overlay's cursor-trail
 * emitter calls immediately before it writes the primitive's colour and vertex
 * fields (declared with the same prototype by the sibling sprite emitters).
 */
void func_8017AA80(u32 arg0);

/* @source 0x8017C1A8 - LIBGPU texture-window setter (the shipped SDK map has
 * no row for this address): fills the three-word DR_TWIN packet from the
 * (x, y, w, h) rect, whose x/y are read as bytes and w/h as signed halfwords,
 * as the 0xE2-prefixed texture-window word.
 */
void func_8017C1A8(DR_TWIN *p, RECT *tw);

/* @source 0x80155A08 - shared grid-cell primitive linker (declared the same way
 * by the world targets): takes the two packed fixed-point coordinates of one
 * grid cell plus its layer kind and primitive size class.
 */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801AC1DC - game-side local-origin resolver for the scratchpad work
 * object published at 0x1F800044: builds the work object's matrix from its
 * words 0x64/0x68/0x6C, transforms the selected record's three recorded
 * halfword offsets through it and stores the three fixed-point result words.
 */
void func_801AC1DC(s32 *origin, u32 record_byte);

/* @source 0x8015B410 - shared view-matrix builder the overlay's primitive
 * emitters call inside a PushMatrix/PopMatrix pair; func_8015B4B0 rebuilds the
 * matrix copy handed to it (both declared with the same prototypes by the
 * world targets' quad emitters).
 */
void func_8015B410(MATRIX* arg0);
void func_8015B4B0(MATRIX* arg0);

/* @source 0x801782FC - the overlay-local rotation result pair: the
 * 0x1000-per-turn angle passed as arg0 yields the local sine/cosine result,
 * which the emitters scale by their radius and shift right by 8 or 12.
 */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x8017A97C - engine primitive stamper this overlay's quad emitters
 * call on the cursor before writing the primitive's fields (declared with the
 * same prototype by the sibling sprite emitters).
 */
void func_8017A97C(void* arg0);

void func_801BE1B0(u32 arg0);
void func_801C187C(s32 arg0);

void func_801F7134(s32 chapter_id);
/* Sprite emitter shared with the cursor-trail handlers: projects both cursor
 * points and tints the primitive. The two 12-byte cursors plus the 3-byte tint
 * are proved by this overlay's callers (argument image) and by the callee's own
 * argument reads.
 * @source 0x801F78EC
 */
void func_801F78EC(Scena00Cursor first, Scena00Cursor second, Scena00Tint tint);
/* Overlay-local helper invoked once per frame by the dispatch-byte-0x14 entry
 * of this code span's handler table before that handler advances its ramp
 * words; the call site passes no arguments and consumes no return value.
 * @source 0x801F8360
 */
void func_801F8360(void);
/* Overlay-local primitive emitter called by the handler-table entries of this
 * code span; only the caller-observed four-word argument image is proven (the
 * first argument is scaled by the callee, the other three are stored into the
 * scratch work object's words at offsets 0x64/0x68/0x6C).
 * @source 0x801F85A8
 */
void func_801F85A8(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Overlay-local sprite emitter run by the handler-table entries of this code
 * span; only the caller-observed no-argument image is proved.
 * @source 0x801F8BCC
 */
void func_801F8BCC(void);
/* Following per-frame handler in this overlay's code span, run by the trail
 * handler when the work object is still fading; only the caller-observed
 * no-argument image is proven.
 * @source 0x801F8BCC
 */
void func_801F8BCC(void);
/* Route-stage companion the handler at 0x801F92DC runs unconditionally once
 * its route-1 block ends; only the caller-observed no-argument image is
 * proven.
 * @source 0x801F9644
 */
void func_801F9644(void);
/* Second route-stage companion the handler at 0x801F92DC runs unconditionally
 * after func_801F9644; only the caller-observed no-argument image is proven.
 * @source 0x801F9868
 */
void func_801F9868(void);
/* Following overlay-local beam entry this code span's wrapper at 0x801F8D70
 * runs; the wrapper sets up no argument register image, so the no-argument
 * prototype is proven by that call site.
 * @source 0x801F8D90
 */
void drawLinkedRecordCursorBeam(void);
s32  returnZero(void);
void resetEffectBank(void);

#endif
