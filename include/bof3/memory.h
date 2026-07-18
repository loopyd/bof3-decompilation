#ifndef BOF3_MEMORY_H
#define BOF3_MEMORY_H

#include "bof3/defines.h"

/*
 * Address conversion
 * ------------------
 *
 * Convert an address to a typed pointer.
 * No memory access occurs.
 *
 * Add const and volatile directly to the type:
 *
 *     PTR_AT(u32, address)
 *     PTR_AT(const u32, address)
 *     PTR_AT(volatile u32, address)
 *     PTR_AT(const volatile u32, address)
 */
#define PTR_AT(type, address) ((type*)(address))

/*
 * Direct object access
 * --------------------
 *
 * Produce an lvalue for an object stored directly at an address.
 *
 *     OBJECT_AT(u32, address) = value;
 *     value = OBJECT_AT(volatile u16, address);
 */
#define OBJECT_AT(type, address) (*PTR_AT(type, address))

/*
 * Pointer-slot access
 * -------------------
 *
 * A pointer slot is a memory location containing a pointer.
 *
 * PTR_SLOT_AT:
 *     The pointer cell is an ordinary object.
 *
 * VOLATILE_PTR_SLOT_AT:
 *     The pointer cell is volatile and is reloaded on every evaluation.
 *
 * Target qualifiers are specified as part of type:
 *
 *     PTR_SLOT_AT(Entity, address)
 *         Entity **
 *
 *     PTR_SLOT_AT(volatile Entity, address)
 *         volatile Entity **
 *
 *     VOLATILE_PTR_SLOT_AT(Entity, address)
 *         Entity * volatile *
 *
 *     VOLATILE_PTR_SLOT_AT(volatile Entity, address)
 *         volatile Entity * volatile *
 */
#define PTR_SLOT_AT(type, address) OBJECT_AT(type*, address)

#define VOLATILE_PTR_SLOT_AT(type, address) OBJECT_AT(type* volatile, address)

/*
 * Byte-offset access
 * ------------------
 *
 * Useful while a structure is incomplete or its fields are not understood.
 */
#define BYTE_OFFSET(base, byte_offset) ((u8*)(base) + (u32)(byte_offset))

#define FIELD_PTR(type, base, byte_offset) \
  PTR_AT(type, BYTE_OFFSET(base, byte_offset))

#define FIELD_AT(type, base, byte_offset) \
  OBJECT_AT(type, BYTE_OFFSET(base, byte_offset))

/*
 * Fixed-address function conversion.
 *
 * Always use a function-pointer typedef as function_type.
 *
 *     typedef void (*Handler)(void);
 *     FUNCTION_AT(Handler, 0x80123456u)();
 */
#define FUNCTION_AT(function_type, address) ((function_type)(address))

/*
 * PS1 memory-mapped I/O
 * ---------------------
 *
 * These macros are for hardware registers, not scratchpad RAM.
 */
#define MMIO8(address) OBJECT_AT(volatile u8, address)

#define MMIO16(address) OBJECT_AT(volatile u16, address)

#define MMIO32(address) OBJECT_AT(volatile u32, address)

#endif
