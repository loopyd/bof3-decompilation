# Original-only reconstruction runs

The development CLI prepares inputs, runs the plugin, independently verifies
candidates and reproduces fixed-corpus reports. It retains all 118 admitted rows,
seven separate exclusions, unchanged generated C, raw evidence and content hashes.
Complete original function bytes and required data contents/placement must pass
independent acceptance. Compilation and bounded observations grant no full
milestone credit.

The customized native implementation lives in the sibling `rz-psxdec` and
`retdec-psx` forks. [README.md](README.md) describes their managed build.
Supplied-context capture remains separate from original-only reconstruction.

Ordinary runs mount the selected member's original image for native callee
discovery. `--mount-image` replaces that default with the explicit mount order.
Both routes verify image ownership and retain unresolved callee, memory and
declaration obligations. Discovery retains queued paths after an unresolved
indirect transfer.
A canonical `jalr` linking through `ra` also exposes its potential
continuation after the delay slot. Its target, return and effects remain unknown;
discovery alone cannot complete a call or declaration contract. Other indirect
transfers expose no fallthrough path.

A discovered callee's final load can remain an unresolved translation boundary
without aborting other functions' analysis. This requires a retained discovery
bound, an authenticated nonbranch predecessor and no admitted successor. The
callee remains ineligible for complete call or declaration contracts.
Reaching the address bound ends that path; queued paths inside the bound can
still be discovered.

Callsite-path diagnostics are omitted when an entry-reachable prefix crosses a
call without a complete binding and the required register preservation. The
independent reader also checks the original control flow and diagnostic seeds.

A verified bound path can retain `entryPath.symbolicFrame` for conditional SP
and saved-register restoration. The reader independently replays original stores,
reloads and MIPS load delays before using those identities in the caller. Recovery
rejects nested calls, unknown stores and unsupported SP changes. It assumes an
aligned ordinary-RAM frame, ordinary-RAM reads outside that frame, normal accesses
and stable instructions without external interference. The call remains partial.
These facts supply no assigned result, complete memory effects or C declaration.

Register dataflow retains canonical bitwise `AND` results when both operands are
known constants. Call-path replay independently derives those constants from
original instructions. A known stored value does not establish its destination,
memory effects or declaration.

Fixed-address recovery can emit `instruction-fixed-first-use/v1` when the complete
entry-to-access prefix proves a unique constant address before any call. The
independent reader reconstructs the participating original instructions, edges,
definition chain and MIPS load delays. The fact applies only on first reach of
that access, before its instruction executes; incomplete later control does not
invalidate that prefix. Ambiguous paths, prefix cycles, unsupported control and
crossed calls refuse this rule. Existing whole-function fixed-address formats
retain their stricter coverage requirements.

The first-use record carries its entry, use, operand phase, equality evidence and
condition. It can establish the access address and alignment, not file-backed
contents, runtime contents, object extent, qualifiers, aliases, placement,
declarations or memory effects. Those obligations remain unresolved. Consumers
that require backed storage or ordinary declaration authority cannot use this
conditional record.

A direct call whose analyzed callee summary records at least one normal exit, and
which has no general-purpose register load pending at the call boundary, is treated
as a normally returning O32 callee: it restores `sp` and leaves the caller's `gp`,
`fp` and `s0`-`s7` registers intact. This is an admitted convention, not a fact
derived from the callee's bytes. It keeps the caller's post-call stores and loads
provable when the callee contract stays partial.

The convention covers registers only. It does not grant a result, call effect,
declaration or normal-return contract, it does not cover `ra`, it retains no frame
contents, and it never overrides a callee fact that settles a register to a
different value. `jalr`, an unresolved target, a callee without an analyzed summary
or without a normal exit, and a call with a pending register load keep the previous
refusal. The independent reader accepts the resulting preservation facts without
replaying them.

For the selected root, the return contract is accepted on its own coverage: complete
decoded instructions, a complete entry summary, a settled `v0` at every normal exit,
matching exit definitions, and a census that makes every `jr ra` in the range an
analysed exit. It does not require the control closure of the root's callees,
because a callee that never returns only removes paths from the claim.

It also does not require proof that each exit returns through the entry link. That
equality stays unprovable when `ra` is spilled to the frame and reloaded, which the
register flow does not model, so the census and the settled `v0` carry the claim
instead. A callee contract still requires the complete control summary. A complete
result can also complete the declaration projection when every demand is a parameter
or the continuation, so the root may reach a declaration while its control summary
stays partial.

A projected i32 declaration rests on complete entry and result facts and on the
registers the function reads at entry. Arguments reach O32 functions in `a0`-`a3`
only, so those demands become the parameter lanes. The stack pointer is accepted
because it is the frame base, and a demanded `gp`, `fp`, `s0`-`s7` or `ra` is
accepted only when the function saves it or passes it through, and that register's
entry value stays unproved: the rule bounds the role it appears in, not its value.
Any other entry demand refuses the declaration, and the independent reader applies
the same rule.

Native recovery can emit a body proposal while the original callee declaration
remains unknown. A complete `jr ra; nop` body can produce an empty C89 `void`
candidate. Supported straight-line bodies with direct calls can produce ordinary
C89 proposals with conditional signatures. This path rejects indirect or
exceptional control, incomplete lowering and escaping stack use. Both variants
preserve original facts and unresolved obligations. A call-free body with complete
direct control flow, unchanged SP and one checked `LW` from `sp+16` before any
branch or store can also produce a conditional five-word carrier. It assumes an
O32 word layout and a valid, aligned entry slot stable until the load. The inferred
configuration tags this carrier separately from original declaration evidence.
These proposals preserve every original obligation and grant no semantic
admission; compilation and byte comparison remain independent checks.

The independent evaluator supports complete literal-zero and empty-body C
translation units with bounded instruction schedules. For an empty-body proposal,
it can verify clean C and conditional schedule agreement while retaining unknown
original return compatibility and entry conditions. A body check alone grants no
autonomous acceptance.

Byte measurement can resolve an undefined direct-call symbol to its authenticated
original JAL address. Fresh linking and retained replay derive these addresses
independently from native evidence and object relocations. Numeric placement
does not prove callee declarations, image residency or semantic bindings.

The linked ELF is a measurement container. The verifier accounts recognized
compiler metadata separately from original runtime storage. Metadata accounting
does not establish required data contents, placement, initialization or bindings.

## Inputs and snapshots

Use a completed [prepared run](preparation.md), the verified receipt printed by
the fork build lifecycle, its fixed configuration and the explicit Rizin
executable. The run records complete content manifests for both forks, the driver,
native build and prerequisites, and loads the plugin from its managed build path.
It uses these files directly without copying source or build trees, archives or
native build inputs into each attempt. It rechecks their identities before each
native execution and afterward. Changed sources, original inputs, configuration
or selected build inputs refuse reuse. Setup reuses one managed build directory
and overwrites its current receipt. Prepare a fresh run when rebuilding changes
the referenced inputs. Reconstruction attempts contain bounded mission inputs,
compiler workspace inputs and outputs. Installed compiler tools run in place by
absolute path. No symlinks or separate snapshot store are required.

An original-only native workspace contains only original image bytes and the explicit request
and configuration. Authored Answer bodies, declarations, maps, source-specific
compiler profiles and prior answers are withheld. The pipeline trusts its pinned
local tools and records their explicit inputs.

To supply observed entry context, pass `--entry-receipt` and
`--entry-cpu-receipt` to `run`. The entry must be a passed built-in
[target mission](../../.agents/skills/psx-emulator/references/target.md) with
explicit `target=ADDRESS` and `savestate=1`, using only original BIOS and disc
inputs. Its optional controller and Vsync arguments remain supported. External
scripts, injected executables and restored entry states are excluded. Query that
captured state with the built-in `state` mission, `path=registers`, `depth=2` and
`--input state=PATH`, supplying only that state and the same BIOS. This query
inspects the file without restoring it. The importer verifies both captures, their
shared state and emulator identity, the selected PC, complete RAM and mounted
original image owner. This supplies observed context, not a universal entry or
declaration contract.

To derive occurrence-scoped pointer-load evidence, add `--pointer-load-receipt PATH`
alongside the matching entry and CPU receipts. Use a passed built-in RAM-interval
`history` mission that resumes that exact captured state. Its range must cover the
pointer cells and the complete dispatched instruction slice used by the derivation.
Declare a stop and event, payload and frame bounds before recording. Begin/end
contexts, capture hashes, start cycle, original instruction ownership and ordered
RAM replay must agree. Missing boundaries, mapping drift, incomplete recording or
a write overlapping a selected cell refuses the capture. This option cannot combine
with copied-original observations or source-form search.

The initial pointer rule requires an aligned ordinary-RAM full-word `LW` into a
nonzero GPR, immediately preceded by a recorded `LUI` defining its nonzero base.
The native producer and independent reader derive the address, four bytes and
conditional destination visibility from the recording. They check the intervening
instruction for premature use, cancellation, replacement and unsupported effects.
Unsupported later effects withdraw the occurrence; unsupported `JALR` forms are
not treated as no-ops. A record is keyed by occurrence, path and interval, not PC
alone.

The output is `originalCallSummary.observedPointerLoad`, tagged
`psx-observed-pointer-load/v1`. It is conditional diagnostic evidence, not a
recorded successful read or retirement. ISR status, normal-completion assumptions
and host-access exclusions remain explicit. The record supplies no universal
constant, MMIO pointee value, object declaration or complete call/effect contract.
A validated record may survive in partial output when generation fails; it does
not imply that C was generated or that the selected member was accepted.

To inspect original BIOS instructions observed after a RAM copy, pass
`--copied-original-receipt PATH` and `--copied-original-offset OFFSET` to `run`.
Supply one selected caller and explicit `--mount-image` inputs. The receipt must
contain a passed original-only RAM history, committed writes, consumed words and
entry markers. Its `observations.json` uses `bof3.original-handler-interval/v1`
and records the caller entry, handler entry, final registers, controller input
and audited exclusions. The importer checks these records against the original
BIOS slice. This mode cannot combine entry-state context or source-form selection.

The native workspace maps the BIOS slice into the observed RAM window and binds
the capture to its memory-context identity. It analyzes the observed entry as a
separate discovery lead without inventing a caller edge. Every function retains
an unresolved architectural condition, and copied bytes supply no ordinary data
facts. This diagnostic cannot produce an accepted declaration or generated C.
The observation applies only to the captured interval and its recorded assumptions.

## Run a selected pair

The retained prepared Problem includes the following two target-qualified member
keys. This example selects the metadata-free leaf and a parameterized member
that compiles through the generic pipeline. Selection supplies no signature or
expected result.
Choose a fresh destination; an existing one is refused. Relative destinations
are under the default run root, `out/retdec-118-reconstruction/`; use an absolute
path to select another location. Set `native_receipt` to the verified current
receipt printed by setup.

Initialize one cumulative attempt ledger for your campaign:

```sh
mission=reconstruction
attempt_ledger="out/retdec-118-reconstruction/attempt-ledgers/$mission"
bin/harness reconstruction ledger init "$attempt_ledger" \
	--prepared-run "$prepared_run" --mission "$mission"
```

Reuse that ledger for later runs, supplied captures and independent verification.
Before each operation, read its current digest:

```sh
ledger_digest=$(bin/harness reconstruction ledger status "$attempt_ledger" |
	.venv/bin/python -c 'import json, sys; print(json.load(sys.stdin)["digest"])')
```

The ledger records attempts without grants or execution quotas. Historical gaps
remain visible. An unfinished execution requires terminal-writer review before
another writer starts.

```sh
bin/harness reconstruction run \
  --prepared-run "$prepared_run" \
  --destination selected-pair-fresh-run \
  --native-receipt "$native_receipt" \
  --configuration tools/retdec-psx/native-config.json \
  --rizin toolchains/rizin/bin/rizin \
  --member exe/slus_004_22@8014aa04 \
  --member exe/slus_004_22@80166cb0 \
  --output-limit 16777216 \
  --attempt-ledger "$attempt_ledger" --expected-ledger-digest "$ledger_digest" \
  --json
```

Output-limit, process or cleanup failures remain failed attempts. Retrying uses a
fresh destination and records new limits.

To select an original-dataflow source alternative, add `--source-form
signed-narrow-cast` or `--source-form unsigned-mask-subtract`. Omitting the option
retains the existing form. Each run selects one form and compiles its unchanged C.
Unsupported evidence remains a refusal.

## Measure all frozen members

In the command above, replace both `--member` arguments with `--all-members`, use
a fresh destination. The CLI derives all 118 keys from the verified Problem in
recorded order. Failed and unsupported members remain visible.

## Continue a source-form search

Add `--search-source-forms` to compare both supported forms and retain each
member's best complete-byte result:

```sh
bin/harness reconstruction run \
	--prepared-run "$prepared_run" \
	--destination source-search \
	--native-receipt "$native_receipt" \
	--configuration tools/retdec-psx/native-config.json \
	--rizin toolchains/rizin/bin/rizin \
	--member exe/slus_004_22@80166cb0 \
	--member exe/slus_004_22@8014aa04 \
	--search-source-forms \
	--attempt-ledger "$attempt_ledger" --expected-ledger-digest "$ledger_digest" \
	--json
```

An interruption retains completed outputs, failures and cumulative attempts in
`report.json` and numbered `progress-*.json` files. Set `checkpoint` to the
absolute canonical path of one retained report or progress file. Hash that file:

```sh
checkpoint_sha256=$(sha256sum -- "$checkpoint" | cut -d ' ' -f 1)
```

Resume with the same prepared run, receipt, configuration, tools and member order.
Choose a fresh destination:

```sh
bin/harness reconstruction run \
	--prepared-run "$prepared_run" \
	--destination source-search-resumed \
	--native-receipt "$native_receipt" \
	--configuration tools/retdec-psx/native-config.json \
	--rizin toolchains/rizin/bin/rizin \
	--member exe/slus_004_22@80166cb0 \
	--member exe/slus_004_22@8014aa04 \
	--search-source-forms \
	--attempt-ledger "$attempt_ledger" --expected-ledger-digest "$ledger_digest" \
	--resume-from "$checkpoint" --resume-sha256 "$checkpoint_sha256" --json
```

Resume verifies retained outputs and current inputs. A different hash or input
identity refuses reuse. Earlier attempts remain recorded. Completed results are
reused only after verification.

Grant-based search checkpoints use a retired schema. Start a fresh capture and
retain the same cumulative ledger.

## Capture fresh supplied context

`capture-supplied` runs ordinary `pdz` with Answer-associated prototypes and
declared metadata aids. Keep this input separate from original-only generation.
The native process receives the projected aids and original image, without Answer
C bodies, maps or source-specific compiler profiles. Each attempt uses fresh
HOME, XDG, temporary and cache directories.

Provide a bounded `supplied-context-v1` JSON file. Each prototype must exactly
match the active prepared Answer. This example requests one entry prototype:

```json
{
  "schema": "supplied-context-v1",
  "contexts": [
    {
      "member_key": "exe/slus_004_22@8014aa04",
      "prototype": "void bootNoop(void)",
      "metadata": []
    }
  ]
}
```

Set `prepared_run`, `native_receipt`, `rizin` and `context` to their absolute
paths. Choose an absent absolute destination:

```sh
bin/harness reconstruction capture-supplied \
  --prepared-run "$prepared_run" \
  --destination "$destination" \
  --native-receipt "$native_receipt" \
  --rizin "$rizin" \
  --context "$context" \
  --output-limit 16777216 \
  --attempt-ledger "$attempt_ledger" --expected-ledger-digest "$ledger_digest" \
  --json
```

The capture retains raw output and 14 state listings before aids, before
decompilation and afterward. It requires complete token, console and source
equality before qualifying a source. It does not compile that source. Pass the
completed capture as `--generation` to `verify` for the independent first C89
check and canonical-profile rebuild. The verifier derives the mode from observed
inputs. Adding a requested mode label cannot establish its provenance.

## Results and limits

`run.json` records snapshots, inputs, tools, configuration and limits. Member
directories retain native status, raw streams, generated C, PC spans and compiler
stage records. `report.json` contains exactly 118 admitted rows. Unselected,
unresolved and failed members remain visible.

The strict C89 and uninitialized-return diagnostic precedes the canonical
GCC 2.7.2-PSX, maspsx and assembler stages. Reconstruction selects this single
profile; historical compilers remain available for explicit comparisons through
the compiler catalog. The generated C is compiled unchanged.
Each stage records the exact rendered environment and command. A recorded
`raw_exit` belongs to the managed supervisor process; it does not identify a
native child's kernel signal status. Missing or uncertain exits remain missing.

Generation and independent verification execute the installed driver, `cc1`, `cpp`,
assembler, linker and existing maspsx by absolute path. Component hashes,
profile, ordered options and recipe identity remain verified. The canonical
profile requires absent external GCC `specs` files at every declared lookup
path and rechecks that absence. Compiler binaries and maspsx stay in their
installed locations; runs create no bootstrap or empty specs file. The verifier
grants a 32-bit signed `int` fact only when the driver, `cc1` and `cpp` match the
pinned canonical release and the verified recipe's configuration and search
inputs. Signed representation remains unknown. The complete literal-zero source
rule can use this width; it does not establish an original declaration or entry
domain.

Object extraction proves ELF correspondence only. It does not establish Original
byte identity, declarations, associated-data contents or placement. The report
keeps semantic and admission flags false until the independent evaluator supplies
their required evidence. Compilation and a successful plugin exit cannot satisfy
the 118-function milestone.

## Independently verify and reproduce a report

Use a completed current preparation and generation directory. Set
`prepared_run` and `generation` to their recorded paths. Choose fresh destinations.
The verifier rebuilds unchanged candidate sources and checks complete original and
ELF extents, associated obligations and provenance. Missing or refused members
remain visible in the fixed denominator.

```sh
bin/harness reconstruction verify \
  --baseline tools/retdec-psx/corpus.json \
  --prepared-run "$prepared_run" \
  --generation "$generation" \
  --member exe/slus_004_22@80166cb0 \
  --member exe/slus_004_22@8014aa04 \
  --destination selected-pair-verification \
  --require-mode autonomous \
  --input-read-limit 274877906944 \
  --artifact-limit 134217728 \
  --output-limit 536870912 \
  --attempt-ledger "$attempt_ledger" --expected-ledger-digest "$ledger_digest" \
  --json

bin/harness reconstruction report \
  --baseline tools/retdec-psx/corpus.json \
  --prepared-run "$prepared_run" \
  --verification out/retdec-118-reconstruction/selected-pair-verification \
  --destination selected-pair-report \
  --require-mode autonomous \
  --input-read-limit 68719476736 \
  --artifact-limit 134217728 \
  --output-limit 536870912 \
  --json
```

Use `--all-members` instead of both `--member` arguments to verify all frozen keys.
Repeat `--generation` or `--verification` to consume multiple completed inputs.
`report` reproduces verification evidence without executing native or compiler
tools. Both commands require explicit input, artifact and output byte limits.
Physical reads are counted without a read-count ceiling or execution allowance.
Choose bounds for the selected inputs; these examples do not guarantee capacity
for a full 118-candidate verification.

`verification-result.json` and `report-result.json` contain terminal publications.
Exit 0 requires every row to satisfy the requested mode. Exit 1 publishes an
incomplete result. Exit 2 refuses the operation. Owned workspaces retain
`refusal.json` when bounded publication succeeds. Parser and destination-policy
refusals can return without new files. A refusal is not a report input. The required
mode is one of `autonomous`, `metadata_assisted` or `signature_only`. Each report
keeps those evidence buckets separate and counts semantic and complete exact
results separately. Assisted evidence cannot grant autonomous exactness.

Private original images, generated material containing those bytes and captures
stay local. Remove obsolete runs and disposable checks when they are no longer
needed. Use `openspec list` to discover the current reconstruction change and its
remaining implementation and acceptance tasks.
