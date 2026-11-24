#include "bof3/bof3.h"

extern u8* workObjectPointer; /* @source 0x1F800044 @kind unknown */
void renderTranslucentLineBetween(
    void* first, void* second);

/* @source 0x801F9EC4
 * @behavior Subtract 0x20000 modulo 2^32 from work word 0x0C, then call
 * the overlay helper at 0x801FA19C with work addresses 0x34 and 0x0C.
 * Reload the scratchpad work-pointer cell after the call and increment
 * work byte 1 modulo 256 if word 0x0C equals 0x98000. Takes no arguments
 * and returns no value. The broader field and helper roles remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtractWorkAdvanceState(void) {
  u8* work;
  u8* nextWork;

  work = workObjectPointer;
  *(u32*)(work + 0x0C) -= 0x20000u;
  renderTranslucentLineBetween(
      work + 0x34, work + 0x0C);
  nextWork = workObjectPointer;
  if (*(u32*)(nextWork + 0x0C) == 0x98000u) {
    nextWork[1]++;
  }
}
