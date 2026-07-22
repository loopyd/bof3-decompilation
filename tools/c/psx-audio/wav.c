#include "audio.h"
#include "util.h"

static int wav_write_header(FILE *f, int channels, int64_t frames, int rate) {
    uint32_t data_size = (uint32_t)(frames * channels * 2);
    uint32_t byte_rate = (uint32_t)(rate * channels * 2);
    uint16_t block_align = (uint16_t)(channels * 2);

    fwrite("RIFF", 1, 4, f);
    wr_u32le(f, 36 + data_size);
    fwrite("WAVE", 1, 4, f);

    fwrite("fmt ", 1, 4, f);
    wr_u32le(f, 16);
    wr_u16le(f, 1);
    wr_u16le(f, (uint16_t)channels);
    wr_u32le(f, (uint32_t)rate);
    wr_u32le(f, byte_rate);
    wr_u16le(f, block_align);
    wr_u16le(f, 16);

    fwrite("data", 1, 4, f);
    wr_u32le(f, data_size);
    return 0;
}

int wav_write_mono(const char *path, const int16_t *pcm, int64_t count, int rate) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    wav_write_header(f, 1, count, rate);
    fwrite(pcm, sizeof(int16_t), (size_t)count, f);
    fclose(f);
    return 0;
}

int wav_write_stereo(const char *path, const int16_t *pcm, int64_t frames, int rate) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    wav_write_header(f, 2, frames, rate);
    fwrite(pcm, sizeof(int16_t) * 2, (size_t)frames, f);
    fclose(f);
    return 0;
}
