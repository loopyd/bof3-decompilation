# Target contract

BOF3 binaries load independently. Identity is payload plus verified load address;
EMI entry is shipped archive path plus slot; function is `TARGET@0xADDRESS`.
Equal payloads/addresses stay separate until relocatability/ownership proven.
Original bytes/PS-X header/reviewed configuration outrank analyzer output.

| Fact | Owner |
| --- | --- |
| Binary/load/claims | `config/targets/<target>/target.toml` |
| Reviewed layout | Target `splat.yaml` |
| Execution options | `config/splat.yaml` |
| Local map | Target `symbols.txt` |
| SDK maps | `config/sdk/psyq-{slus,logo}.txt` |
| Reviewed analysis | Target `reviewed.rz` |
| Authored C | Manifest-owned metadata-tagged `src/bof3/` |

Explicit `sources`/`support_sources`/`headers`/`psyq_source` claims plus
function-level `@source`/`@behavior` own identity. Filename/directory ancestry is
never fallback; `source_dir` historical Splat root only. Keep new production lifts
one function per C source until source-consolidation owner's registry/native gates
are accepted. Existing grouped sources gain no grandfathered acceptance.

Target-private headers under `include/bof3/<subsystem>/`; shared declarations only
when multiple targets or external contract justify them. Header order: guard,
includes, types, extern data, prototypes, macros/helpers. PsyQ signatures identify
objects/addresses; official headers supply declarations, Rizin supplies sites;
none replaces others. Keep SDK external, verified archive members in manifest.
Sorted maps use `name = 0xADDRESS;`, uppercase address digits.

Generated `out/`, build products and analysis are disposable, not reviewed truth.
Generated PsyQ source is tracked only because build compiles it; never hand-edit.
Do not hand-edit build/toolchain state or commit user media/source-bearing evidence.
Work within repo, preserve unrelated dirty work. Git/external mutation requires
caller authority; autonomous exception is bounded in native-execution reference.
Complete assigned scope or report evidenced blocker and smallest unblocking action.

## Boundaries and data

PS-X header `t_addr` at 0x18 must equal manifest load; `t_size` equals payload size,
file payload offset 0x800. Raw overlay payload offset 0. Always
`runtime - load = payload offset`; never load an EMI at first-function address.
Raw entries may contain leading pointer/count/data headers. A plausible instruction
or Splat label does not outweigh original bytes: check calls, prologue/return paths
and whole range before reviewed boundary promotion.

Never leave data labelled `asm`. An `asm` subsegment claims instructions; text, byte
and pointer blobs, count words, string runs and handler tables are data and take
`rodata` with a `T_` or role name. Splitting a function out of a mixed blob means
relabelling the pieces: leading and trailing bytes become `rodata` at the same
boundaries. A `func_` label on a pointer table is a contradiction; rename the row and
relabel the segment. Keep the indentation of the original kind line when editing
Splat, and re-run `source splat` for the target, because a misindented kind parses as
a nested list and fails with `unsupported Splat subsegment`.

Data embedded inside code needs a symbol type, not a subsegment. Splat accepts
`name = 0xADDRESS; // type:jtbl` for a jump table, plus `jtbl_label` and `label` for
its branches; the map parser preserves the attribute and the index classifies those
rows as data. A function's asm subsegment therefore stays whole while its table is
declared, which is the only form that keeps the function's own addressing intact.

Audit the rule with `bin/harness analysis data-labels TARGET --check`. It walks every
`asm` subsegment in 32-byte windows and reports merged runs whose words are mostly
in-image pointers, mostly printable text, or mostly repeated or zero. Windows exist
because one `asm` row can span a pointer table plus unrelated bytes, and a whole-range
ratio averages the evidence away. A single window is reported only when every word is
a pointer, since a code sequence dense with address constants looks similar otherwise.
`--check` exits 1 while a finding remains. Promotion of the `asm` range stays a
reviewed edit plus `source splat` for the target; a range the map names as a global
takes `textbin`, which is how `analysis index` stops reporting it as a function/global
conflict.

Runtime-filled tables can be zero on disc; inspect producers/consumers. Cross-target
pointers can reference simultaneously loaded companion overlays; preserve reviewed
entry, never create out-of-payload local boundary. Same address alone proves nothing.
On byte-match, materialize proven owned BSS/initialized data with original contents,
keep other objects extern, verify following addresses unchanged.

## Duplicate promotion

Hash plus size is lead. Prove each reviewed range/original bytes, match deterministic
representative, independently port second target with local names/declarations.
Only two cross-target exact same-shape members justify a shared template
header such as `include/<domain>/<role>_templates.h`. Never multiply partial source across a byte group.
Normalize evidenced names before sharing; parameters only for actual differences.
Each member keeps metadata wrapper, declarations/map/Splat and own native checks.
No cross-overlay linked extern/wrapper call. Runtime service instead needs single
SLUS implementation plus EMI callsite evidence; stable contract alone may promote
shared header. No generic engine owner without real link target.
