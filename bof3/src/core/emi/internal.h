#ifndef BOF3_SRC_CORE_EMI_INTERNAL_H
#define BOF3_SRC_CORE_EMI_INTERNAL_H

#include "bof3/bof3.h"

#define EMI_SECTOR_SIZE  0x800u
#define EMI_MAGIC_OFFSET 0x08u
#define EMI_MAGIC_SIZE   8u

typedef struct EmiTocEntry {
  u32 size;
  u32 load_arg;
  u32 first_word;
  u16 type;
  u16 unk;
} EmiTocEntry;

typedef struct EmiActiveEntry {
  u32  active_lba;
  u32  remaining_size;
  u32  load_arg;
  u32  first_word;
  u16  type;
  bool header_mode;
} EmiActiveEntry;

bool emi_header_is_valid(const void* header, size_t size);
u32  emi_next_payload_offset(u32 current_offset, u32 current_size);
void emi_build_entry_lbas(u32 base_lba, const EmiTocEntry* entries,
                          size_t entry_count, u32* entry_lbas);
void emi_stream_init_slot(u32 slot_id);
u32  emi_slot_to_lba(const u32* slot_lba_table, size_t slot_count, u32 slot_id);
void func_8016728c(u8 index, u8 family);

#endif
