#ifndef FRONTEND_STATE_H
#define FRONTEND_STATE_H

#include "base/types.h"
#include "memory/access.h"

#define g_GameState PSX_REF(volatile u8, 0x80143BB0u)

#define g_StreamHint PSX_REF(volatile u8, 0x80145024u)

#define g_GlobalFlag832E PSX_REF(volatile u8, 0x8014832Eu)

#define FrontendSetMode func_8014ECAC

void func_80196070(void);

#endif
