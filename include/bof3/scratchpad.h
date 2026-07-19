#ifndef BOF3_SCRATCHPAD_H
#define BOF3_SCRATCHPAD_H

#include "bof3/memory.h"

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
 * Pointer stored in a scratchpad slot.
 *
 * The cell is intentionally NOT marked volatile: the constant-address codegen
 * (lui + offset load, e.g. `lw v1,68(v1)` for 0x1F800044) matches the original
 * binary. Marking the cell `volatile` forces `lui + ori + lw 0`, which diverges.
 * To force a reload on every evaluation, qualify the type and/or use PSX_REF
 * directly.
 *
 *     SPAD_PTR_SLOT(Entity, 0x44u)
 *         Entity **
 *
 *     SPAD_PTR_SLOT(volatile Entity, 0x44u)
 *         volatile Entity **
 */
#define SPAD_PTR_SLOT(type, byte_offset) \
  PSX_REF(type*, SPAD_ADDRESS(byte_offset))

#endif
