"""Run trusted Lua missions through an identified, isolated PCSX-Redux process."""

from __future__ import annotations

import math
import json
import re
from pathlib import Path
import time

from harness.common.process import run_bounded
from harness.io import file_sha256, write_json
from harness.runtime.assets import collect_assets, stage_assets
from harness.runtime.disc import collect_disc, encode_bindings, stage_disc, verify_disc
from harness.runtime.origin import verify_origin
from harness.toolchain.pcsx import PcsxReduxToolchain


def capture_input(path: Path) -> dict:
    path = path.resolve(strict=True)
    if not path.is_file():
        raise ValueError(f"runtime input must be a regular file: {path}")
    return {
        "path": str(path),
        "bytes": path.stat().st_size,
        "sha256": file_sha256(path),
    }


def validate_completion(output: Path) -> dict:
    marker = output / "completion.json"
    if marker.is_symlink() or not marker.is_file() or marker.stat().st_size > 65536:
        raise ValueError(
            "runtime requires a regular completion.json (at most 65536 bytes)"
        )
    completion = json.loads(marker.read_text(encoding="utf-8"))
    if (
        not isinstance(completion, dict)
        or set(completion) != {"schema", "captures"}
        or completion["schema"] != "psx.runtime-completion/v1"
    ):
        raise ValueError("invalid runtime completion schema")
    captures = completion["captures"]
    if not isinstance(captures, list) or len(captures) > 128:
        raise ValueError("runtime captures must be a list of at most 128 files")
    seen = {
        "completion.json",
        "receipt.json",
        "mission.lua",
        "stdout.log",
        "stderr.log",
    }
    verified = []
    total = 0
    for item in captures:
        if not isinstance(item, dict) or set(item) != {"path", "bytes"}:
            raise ValueError("invalid runtime capture descriptor")
        name, size = item["path"], item["bytes"]
        if (
            not isinstance(name, str)
            or not name
            or Path(name).name != name
            or name in seen
            or name in {".", ".."}
        ):
            raise ValueError(
                "runtime capture paths must be unique, unreserved leaf names"
            )
        if type(size) is not int or not 0 <= size <= 64 * 1024 * 1024:
            raise ValueError("runtime capture size must be 0..67108864")
        total += size
        if total > 128 * 1024 * 1024:
            raise ValueError("runtime captures exceed 128 MiB")
        path = output / name
        if path.is_symlink() or not path.is_file() or path.stat().st_size != size:
            raise ValueError(f"missing or invalid runtime capture: {name}")
        seen.add(name)
        verified.append({"path": name, "bytes": size, "sha256": file_sha256(path)})
    return {"schema": completion["schema"], "captures": verified}


def run_session(
    toolchain: PcsxReduxToolchain,
    *,
    bios: Path,
    executable: Path | None = None,
    script: Path,
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
    parameters = {}
    for argument in arguments or []:
        name, separator, value = argument.partition("=")
        if (
            not separator
            or not re.fullmatch(r"[A-Za-z][A-Za-z0-9_]{0,63}", name)
            or "\x00" in value
            or len(value.encode("utf-8")) > 4096
            or name.upper() in parameters
        ):
            raise ValueError(
                "runtime arguments require unique NAME=VALUE pairs (names ignore case)"
            )
        parameters[name.upper()] = value
    if len(parameters) > 64:
        raise ValueError("runtime accepts at most 64 mission arguments")
    assets = collect_assets(modules or [], files or [])
    media, cue = collect_disc(disc)
    toolchain.verify()
    identity = toolchain.runtime_identity()
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
        [
            "-cli",
            "-portable",
            str(output),
            "-interpreter",
            "-softgpu",
            "-debugger",
            "-no-gdb",
            "-no-webserver",
            "-no-pcdrv",
            "-no-fastboot",
            "-noupdate",
            "-run",
            "-bios",
            inputs["bios"]["path"],
            *(["-exe", inputs["executable"]["path"]] if executable is not None else []),
            "-dofile",
            str(staged),
            *(["-iso", str(output / "disc" / "media.cue")] if media else []),
        ]
    )
    receipt = {
        "schema": "psx.runtime-session/v1",
        "emulator": identity,
        "inputs": inputs,
        "argv": command,
        "arguments": parameters,
        "origin": provenance,
        "settings": {
            "schema": "psx.runtime-settings/v1",
            "source": "fixed harness launch flags and fresh portable directory",
            "cpu": "interpreter",
            "gpu": "software",
            "debugger": True,
            "disabled": ["gdb", "webserver", "pcdrv", "fastboot", "updates"],
            "defaults": "pinned emulator build; no preexisting portable configuration",
            "limits": [
                "Not a snapshot of all effective native settings.",
                "Lua may change settings during a mission.",
                "Host audio, physical controllers and scheduler are not pinned.",
                "Origin verification does not establish original runtime settings.",
            ],
        },
        "timeout_seconds": timeout,
        "output_limit": output_limit,
        "status": "running",
        "limitations": [
            "Trusted Lua scripts are not sandboxed.",
            "Process success alone does not establish guest behavior or fidelity.",
        ],
    }
    write_json(output / "receipt.json", receipt)
    started = time.monotonic()
    environment = {
        key: value
        for key, value in toolchain.environment.items()
        if not key.startswith("PSX_RUNTIME_")
    }
    if entry is not None:
        environment["PSX_RUNTIME_ENTRY"] = str(entry)
    if provenance is not None:
        environment["PSX_RUNTIME_ORIGIN_VALIDATED"] = "1"
    environment["PSX_RUNTIME_ARGUMENT_NAMES"] = ",".join(parameters)
    environment["LUA_PATH"] = str(output / "lua" / "?.lua")
    environment["LUA_CPATH"] = ""
    environment.update(
        {f"PSX_RUNTIME_ARG_{key}": value for key, value in parameters.items()}
    )
    try:
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
        environment["PSX_RUNTIME_DIRECTORY"] = str(output)
        environment["PSX_RUNTIME_MEDIA"] = encode_bindings(media)
        environment.update(
            {
                f"PSX_RUNTIME_INPUT_{key.split(':', 1)[1]}": str(path)
                for key, path in staged_assets.items()
                if key.startswith("file:")
            }
        )
        result = run_bounded(
            output,
            command,
            timeout=timeout,
            output_limit=output_limit,
            deadline=deadline,
            env=environment,
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
            or file_sha256(staged) != inputs["script"]["sha256"]
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
