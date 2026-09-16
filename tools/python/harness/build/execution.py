"""Bind successful compiler execution edges to a captured invocation recipe."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

from harness.build.invocation import Artifact, Invocation, Stage
from harness.common.deadlines import check_deadline
from harness.common.digests import digest
from harness.common.files import read_file

_SCHEMA = "bof3.compiler-execution/v1"
_LIMIT = 64 * 1024 * 1024


def hash_image(content: bytes) -> str:
    return hashlib.sha256(content).hexdigest()


def _input_names(stage: Stage) -> set[str]:
    return {
        value.name
        for value in stage.arguments
        if isinstance(value, Artifact) and value != stage.output
    }


def _read_inputs(stage: Stage, temporary: Path) -> dict[str, str]:
    return {
        name: hash_image(read_file(temporary, name, max_bytes=_LIMIT))
        for name in sorted(_input_names(stage))
    }


def capture_stage(stage: Stage, temporary: Path) -> tuple[dict, list, dict, str | None]:
    """Capture the actual command mapping and stdin passed to one native boundary."""
    check_deadline()
    arguments, environment = stage.render(temporary)
    content = (
        read_file(temporary, stage.stdin.name, max_bytes=_LIMIT)
        if stage.stdin is not None
        else None
    )
    entry = {
        "kind": "process",
        "name": stage.name,
        "arguments": list(arguments),
        "environment_fingerprint": digest(environment),
        "cwd": str(stage.cwd),
        "stdin": hash_image(content) if content is not None else None,
        "inputs": _read_inputs(stage, temporary),
    }
    return (
        entry,
        arguments,
        environment,
        content.decode("utf-8") if content is not None else None,
    )


def complete_stage(entry: dict, stage: Stage, temporary: Path, stdout: str) -> dict:
    """Finish an entry only after the supervised child returned successfully."""
    check_deadline()
    if _read_inputs(stage, temporary) != entry["inputs"]:
        raise ValueError("compiler generated input changed during execution")
    return {
        **entry,
        "exit_code": 0,
        "stdout": hash_image(stdout.encode("utf-8")),
        "output": (
            hash_image(read_file(temporary, stage.output.name, max_bytes=_LIMIT))
            if stage.output is not None
            else None
        ),
    }


class Execution:
    def __init__(
        self, invocation: Invocation, fingerprint: str, grouped: bool, temporary: Path
    ) -> None:
        self.invocation = invocation
        self.fingerprint = fingerprint
        self.grouped = grouped
        self.temporary = temporary
        self.events: list[dict] = []

    def _product(self, artifact: str) -> str:
        if artifact == "partitioned.s":
            candidates = [
                entry["output"] for entry in self.events if entry["kind"] == "partition"
            ]
        else:
            names = {
                stage.name
                for stage in self.invocation.stages
                if stage.output is not None and stage.output.name == artifact
            }
            candidates = [
                entry["output"]
                for entry in self.events
                if entry["kind"] == "process" and entry["name"] in names
            ]
        if len(candidates) != 1:
            raise ValueError("compiler generated image lacks one completed producer")
        return candidates[0]

    def check_stage(self, stage: Stage, entry: dict) -> None:
        if entry["inputs"] != {
            name: self._product(name) for name in _input_names(stage)
        }:
            raise ValueError("compiler generated input differs from its producer")
        if stage.stdin is not None and entry["stdin"] != self._product(
            stage.stdin.name
        ):
            raise ValueError("compiler stdin differs from its producer")

    def check_publication(self, content: bytes, artifact: str) -> None:
        if hash_image(content) != self._product(artifact):
            raise ValueError("compiler publication differs from its producer")

    def record_partition(self, before: str, after: bytes) -> None:
        if not self.events or self.events[-1].get("stdout") != hash_image(
            before.encode("utf-8")
        ):
            raise ValueError("compiler partition input differs from maspsx output")
        self.events.append(
            {
                "kind": "partition",
                "source": str(self.invocation.source),
                "grouped": self.grouped,
                "input": hash_image(before.encode("utf-8")),
                "output": hash_image(after),
            }
        )

    def record_publication(self, output: Path, content: bytes, artifact: str) -> None:
        self.events.append(
            {
                "kind": "publication",
                "artifact": artifact,
                "path": str(output),
                "sha256": hash_image(content),
            }
        )

    def finish(self) -> dict:
        """Called by the driver only after cleanup and terminal dispatch checks."""
        check_deadline()
        result = {
            "schema": _SCHEMA,
            "invocation_fingerprint": self.fingerprint,
            "recipe_fingerprint": digest(self.invocation.describe()),
            "temporary": str(self.temporary),
            "events": self.events,
            "cleanup": "completed",
        }
        verify_execution(self.invocation, self.fingerprint, self.grouped, result)
        return result


def _checksum(value: object) -> str:
    if not isinstance(value, str) or re.fullmatch(r"[0-9a-f]{64}", value) is None:
        raise ValueError("compiler execution image checksum is invalid")
    return value


def _expect(actual: object, expected: dict) -> None:
    if not isinstance(actual, dict) or digest(actual) != digest(expected):
        raise ValueError(
            "compiler execution event differs from its recipe or image edges"
        )


def verify_execution(
    invocation: Invocation,
    fingerprint: str,
    grouped: bool,
    value: object,
    *,
    outputs: dict[str, str] | None = None,
) -> str:
    """Check retained execution edges, never rerun stages or reread removed staging."""
    check_deadline()
    if not isinstance(value, dict) or set(value) != {
        "schema",
        "invocation_fingerprint",
        "recipe_fingerprint",
        "temporary",
        "events",
        "cleanup",
    }:
        raise ValueError("compiler execution record fields differ")
    temporary_name = value["temporary"]
    if not isinstance(temporary_name, str) or "\x00" in temporary_name:
        raise ValueError("compiler execution staging binding is invalid")
    temporary = Path(temporary_name)
    if (
        not temporary.is_absolute()
        or str(temporary) != temporary_name
        or ".." in temporary.parts
        or not temporary.name.startswith(".bof3-cc-")
    ):
        raise ValueError("compiler execution staging binding is invalid")
    events = value["events"]
    count = len(invocation.stages)
    count += 3 if invocation.mode == "c" else int(invocation.output is not None)
    if not isinstance(events, list) or len(events) != count:
        raise ValueError("compiler execution event sequence is incomplete")
    _expect(
        value,
        {
            "schema": _SCHEMA,
            "invocation_fingerprint": fingerprint,
            "recipe_fingerprint": digest(invocation.describe()),
            "temporary": temporary_name,
            "events": events,
            "cleanup": "completed",
        },
    )
    cursor = 0
    products: dict[str, str] = {}
    stdout = None
    for stage in invocation.stages:
        check_deadline()
        if invocation.mode == "c" and stage.name == "assembler":
            entry = events[cursor]
            if not isinstance(entry, dict):
                raise ValueError("compiler partition evidence is invalid")
            partitioned = _checksum(entry.get("output"))
            _expect(
                entry,
                {
                    "kind": "partition",
                    "source": str(invocation.source),
                    "grouped": grouped,
                    "input": stdout,
                    "output": partitioned,
                },
            )
            products["partitioned.s"] = partitioned
            cursor += 1
        entry = events[cursor]
        if not isinstance(entry, dict):
            raise ValueError("compiler process evidence is invalid")
        arguments, environment = stage.render(temporary)
        stdout = _checksum(entry.get("stdout"))
        produced = _checksum(entry.get("output")) if stage.output is not None else None
        _expect(
            entry,
            {
                "kind": "process",
                "name": stage.name,
                "arguments": arguments,
                "environment_fingerprint": digest(environment),
                "cwd": str(stage.cwd),
                "stdin": products[stage.stdin.name]
                if stage.stdin is not None
                else None,
                "inputs": {
                    name: products[name] for name in sorted(_input_names(stage))
                },
                "exit_code": 0,
                "stdout": stdout,
                "output": produced,
            },
        )
        if stage.output is not None:
            products[stage.output.name] = produced
        cursor += 1
    publications = []
    if invocation.mode == "c":
        publications.append(
            ("compiler.s", invocation.output.with_name(invocation.output.name + ".s"))
        )
    if invocation.output is not None:
        publications.append(
            (
                "direct-output" if invocation.mode == "direct" else "translation.o",
                invocation.output,
            )
        )
    expected_outputs = {}
    for artifact, path in publications:
        checksum = products[artifact]
        _expect(
            events[cursor],
            {
                "kind": "publication",
                "artifact": artifact,
                "path": str(path),
                "sha256": checksum,
            },
        )
        expected_outputs[str(path)] = checksum
        cursor += 1
    if outputs is not None and outputs != expected_outputs:
        raise ValueError("compiler execution publications differ from current outputs")
    check_deadline()
    return digest(value)
