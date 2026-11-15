#ifndef MEMORY_SCRATCHPAD_H
#define MEMORY_SCRATCHPAD_H

#include "memory/access.h"

/*
 * PS1 scratchpad RAM.
 *
 * Address range:
 *     0x1F800000-0x1F8003FF
 *
 * All offsets are byte offsets.
 */
#define SPAD_BASE 0x1F800000u
#define SPAD_SIZE 0x00000400u

/*
 * Return the absolute address of a scratchpad byte offset.
 */
#define SPAD_ADDRESS(byte_offset) (SPAD_BASE + (u32)(byte_offset))

/*
 * Typed pointer into scratchpad.
 *
 * No memory read occurs.
 *
 *     SPAD_ADDR(u8, 0x20u)
 *     SPAD_ADDR(volatile u32, 0x40u)
 */
#define SPAD_ADDR(type, byte_offset) PSX_PTR(type, SPAD_ADDRESS(byte_offset))

/*
 * Lvalue for an object stored directly in scratchpad.
 *
 *     SPAD_REF(u32, 0x20u) = value;
 *     value = SPAD_REF(volatile u16, 0x40u);
 */
#define SPAD_REF(type, byte_offset) PSX_REF(type, SPAD_ADDRESS(byte_offset))

/*
 * Typed base of scratchpad pointer cells. Index by four-byte cell number.
 * Keeping the base at zero lets the compiler use an offset load rather than
 * materialize a full cell address.
 *
 *     SPAD_PTR_TABLE(Entity)[0x11]
 *         Entity *
 */
#define SPAD_PTR_TABLE(type) SPAD_ADDR(type*, 0u)

/*
 * Lvalue for a pointer stored in a scratchpad cell.
 *
 * The pointer cell is unqualified. Qualifiers in type apply to the pointee.
 * When a volatile cell is required, use
 * PSX_REF(Entity* volatile, SPAD_ADDRESS(0x44u)).
 *
 *     SPAD_PTR_SLOT(Entity, 0x44u)
 *         Entity* lvalue
 *
 *     SPAD_PTR_SLOT(volatile Entity, 0x44u)
 *         volatile Entity* lvalue
 */
#define SPAD_PTR_SLOT(type, byte_offset)                                       \
  PSX_REF(type*, SPAD_ADDRESS(byte_offset))


#endif
