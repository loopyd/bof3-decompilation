#include "bof3/bof3.h"

/* @source 0x801F6CEC
 * @behavior No-op record callback: it takes no arguments, reads no state and
 * writes nothing, returning immediately. The original body is only the standard
 * `jr $ra` epilogue (8 bytes), because this function is installed as entry 0 of
 * the three-entry callback table D_801F6D7C, which func_801F6CAC indexes with
 * the signed byte at offset 0x7A of the record it is given.
 */
void noopRecordCallbackByByte7A_scena18(void) {}
