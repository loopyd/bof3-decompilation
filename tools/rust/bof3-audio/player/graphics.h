#ifndef PLAYER_GRAPHICS_H
#define PLAYER_GRAPHICS_H

#include "psx.h"

void graphics_initialize(void);
void graphics_begin_frame(u32 frames);
void graphics_present(void);
void graphics_rectangle(u32 x, u32 y, u32 width, u32 height, u32 color);
void graphics_shade(u32 x, u32 y, u32 width, u32 height);
void graphics_text_scaled(u32 x, u32 y, const char *string, u32 width, u32 height);
void graphics_text(u32 x, u32 y, const char *string);
void graphics_logo(void);

#endif
