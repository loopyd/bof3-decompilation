#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

/* PS1 scratchpad pointer - points to a per-overlay work area */
#define SCRATCH_PTR ((volatile void**)0x1F800044u)
#define SCRATCH     ((volatile u8*)0x1F800044u)

/* Global work pointer used by front-end/game modules */
#define GLOBAL_WORK (*(volatile u8**)0x80146250u)

#endif
