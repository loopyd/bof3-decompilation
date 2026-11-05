#include "bof3/bof3.h"

typedef void (*Scena0300WorkByte1Handler)(void);

extern u8* D_1F800044;
/* @source 0x801FCFAC @kind data */
extern Scena0300WorkByte1Handler workByte1HandlersScenarioScena0300_801FCFAC[];

/* @source 0x801F85FC
 * @behavior Reads the work-object pointer at scratchpad cell 0x1F800044 and
 * dispatches through the overlay-local table at 0x801FCFAC using the object's
 * unsigned byte at offset 1. Takes no arguments, performs no stores of its
 * own and discards the handler result; the caller must supply a valid index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0300_801F85FC(void) {
  workByte1HandlersScenarioScena0300_801FCFAC[D_1F800044[1]]();
}
