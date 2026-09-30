# Runtime missions

Contents: [purpose](#purpose), [preflight](#procedure), [invocation](#invocation),
[evidence contract](#evidence-contract), [discovery/extensions](#discovery).

## Purpose

Run bounded Lua [actions](../SKILL.md) through [runtime.sh](../scripts/runtime.sh).
The harness owns staging, cleanup and receipts; the requesting project owns
semantic acceptance. Inspect existing states before rerunning or writing a parser.

## Procedure

Verify `bin/harness runtime status`; preserve dirty work and failed evidence.
Follow [Native](native.md) for build customizations and dependency authorization.

<a id="invocation"></a>

Declare modules explicitly: except smoke, actions need `support`; state/device
inspection and restoration need `snapshot`; [native writes](bus.md) need `bus`.
The selected procedure lists additional modules.

```sh
sh .pi/skills/psx-emulator/scripts/runtime.sh run \
  --bios /path/to/rom.bin \
  --script .pi/skills/psx-emulator/scripts/state.lua \
  --module support=.pi/skills/psx-emulator/scripts/support.lua \
  --module snapshot=.pi/skills/psx-emulator/scripts/snapshot.lua \
  --input state=/path/to/state.pbuf --argument path=registers.pc \
  --output out/state-query --timeout 15
```

Use fresh output under an existing parent and a 512 KiB BIOS; its hash does not
infer a model. Acquisition/doctor policies are separate. `--executable` loads a
PS-X EXE through shell handoff; `--disc` mounts [CUE/BINARY media](cdrom.md).
GUI gzip conversion is unsupported.

For provenance-bound continuation, including [Replay](replay.md), supply
`--origin receipt.json` and `--input state=PATH`, without an EXE. Verification
requires a successful origin, exact state size/hash and matching emulator, SDL,
bootstrap BIOS and declared disc identities. The harness hashes the receipt and
sets `PSX_RUNTIME_ORIGIN_VALIDATED=1`. This is supplied provenance, not authenticated
proof of embedded ROM, hidden host state or original effective settings.

| Input | Contract |
| --- | --- |
| `--argument NAME=VALUE` | Literal strings; ASCII letter then letters/digits/underscores; case-insensitive unique names; 64 maximum, 4096 UTF-8 bytes/value, no NUL |
| `--module NAME=PATH` | Lower-case Lua name, staged at `lua/NAME.lua`; maximum 1 MiB/module |
| `--input NAME=PATH` | Staged at `inputs/NAME`; maximum 64 MiB/input |
| Assets | 64 modules/inputs combined, 128 MiB total; original and staged hashes checked before/after execution |

Environment: `PSX_RUNTIME_ARG_NAME`, `PSX_RUNTIME_ARGUMENT_NAMES` (validation),
`PSX_RUNTIME_INPUT_NAME` and decimal EXE entry `PSX_RUNTIME_ENTRY`. Inherited
`PSX_RUNTIME_*` and Lua paths are cleared; only staged/bundled modules are searched.
Arguments enter receipts: never include credentials. Numbers accept decimal/`0x`;
supported `savestate=0|1` defaults to 0.

Maintained actions load `support` before native calls. It disables host Lua JIT
compilation, clears existing traces and rejects re-enabled mission entry; keep it
disabled throughout the mission. This controls native callback reentry and is
separate from guest CPU mode. See [API](api.md) for the calling contract.

For native media association, the harness supplies `PSX_RUNTIME_DIRECTORY` (the
resolved output directory) and `PSX_RUNTIME_MEDIA`: ordered tab-separated
`disc:trackNN.bin`, decimal byte count, lowercase SHA-256 rows, each newline-ended.
No disc means an explicitly empty value. [mount.lua](../scripts/mount.lua) validates
these literal identities and derives exact staged paths; it never executes data
or matches basenames. Final passed-receipt/input checks remain authoritative.

Launch uses interpreter, software GPU, debugger and fresh portable settings;
GDB/web, PCdrv, fastboot and updates are disabled. Timeout: 60 seconds by default,
maximum 3600; default log limit: 2 MiB. `--work-deadline` is an absolute monotonic
cutoff, never renewed. Failure, timeout and overflow trigger cleanup.

## Application

<a id="evidence-contract"></a>

Review Lua's unsandboxed host access. Write captures under the fresh working
directory. [support.lua](../scripts/support.lua) retains callbacks, guards errors/
arguments and writes `completion.json` last before quitting:

```json
{"schema":"psx.runtime-completion/v1","captures":[{"path":"ram.bin","bytes":2097152}]}
```

Missing/malformed markers, duplicate/reserved names, path escapes, symlinks and
size mismatches reject. Limits: 128 captures, 64 MiB/file, 128 MiB total.
`psx.runtime-session/v1` retains input/tool/capture hashes, staging, arguments,
outcome and timing, including failed receipts/logs. Endpoint hashes neither
provide atomic snapshots nor prove intermediate immutability.

`psx.runtime-settings/v1` records launch flags/defaults, not effective settings.
Lua changes, host audio, physical input and scheduling remain uncontrolled.
Distinguish pre-instruction observations, retired steps, host edits and cycles;
qualify executable/overlay and [RAM alias](../../psx-rizin/references/psx-abi-and-addressing.md).
Transport success establishes neither hardware accuracy nor semantic acceptance.
Keep proprietary inputs/captures outside skill assets.

<a id="discovery"></a>

Maintain one tree at `.pi/skills/psx-emulator`, shared through relative discovery
symlinks under `.agents/skills` and `.codex/skills`, as for `psx-rizin`.
Follow [API](api.md) for extensions, declaring inputs, observations, stop and exclusions.
