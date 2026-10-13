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

Runtime-filled tables can be zero on disc; inspect producers/consumers. Cross-target
pointers can reference simultaneously loaded companion overlays; preserve reviewed
entry, never create out-of-payload local boundary. Same address alone proves nothing.
On byte-match, materialize proven owned BSS/initialized data with original contents,
keep other objects extern, verify following addresses unchanged.

## Duplicate promotion

Hash plus size is lead. Prove each reviewed range/original bytes, match deterministic
representative, independently port second target with local names/declarations.
Only two cross-target exact same-shape members justify worthwhile shared
`src/shared/<domain>/<role>.inc`. Never multiply partial source across a byte group.
Normalize evidenced names before sharing; parameters only for actual differences.
Each member keeps metadata wrapper, declarations/map/Splat and own native checks.
No cross-overlay linked extern/wrapper call. Runtime service instead needs single
SLUS implementation plus EMI callsite evidence; stable contract alone may promote
shared header. No generic engine owner without real link target.
