"""The uniform, answer-free C89 typedef prelude admitted as an explicit input.

GCC 2.7.2 ships no ``stdint.h``, and the design permits a uniform generated C89
typedef prelude when it carries no function-specific answers.  These bytes are
the whole prelude: the C89-representable exact-width and pointer typedefs only,
no declaration for any function, no macro that encodes a recovered fact and no
target constant.  It is staged as an ordinary explicit input with a fixed
relative name and bound hash, not as a mounted SDK include tree.

The prelude declares no 64-bit type: ``long long`` is a GCC extension, not C89,
so a 64-bit candidate requirement is reported as unsupported until a separately
reviewed representation exists rather than being labelled C89.
"""

from __future__ import annotations

PRELUDE_PATH = "include/stdint.h"
PRELUDE_ROLE = "prelude"
STDINT_PRELUDE = b"""\
#ifndef RETDEC118_STDINT_H
#define RETDEC118_STDINT_H 1

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef int intptr_t;
typedef unsigned int uintptr_t;

#endif
"""


def stdint_prelude(path: str = PRELUDE_PATH) -> dict:
    """The uniform prelude as one explicit named input."""

    return {"name": path, "role": PRELUDE_ROLE, "data": STDINT_PRELUDE}
