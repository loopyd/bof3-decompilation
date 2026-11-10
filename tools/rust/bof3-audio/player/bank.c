#include "bank.h"
#include "audio.h"

void bank_copy(u8 *destination, const u8 *source, u32 bytes)
{
    while (bytes-- != 0) *destination++ = *source++;
}

void bank_fail(int error)
{
    player_status.error = error;
    player_status.phase = 255;
    for (;;) CALL(0x80174700, 0, 0, 0, 0);
}

void bank_load(const struct BankAsset *asset)
{
    u32 offset, bytes;
    int result;
    CALL(0x80161808, asset->layout, 0, 0, 0);
    if (asset->spu_base + ((asset->body_bytes + 63) & ~63u) >
        (u32)HALF(0x1f801da2) * 8) bank_fail(-4);
    bank_copy((u8 *)asset->header_address, asset->header, asset->header_bytes);
    bank_copy((u8 *)asset->auxiliary_address, asset->auxiliary, asset->auxiliary_bytes);
    player_status.phase = 2;
    result = CALL(0x80173c50, asset->header_address, asset->bank, asset->spu_base, 0);
    if (result != (int)asset->bank) bank_fail(result);
    for (offset = 0; offset < asset->body_bytes; offset += bytes) {
        bytes = asset->body_bytes - offset;
        if (bytes > 2048) bytes = 2048;
        result = CALL(0x80174354, asset->body + offset, bytes, asset->bank, 0);
        if (result != (offset + bytes == asset->body_bytes ? (int)asset->bank : -2)) bank_fail(result);
        if (CALL(0x80174598, 1, 0, 0, 0) != 1) bank_fail(-3);
    }
    player_status.phase = 3;
}
