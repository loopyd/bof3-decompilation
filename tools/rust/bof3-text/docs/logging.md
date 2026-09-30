# Logging, colour and JSON

## Verbosity

`-v` may be repeated. It selects the log level; the primary command output is
unaffected.

| Flags | Level | What you see |
| --- | --- | --- |
| (none) | `warn` | Only warnings and errors |
| `-v` | `info` | One summary line per command (counts, destinations) |
| `-vv` | `debug` | Per-file detail, for example which codec a document was read as |
| `-vvv`, `-vvvv` | `trace` | Everything |

Log records go to **stderr**; command results go to **stdout**, so the two never
interleave.

`RUST_LOG` overrides the computed level when set, using the standard
`env_logger` filter syntax:

```sh
RUST_LOG=debug bin/harness text index ARCHIVE.EMI
RUST_LOG=bof3_text_v1=trace bin/harness text extract ARCHIVE.EMI -o /tmp/a.json
```

## Colour

```sh
bin/harness text index ARCHIVE.EMI              # coloured when stderr is a terminal
bin/harness text index ARCHIVE.EMI -n           # no colour
bin/harness text index ARCHIVE.EMI --no-color   # no colour
bin/harness text index ARCHIVE.EMI -nc          # no colour (accepted spelling)
NO_COLOR=1 bin/harness text index ARCHIVE.EMI   # no colour
```

Colour is enabled only when none of these is set *and* the stream is a terminal.
It is used for the log level label and for status words such as `ok`,
`extracted` and `packed`. Piping the output therefore yields plain text
automatically.

## JSON

`--json` makes the tool machine-readable in two independent ways:

1. Command output on **stdout** becomes a JSON document (see
   [commands.md](commands.md)).
2. Log records on **stderr** become one JSON object per line.

```sh
bin/harness text index ARCHIVE.EMI --json
# {"archive":"…","entries":[{"banks":1,"entry":11,"offset":753664,…}]}

bin/harness text -vv index ARCHIVE.EMI --json -n 2>/tmp/log.jsonl
# /tmp/log.jsonl: {"level":"info","message":"1 dialogue subfile(s) in …","target":"bof3_text_v1::cli"}
```

Log objects have exactly `level`, `message` and `target`. With `--json`, colours
are irrelevant, so `-n` is unnecessary; it is accepted and ignored.

## Examples

```sh
# quiet success, loud failure
bin/harness text pack --original A.EMI --text a.json -o B.EMI

# full trace for a single failing archive
bin/harness text -vvvv extract A.EMI -o /tmp/a.json

# structured logs plus structured result
bin/harness text -v query A.EMI --grep McNeil --json > matches.json 2> query.log
```
