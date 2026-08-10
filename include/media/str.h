#ifndef MEDIA_STR_H
#define MEDIA_STR_H

#include "base/types.h"

/* Extracted STR/XA sector layout (docs/specs/formats/str-xa.md):
 * [XA subheader 8][payload 2324][EDC 4] = 2336 bytes.
 * Raw CD sectors add a 16-byte outer header for 2352 bytes. */
enum {
  STR_SECTOR_SIZE = 2336,
  STR_RAW_SECTOR_SIZE = 2352,
  STR_RAW_HEADER_SIZE = 16,
  STR_XA_SUB_SIZE = 8,
  STR_PAYLOAD_SIZE = 2324,
  STR_EDC_SIZE = 4,
};

#endif
