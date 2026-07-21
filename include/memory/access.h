#ifndef MEMORY_ACCESS_H
#define MEMORY_ACCESS_H

#include "base/types.h"

/*
 * Address conversion
 * ------------------
 *
 * Convert an address to a typed pointer.
 * No memory access occurs.
 *
 * Add const and volatile directly to the type:
 *
 *     PSX_PTR(u32, address)
 *     PSX_PTR(const u32, address)
 *     PSX_PTR(volatile u32, address)
 *     PSX_PTR(const volatile u32, address)
 */
#define PSX_PTR(type, address) ((type*)(address))

/*
 * Direct object access
 * --------------------
 *
 * Produce an lvalue for an object stored directly at an address.
 *
 *     PSX_REF(u32, address) = value;
 *     value = PSX_REF(volatile u16, address);
 */
#define PSX_REF(type, address) (*PSX_PTR(type, address))

/*
 * Byte-offset field access
 * ------------------------
 *
 * Use while a structure is incomplete or its fields are not yet named.
 * Replace with real struct members once the layout is understood.
 *
 *     FIELD_ADDR(volatile u16, base, 0x80u)
 *     value = FIELD_REF(u32, work, 0x18u)
 */
#define FIELD_ADDR(type, base, byte_offset)                                    \
  PSX_PTR(type, (u8*)(base) + (u32)(byte_offset))

#define FIELD_REF(type, base, byte_offset)                                     \
  (*FIELD_ADDR(type, base, byte_offset))

/*
 * Fixed-address function pointer.
 *
 * Always use a function-pointer typedef as function_type:
 *
 *     typedef void (*Handler)(void);
 *     FUNCTION_AT(Handler, 0x80123456u)();
 */
#define FUNCTION_AT(function_type, address) ((function_type)(address))

#endif
