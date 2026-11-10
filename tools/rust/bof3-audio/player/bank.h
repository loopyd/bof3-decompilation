#ifndef PLAYER_BANK_H
#define PLAYER_BANK_H

#include "psx.h"

struct BankAsset {
    u32 layout, bank, spu_base, header_address, header_bytes, body_bytes;
    u32 auxiliary_address;
    const u8 *header, *body, *auxiliary;
    u32 auxiliary_bytes;
};

void bank_load(const struct BankAsset *asset);
void bank_copy(u8 *destination, const u8 *source, u32 bytes);
void bank_fail(int error);

#endif
