#ifndef BOF3_SCRATCHPAD_H
#define BOF3_SCRATCHPAD_H

#include "bof3/memory.h"

/*
 * PS1 scratchpad RAM
 * ------------------
 *
 * Address range:
 *     0x1F800000-0x1F8003FF
 *
 * All offsets are byte offsets.
 */
#define PSX_SPAD_BASE 0x1F800000u
#define PSX_SPAD_SIZE 0x00000400u

#define SPAD_ADDRESS(byte_offset) (PSX_SPAD_BASE + (u32)(byte_offset))

/*
 * Pointer to an object stored directly in scratchpad.
 *
 * No memory read occurs.
 */
#define SPAD_PTR(type, byte_offset) PTR_AT(type, SPAD_ADDRESS(byte_offset))

/*
 * Lvalue for an object stored directly in scratchpad.
 */
#define SPAD_OBJECT(type, byte_offset) \
  OBJECT_AT(type, SPAD_ADDRESS(byte_offset))

/*
 * Pointer stored in a scratchpad slot.
 *
 * SPAD_PTR_SLOT:
 *     Ordinary pointer cell.
 *
 * SPAD_VOLATILE_PTR_SLOT:
 *     Volatile pointer cell, reloaded on every evaluation.
 *
 * Add volatile to type when the pointed-to object is also volatile.
 */
#define SPAD_PTR_SLOT(type, byte_offset) \
  PTR_SLOT_AT(type, SPAD_ADDRESS(byte_offset))

#define SPAD_VOLATILE_PTR_SLOT(type, byte_offset) \
  VOLATILE_PTR_SLOT_AT(type, SPAD_ADDRESS(byte_offset))

#endif
