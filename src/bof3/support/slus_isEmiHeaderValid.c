#include <string.h>

#include "bof3/core/slus_internal.h"

static const unsigned char EMI_MAGIC[EMI_MAGIC_SIZE] = {
    'M', 'A', 'T', 'H', '_', 'T', 'B', 'L',
};

/* @behavior validates the fixed EMI header magic within a bounded buffer. */
bool isEmiHeaderValid(const void* header, size_t size) {
  const unsigned char* bytes = (const unsigned char*)header;

  if (header == NULL || size < (EMI_MAGIC_OFFSET + EMI_MAGIC_SIZE)) {
    return false;
  }

  return memcmp(bytes + EMI_MAGIC_OFFSET, EMI_MAGIC, EMI_MAGIC_SIZE) == 0;
}
