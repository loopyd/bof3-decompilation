# PCSX-Redux harness

This guide owns BOF3's integration with the Linux PCSX-Redux Agentic fork. The
[fork README](../../third_party/pcsx-redux-agentic/README.md) covers the emulator;
the [psx-emulator skill](../../.pi/skills/psx-emulator/SKILL.md) owns mission
selection, arguments, capture interpretation, and native API procedures.

## Build and discover

The `pcsx-redux` component builds the registered
`third_party/pcsx-redux-agentic` submodule with the approved SDL3 prefix. The root
stage-0 gitlink, fork HEAD, and recursive submodule revisions must agree; tracked
sources must match their committed blobs. Setup records the binary, source
revisions, generated inputs, SDL linkage, embedded resources, and native state
schema. Dependency installation requires specific authorization.

```sh
bin/harness setup --component pcsx-redux
bin/harness runtime status
bin/harness runtime missions
```

The verified catalogue contains 19 entry missions and 35 shared modules. Canonical
Lua is embedded in the emulator; no loose canonical files are required at runtime.
Both builtin and trusted external-script routes preload those modules. External
modules cannot shadow their names.

## Run a mission

Create the output parent and use a fresh output directory. Supply your own licensed
BIOS, executables, and disc images; keep proprietary inputs and captures local.

```sh
mkdir -p out/runtime
bin/harness runtime run --bios inputs/external/bios/scph5501.bin --mission smoke --output out/runtime/smoke --timeout 30
```

Select exactly one of `--mission NAME` and `--script TRUSTED_LUA`. The external
route accepts noncanonical `--module NAME=PATH` inputs. Lua runs as trusted host
code, without a sandbox. The harness stages inputs and fresh settings, bounds the
process, cleans it up, and validates captures and completion before issuing a
passed receipt. See [runtime procedures](../../.pi/skills/psx-emulator/references/runtime.md)
for arguments, origins, state continuation, limits, and evidence requirements.

## Skill installation

The skill lives in the fork's `.agents/skills/psx-emulator`. The manager links it
at this project's `.pi/skills/psx-emulator` and maintains existing `.agents` and
`.codex` aliases without replacing sibling skills:

```sh
third_party/pcsx-redux-agentic/scripts/manage-psx-emulator-skill.sh install .
third_party/pcsx-redux-agentic/scripts/manage-psx-emulator-skill.sh check .
```

The manager refuses unowned directories and unrelated links. It installs no
dependencies and changes no home hooks. Its `remove` operation removes only the
expected skill links.

## Evidence limits

Receipts bind supplied inputs, the managed binary, limits, and captures. They do
not establish hardware fidelity or deterministic replay. Report observed behavior
and unresolved limits in the requesting mission's evidence. Native conversion
measurements cover their stated caller operations; they do not establish a
whole-mission or emulator speedup.

Build ownership lives in
[toolchain/pcsx.py](../../tools/python/harness/toolchain/pcsx.py); runtime staging,
launch, completion, and receipt ownership live in
[runtime/](../../tools/python/harness/runtime/).
