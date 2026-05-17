#ifndef BOF3_CONTEXT_15_SYMBOLS_H
#define BOF3_CONTEXT_15_SYMBOLS_H

/* address and table pointer definitions */

#define BOF3_BATTLE_SELECTION_SLOT_SUBSTATE_TABLE ((BattleSelectionHandler const volatile*)0x800b43c0u)
#define BOF3_BATTLE_SELECTION_CONFIRM_SUBSTATE_TABLE ((BattleSelectionHandler const volatile*)0x800b43ecu)
#define BOF3_BATTLE_SELECTION_RESULT_SUBSTATE_TABLE ((BattleSelectionHandler const volatile*)0x800b43f4u)
#define BOF3_BATTLE_SELECTION_FINALIZE_SUBSTATE_TABLE ((BattleSelectionHandler const volatile*)0x800b4408u)
#define BOF3_BATTLE_SELECTION_SECONDARY_SUBSTATE_TABLE ((BattleSelectionHandler const volatile*)0x800b4450u)
#define BOF3_BATTLE_LOCAL_PANEL_RULE(class_id, slot_index) ((volatile u8*)(0x800e407cu + ((u32)(class_id) * 0x88u) + ((u32)(slot_index) * 0x10u)))
#endif
