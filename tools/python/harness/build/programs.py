"""Bind declared stage programs and scripts to watched physical lookup generations."""

from __future__ import annotations

import copy
import os
from pathlib import Path

from harness.build.invocation import Invocation
from harness.build.observations import MAX_CANDIDATES, validate_description
from harness.build.runtime import derive_candidates, describe_seeds
from harness.common.deadlines import check_deadline
from harness.common.lookups import LookupBatch
from harness.common.observation import PathWatch
from harness.common.process import resolve_supervisor

_SCHEMA = "bof3.program-inputs/v2"


def describe_requests(invocation: Invocation) -> list[dict]:
    """Derive lookup roles from the same operands and environments used by execution."""
    requests = []
    count = 0
    for stage in invocation.stages:
        check_deadline()
        environment = dict(stage.environment)
        if stage.supervisor != resolve_supervisor():
            raise ValueError("compiler supervisor command changed")
        if not stage.arguments or not isinstance(stage.arguments[0], str):
            raise ValueError("compiler program must be a literal operand")
        if (
            not isinstance(stage.files, tuple)
            or len(set(stage.files)) != len(stage.files)
            or any(
                type(index) is not int or not 0 < index < len(stage.arguments)
                for index in stage.files
            )
        ):
            raise ValueError("compiler script roles differ from their operands")
        roles = [
            ("executable", 0, stage.arguments[0], None),
            ("supervisor", None, stage.supervisor[0], list(stage.supervisor)),
            *(("script", index, stage.arguments[index], None) for index in stage.files),
        ]
        for role, operand, spelling, command in roles:
            if not isinstance(spelling, str) or not spelling or "\0" in spelling:
                raise ValueError("compiler program/script operand is invalid")
            candidates = (
                [spelling]
                if role == "script" or "/" in spelling
                else [
                    os.path.join(entry, spelling)
                    for entry in os.get_exec_path(environment)
                ]
            )
            count += len(candidates)
            if count > MAX_CANDIDATES:
                raise ValueError("compiler executable candidate budget exceeded")
            requests.append(
                {
                    "stage": stage.name,
                    "role": role,
                    "operand": operand,
                    "spelling": spelling,
                    "command": command,
                    "cwd": str(stage.cwd),
                    "candidates": candidates,
                }
            )
    check_deadline()
    return requests


def describe_inputs(invocation: Invocation) -> dict:
    """Bind direct requests and runtime-policy causes in one configured cache key."""
    return {
        "requests": describe_requests(invocation),
        "runtime_seeds": describe_seeds(invocation),
    }


def _capture(root: Path, inputs: dict) -> dict:
    check_deadline()
    requests, seeds = inputs["requests"], inputs["runtime_seeds"]
    with LookupBatch() as batch:
        repository = batch.observe(str(root), cwd=root)
        if repository["status"] != "directory" or repository["terminal"] != str(root):
            raise ValueError("program observation root is not a canonical directory")
        observations = []
        for request in requests:
            check_deadline()
            lookups = [
                batch.observe(name, cwd=Path(request["cwd"]))
                for name in request["candidates"]
            ]
            if request["role"] == "script" and lookups[0]["status"] != "file":
                raise ValueError("compiler script is missing or not a regular file")
            observations.append({**request, "lookups": lookups})
        derived, edges = derive_candidates(root, seeds, observations, batch.describe())
        if (
            sum(len(request["candidates"]) for request in [*requests, *derived])
            > MAX_CANDIDATES
        ):
            raise ValueError("combined program candidate budget exceeded")
        for request in derived:
            check_deadline()
            observations.append(
                {
                    **request,
                    "lookups": [
                        batch.observe(name, cwd=Path(request["cwd"]))
                        for name in request["candidates"]
                    ],
                }
            )
        nodes = batch.describe()
    result = {
        "schema": _SCHEMA,
        "roots": {
            "repository": {"path": str(root), "identity": nodes[str(root)]["identity"]},
            "external": {
                "path": "/",
                "identity": nodes["/"]["identity"],
                "policy": "declared-lookups-only",
                "write_authorized": False,
            },
        },
        "requests": observations,
        "runtime_seeds": seeds,
        "runtime_edges": edges,
        "nodes": {
            name: {
                "root": "repository" if Path(name).is_relative_to(root) else "external",
                **node,
            }
            for name, node in nodes.items()
        },
        "complete": False,
    }
    validate_description(result)
    check_deadline()
    return result


class ProgramSnapshot:
    """Retain one pass generation under live path watches until explicitly closed."""

    def __init__(
        self, root: Path, invocation: Invocation, *, expected: dict | None = None
    ) -> None:
        self.root = root
        self._inputs = describe_inputs(invocation)
        self._watch: PathWatch | None = None
        self._failed = False
        self._closed = False
        try:
            if expected is None:
                self._description = _capture(root, self._inputs)
            else:
                self._description = copy.deepcopy(expected)
                validate_description(self._description)
                if self._description["schema"] != _SCHEMA or self._description["roots"][
                    "repository"
                ]["path"] != str(root):
                    raise ValueError("program verification baseline differs")
            paths = {Path(name) for name in self._description["nodes"] if name != "/"}
            self._watch = PathWatch(paths)
            self.validate(invocation)
        except BaseException:
            self.close()
            raise

    def _require_active(self) -> None:
        try:
            check_deadline()
            if self._closed or self._failed:
                raise ValueError("program snapshot is closed or failed")
        except BaseException:
            self._failed = True
            raise

    def describe(self) -> dict:
        self._require_active()
        return copy.deepcopy(self._description)

    def validate(self, invocation: Invocation) -> None:
        try:
            check_deadline()
            if self._closed or self._failed or self._watch is None:
                raise ValueError("program snapshot is closed or failed")
            self._watch.validate()
            if (
                describe_inputs(invocation) != self._inputs
                or _capture(self.root, self._inputs) != self._description
            ):
                raise ValueError("compiler program inputs changed")
            self._watch.validate()
            check_deadline()
        except BaseException:
            self._failed = True
            raise

    def protect_outputs(self, outputs: list[Path]) -> None:
        try:
            self._require_active()
            for output in outputs:
                for artifact in {output, output.resolve()}:
                    for name, node in self._description["nodes"].items():
                        check_deadline()
                        path = Path(name)
                        if (
                            artifact == path
                            or path.is_relative_to(artifact)
                            or (
                                path.parent == artifact.parent
                                and path.name.startswith(
                                    f".{artifact.name}.transaction-"
                                )
                            )
                            or path.is_relative_to(
                                artifact.parent / "out/reviews/evidence/quarantine"
                            )
                            or (
                                node["kind"] != "directory"
                                and artifact.is_relative_to(path)
                            )
                        ):
                            raise ValueError(
                                "compiler artifact collides with a program dependency"
                            )
            check_deadline()
        except BaseException:
            self._failed = True
            raise

    def protect_temporary(self, directory: Path, prefix: str) -> None:
        """Reject dependencies in the prospective allocation family before creation."""
        try:
            self.protect_outputs([directory / prefix])
            for name in self._description["nodes"]:
                check_deadline()
                path = Path(name)
                for parent in {directory.absolute(), directory.resolve()}:
                    if path.is_relative_to(parent) and path != parent:
                        if path.relative_to(parent).parts[0].startswith(prefix):
                            raise ValueError(
                                "compiler staging family collides with a program dependency"
                            )
            check_deadline()
        except BaseException:
            self._failed = True
            raise

    def close(self) -> None:
        self._closed = True
        if self._watch is not None:
            self._watch.close()


def capture_description(
    root: Path, invocation: Invocation, *, expected: dict | None = None
) -> dict:
    """Freshly capture a generation, optionally checking a known baseline under watches."""
    snapshot = ProgramSnapshot(root, invocation, expected=expected)
    try:
        return snapshot.describe()
    finally:
        snapshot.close()
