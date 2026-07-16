#include "internal.h"

/* @behavior clears the key state bytes across all 0x30 queued-slot entries.
 * @source 0x801E5A38
 */
void func_801E5A38(void) {
  u8 index;

  index = 0u;
  do {
    u32 offset;

    offset = (u32)index * 0x78u;
    index += 1u;
    *(volatile u8*)(0x801ec330u + offset) = 0u;
    *(volatile u8*)(0x801ec335u + offset) = 0u;
    *(volatile u8*)(0x801ec336u + offset) = 0u;
    *(volatile u8*)(0x801ec331u + offset) = 0u;
    *(volatile u8*)(0x801ec332u + offset) = 0u;
    *(volatile u8*)(0x801ec333u + offset) = 0u;
    *(volatile u8*)(0x801ec334u + offset) = 0u;
    *(volatile u8*)(0x801ec378u + offset) = 0u;
    *(volatile u8*)(0x801ec38du + offset) = 0u;
    *(volatile u8*)(0x801ec38eu + offset) = 0u;
    *(volatile u8*)(0x801ec38fu + offset) = 0u;
  } while (index < 0x30u);
}
