# Runtime specs

Executable roles, loading behavior, and reviewed overlay evidence. Start with
the model pages, then follow the matching flow or module case study.

## Model

* [Runtime layout](runtime-layout.md) - Executable boundaries and loading model.
* [Module map](module-map.md) - Conservatively documented code-bearing modules.
* [EMI loader](emi-loader.md) - Main-executable streaming and type dispatch.
* [Asset loading](asset-loading.md) - How TOC entries become live assets.
* [Overlay duplication](overlay-duplication.md) - Duplicate-payload handling constraint.

## Boot and front end

* [Boot sequence and state transitions](boot-sequence.md) - Main boot flow.
* [Logo boot path](logo-boot.md) - Secondary `LOGO.EXE` handoff.
* [FIRST frontend pack](first-overlay.md) - Bootstrap before the game overlay.
* [GAME.EMI overlay](game-overlay.md) - Title and front-controller evidence.
* [Title assets](title-assets.md) - Asset packs beneath the title path.

## Subsystems and reviewed modules

* [Audio system](audio-system.md) - EMI audio-handler and PSYQ behavior.
* [EMI graphics pipeline](emi-graphics-pipeline.md) - Graphics extraction model.
* [STR playback](str-playback.md) - Stream-media constraints.
* [Representative battle overlay](battle-overlay.md) - `BATTLE.EMI#3` case study.
* [SCENA16 overlay](scena16-overlay.md) - Documented scenario-overlay handoff.

For on-disc layout, see [format specs](../formats/index.md). For archive-family
observations, see [content specs](../content/index.md).
