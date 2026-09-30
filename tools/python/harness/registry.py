"""Declarative harness domain/action registry; execution stays with each owner."""

from __future__ import annotations

from collections.abc import Mapping

from .common.dispatch import Action, Domain

_LIFT = "harness.commands.lift"
EXAMPLE_TEXT_EXTRACT = (
    "bin/harness text extract out/extracted/BIN/WORLD00/AREA000.EMI "
    "-o out/text/AREA000.json"
)
EXAMPLE_TEXT_PACK = (
    "bin/harness text pack --original out/extracted/BIN/WORLD00/AREA000.EMI "
    "--text out/text/AREA000.json -o out/text/AREA000.EMI"
)
EXAMPLE_TEXT_QUERY = (
    "bin/harness text query out/extracted/BIN/WORLD00/AREA000.EMI --grep McNeil"
)


def _lift(name: str, help: str, example: str = "") -> Action:
    if not example:
        example = f"bin/harness lift {name} exe/logo@0x801CE758"
    return Action(
        help,
        _LIFT,
        kind="passthrough",
        example=example,
    )


DOMAINS: Mapping[str, Domain] = {
    "setup": Domain(
        "prepare and build repository toolchains",
        actions={},
        passthrough=Action(
            "prepare repository toolchains",
            "harness.commands.setup",
            kind="passthrough",
            example="bin/harness setup --component pcsx-redux",
        ),
    ),
    "doctor": Domain(
        "inspect repository prerequisites and built tools",
        actions={},
        passthrough=Action(
            "inspect setup health",
            "harness.commands.doctor",
            kind="passthrough",
            example="bin/harness doctor",
        ),
    ),
    "patch": Domain(
        "list, apply, check and revert target patch folders",
        actions={},
        passthrough=Action(
            "list, apply, check or revert target patches",
            "harness.commands.patch",
            kind="passthrough",
            example="bin/harness patch check --target pcsx-redux",
        ),
    ),
    "runtime": Domain(
        "bounded PCSX-Redux command and capture missions",
        actions={},
        passthrough=Action(
            "inspect or execute an emulator mission",
            "harness.runtime.cli",
            kind="passthrough",
            example="bin/harness runtime status",
        ),
    ),
    "agent": Domain(
        "instruction evidence for one agent mission",
        actions={
            "context": Action(
                "assemble bounded worker/cleanup/reverse/review context",
                "harness.commands.agent",
                example="bin/harness agent context worker",
            ),
        },
    ),
    "analysis": Domain(
        "cross-target index, query and readiness evidence",
        actions={
            "index": Action(
                "rebuild or recover the fresh cross-target query cache",
                "harness.commands.index",
                example="bin/harness analysis index",
            ),
            "query": Action(
                "query fresh indexed evidence",
                "harness.commands.rev_query",
                example="bin/harness analysis query symbols func_",
            ),
            "readiness": Action(
                "bounded aggregate readiness over fresh evidence",
                "harness.commands.analysis_readiness",
                example="bin/harness analysis readiness",
            ),
            "boundary": Action(
                "research function/global conflicts without the reverse index",
                "harness.commands.boundary",
                example="bin/harness analysis boundary emi/etc/commu00/00 --json",
            ),
            "scan": Action(
                "scan an image for structured data",
                "harness.commands.data_scan",
                example="bin/harness analysis scan --all",
            ),
            "data-candidates": Action(
                "rank unlifted asm ranges that look like data, not functions",
                "harness.analysis.datacandidates",
                example="bin/harness analysis data-candidates emi/etc/shop/00 --json",
            ),
            "carve": Action(
                "probe a bin->asm carve in isolation (evidence only)",
                "harness.analysis.carve",
                example="bin/harness analysis carve emi/etc/shop/00@0x801D3828 0x2C28:0x2D78 --json",
            ),
            "scratchpad": Action(
                "manage decomp.me scratchpad artifacts",
                "harness.commands.scratchpad",
                example="bin/harness analysis scratchpad --help",
            ),
            "rz-project": Action(
                "isolated Rizin analyze/status/open",
                "harness.commands.rz_project",
                example="bin/harness analysis rz-project analyze exe/logo",
            ),
            "rizin": Action(
                "run the pinned local Rizin analyzer",
                "harness.commands.tool",
                kind="fixed",
                name="rizin",
                example="bin/harness analysis rizin -c 'axt @ 0x801CE758'",
            ),
            "spimdisasm": Action(
                "run the pinned spimdisasm disassembler",
                "harness.commands.tool",
                kind="fixed",
                name="spimdisasm",
                example="bin/harness analysis spimdisasm --help",
            ),
        },
    ),
    "audio": Domain(
        "build, run and package the Rust BOF3 audio tools",
        actions={
            "build": Action(
                "Cargo build; optionally run a Rust audio operation",
                "harness.commands.audio",
                entry="build_main",
                example="bin/harness audio build",
            ),
            "package": Action(
                "package Rust sources, local crates, lockfiles and licenses",
                "harness.commands.audio",
                entry="package_main",
                example="bin/harness audio package out/bof3-audio-source.zip",
            ),
            **{
                name: Action(
                    f"run Rust audio {name}",
                    "harness.commands.audio",
                    kind="passthrough",
                    example=f"bin/harness audio {name} --help",
                )
                for name in (
                    "index",
                    "query",
                    "map",
                    "extract",
                    "render",
                    "pack",
                    "verify",
                )
            },
        },
    ),
    "build": Domain(
        "compile all lifts, one target, or one function",
        actions={
            "variants": Action(
                "resolve and stage per-object compiler variants",
                "harness.commands.compiler_variants",
                example="bin/harness build variants path gcc-2.7.2-psx",
            ),
        },
        passthrough=Action(
            "build all lifts, one TARGET, or one TARGET@0xADDRESS",
            "harness.commands.build",
            kind="passthrough",
            example="bin/harness build exe/logo@0x801CE758",
        ),
    ),
    "combiner": Domain(
        "inspect, preserve and rehearse multi-function images",
        actions={},
        passthrough=Action(
            "inspect, preserve and rehearse multi-function images",
            "harness.combiner.cli",
            kind="passthrough",
            example=(
                "bin/harness combiner inspect-source "
                "src/bof3/battle/func_800A0A04_battle15.c"
            ),
        ),
    ),
    "decomp": Domain(
        "deterministic parent-owned lift diagnosis and audit",
        actions={
            "diagnose": Action(
                "diagnose one parent lift request",
                "harness.commands.agent_run",
                kind="passthrough",
                example=(
                    "bin/harness decomp diagnose out/mission-request.json "
                    "--expected-request-digest PIN --output out/reviews/lift-diagnosis/BEFORE"
                ),
            ),
            "audit": Action(
                "audit retained lift candidates",
                "harness.commands.agent_run",
                kind="passthrough",
            ),
        },
    ),
    "emi": Domain(
        "EMI target and archive operations",
        actions={
            "target": Action(
                "preview or create one EMI target",
                "harness.commands.emi_target",
                example="bin/harness emi target BIN/BATTLE/BATL_END.EMI#0",
            ),
            "archive": Action(
                "list, extract or repack EMI archives",
                "harness.emi.archive",
                example="bin/harness emi archive list archive.EMI",
            ),
        },
    ),
    "lift": Domain(
        "target-qualified one-function lifting loop",
        actions={
            "asm-diff": _lift("asm-diff", "compare one authored lift's assembly"),
            "byte-match": _lift("byte-match", "compare one authored lift's bytes"),
            "m2c": _lift(
                "m2c",
                "emit a C seed for one function",
                "bin/harness lift m2c exe/logo@0x801CE758 -o candidate.c",
            ),
            "m2ctx": _lift("m2ctx", "emit the target context for one function"),
            "promote": _lift("promote", "validate a canonical candidate"),
            "flag-search": Action(
                "rank known compiler flag profiles",
                "harness.commands.flag_search",
                example="bin/harness lift flag-search exe/logo@0x801CE758",
            ),
            "permute": Action(
                "bounded source-shape search",
                "harness.commands.permute",
                example="bin/harness lift permute exe/logo@0x801CE758 --time-limit 30",
            ),
            "status": Action(
                "audit exact/partial/invalid lifts",
                "harness.commands.decomp_status",
                example="bin/harness lift status exe/logo",
            ),
            "companion-check": Action(
                "gate a lift through a declared EMI companion call",
                "harness.commands.companion_check",
                example="bin/harness lift companion-check exe/logo@0x801CE758",
            ),
            "gate": Action(
                "run the combined per-selector native gates",
                "harness.commands.gate",
                example="bin/harness lift gate exe/logo@0x801CE758",
            ),
            "clone": Action(
                "generate a self-contained clone of a byte-identical duplicate lift",
                "harness.commands.clone",
                example=(
                    "bin/harness lift clone emi/bmagic/magic003/03@0x801F0A28 "
                    "--from src/bof3/battle/clearSharedWorkCells.c --json"
                ),
            ),
            "sweep": Action(
                "measure many clean-C candidate shapes in one call",
                "harness.commands.sweep",
                example="bin/harness lift sweep exe/logo@0x801CE758 SRC.c v1.c v2.c",
            ),
            "next": Action(
                "select the next easiest lift candidate",
                "harness.commands.next_lift",
                example="bin/harness lift next emi/battle/battle/15",
            ),
            "campaign": Action(
                "delta-based lift campaign bookkeeping",
                "harness.commands.campaign",
                example="bin/harness lift campaign sync",
            ),
        },
    ),
    "macros": Domain(
        "macro opportunity indexing and reviewed resolution",
        actions={},
        passthrough=Action(
            "macro opportunity indexing and reviewed resolution",
            "harness.macros.cli",
            kind="passthrough",
            example=(
                "bin/harness macros describe src/bof3/battle/func_800A0A04_battle15.c"
            ),
        ),
    ),
    "media": Domain(
        "PS-X disc and STR media operations",
        actions={
            "disc": Action(
                "inspect and extract original disc files",
                "harness.media.disc",
                example="bin/harness media disc extract -i inputs/external/disc.cue -o out/extracted",
            ),
            "str": Action(
                "inspect, validate or convert STR media",
                "harness.commands.str_media",
                example="bin/harness media str inspect out/extracted/INTRO.STR",
            ),
        },
    ),
    "naming": Domain(
        "symbol/naming opportunity, evidence and reviewed transactions",
        actions={
            "evidence": Action(
                "run one bounded naming evidence collection",
                "harness.naming.runner",
                example=(
                    "bin/harness naming evidence exe/test "
                    "out/reviews/reports/exe__test.json"
                ),
            ),
        },
        passthrough=Action(
            "symbol/naming opportunity, evidence and reviewed transactions",
            "harness.naming.cli",
            kind="passthrough",
            example="bin/harness naming opportunities emi/battle/battle/15",
        ),
    ),
    "plans": Domain(
        "persistent repository plan management",
        actions={},
        passthrough=Action(
            "persistent repository plan management",
            "harness.commands.plans",
            kind="passthrough",
            example="bin/harness plans list",
        ),
    ),
    "psyq": Domain(
        "PsyQ signature evidence and build header staging",
        actions={
            "scan": Action(
                "scan reused PsyQ signatures",
                "harness.commands.psyq",
                kind="passthrough",
                example="bin/harness psyq scan --all",
            ),
            "calls": Action(
                "list PsyQ signature calls",
                "harness.commands.psyq",
                kind="passthrough",
                example="bin/harness psyq calls --all",
            ),
            "proposal": Action(
                "emit a PsyQ signature proposal",
                "harness.commands.psyq",
                kind="passthrough",
                example="bin/harness psyq proposal --all",
            ),
            "import": Action(
                "stage PsyQ build headers",
                "harness.commands.psyq_import",
                example="bin/harness psyq import --archive inputs/external/psyq-4.7.zip",
            ),
        },
    ),
    "source": Domain(
        "generated split/symbol inputs and source validation",
        actions={
            "splat": Action(
                "regenerate reviewed segment output",
                "harness.commands.splat",
                example="bin/harness source splat exe/logo",
            ),
            "symbols": Action(
                "map check/normalize, bindings and PsyQ import",
                "harness.commands.symbols",
                example="bin/harness source symbols normalize exe/logo --write",
            ),
            "validate": Action(
                "validate source ownership and generated snapshots",
                "harness.commands.validate_sources",
                example="bin/harness source validate",
            ),
            "docs": Action(
                "check spec, tool-doc and evidence drift against code",
                "harness.commands.docs.drift",
                example="bin/harness source docs",
            ),
        },
    ),
    "text": Domain(
        "text blocks, corpus index and raw-text instances: index, scan, extract, validate, query, pack, probe, map, build-index, search, verify",
        actions={
            "prepare": Action(
                "prepare corpus search artifacts from extracted US archives",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text prepare --root out/extracted/BIN",
            ),
            "index": Action(
                "list the text subfiles of an EMI archive and their classes",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text index out/extracted/BIN/WORLD00/AREA000.EMI",
            ),
            "extract": Action(
                "write one text block or raw-text instance as an editable JSON document",
                "harness.commands.text",
                kind="passthrough",
                example=EXAMPLE_TEXT_EXTRACT,
            ),
            "validate": Action(
                "check an edited dialogue text document",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text validate out/text/AREA000.json",
            ),
            "scan": Action(
                "latch candidate raw-text runs in every subfile (heuristic leads)",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text scan out/extracted/BIN/WORLD00/AREA000.EMI --json",
            ),
            "probe": Action(
                "probe every subfile for text-bank structure",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text probe out/extracted/BIN/WORLD00/AREA000.EMI --round-trip",
            ),
            "windows": Action(
                "build the classified-window registry",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text windows --root out/extracted/BIN",
            ),
            "vocabulary": Action(
                "derive the readability vocabulary from the verified rows",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text vocabulary",
            ),
            "readability": Action(
                "apply the readability rule to text (a calibration diagnostic)",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text readability --text 'Worker CTTTT'",
            ),
            "payloads": Action(
                "list the payloads the inventory identifies, per archive",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text payloads --root out/extracted/BIN",
            ),
            "verify": Action(
                "round-trip every text subfile of a known class",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text verify",
            ),
            "map": Action(
                "map text-versus-data roles across every archive (read-only)",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text map",
            ),
            "build-index": Action(
                "build or check the retained corpus text index",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text build-index",
            ),
            "search": Action(
                "search the retained corpus text index",
                "harness.commands.text",
                kind="passthrough",
                example="bin/harness text search --grep spring",
            ),
            "pack": Action(
                "repack an edited JSON document into a separate EMI output",
                "harness.commands.text",
                kind="passthrough",
                example=EXAMPLE_TEXT_PACK,
            ),
            "query": Action(
                "list, search or scan text; --class heuristic searches raw-text instances",
                "harness.commands.text",
                kind="passthrough",
                example=EXAMPLE_TEXT_QUERY,
            ),
        },
    ),
    "types": Domain(
        "type declarations, representation and reviewed transactions",
        actions={},
        passthrough=Action(
            "type declarations, representation and reviewed transactions",
            "harness.types.cli",
            kind="passthrough",
            example="bin/harness types baseline",
        ),
    ),
}
