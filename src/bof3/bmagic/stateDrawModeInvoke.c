#include "bof3/bof3.h"
#include "gpu/prim.h"

/*
 * Scratchpad work-object pointer cell, outside this overlay's image and re-read
 * after the indirect call because the invoked handler may rewrite it. This
 * target's map binds no name there, so the cell keeps its address-anchored name
 * and the compiler re-materialises the constant address for every access, which
 * is the original's lui/lw pair rather than a kept-base accessor.
 */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/*
 * Shared main-executable cell this overlay step brackets its handler call with.
 * It sits outside this payload and the body only publishes its two literals, so
 * it keeps the address-anchored name its other callers in this directory use.
 */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/*
 * Handler table this step indexes, bound at the word the scaled index is added
 * to: it is word 2 of the six-word code pointer run at 0x801EF6CC (entry 0 =
 * 0x801EF104, this function itself; entry 1 = 0x801EF4B4, the work byte 2
 * dispatcher; then 0x801EF1EC, 0x801EF380, 0x801EF3E4 and 0x801EF454), whose
 * first word the byte 1 dispatcher at 0x801EF0C0 uses as its run base. The four
 * words from here close the payload, and the index is not bounded.
 */
extern void (*D_801EF6D4[])(void); /* @source 0x801EF6D4 @kind data */

/*
 * Shared main-executable service this overlay calls from the guarded tail; it
 * takes no arguments and its result is discarded.
 */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* Shared graphic-type query used for the draw-mode tpage argument. */
extern s32 GetGraphType(void);

/* @source 0x801EF104
 * @behavior Work byte 2 step entry of this overlay's work object, entry 0 of the
 *           six-word code pointer run at 0x801EF6CC that the exact lift
 *           callStateTablef6Bmagic indexes with
 *           work-object byte 1. It selects the draw-mode tpage from
 *           GetGraphType - 0xA5 for graph types 1 and 2, 0x35 otherwise - seeds
 *           SetDrawMode on the shared primitive cursor at 0x8014598C with dfe 0,
 *           dtd 1 and a null tw (the zero store at 0x10 of the 0x20-byte frame
 *           fills that argument slot) and appends a 0xC-byte primitive for
 *           output list 3 through the shared primitive-append helper at
 *           0x8014E5A0. It publishes 0x800C3800 into the shared cell at
 *           0x801459F0, reads the work object published at the scratchpad
 *           pointer cell 0x1F800044, scales that object's unsigned byte 2 by
 *           four and invokes the selected entry of the handler table based at
 *           0x801EF6D4 with no arguments, without bounding the index. After the
 *           handler returns it re-reads the pointer cell and, when the work
 *           object's byte 0 and byte 2 are both non-zero, runs the shared
 *           main-executable service func_8014D290. It then publishes 0x800D3800
 *           into 0x801459F0 and returns. Each literal reaches the cell through
 *           its own lui/ori pair and a lui %hi / sw %lo store, the second one
 *           re-materialised in both guard delay slots and again after the
 *           service call, and the 0x20-byte frame keeps only $ra.
 *
 *           Shape evidence: the draw-mode preamble is word for word the preamble
 *           of the exact lift setDrawModeThenDispatchWorkByte2HandlerBmagicMagic12003_801EFD20,
 *           whose body differs from this span in the three tpage literals alone
 *           up to the table load, so the graph-type select is written the way
 *           that sibling spells it. The same preamble opens the exact lift at
 *           0x801F0764 in magic113 and the still-unlifted span at 0x801EFAF4 in
 *           magic004, each again differing only in its own literals there. The
 *           guarded tail is word for
 *           word the body of the exact lift dispatchStateByte2Tablef8c4BmagicMagic11503_801EF098,
 *           which carries the same cell re-read, the same two-byte guard and the
 *           same literal pair without the preamble; a full-payload scan of the
 *           0xE8-byte span over the payloads under out/binaries returns no
 *           byte-identical copy, so the two shapes were joined rather than
 *           cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateDrawModeInvoke(
    void) {
  u8 *work;
  s32 graph;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);
  D_801459F0 = 0x800C3800;
  D_801EF6D4[D_1F800044[2]]();
  work = D_1F800044;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
  }
  D_801459F0 = 0x800D3800;
}
