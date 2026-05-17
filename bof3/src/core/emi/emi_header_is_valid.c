#include <string.h>

#include "internal.h"

static const unsigned char EMI_MAGIC[EMI_MAGIC_SIZE] = {
    'M', 'A', 'T', 'H', '_', 'T', 'B', 'L',
};

bool emi_header_is_valid(const void* header, size_t size) {
  const unsigned char* bytes = (const unsigned char*)header;

  if (header == NULL || size < (EMI_MAGIC_OFFSET + EMI_MAGIC_SIZE)) {
    return false;
  }

  return memcmp(bytes + EMI_MAGIC_OFFSET, EMI_MAGIC,
                EMI_MAGIC_SIZE) == 0;
}
