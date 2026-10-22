#ifndef PLAYER_PSX_H
#define PLAYER_PSX_H

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int (*Call)(u32, u32, u32, u32);
#define CALL(address, a, b, c, d) (((Call)(address))((u32)(a), (u32)(b), (u32)(c), (u32)(d)))
#define HALF(address) (*(volatile u16 *)(address))
#define BYTE(address) (*(volatile u8 *)(address))
#define WORD(address) (*(volatile u32 *)(address))

#endif
