# Reviewed GAME.EMI#0 callback and storage tables. Runtime addresses are
# target-specific; semantic callback roles remain address-based until proven.
f data.DAT_801c7b08 0xc @ 0x801c7b08
f data.DAT_801c7b14 0x30 @ 0x801c7b14
f data.DAT_801c7b44 0xc @ 0x801c7b44
f data.DAT_801c7b54 0x20 @ 0x801c7b54
f data.DAT_801c7b7c 0xc @ 0x801c7b7c
f data.DAT_801c7b88 0x10 @ 0x801c7b88
f data.DAT_801c7b98 0xc @ 0x801c7b98
f data.DAT_801c7ba4 0xc @ 0x801c7ba4
f data.DAT_801c7bb0 0xc @ 0x801c7bb0
CC "Reviewed 3-entry callback table dispatched by func_80196f78" @ 0x801c7b08
CC "Reviewed 12-entry callback table dispatched by func_80197068" @ 0x801c7b14
CC "Reviewed 3-entry callback table dispatched by func_801975e4" @ 0x801c7b44
CC "Reviewed 8-entry callback table dispatched by func_80197a24" @ 0x801c7b54
CC "Reviewed 3-entry callback table dispatched by func_80198234" @ 0x801c7b7c
CC "Reviewed 4-entry callback table dispatched by func_801984ac" @ 0x801c7b88
CC "Reviewed 3-entry callback table dispatched by func_80198744" @ 0x801c7b98
CC "Reviewed 3-entry callback table dispatched by func_80198904" @ 0x801c7ba4
CC "Reviewed 3-entry callback table dispatched by func_80198ac4" @ 0x801c7bb0

# Reviewed nested callback family dispatched through state bytes +2 and +3 of
# the work record loaded from 0x80158648.
f data.DAT_801c7bec 0x14 @ 0x801c7bec
f data.DAT_801c7c00 0xc @ 0x801c7c00
f data.DAT_801c7c0c 0xc @ 0x801c7c0c
f data.DAT_801c7c18 0xc @ 0x801c7c18
f data.DAT_801c7c24 0x10 @ 0x801c7c24
CC "Reviewed 5-entry callback table dispatched by func_80199558" @ 0x801c7bec
CC "Reviewed 3-entry callback table dispatched by func_801995a4" @ 0x801c7c00
CC "Reviewed 3-entry callback table dispatched by func_801996a8" @ 0x801c7c0c
CC "Reviewed 3-entry callback table dispatched by func_8019977c" @ 0x801c7c18
CC "Reviewed 4-entry callback table dispatched by func_8019986c" @ 0x801c7c24

f data.ItemObject 0x678 @ 0x801c8964
f data.KeyItemObject 0x100 @ 0x801c8fdc
f data.WeaponObject 0x7c8 @ 0x801c90dc
f data.ArmorObject 0x5d8 @ 0x801c98a4
f data.AccessoryObject 0x410 @ 0x801c9e7c
f data.ShopObject 0x398 @ 0x801ca28c
f data.AbilityObject 0x11d0 @ 0x801ca70c
f data.LevelObject 0x15a8 @ 0x801cb8dc
tl ItemObject = 0x801c8964
tl KeyItemObject = 0x801c8fdc
tl WeaponObject = 0x801c90dc
tl ArmorObject = 0x801c98a4
tl AccessoryObject = 0x801c9e7c
tl ShopObject = 0x801ca28c
tl AbilityObject = 0x801ca70c
tl LevelObject = 0x801cb8dc
