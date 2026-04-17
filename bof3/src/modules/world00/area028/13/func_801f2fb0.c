#include "internal.h"

/* does: seeds one local AREA028 work entry with two random signed offsets and
 * the fixed depth-scale halfword `0x280`.
 * @source: 0x801f2fb0 FUN_801f2fb0
 */
void func_801f2fb0(void* arg0) {
  World00Area028Work* work;

  work = (World00Area028Work*)arg0;
  work->unk_00[0] = 1u;
  work->field_04 = (rand() & 0xff) - 0x80;
  work->field_06 = (rand() & 0xff) - 0x80;
  work->field_08 = 0x280;
}
