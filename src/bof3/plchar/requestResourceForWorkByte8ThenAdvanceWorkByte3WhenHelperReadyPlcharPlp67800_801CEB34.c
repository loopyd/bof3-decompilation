#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell for the
 * byte-8 argument and loads it again after the resource request, so the source
 * keeps the named cell form and evaluates it at both uses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Readiness poll helper of the main executable; the sibling plchar lifts of this
 * body declare the same prototype. The original keeps only the returned byte as
 * the ready test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling world, boss and
 * bmagic lifts declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* Clone dry-run: this 0x60-byte span compared byte-identical to the original
 * bytes of seven already exact lifts of the same body, at plp016 0x801CFC30,
 * plp026 0x801D0054, plp034 0x801CF84C, plp056 0x801CFE58, plp267 0x801CF32C,
 * plp349 0x801CEB24 and plp567 0x801CF130, one `cmp` per payload. The statement
 * order, the two pointer-cell evaluations and the declaration style carry over
 * with this function's own address and name.
 */
/* @source 0x801CEB34
 * @behavior Overlay step published at the code-pointer word 0x801D0584, entry 8
 *           of this overlay's thirty-word handler run based at 0x801D0564 that
 *           the byte-2 dispatcher func_801CE404 indexes with work-record byte 2
 *           scaled by four before its indirect call, so the body runs while that
 *           byte reads 8; the same word is also index 3 of the byte-3 table
 *           based at 0x801D0578 that 0x801CE674 indexes with record byte 3, and
 *           index 5 of the byte-2 table based at 0x801D0570 that 0x801CE630
 *           indexes with record byte 2, so no jal in this payload names it. It
 *           calls the shared readiness helper func_8014DAEC and keeps only the
 *           low byte of what that helper returns as a ready test: while the
 *           helper reports not ready the handler writes nothing. Once it reports
 *           ready the handler passes the work record's unsigned byte 8 to the
 *           shared main-executable resource request helper func_8014D6B8, then
 *           re-loads the scratchpad pointer cell 0x1F800044 and advances the
 *           record's unsigned byte 3, the work-step index, by one. Takes no
 *           arguments, returns nothing, reads or writes no other state, and the
 *           0x18-byte frame only keeps $ra at 0x10 across its two calls. The
 *           roles of record byte 8 and of the requested resource are not
 *           established here, so the name describes the effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceForWorkByte8ThenAdvanceWorkByte3WhenHelperReadyPlcharPlp67800_801CEB34(
    void) {
  if (func_8014DAEC()) {
    func_8014D6B8(D_1F800044[8]);
    D_1F800044[3]++;
  }
}
