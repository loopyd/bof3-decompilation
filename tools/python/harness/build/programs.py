"""Bind declared stage programs and scripts to watched physical lookup generations."""

from __future__ import annotations

import copy
import json
import os
import stat
from pathlib import Path

from harness.build.invocation import Invocation
from harness.common.deadlines import check_deadline
from harness.common.lookups import LookupBatch, resolve_lookup
from harness.common.observation import PathWatch
from harness.common.process import resolve_supervisor

_SCHEMA = "bof3.program-inputs/v1"
_MAX_CANDIDATES = 2048
_MAX_RECORD_BYTES = 512 * 1024


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
            if count > _MAX_CANDIDATES:
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


def _capture(root: Path, requests: list[dict]) -> dict:
    check_deadline()
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


def validate_description(value: object) -> None:
    """Validate retained observational structure without asserting live freshness."""
    check_deadline()
    if (
        not isinstance(value, dict)
        or set(value) != {"schema", "roots", "requests", "nodes", "complete"}
        or value["schema"] != _SCHEMA
        or value["complete"] is not False
    ):
        raise ValueError("program input record fields differ")
    if (
        len(json.dumps(value, sort_keys=True, allow_nan=False).encode())
        > _MAX_RECORD_BYTES
    ):
        raise ValueError("program input record exceeds its byte bound")
    roots, nodes, requests = value["roots"], value["nodes"], value["requests"]
    if (
        not isinstance(roots, dict)
        or set(roots) != {"repository", "external"}
        or not isinstance(nodes, dict)
        or not 1 <= len(nodes) <= 8192
        or not isinstance(requests, list)
        or not requests
    ):
        raise ValueError("program roots or nodes are invalid")
    for kind, description in roots.items():
        fields = {"path", "identity"} | (
            {"policy", "write_authorized"} if kind == "external" else set()
        )
        if (
            not isinstance(description, dict)
            or set(description) != fields
            or not isinstance(description["path"], str)
        ):
            raise ValueError("program root fields differ")
        path = Path(description["path"])
        if (
            not path.is_absolute()
            or str(path) != description["path"]
            or ".." in path.parts
        ):
            raise ValueError("program root is not canonical")
        if kind == "external" and (
            description["path"] != "/"
            or description["policy"] != "declared-lookups-only"
            or description["write_authorized"] is not False
        ):
            raise ValueError("external program root policy differs")
    repository = Path(roots["repository"]["path"])
    total_bytes = 0
    directories = 0
    for name, node in nodes.items():
        check_deadline()
        if not isinstance(name, str) or not isinstance(node, dict):
            raise ValueError("program node is invalid")
        path = Path(name)
        kind = node.get("kind")
        if (
            not path.is_absolute()
            or str(path) != name
            or ".." in path.parts
            or node.get("root")
            != ("repository" if path.is_relative_to(repository) else "external")
        ):
            raise ValueError("program node root binding differs")
        fields = {"root", "kind"}
        if kind != "missing":
            fields.add("identity")
        if kind == "file":
            fields.add("sha256")
        if kind == "symlink":
            fields.add("target")
        if (
            kind not in {"missing", "directory", "file", "symlink", "nonregular"}
            or set(node) != fields
        ):
            raise ValueError("program node fields differ")
        if kind != "missing":
            identity = node["identity"]
            if (
                not isinstance(identity, list)
                or len(identity) != (5 if kind == "directory" else 9)
                or any(type(item) is not int for item in identity)
            ):
                raise ValueError("program node identity is invalid")
            mode = identity[2]
            expected_kind = (
                "directory"
                if stat.S_ISDIR(mode)
                else "file"
                if stat.S_ISREG(mode)
                else "symlink"
                if stat.S_ISLNK(mode)
                else "nonregular"
            )
            if kind != expected_kind:
                raise ValueError("program node kind differs from its mode")
        if kind == "file" and (
            identity[5] != 1
            or identity[6] < 0
            or identity[6] > 64 * 1024 * 1024
            or not isinstance(node["sha256"], str)
            or len(node["sha256"]) != 64
            or any(character not in "0123456789abcdef" for character in node["sha256"])
        ):
            raise ValueError("program file state is invalid")
        if kind == "symlink" and (
            not isinstance(node["target"], str)
            or not node["target"]
            or "\0" in node["target"]
            or len(os.fsencode(node["target"])) > 16384
        ):
            raise ValueError("program alias state is invalid")
        directories += kind == "directory"
        total_bytes += identity[6] if kind == "file" else 0
    if directories > 512 or total_bytes > 256 * 1024 * 1024:
        raise ValueError("program retained capture budget exceeded")
    for description in roots.values():
        node = nodes.get(description["path"])
        if (
            node is None
            or node["kind"] != "directory"
            or node["identity"] != description["identity"]
        ):
            raise ValueError("program root identity is unbound")
    reached = {"/"}

    def observe(parent: Path, component: str) -> dict:
        name = str(parent / component)
        if name not in nodes or nodes[str(parent)]["kind"] != "directory":
            raise ValueError("program lookup traverses an unbound directory")
        reached.add(name)
        return nodes[name]

    anchor = resolve_lookup(str(repository), cwd=repository, observe=observe)
    if anchor["terminal"] != str(repository) or anchor["status"] != "directory":
        raise ValueError("program repository anchor differs")
    count = 0
    for request in requests:
        check_deadline()
        if not isinstance(request, dict) or set(request) != {
            "stage",
            "role",
            "operand",
            "spelling",
            "command",
            "cwd",
            "candidates",
            "lookups",
        }:
            raise ValueError("program request fields differ")
        if request["role"] not in {"executable", "script", "supervisor"} or any(
            not isinstance(request[name], str)
            or not request[name]
            or "\0" in request[name]
            for name in ("stage", "spelling", "cwd")
        ):
            raise ValueError("program request role or spelling is invalid")
        cwd = Path(request["cwd"])
        if not cwd.is_absolute() or str(cwd) != request["cwd"] or ".." in cwd.parts:
            raise ValueError("program request CWD is not canonical")
        candidates, lookups = request["candidates"], request["lookups"]
        if (
            not isinstance(candidates, list)
            or not candidates
            or any(
                not isinstance(name, str) or not name or "\0" in name
                for name in candidates
            )
            or not isinstance(lookups, list)
            or len(candidates) != len(lookups)
        ):
            raise ValueError("program request candidates are invalid")
        count += len(candidates)
        if count > _MAX_CANDIDATES:
            raise ValueError("program request candidate budget exceeded")
        role = request["role"]
        if (role == "script" or "/" in request["spelling"]) and candidates != [
            request["spelling"]
        ]:
            raise ValueError("program literal candidates differ")
        if role == "supervisor":
            command = request["command"]
            if (
                request["operand"] is not None
                or not isinstance(command, list)
                or command != [request["spelling"], "-m", "harness.common.process"]
            ):
                raise ValueError("program supervisor binding differs")
        elif (
            request["command"] is not None
            or type(request["operand"]) is not int
            or (
                request["operand"] != 0
                if role == "executable"
                else request["operand"] <= 0
            )
        ):
            raise ValueError("program operand binding differs")
        for spelling, lookup in zip(candidates, lookups):
            expected = resolve_lookup(spelling, cwd=cwd, observe=observe)
            if lookup != expected:
                raise ValueError("program lookup replay differs")
            if (
                not isinstance(lookup, dict)
                or set(lookup) != {"spelling", "cwd", "trail", "terminal", "status"}
                or lookup["spelling"] != spelling
                or lookup["cwd"] != request["cwd"]
            ):
                raise ValueError("program lookup binding differs")
            if (
                not isinstance(lookup["trail"], list)
                or not 1 <= len(lookup["trail"]) <= 257
                or any(
                    not isinstance(name, str) or name not in nodes
                    for name in lookup["trail"]
                )
                or not isinstance(lookup["terminal"], str)
                or lookup["terminal"] not in nodes
            ):
                raise ValueError("program lookup references missing nodes")
            terminal_kind = nodes[lookup["terminal"]]["kind"]
            if lookup["status"] not in {
                "missing",
                "directory",
                "file",
                "nonregular",
                "not-directory",
            } or (
                terminal_kind not in {"file", "nonregular"}
                if lookup["status"] == "not-directory"
                else lookup["status"] != terminal_kind
            ):
                raise ValueError("program lookup terminal differs")
        if role == "script" and (len(lookups) != 1 or lookups[0]["status"] != "file"):
            raise ValueError("program script is not one regular file")
    if reached != set(nodes):
        raise ValueError("program graph contains undeclared nodes")
    check_deadline()


class ProgramSnapshot:
    """Retain one pass generation under live path watches until explicitly closed."""

    def __init__(self, root: Path, invocation: Invocation) -> None:
        self.root = root
        self._requests = describe_requests(invocation)
        self._watch: PathWatch | None = None
        self._failed = False
        self._closed = False
        try:
            self._description = _capture(root, self._requests)
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
                describe_requests(invocation) != self._requests
                or _capture(self.root, self._requests) != self._description
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


def capture_description(root: Path, invocation: Invocation) -> dict:
    """Return an endpoint-verified configured generation, not a live execution fence."""
    snapshot = ProgramSnapshot(root, invocation)
    try:
        return snapshot.describe()
    finally:
        snapshot.close()
