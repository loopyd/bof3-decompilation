#ifndef BOF3_SRC_MODULES_LOGO_INTERNAL_H
#define BOF3_SRC_MODULES_LOGO_INTERNAL_H

#include "bof3/bof3.h"


/* clang-format off */
/* clang-format on */


extern u32 DAT_8003b800;
void SetDispMask(int mask);

void func_801ce758(void);
void logo_stream_boot(void* work_base, u_long disc_lba);
bool logo_stream_tick(void);
void logo_stream_shutdown(void);

/* from old bof3/include/bof3/modules/logo.h */
void func_801cedfc(void);

#endif
