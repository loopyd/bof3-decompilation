#ifndef BATTLE_RAM_H
#define BATTLE_RAM_H

#include "base/types.h"
#include "memory/access.h"

/*
 * Shared battle RAM layout evidenced in both battle/03 and battle/15.
 * Per-target internal.h files keep their own target-specific accessors;
 * these names describe addresses common to both targets.
 *
 * Strides are struct sizes (hex). Addresses are fixed RAM (hex).
 * See docs/specs/runtime/memory-layouts.md for the documented layout.
 */

enum {
  BTL_LOCAL_WORK_STRIDE = 0x140,
  BTL_ENEMY_WORK_STRIDE = 0x118,
  BTL_QUEUED_SLOT_STRIDE = 0x78,
  BTL_RING_RECORD_STRIDE = 0x08,
  BTL_KIND_RECORD_STRIDE = 0x14,
};

#define BTL_LOCAL_WORK_BASE  0x80145E90u
#define BTL_ENEMY_WORK_BASE  0x801EB630u
#define BTL_QUEUED_SLOT_BASE 0x801EC330u

#define BtlLocalWork(i)                                                        \
  PSX_PTR(volatile u8, BTL_LOCAL_WORK_BASE + (u32)(i) * BTL_LOCAL_WORK_STRIDE)
#define BtlEnemyWork(i)                                                        \
  PSX_PTR(volatile u8, BTL_ENEMY_WORK_BASE + (u32)(i) * BTL_ENEMY_WORK_STRIDE)
#define BtlQueuedSlot(i)                                                       \
  PSX_PTR(volatile u8, BTL_QUEUED_SLOT_BASE + (u32)(i) * BTL_QUEUED_SLOT_STRIDE)

#define g_BtlCurrentBattler PSX_REF(volatile u8*, 0x801EB4E8u)
#define g_BtlMessageSlot    PSX_REF(volatile u8*, 0x801EBF08u)

#define BtlRingFlag(i)                                                         \
  PSX_REF(volatile u8, 0x801EB5B1u + (u32)(i) * BTL_RING_RECORD_STRIDE)
#define BtlRingHandle(i)                                                       \
  PSX_REF(volatile u32, 0x801EB5B4u + (u32)(i) * BTL_RING_RECORD_STRIDE)

#define BtlKindFlags(kind)                                                     \
  PSX_REF(volatile u8, 0x801CA718u + (u32)(kind) * BTL_KIND_RECORD_STRIDE)

#define g_BtlActiveCount PSX_REF(volatile u8, 0x801462F0u)
#define g_BtlState       PSX_REF(volatile u8, 0x801462E5u)

#endif
