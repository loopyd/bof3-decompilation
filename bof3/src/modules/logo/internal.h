#ifndef BOF3_SRC_MODULES_LOGO_INTERNAL_H
#define BOF3_SRC_MODULES_LOGO_INTERNAL_H

#include "bof3/modules/logo/logo.h"

/* clang-format off */
#include <sys/types.h>
#include <libcd.h>
#include <libetc.h>
/* clang-format on */

#include "bof3/defines.h"

extern u32 DAT_8003b800;
void       SetDispMask(int mask);

void func_801ce758(void);
void logo_stream_boot(void* work_base, u_long disc_lba);
bool logo_stream_tick(void);
void logo_stream_shutdown(void);

#endif
