#ifndef EMI_LOADER_H
#define EMI_LOADER_H

#include "bof3/defines.h"

void func_80161fdc(u32 slot_id);
s32 func_80162d00(void);

#define emi_stream_init_slot func_80161fdc
#define emi_loader_is_ready func_80162d00

#endif
