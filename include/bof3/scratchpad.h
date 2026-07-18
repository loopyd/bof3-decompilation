#ifndef BOF3_SCRATCHPAD_H
#define BOF3_SCRATCHPAD_H

/*
 * PS1 scratchpad (0x1F800000–0x1F8003FF) access helpers.
 *
 * The scratchpad is 1 KB of on-chip RAM, NOT hardware I/O.
 * Use these macros instead of raw address casts.
 *
 * Reference: https://psx-spx.consoledev.net/memorymap/
 */

#define PSX_SPAD_BASE 0x1F800000u

/* Address of an object stored directly in scratchpad. */
#define SPAD_ADDR(type, byte_offset) \
    ((type *)(PSX_SPAD_BASE + (byte_offset)))

/* Same, with volatile access to the object. */
#define SPAD_VADDR(type, byte_offset) \
    ((volatile type *)(PSX_SPAD_BASE + (byte_offset)))

/*
 * Load a pointer stored in a scratchpad slot.
 *
 * The pointer cell itself is volatile, so every evaluation reloads it.
 * The pointed-to object is not volatile.
 */
#define SPAD_PTR_SLOT(type, byte_offset) \
    (*(type * volatile *)(PSX_SPAD_BASE + (byte_offset)))

/* Both the pointer cell and pointed-to object are volatile. */
#define SPAD_VPTR_SLOT(type, byte_offset) \
    (*(volatile type * volatile *)(PSX_SPAD_BASE + (byte_offset)))

#endif
