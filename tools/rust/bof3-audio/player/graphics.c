#include "graphics.h"

extern const u32 player_font[], player_palette[], player_logo[];
static u32 glyph_packets[1024];
static u32 glyph_head;
static u32 draw_y;

static void gpu(u32 command)
{
    while ((WORD(0x1f801814) & 0x04000000) == 0) {}
    WORD(0x1f801810) = command;
}

static u32 xy(u32 x, u32 y) { return x | (y << 16); }

static void upload(u32 x, u32 y, u32 width, u32 height, const u32 *pixels)
{
    u32 count;
    gpu(0xa0000000);
    gpu(xy(x, y));
    gpu(xy(width, height));
    count = width * height / 2;
    while (count-- != 0) gpu(*pixels++);
}

void graphics_rectangle(u32 x, u32 y, u32 width, u32 height, u32 color)
{
    gpu(0x60000000 | color);
    gpu(xy(x, y));
    gpu(xy(width, height));
}

static int sine(u32 angle)
{
    static const int quarter[17] = {
        0, 100, 200, 297, 392, 483, 569, 650, 724,
        792, 851, 903, 946, 980, 1004, 1019, 1024
    };
    u32 index, fraction;
    int value;
    angle &= 255;
    index = angle & 127;
    if (index > 64) index = 128 - index;
    fraction = index & 3;
    index >>= 2;
    value = quarter[index];
    if (index < 16) value += (quarter[index + 1] - value) * fraction / 4;
    return angle & 128 ? -value : value;
}

static void background(u32 frames)
{
    static const u8 faces[6][4] = {
        {0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 4, 5},
        {2, 3, 6, 7}, {0, 2, 4, 6}, {1, 3, 5, 7}
    };
    u32 points[8], vertex, face, corner, axis;
    int x, y, z, rotated_x, rotated_y, rotated_z, yaw_sin, yaw_cos;
    int pitch_sin, pitch_cos;
    gpu(0xe1000200);
    gpu(0x38000000); gpu(xy(0, 0));
    gpu(0); gpu(xy(320, 0));
    gpu(0x380624); gpu(xy(0, 240));
    gpu(0x380624); gpu(xy(320, 240));
    yaw_sin = sine(frames / 3);
    yaw_cos = sine(frames / 3 + 64);
    pitch_sin = sine(frames / 5 + 24);
    pitch_cos = sine(frames / 5 + 88);
    for (vertex = 0; vertex < 8; ++vertex) {
        x = vertex & 1 ? 64 : -64;
        y = vertex & 2 ? 64 : -64;
        z = vertex & 4 ? 64 : -64;
        rotated_x = (x * yaw_cos + z * yaw_sin) / 1024;
        rotated_z = (z * yaw_cos - x * yaw_sin) / 1024;
        rotated_y = (y * pitch_cos - rotated_z * pitch_sin) / 1024;
        rotated_z = (y * pitch_sin + rotated_z * pitch_cos) / 1024;
        points[vertex] = xy(160 + rotated_x * 300 / (rotated_z + 320),
                            148 + rotated_y * 300 / (rotated_z + 320));
    }
    /* Quarter-additive faces and edges keep the cube below the HUD's contrast. */
    gpu(0xe1000260);
    for (face = 0; face < 6; ++face) {
        gpu(0x2a402030);
        for (corner = 0; corner < 4; ++corner) gpu(points[faces[face][corner]]);
    }
    for (vertex = 0; vertex < 8; ++vertex) {
        for (axis = 1; axis <= 4; axis <<= 1) {
            if ((vertex & axis) == 0) {
                gpu(0x42903068);
                gpu(points[vertex]); gpu(points[vertex | axis]);
            }
        }
    }
}

void graphics_text_scaled(u32 x, u32 y, const char *string, u32 width, u32 height)
{
    u32 *packet, *end, length, index, px, py, uv, u, v;
    if (width == 0 || height == 0) return;
    glyph_head = 0x00ffffff;
    WORD(0x8014598c) = (u32)glyph_packets;
    WORD(0x801459d4) = (u32)&glyph_head;
    BYTE(0x80143d44) = 12;
    CALL(0x80150098, 0, 0, 0, string);
    end = (u32 *)WORD(0x8014598c);
    for (packet = glyph_packets; packet < end; packet += length + 1) {
        length = *packet >> 24;
        if (length == 3) {
            px = x + (packet[2] & 0xffff) * width / 8;
            py = y + (packet[2] >> 16) * height / 8;
            if (width == 8 && height == 8) {
                gpu(packet[1]); gpu(xy(px, py)); gpu(packet[3]);
            } else {
                uv = packet[3]; u = uv & 255; v = (uv >> 8) & 255;
                gpu(0x2e000000 | (packet[1] & 0xffffff));
                gpu(xy(px, py)); gpu(uv);
                gpu(xy(px + width, py)); gpu((u + 8) | (v << 8) | 0x002f0000);
                gpu(xy(px, py + height)); gpu(u | ((v + 8) << 8));
                gpu(xy(px + width, py + height)); gpu((u + 8) | ((v + 8) << 8));
            }
        } else {
            for (index = 1; index <= length; ++index) gpu(packet[index]);
        }
    }
}

void graphics_text(u32 x, u32 y, const char *string)
{
    graphics_text_scaled(x, y, string, 8, 8);
}

void graphics_initialize(void)
{
    CALL(0x8017af0c, 0, 0, 0, 0);
    WORD(0x1f801814) = 0x08000001;
    WORD(0x1f801814) = 0x06c60260;
    WORD(0x1f801814) = 0x07042018;
    WORD(0x1f801814) = 0x03000000;
    gpu(0xe6000000);
    upload(960, 0, 64, 256, player_font);
    upload(0, 480, 256, 1, player_palette);
    upload(512, 0, 160, 80, player_logo);
}

void graphics_logo(void)
{
    gpu(0xe1000108);
    gpu(0x65808080);
    gpu(xy(12, 4));
    gpu(0);
    gpu(xy(160, 80));
}

void graphics_shade(u32 x, u32 y, u32 width, u32 height)
{
    gpu(0xe1000200);
    gpu(0x62000000); gpu(xy(x, y)); gpu(xy(width, height));
}

void graphics_begin_frame(u32 frames)
{
    draw_y = draw_y == 0 ? 240 : 0;
    gpu(0xe3000000 | (draw_y << 10));
    gpu(0xe4000000 | 319 | ((draw_y + 239) << 10));
    gpu(0xe5000000 | (draw_y << 11));
    background(frames);
}

void graphics_present(void)
{
    while ((WORD(0x1f801814) & 0x04000000) == 0) {}
    WORD(0x1f801814) = 0x05000000 | (draw_y << 10);
}
