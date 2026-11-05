#include "bof3/bof3.h"

typedef void (*Scena0300WorkByte1Handler)(void);

extern u8* D_1F800044;
/* @source 0x801FCF88 @kind data */
extern Scena0300WorkByte1Handler workByte1HandlersScenarioScena0300_801FCF88[];

/* @source 0x801F7DF0
 * @behavior Reads the work-object pointer at scratchpad cell 0x1F800044 and
 * dispatches through the overlay-local table at 0x801FCF88 using the object's
 * unsigned byte at offset 1. The table contains handlers 0x801F7E34,
 * 0x801F7E6C and 0x801F7F84. Takes no arguments, performs no stores of its
 * own and discards the handler result; the caller must supply a valid index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0300_801F7DF0(void) {
  workByte1HandlersScenarioScena0300_801FCF88[D_1F800044[1]]();
}
