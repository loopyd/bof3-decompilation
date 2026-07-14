# Projects, replay, and deterministic export

## Ownership model

Use three layers with different trust:

1. Generated native project database: convenient local cache, disposable.
2. Tracked reviewed replay and C type inputs: durable analyzer intent.
3. Deterministic generated exports: reproducible evidence for comparison.

Never make a native project database the only copy of a reviewed name, comment,
function boundary, or type placement. Project serialization and commands differ
between Rizin/radare2 versions, and radare2's official book explicitly warns
that projects have historical stability limitations.

## Native Rizin

Probe `P?` on the installed version. Current documented commands include:

```text
Ps path/project.rzdb       # save
Po path/project.rzdb       # open
Pi path/project.rzdb       # inspect project information
Poo path/project.rzdb      # open over currently loaded binaries, where supported
```

Startup supports `rizin -p path/project.rzdb`. Save only after raw mapping,
architecture, width, endianness, and input identity are correct.

## Native radare2

Probe `P?`; current official book documents:

```text
P+ NAME                    # save without change check
Ps NAME                    # save
P NAME                     # open
P- NAME                    # delete
Pl                          # list
Pz e project.zrp           # inspect exact native help; export archive
Pz i project.zrp           # inspect exact native help; import archive
```

Projects live below `dir.projects`; `r2 -p NAME` opens one. Keep `prj.files`
disabled unless intentionally copying the binary, and disable native project VCS
when the repository already owns provenance.

## Reviewed replay order

Keep replay files deterministic, target-specific, shallow, and reviewable:

```text
# Compatibility: engine-neutral reviewed subset; probe every command before use.
# Engine-specific commands must declare engine and verified version/capability.

# 1. Reviewed function definitions/names
af func_XXXXXXXX 0xXXXXXXXX
CC "Reviewed alias: semantic_name; canonical func_XXXXXXXX retained" @ 0xXXXXXXXX

# 2. Reviewed data flags and sizes
f DAT_XXXXXXXX 4 @ 0xXXXXXXXX

# 3. Type imports and version-checked type placements
to <reviewed-analysis-types.h>

# 4. Concise evidence comments
CC "Observed call target; identity verified against SDK prototype" @ 0xXXXXXXXX
```

Use only commands proven on the adapter's selected engine. Do not store UI
state, seek history, user preferences, absolute workstation paths, generated
analyzer names, bulk guesses, or decompiler text in reviewed replay.

Keep the default replay subset engine-neutral. If a required command is native
to one engine (for example modern Rizin `avga` versus a legacy/adapter `tl`
placement), mark the engine plus verified version/capability in a nearby replay
comment and make the adapter reject incompatible execution. There is no safe
assumption that a `.r2` command script is portable merely because both engines
accept the filename extension.

## Export contract

Every export should record:

- schema version;
- target identity and exact input path/hash;
- load address, architecture, bits, endianness, and optional verified CPU;
- engine and plugin versions/capabilities;
- replay/type input paths and hashes;
- functions, flags, strings, xrefs, comments, and reviewed type placements;
- skipped/unsupported queries and their reason.

Prefer native JSON commands, canonicalize numeric values, and sort arrays by
address plus name/type. Do not compare raw analyzer output order across versions.
Rebuild from clean input and replay to test reproducibility.

Compatibility is capability-based, not a guessed minimum version. This skill
was locally exercised with Rizin 0.8.2 and radare2 6.1.4; future releases still
must pass command/schema probes because neither project promises this repository
a stable cross-engine command contract.

## Repository example

```sh
bin/harness analysis doctor
bin/harness analysis init emi/etc/game/00
bin/harness analysis query emi/etc/game/00 functions
bin/harness analysis export emi/etc/game/00
bin/harness analysis graph emi/etc/game/00
```

The adapter owns engine differences. Generated projects and exports stay under
`out/analysis/`; reviewed commands/types stay under `config/analysis/`.

## Official sources

- Rizin projects: https://rizin.re/posts/introducing-projects/
- Rizin CLI/project option: https://book.rizin.re/src/first_steps/commandline_options.html
- radare2 projects: https://book.rada.re/projects/usage.html
- radare2 firmware replay-script practice: https://book.rada.re/r2fwrev/setup.html
