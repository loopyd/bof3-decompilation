# Saved-state inspection

## Purpose

Use [state.lua](../scripts/state.lua) with `support`, `snapshot` and
[Runtime](runtime.md#invocation) to query/export/compare state without loading it.
Decode with the running build's schema and bundled `pb`/`protoc`; never guess
offsets or substitute an external parser.

## Procedure

Choose action, schema path and bounds; inspect a parent/schema for unknown names.
Validate receipt, schema and input hashes before reusing bytes.

| Action | Inputs / arguments | Captures |
| --- | --- | --- |
| `schema` | No state | `state.proto` |
| `query` (default) | `--input state=PATH`; optional `path`, `depth` | `query.json` |
| `export` | State, bytes-field `path`; optional `offset`, `length` | `region.bin`, `region.json` |
| `compare` | State and `--input other=PATH`; optional subtree `path`, `depth`, `limit`; declare `difference` | `comparison.json`, `state.proto` |

Use version-4 raw API states from `savestate=1`; GUI gzip rejects, even renamed.
Equal versions do not establish build compatibility.

Paths use dotted names and **one-based** indices: `registers.pc`,
`registers.gpr[1]`, `spu.channel[1].adsr_ex`, `memory.ram`. Quote brackets in shell
arguments. Empty path selects root; unknown names/invalid indices fail.
Queries/exports reject absence; comparisons report it.

Query `depth=0..8` (1); zero lists message field/type/presence or array counts.
Beyond 4096 expanded nodes, narrow the query. Previews disclose truncation at
32 bytes; large integers retain `#decimal` strings.

Export `offset` is zero-based; `length` defaults to the remainder and must fit.
`memory.ram` is 8 MiB: use `length=2097152` to compare with `ram.bin`.
`spu.ram` is 512 KiB. Serialized regions imply neither bus aliases nor meaningful
guest memory throughout.

<a id="structured-comparison"></a>

Comparison (`psx.runtime-comparison/v2`) accepts any field or whole state, reporting
paths, one-based indices, zero-based contiguous byte ranges and ≤32 preview
bytes/side. Preview truncation preserves counts/ranges. Absence differs from
encoded zero/false; both presence flags survive when both fields are absent.

Comparison `depth=0..64` (16), `limit=1..65536` (1024); traversal caps at 100000
nodes and 64 MiB compared byte positions. Exceeded bounds fail without partial
equality/completion; narrow the subtree. `offset`/`length` are export-only.

Before comparison, `pb.slice` rejects unknown fields, incompatible wire types,
duplicate singular fields, truncation, unsupported versions, overflowing varints/
narrow integers and non-0/1 booleans. Tags plus scalar elements (including packed)
cap at 100000. Presence and uint64 values remain exact.

## Application

Retain `state.proto` and origin receipts. Structural compatibility proves neither
build identity, complete hardware serialization nor repeatability.

<a id="restore-and-branch-an-experiment"></a>

Execution actions validate core memory/register/SPU shapes and restore via
`snapshot`. Boundary: restored PC without target/EXE, explicit target after
resumption, or EXE header entry by default. Omit EXE for immediate inspection.

Restoration replaces ROM/program memory; bootstrap BIOS hash does not identify
embedded ROM. Inputs stay unchanged; `savestate=1` writes a new `state.pbuf`.
Equality proves neither cycle accuracy, hidden-device restoration nor host
scheduling/audio replay.
