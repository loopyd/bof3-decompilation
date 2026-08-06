#include "internal.h"

/* Main-RAM globals owned by the loaded image. */
WEAK_SYMBOL_AT(D_80143BB0, 0x80143bb0);
WEAK_SYMBOL_AT(sisyou_master_index, 0x801448ed);

/* EMI-local data. */
WEAK_SYMBOL_AT(sisyou_master_action_base_table, 0x801d41bc);
WEAK_SYMBOL_AT(sisyou_mode_index, 0x801d4285);

/* Main-exe functions called by this overlay. */
WEAK_SYMBOL_AT(func_80150224, 0x80150224);

/* EMI-local functions. */
WEAK_SYMBOL_AT(func_801D10AC, 0x801d10ac);
