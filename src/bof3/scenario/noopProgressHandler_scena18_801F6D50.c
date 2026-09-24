#include "bof3/bof3.h"

/* @source 0x801F6D50
 * @behavior Empty per-frame progress handler for the scena18 overlay: it takes no
 * arguments, reads no state and writes no state, returning immediately through the
 * plain `jr $ra` epilogue (8 bytes: 03E00008 00000000). Its address is the seventh
 * and last word of this overlay's seven-entry four-byte per-frame handler table at
 * 0x801F6D6C (0x801F6C40, 0x801F6C54, 0x801F6C68, 0x801F6CA4, 0x801F6CEC,
 * 0x801F6D04, 0x801F6D50), which dispatchProgressHandler_scena18 indexes with the
 * signed shared progress byte D_80146872, so while that byte reads 6 the dispatch
 * calls this handler and the frame performs no handler work.
 */
void noopProgressHandler_scena18_801F6D50(void) {}
