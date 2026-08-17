# Module map

## Executables

| Target | Load address | Role |
| --- | --- | --- |
| `exe/slus_004_22` | `0x80096800` | Main executable: boot, EMI loader, engine services |
| `exe/logo` | `0x801CE000` | Logo sequence player (independently loaded) |

## EMI code targets

| Target | Load address | Role |
| --- | --- | --- |
| `emi/etc/game/00` | `0x80195800` | Game frontend: title, save, status menus |
| `emi/etc/game/01` | `0x801D0C00` | Game frontend: item, equip, config menus |
| `emi/etc/shop/00` | `0x801D0C00` | Shop program |
| `emi/etc/commu00/00` | `0x801EEC00` | Fairy communication minigame |
| `emi/etc/bate/03` | `0x80033A00` | Battle-event content |
| `emi/etc/sisyou/00` | `0x801D0C00` | Master/skill menu |
| `emi/battle/battle/03` | `0x801D0C00` | Main battle program |
| `emi/battle/battle/15` | `0x80096800` | Battle program (alternate slot) |
| `emi/battle/batl_re2/01` | `0x80036E00` | Battle replay/result screen |
| `emi/scenario/scena00/00` | `0x801F6C00` | Scenario controller |
| `emi/scenario/scena16/00` | `0x801F6C00` | Scenario controller (late-game) |
| `emi/scenario/sce10eff/00` | `0x801D0C00` | Scenario effect controller |
| `emi/world00/area008/13` | `0x801F2C00` | World area code |
| `emi/world00/area016/13` | `0x801F2C00` | World area code |
| `emi/world00/area024/14` | `0x801F2C00` | World area code |
| `emi/world00/area026/13` | `0x801F2C00` | World area code |
| `emi/world00/area027/13` | `0x801F2C00` | World area code |
| `emi/world00/area028/13` | `0x801F2C00` | World area code |
| `emi/world00/area030/04` | `0x801D0C00` | World area code |
| `emi/world00/area030/05` | `0x800F5000` | World area code |
| `emi/world00/area032/13` | `0x801F2C00` | World area code |

Load addresses are from reviewed target manifests
(`config/targets/<target>/target.toml`). Overlapping addresses across
independently loaded targets are normal; they do not share runtime memory.
