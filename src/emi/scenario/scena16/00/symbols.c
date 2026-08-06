#include "internal.h"

/* SLUS addresses called by this independently loaded overlay. */
WEAK_SYMBOL_AT(game_stop_selection_fx, 0x8015d404);
WEAK_SYMBOL_AT(game_queue_frontend_cue, 0x8015df18);

/* Main-RAM globals owned by the loaded image. */
WEAK_SYMBOL_AT(SCENA16_D_80145EC4, 0x80145ec4);
WEAK_SYMBOL_AT(SCENA16_D_80145EC8, 0x80145ec8);
WEAK_SYMBOL_AT(SCENA16_D_80149308, 0x80149308);
WEAK_SYMBOL_AT(SCENA16_D_8014832E, 0x8014832e);
WEAK_SYMBOL_AT(scena16_primary_stateTable, 0x801f854c);
WEAK_SYMBOL_AT(scena16_secondary_stateTable, 0x801f8558);
WEAK_SYMBOL_AT(scena16_record_callbackTable, 0x801f856c);
WEAK_SYMBOL_AT(SCENA16_D_80181EBA, 0x80181eba);
