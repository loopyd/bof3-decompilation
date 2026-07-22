#ifndef PSX_AUDIO_PSF_H
#define PSX_AUDIO_PSF_H

#include <stddef.h>
#include <stdint.h>

#define PSF1_RAM_SIZE 0x200000u

typedef enum {
    PSF1_OK = 0,
    PSF1_ERROR_ARGUMENT,
    PSF1_ERROR_IO,
    PSF1_ERROR_FORMAT,
    PSF1_ERROR_CRC,
    PSF1_ERROR_COMPRESSION,
    PSF1_ERROR_RANGE,
    PSF1_ERROR_RECURSION,
    PSF1_ERROR_MEMORY
} Psf1Status;

typedef struct {
    uint8_t *ram;
    uint32_t initial_pc;
    uint32_t initial_sp;
    uint32_t loaded_min;
    uint32_t loaded_max;
    int refresh_rate;
} Psf1Image;

Psf1Status psf1_load_file(const char *path, Psf1Image *image);
Psf1Status psf1_write_file(const char *path, const uint8_t *exe,
                           size_t exe_size, const char *tags);
void psf1_image_free(Psf1Image *image);
const char *psf1_status_string(Psf1Status status);

#endif
