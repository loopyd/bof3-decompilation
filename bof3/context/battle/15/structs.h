#ifndef BOF3_CONTEXT_15_STRUCTS_H
#define BOF3_CONTEXT_15_STRUCTS_H

/* struct, typedef, and type definitions */

typedef void (*BattleSelectionHandler)(void);
typedef struct BattleLocalPanelEntry {
  u8  owner_index;
  u8  unk_01;
  u16 panel_id;
} BattleLocalPanelEntry;
#endif
