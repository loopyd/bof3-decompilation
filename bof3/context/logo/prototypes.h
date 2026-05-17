#ifndef BOF3_CONTEXT_LOGO_PROTOTYPES_H
#define BOF3_CONTEXT_LOGO_PROTOTYPES_H

/* function prototypes */

void       SetDispMask(int mask);
void func_801ce758(void);
void logo_stream_boot(void* work_base, u_long disc_lba);
bool logo_stream_tick(void);
void logo_stream_shutdown(void);
#endif
