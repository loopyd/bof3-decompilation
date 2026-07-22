#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psf.h"

static void write_le32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

static uint8_t *make_exe(uint32_t pc, uint32_t sp, uint32_t address,
                         const uint8_t *text, size_t text_size,
                         size_t *exe_size)
{
    uint8_t *exe = (uint8_t *)calloc(1, 0x800 + text_size);
    if (!exe)
        return NULL;
    memcpy(exe, "PS-X EXE", 8);
    write_le32(exe + 0x10, pc);
    write_le32(exe + 0x18, address);
    write_le32(exe + 0x1c, (uint32_t)text_size);
    write_le32(exe + 0x30, sp);
    memcpy(exe + 0x800, text, text_size);
    *exe_size = 0x800 + text_size;
    return exe;
}

int main(void)
{
    const uint8_t library_text[] = { 1, 2 };
    const uint8_t mini_text[] = { 9, 8 };
    uint8_t *library_exe;
    uint8_t *mini_exe;
    size_t library_size;
    size_t mini_size;
    Psf1Image image;
    Psf1Status status;
    int failed = 0;
    FILE *corrupt;

    library_exe = make_exe(0x80010000u, 0x801ffff0u, 0x80010000u,
                           library_text, sizeof(library_text), &library_size);
    mini_exe = make_exe(0x80020000u, 0x801efff0u, 0x80010001u,
                        mini_text, sizeof(mini_text), &mini_size);
    if (!library_exe || !mini_exe)
        return 1;
    if (psf1_write_file("test.psflib", library_exe, library_size,
                        "_refresh=60\n") != PSF1_OK ||
        psf1_write_file("test.minipsf", mini_exe, mini_size,
                        "_lib=test.psflib\n_refresh=50\n") != PSF1_OK)
        failed = 1;
    if (!failed &&
        psf1_write_file("corrupt.psf", library_exe, library_size, NULL) == PSF1_OK) {
        int byte;
        corrupt = fopen("corrupt.psf", "r+b");
        if (!corrupt || fseek(corrupt, 12, SEEK_SET) != 0 ||
            (byte = fgetc(corrupt)) == EOF || fseek(corrupt, 12, SEEK_SET) != 0 ||
            fputc(byte ^ 0xff, corrupt) == EOF) {
            failed = 1;
        }
        if (corrupt)
            fclose(corrupt);
        if (!failed && psf1_load_file("corrupt.psf", &image) != PSF1_ERROR_CRC)
            failed = 1;
    }
    if (!failed) {
        status = psf1_load_file("test.minipsf", &image);
        if (status != PSF1_OK) {
            fprintf(stderr, "%s\n", psf1_status_string(status));
            failed = 1;
        } else {
            failed = image.initial_pc != 0x80010000u ||
                     image.initial_sp != 0x801ffff0u ||
                     image.refresh_rate != 50 ||
                     image.loaded_min != 0x10000u ||
                     image.loaded_max != 0x10003u ||
                     image.ram[0x10000] != 1 ||
                     image.ram[0x10001] != 9 ||
                     image.ram[0x10002] != 8;
            psf1_image_free(&image);
        }
    }
    remove("test.psflib");
    remove("test.minipsf");
    remove("corrupt.psf");
    free(library_exe);
    free(mini_exe);
    return failed;
}
