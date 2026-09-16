"""Validate versioned retained compiler program observations without live reads."""

from __future__ import annotations

import json
import os
import stat
from pathlib import Path

from harness.build.runtime import derive_candidates
from harness.common.deadlines import check_deadline
from harness.common.lookups import resolve_lookup

MAX_CANDIDATES = 2048
_MAX_RECORD_BYTES = 512 * 1024


def validate_description(value: object) -> None:
    """Validate retained observational structure without asserting live freshness."""
    check_deadline()
    if (
        not isinstance(value, dict)
        or not isinstance(value.get("schema"), str)
        or value.get("schema")
        not in {"bof3.program-inputs/v1", "bof3.program-inputs/v2"}
        or set(value)
        != (
            {"schema", "roots", "requests", "nodes", "complete"}
            | (
                {"runtime_seeds", "runtime_edges"}
                if value["schema"] == "bof3.program-inputs/v2"
                else set()
            )
        )
        or value["complete"] is not False
    ):
        raise ValueError("program input record fields differ")
    try:
        encoded = json.dumps(value, sort_keys=True, allow_nan=False).encode()
    except (TypeError, ValueError) as error:
        raise ValueError("program input record is not JSON-compatible") from error
    if len(encoded) > _MAX_RECORD_BYTES:
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
        if not isinstance(kind, str):
            raise ValueError("program node kind is invalid")
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
        if (
            not isinstance(request["role"], str)
            or request["role"]
            not in (
                {"executable", "script", "supervisor"}
                | (
                    {"runtime-source", "runtime-executable"}
                    if value["schema"] == "bof3.program-inputs/v2"
                    else set()
                )
            )
            or any(
                not isinstance(request[name], str)
                or not request[name]
                or "\0" in request[name]
                for name in ("stage", "spelling", "cwd")
            )
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
        if count > MAX_CANDIDATES:
            raise ValueError("program request candidate budget exceeded")
        role = request["role"]
        if (
            role in {"script", "runtime-source"} or "/" in request["spelling"]
        ) and candidates != [request["spelling"]]:
            raise ValueError("program literal candidates differ")
        if role == "supervisor":
            command = request["command"]
            if (
                request["operand"] is not None
                or not isinstance(command, list)
                or command != [request["spelling"], "-m", "harness.common.process"]
            ):
                raise ValueError("program supervisor binding differs")
        elif role in {"runtime-source", "runtime-executable"}:
            if request["operand"] is not None or request["command"] is not None:
                raise ValueError("runtime request operand binding differs")
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
    if value["schema"] == "bof3.program-inputs/v2":
        boundary = next(
            (
                index
                for index, request in enumerate(requests)
                if request["role"].startswith("runtime-")
            ),
            len(requests),
        )
        if any(
            not request["role"].startswith("runtime-")
            for request in requests[boundary:]
        ):
            raise ValueError("runtime candidates precede primary requests")
        expected, edges = derive_candidates(
            repository, value["runtime_seeds"], requests[:boundary], nodes
        )
        _validate_edges(value["runtime_edges"], len(edges), boundary, len(requests))
        actual = [
            {key: item for key, item in request.items() if key != "lookups"}
            for request in requests[boundary:]
        ]
        if expected != actual or edges != value["runtime_edges"]:
            raise ValueError("runtime candidate causes differ")
    check_deadline()


def _validate_edges(value: object, count: int, boundary: int, requests: int) -> None:
    if not isinstance(value, list) or len(value) != count:
        raise ValueError("runtime edge count differs")
    for edge in value:
        check_deadline()
        if (
            not isinstance(edge, dict)
            or set(edge)
            != {
                "provider",
                "source",
                "source_sha256",
                "status",
                "requests",
                "unresolved",
            }
            or type(edge["source"]) is not int
            or not 0 <= edge["source"] < boundary
            or not isinstance(edge["requests"], list)
            or any(
                type(index) is not int or not boundary <= index < requests
                for index in edge["requests"]
            )
        ):
            raise ValueError("runtime causal indices are invalid")
