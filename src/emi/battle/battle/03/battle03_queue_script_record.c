#include "internal.h"

/* @behavior queues the selected 13-byte battle script record and updates its selector byte.
 * @source 0x801DE94C
 */
void battle03_queue_script_record(u32 arg0, s8 arg1) {
  func_801501E4(&D_801492B8, D_801EB000 + ((arg0 & 0xffu) * 0xdu), 0xcu);
  D_801EB4F2 = arg1;
}
