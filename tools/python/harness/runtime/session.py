"""Run trusted Lua missions through an identified, isolated PCSX-Redux process."""

from __future__ import annotations

import math
import json
import hashlib
from dataclasses import dataclass
from pathlib import Path
import time

from harness.common.process import resolve_isolated_supervisor, run_bounded
from harness.io import file_sha256, write_json
from harness.runtime.assets import collect_assets, stage_assets
from harness.runtime.completion import validate_completion
from harness.runtime.disc import collect_disc, encode_bindings, stage_disc, verify_disc
from harness.runtime.launch import (
    PROCESS_LIMITATIONS,
    emulator_arguments,
    launch_settings,
    mission_environment,
    parse_arguments,
)
from harness.runtime.origin import verify_origin, validate_selection
from harness.toolchain.pcsx import PcsxReduxToolchain
from harness.common.artifacts import ConsumedFile, FileIdentity
from harness.common.deadlines import bind_deadline, check_deadline, use_deadline
from harness.runtime.origin import decode_object


@dataclass(frozen=True)
class RuntimeCaptureFacts:
    receipt: FileIdentity
    completion: dict
    inputs: tuple[FileIdentity, ...]
    captures: tuple[FileIdentity, ...]
    status: str
    limitations: tuple[str, ...]


@dataclass(frozen=True)
class StateComparisonFacts:
    report: FileIdentity
    schema: FileIdentity
    before: FileIdentity
    after: FileIdentity
    subtree: str
    compared_nodes: int
    compared_bytes: int
    changed_bytes: int
    difference_count: int
    observed_equal: bool
    status: str
    reason: str


def inspect_state_comparison(captures, reader, deadline):
    from harness.io import repo_layout

    with use_deadline(deadline):
        receipt_bytes = reader(captures.receipt.path, captures.receipt.bytes, None)
        if hashlib.sha256(receipt_bytes).hexdigest() != captures.receipt.sha256:
            raise ValueError("runtime session receipt changed")
        receipt = json.loads(receipt_bytes, object_pairs_hook=decode_object)
        selection = validate_selection(receipt)
        builtin = selection["kind"] == "builtin"
        if builtin:
            if (
                selection["name"] != "state"
                or receipt["emulator"].get("source_owner") != "pcsx-redux-agentic"
            ):
                raise ValueError(
                    "Runtime comparison is not the maintained builtin state mission"
                )
            owned = {row["name"]: row for row in selection["resources"]["resources"]}
            source = repo_layout().root / "third_party/pcsx-redux-agentic"
            for name in ("state", "support", "snapshot", "difference"):
                expected = owned[name]
                current = reader(source / expected["path"], 1 << 20, None)
                if (
                    len(current) != expected["source_bytes"]
                    or hashlib.sha256(current).hexdigest() != expected["source_sha256"]
                ):
                    raise ValueError(
                        "Runtime comparison resource is not its recorded current owner"
                    )
        else:
            if receipt["schema"] != "psx.runtime-session/v1":
                raise ValueError(
                    "External v2 receipts do not qualify the maintained builtin comparison"
                )
            for name in (
                "script",
                "module:support",
                "module:snapshot",
                "module:difference",
            ):
                expected = receipt["inputs"][name]
                current = reader(Path(expected["path"]), 1 << 20, None)
                if (
                    len(current) != expected["bytes"]
                    or hashlib.sha256(current).hexdigest() != expected["sha256"]
                ):
                    raise ValueError(
                        "Historical runtime comparison owning source changed"
                    )
        rows = {row.path.name: row for row in captures.captures}
        if not {"comparison.json", "state.proto"} <= set(rows):
            raise ValueError(
                "runtime comparison requires its full report and emitted state schema"
            )
        for name in ("comparison.json", "state.proto"):
            identity = rows[name]
            raw = reader(identity.path, identity.bytes, None)
            if (
                len(raw) != identity.bytes
                or hashlib.sha256(raw).hexdigest() != identity.sha256
            ):
                raise ValueError("runtime comparison/schema capture changed")
            if name == "comparison.json":
                report = json.loads(raw, object_pairs_hook=decode_object)
            elif builtin:
                native = selection["resources"]["state"]
                if (
                    len(raw) != native["schema_bytes"]
                    or hashlib.sha256(raw).hexdigest() != native["schema_sha256"]
                ):
                    raise ValueError(
                        "Runtime comparison emitted schema differs from native build owner"
                    )
        arguments = receipt["arguments"]
        if arguments.get("action") != "compare" or any(
            name in arguments for name in ("offset", "length")
        ):
            raise ValueError("runtime comparison does not select a complete subtree")
        expected_fields = {
            "schema",
            "path",
            "scope",
            "presence",
            "equal",
            "compared_nodes",
            "compared_bytes",
            "changed_bytes",
            "differences",
            "before_present",
            "after_present",
            "difference_count",
            "complete",
            "schema_compatibility",
            "identities",
        }
        depth, limit = (
            int(arguments.get("depth", "16")),
            int(arguments.get("limit", "1024")),
        )
        if (
            set(report) != expected_fields
            or report["schema"] != "psx.runtime-comparison/v2"
            or report["scope"] != "selected schema subtree"
            or report["presence"] != "encoded fields"
            or report["complete"] is not True
            or report["path"] != arguments.get("path", "")
            or report["schema_compatibility"]
            not in (
                {"all encoded fields validated against running schema; version 6"}
                if builtin
                else {
                    "all encoded fields validated against running schema; version 4",
                    "all encoded fields validated against running schema; version 5",
                }
            )
            or report["identities"]
            != "input and emulator hashes in receipt.json; exact schema in state.proto"
        ):
            raise ValueError("runtime comparison schema, scope or completion differs")
        if (
            not 0 <= depth <= 64
            or not 1 <= limit <= 65536
            or type(report["differences"]) is not list
            or not len(report["differences"]) == report["difference_count"] <= limit
        ):
            raise ValueError("runtime comparison count/depth bounds differ")
        for key, maximum in (
            ("compared_nodes", 100000),
            ("compared_bytes", 67108864),
            ("changed_bytes", report["compared_bytes"]),
        ):
            if type(report[key]) is not int or not 0 <= report[key] <= maximum:
                raise ValueError("runtime comparison bounded observation is invalid")
        if any(
            type(report[key]) is not bool
            for key in ("equal", "before_present", "after_present")
        ) or report["equal"] != (report["difference_count"] == 0):
            raise ValueError("runtime comparison observation fields disagree")
        before, after = (
            receipt["inputs"][name] for name in ("file:STATE", "file:OTHER")
        )
        identities = tuple(
            FileIdentity(Path(row["path"]), row["bytes"], row["sha256"])
            for row in (before, after)
        )
        return StateComparisonFacts(
            rows["comparison.json"],
            rows["state.proto"],
            *identities,
            report["path"],
            report["compared_nodes"],
            report["compared_bytes"],
            report["changed_bytes"],
            report["difference_count"],
            report["equal"],
            "matched",
            "retained maintained mission's bounded encoded-subtree observation; valid paired function execution and universal semantics remain unknown",
        )


def inspect_runtime_capture(directory, reader, deadline):
    consumed = []

    def read(path, bound, events=None):
        check_deadline()
        if path.is_symlink() or not path.is_file() or path.resolve() != path:
            raise ValueError("runtime proof requires canonical regular physical files")
        raw = reader(path, bound, events)
        if len(raw) > bound:
            raise ValueError("runtime proof exceeds its input bound")
        item = ConsumedFile(
            FileIdentity(path, len(raw), hashlib.sha256(raw).hexdigest()), raw
        )
        consumed.append(item)
        return raw

    with use_deadline(deadline):
        receipt = json.loads(
            read(directory / "receipt.json", 2 << 20), object_pairs_hook=decode_object
        )
        identity = consumed[0].identity
        if (
            type(receipt) is not dict
            or receipt.get("schema")
            not in {"psx.runtime-session/v1", "psx.runtime-session/v2"}
            or receipt.get("status") != "passed"
        ):
            raise ValueError(
                "runtime proof requires a terminal successful maintained session"
            )
        selection = validate_selection(receipt)
        if receipt["schema"] == "psx.runtime-session/v2":
            source = Path(receipt["emulator"]["binary"]).parent
            for resource in receipt["emulator"]["resources"]["resources"]:
                raw = read(source / resource["path"], resource["source_bytes"])
                if (
                    len(raw) != resource["source_bytes"]
                    or hashlib.sha256(raw).hexdigest() != resource["source_sha256"]
                ):
                    raise ValueError(
                        "Builtin resource physical source identity changed"
                    )
        for value in receipt["inputs"].values():
            data = read(Path(value["path"]), value["bytes"])
            if (
                len(data) != value["bytes"]
                or hashlib.sha256(data).hexdigest() != value["sha256"]
            ):
                raise ValueError("runtime input identity changed")
        for path_key, sha_key in (
            ("binary", "binary_sha256"),
            ("sdl_library", "sdl_sha256"),
        ):
            path = Path(receipt["emulator"][path_key])
            if (
                hashlib.sha256(read(path, path.stat().st_size)).hexdigest()
                != receipt["emulator"][sha_key]
            ):
                raise ValueError("runtime emulator identity changed")
        if selection["kind"] == "external":
            mission = read(
                directory / "mission.lua", receipt["inputs"]["script"]["bytes"]
            )
            if (
                hashlib.sha256(mission).hexdigest()
                != receipt["inputs"]["script"]["sha256"]
            ):
                raise ValueError("runtime maintained mission copy changed")
        elif (directory / "mission.lua").exists() or (directory / "lua").exists():
            raise ValueError("Builtin session unexpectedly stages external Lua")
        for key, relative in receipt.get("assets", {}).items():
            path = directory / relative
            if not path.is_relative_to(directory) or ".." in path.parts:
                raise ValueError("runtime staged asset escapes session")
            expected = receipt["inputs"][key]
            if (
                hashlib.sha256(read(path, expected["bytes"])).hexdigest()
                != expected["sha256"]
            ):
                raise ValueError("runtime staged asset identity changed")
        media = {
            key: value
            for key, value in receipt["inputs"].items()
            if key.startswith("disc:")
        }
        if media:
            staged_disc = receipt.get("disc")
            folder = directory / "disc"
            if (
                "disc:cue" not in media
                or type(staged_disc) is not dict
                or set(staged_disc) != {"path", "sha256"}
                or staged_disc["path"] != "disc/media.cue"
                or folder.is_symlink()
                or not folder.is_dir()
            ):
                raise ValueError("runtime staged disc descriptor differs")
            expected_names = {
                "media.cue",
                *(key.removeprefix("disc:") for key in media if key != "disc:cue"),
            }
            if {
                path.name for path in folder.iterdir()
            } != expected_names or hashlib.sha256(
                read(folder / "media.cue", 1 << 20)
            ).hexdigest() != staged_disc["sha256"]:
                raise ValueError("runtime staged disc census or cue identity changed")
            for key, expected in media.items():
                check_deadline()
                if key == "disc:cue":
                    continue
                staged_track = folder / key.removeprefix("disc:")
                if not staged_track.is_symlink() or staged_track.resolve(
                    strict=True
                ) != Path(expected["path"]):
                    raise ValueError("runtime staged disc track binding changed")
                content = read(staged_track.resolve(strict=True), expected["bytes"])
                if (
                    len(content) != expected["bytes"]
                    or hashlib.sha256(content).hexdigest() != expected["sha256"]
                ):
                    raise ValueError("runtime staged disc track identity changed")
        elif receipt.get("disc") is not None:
            raise ValueError(
                "runtime staged disc has no declared original media inputs"
            )
        completion = validate_completion(directory, reader=read)
        if (
            completion != receipt.get("completion")
            or receipt["process"].get("exit_code") != 0
            or receipt["process"].get("failure")
        ):
            raise ValueError("runtime completion/process observation differs")
        if "origin" in receipt["inputs"]:
            observed = verify_origin(
                Path(receipt["inputs"]["origin"]["path"]),
                receipt["inputs"],
                receipt["emulator"],
                reader=read,
            )
            if observed != receipt.get("origin"):
                raise ValueError("runtime origin consistency changed")
        captures = tuple(
            FileIdentity(directory / row["path"], row["bytes"], row["sha256"])
            for row in completion["captures"]
        )
        return RuntimeCaptureFacts(
            identity,
            completion,
            tuple(item.identity for item in consumed),
            captures,
            "matched",
            (
                "supplied provenance and captures do not justify valid original/candidate execution or universal entry-domain semantics",
            ),
        )


def capture_input(path: Path) -> dict:
    path = path.resolve(strict=True)
    if not path.is_file():
        raise ValueError(f"runtime input must be a regular file: {path}")
    return {
        "path": str(path),
        "bytes": path.stat().st_size,
        "sha256": file_sha256(path),
    }


@bind_deadline
def run_session(
    toolchain: PcsxReduxToolchain,
    *,
    bios: Path,
    executable: Path | None = None,
    script: Path | None = None,
    mission: str | None = None,
    output: Path,
    timeout: float = 60,
    output_limit: int = 2 * 1024 * 1024,
    deadline: float | None = None,
    arguments: list[str] | None = None,
    modules: list[str] | None = None,
    files: list[str] | None = None,
    disc: Path | None = None,
    origin: Path | None = None,
) -> dict:
    if not math.isfinite(timeout) or not 0 < timeout <= 3600:
        raise ValueError(
            "runtime timeout must be finite and between 0 and 3600 seconds"
        )
    if not 1024 <= output_limit <= 16 * 1024 * 1024:
        raise ValueError("runtime output limit must be 1024..16777216 bytes")
    if (script is None) == (mission is None) or (mission is not None and modules):
        raise ValueError(
            "Select one builtin mission or external script; modules require external selection"
        )
    parameters = parse_arguments(arguments)
    assets = collect_assets(modules or [], files or [])
    media, cue = collect_disc(disc)
    toolchain.verify()
    identity = toolchain.runtime_identity()
    canonical_modules = {
        row["name"]
        for row in identity["resources"]["resources"]
        if row["role"] == "module"
    }
    if any(
        key.removeprefix("module:") in canonical_modules
        for key in assets
        if key.startswith("module:")
    ):
        raise ValueError(
            "External modules cannot override canonical native module names"
        )
    if mission is not None:
        rows = identity["resources"]["resources"]
        if not any(row["name"] == mission and row["role"] == "mission" for row in rows):
            raise ValueError("Unknown builtin mission or helper selected as entry")
    inputs = {
        name: capture_input(path)
        for name, path in (
            ("bios", bios),
            ("executable", executable),
            ("script", script),
            ("origin", origin),
        )
        if path is not None
    }
    inputs.update(assets)
    inputs.update(media)
    provenance = verify_origin(origin, inputs, identity) if origin is not None else None
    if inputs["bios"]["bytes"] != 512 * 1024:
        raise ValueError("runtime BIOS must be a 512 KiB PS1 ROM image")
    entry = None
    if executable is not None:
        with Path(inputs["executable"]["path"]).open("rb") as source:
            header = source.read(2048)
        if len(header) != 2048 or header[:8] != b"PS-X EXE":
            raise ValueError("runtime executable must have a PS-X EXE header")
        entry = int.from_bytes(header[16:20], "little")
    # A new directory prevents old captures from satisfying a new mission.
    output = output.absolute()
    output.parent.resolve(strict=True)
    output.mkdir()
    output = output.resolve()
    staged = output / "mission.lua"
    command = toolchain.invocation(
        emulator_arguments(
            output=output,
            bios=inputs["bios"]["path"],
            script=str(staged) if script is not None else None,
            mission=mission,
            executable=inputs["executable"]["path"] if executable is not None else None,
            media_cue=str(output / "disc" / "media.cue") if media else None,
        )
    )
    receipt = {
        "schema": "psx.runtime-session/v2",
        "mission": (
            {"kind": "builtin", "name": mission, "resources": identity["resources"]}
            if mission is not None
            else {
                "kind": "external",
                "script": inputs["script"],
                "modules": {
                    key: row for key, row in inputs.items() if key.startswith("module:")
                },
            }
        ),
        "emulator": identity,
        "inputs": inputs,
        "argv": command,
        "arguments": parameters,
        "origin": provenance,
        "settings": launch_settings(
            "fixed harness launch flags and fresh portable directory"
        ),
        "timeout_seconds": timeout,
        "output_limit": output_limit,
        "status": "running",
        "limitations": list(PROCESS_LIMITATIONS),
    }
    write_json(output / "receipt.json", receipt)
    started = time.monotonic()
    try:
        if script is not None:
            staged.write_bytes(Path(inputs["script"]["path"]).read_bytes())
            if file_sha256(staged) != inputs["script"]["sha256"]:
                raise ValueError("runtime script changed while staging")
        staged_assets = stage_assets(output, assets)
        mounted = stage_disc(output, media, cue)
        if mounted is not None:
            receipt["disc"] = {
                "path": str(mounted.relative_to(output)),
                "sha256": file_sha256(mounted),
            }
        receipt["assets"] = {
            key: str(path.relative_to(output)) for key, path in staged_assets.items()
        }
        verify_disc(output, media, cue)
        environment = mission_environment(
            base=toolchain.environment,
            directory=output,
            arguments=parameters,
            builtin=mission is not None,
            binary_sha256=identity["binary_sha256"],
            source_manifest_sha256=identity["source_manifest_sha256"],
            entry=entry,
            origin_validated=provenance is not None,
            media=encode_bindings(media),
            inputs=[
                (key.split(":", 1)[1], path)
                for key, path in staged_assets.items()
                if key.startswith("file:")
            ],
        )
        result = run_bounded(
            output,
            command,
            timeout=timeout,
            output_limit=output_limit,
            deadline=deadline,
            env=environment,
            supervisor=resolve_isolated_supervisor(),
        )
        (output / "stdout.log").write_text(result.pop("stdout"), encoding="utf-8")
        (output / "stderr.log").write_text(result.pop("stderr"), encoding="utf-8")
        receipt["process"] = result
        receipt["status"] = (
            "passed" if result["exit_code"] == 0 and not result["failure"] else "failed"
        )
        if (
            toolchain.runtime_identity() != identity
            or any(
                capture_input(Path(value["path"])) != value for value in inputs.values()
            )
            or (
                script is not None and file_sha256(staged) != inputs["script"]["sha256"]
            )
            or any(
                file_sha256(path) != assets[key]["sha256"]
                for key, path in staged_assets.items()
            )
        ):
            raise ValueError("runtime inputs changed during execution")
        if receipt["status"] == "passed":
            verify_disc(output, media, cue)
            receipt["completion"] = validate_completion(output)
    except Exception as error:
        receipt["status"] = "failed"
        receipt["error"] = str(error)
        raise
    finally:
        receipt["elapsed_seconds"] = time.monotonic() - started
        write_json(output / "receipt.json", receipt)
    return receipt
