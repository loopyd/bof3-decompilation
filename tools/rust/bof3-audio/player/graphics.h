#ifndef PLAYER_GRAPHICS_H
#define PLAYER_GRAPHICS_H

#include "psx.h"

#define TEXT_BUTTON_CROSS    "\340"
#define TEXT_BUTTON_CIRCLE   "\341"
#define TEXT_BUTTON_SQUARE   "\342"
#define TEXT_BUTTON_TRIANGLE "\343"
#define TEXT_BUTTON_UP       "\344"
#define TEXT_BUTTON_DOWN     "\345"
#define TEXT_BUTTON_LEFT     "\346"
#define TEXT_BUTTON_RIGHT    "\347"
#define TEXT_BUTTON_L1       "\350"
#define TEXT_BUTTON_L2       "\351"
#define TEXT_BUTTON_R1       "\352"
#define TEXT_BUTTON_R2       "\353"
#define TEXT_BUTTON_START    "\354"
#define TEXT_BUTTON_SELECT   "\356"

void graphics_initialize(void);
void graphics_begin_frame(u32 frames);
void graphics_present(void);
void graphics_rectangle(u32 x, u32 y, u32 width, u32 height, u32 color);
void graphics_panel(u32 x, u32 y, u32 width, u32 height,
                    u32 top, u32 bottom, int border);
void graphics_text_scaled(u32 x, u32 y, const char *string, u32 width, u32 height);
void graphics_text(u32 x, u32 y, const char *string);
void graphics_logo(void);

#endif
