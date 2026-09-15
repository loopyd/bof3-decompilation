"""Implementation-run input binding around native owner gates and publication."""

from __future__ import annotations

import hashlib
import os
import shutil
import sys
from pathlib import Path

from harness.common.digests import digest
from harness.common.deadlines import check_deadline
from harness.common.evidence import write_evidence_output
from harness.common.git import git_index_backup
from harness.common.inputs import file_state, input_state, relative
from harness.common.lease import verify_writer
from harness.io import repo_layout
from harness.toolchain.splat import SplatToolchain

SCHEMA = "bof3.type-execution-context/v1"
BUILD_PATHS = (
    "build/cmake/CMakeCache.txt",
    "build/cmake/build.ninja",
    "build/cmake/CMakeFiles/rules.ninja",
    "build/cmake/CMakeFiles/VerifyGlobs.cmake",
)
OVERRIDES = (
    "PSX_CC_DRIVER",
    "PSX_GCC",
    "PSX_AS",
    "PSX_MASPSX",
    "MASPSX_PYTHON",
    "ASPSX_VERSION",
    "CFLAGS",
    "CPPFLAGS",
    "LDFLAGS",
    "CMAKE_GENERATOR",
    "CMAKE_TOOLCHAIN_FILE",
    "PSX_PYTHON",
    "PYTHONHOME",
    "LD_PRELOAD",
    "LD_LIBRARY_PATH",
    "CC",
    "CXX",
)


def _evidence_paths(value) -> set[str]:
    if isinstance(value, dict):
        if value.get("schema") in {
            f"bof3.{owner}-{kind}/v1"
            for owner in ("type", "macro")
            for kind in ("parent-review", "revalidation-parent-review")
        }:
            from harness.common.review import _validate_parent

            _validate_parent(
                value,
                value["schema"],
                value.get("binding", {}),
                value.get("implementation_run_id"),
            )
            # Parent validation owns canonical absolute reviewer evidence; it is
            # not a repo-relative native input or an authorized mutation path.
            return set().union(
                *(
                    _evidence_paths(v)
                    for k, v in value.items()
                    if k != "review_artifact"
                )
            )
        paths = (
            {value["path"]}
            if "path" in value
            and ("sha256" in value or "expected_ranking_digest" in value)
            else set()
        )
        return paths.union(*(_evidence_paths(v) for v in value.values()))
    if isinstance(value, list):
        return set().union(*(_evidence_paths(v) for v in value))
    return set()


def _participating_targets(manifest: dict, targets: list[str]) -> list[str]:
    from harness.common.participation import resolve_limit
    from harness.domain.ids import normalize_target_id

    if (
        not isinstance(targets, list)
        or not 1 <= len(targets) <= resolve_limit(manifest)
        or any(not isinstance(target, str) for target in targets)
        or targets != sorted(set(targets))
        or not set(manifest["targets"]) <= set(targets)
    ):
        raise ValueError("invalid participating targets")
    for target in targets:
        if normalize_target_id(target).value != target:
            raise ValueError("noncanonical participating target")
    return targets


def capture(
    root: Path, manifest: dict, participating_targets: list[str] | None = None
) -> dict:
    if any(name in os.environ for name in OVERRIDES):
        raise ValueError("execution context does not support toolchain overrides")
    tools = {}
    for name in ("cmake", "ninja", "python3", "git"):
        resolved = shutil.which(name)
        if resolved is None:
            raise ValueError(f"execution context requires installed {name}")
        path = Path(resolved).resolve(strict=True)
        tools[name] = {"path": str(path), "state": file_state(path)}
    splat = SplatToolchain(repo_layout(root))
    launcher = splat.executable
    python = splat.python
    if file_state(launcher) is None or not os.access(launcher, os.X_OK):
        raise ValueError("execution context requires installed Splat launcher")
    # ponytail: direct project-Python shebang only; other launchers need an owner.
    if launcher.read_bytes().partition(b"\n")[0] != os.fsencode(f"#!{python}"):
        raise ValueError("execution context requires project-Python Splat launcher")
    config = root / ".venv/pyvenv.cfg"
    if (
        file_state(config) is None
        or "include-system-site-packages = false" not in config.read_text().splitlines()
    ):
        raise ValueError("execution context requires isolated project Python")
    tools[".venv/bin/python"] = {
        "path": str(python.resolve(strict=True)),
        "state": file_state(python.resolve(strict=True)),
    }
    paths = set(manifest["allowed_paths"]) | _evidence_paths(manifest)
    inputs = {}
    targets = manifest["targets"]
    if participating_targets is not None:
        targets = _participating_targets(manifest, participating_targets)
        from harness.domain.manifests import load_target_manifests

        if not set(targets) <= set(load_target_manifests(root)):
            raise ValueError("unknown participating target")
    for target in targets:
        inputs.update(input_state(root, target, paths))
    inputs[launcher.relative_to(root).as_posix()] = file_state(launcher)
    inputs[".venv/pyvenv.cfg"] = file_state(config)
    # Installed imports and .pth/editable hooks are runtime inputs, not just the
    # source submodules. Bytecode caches are derived execution output.
    for path in (root / ".venv/lib").rglob("*"):
        if path.is_file() and "__pycache__" not in path.parts:
            inputs[path.relative_to(root).as_posix()] = file_state(path)
    inputs["out/index/reverse.sqlite"] = file_state(root / "out/index/reverse.sqlite")
    index = git_index_backup(root)
    return {
        **(
            {"participating_targets": targets}
            if participating_targets is not None
            else {}
        ),
        "root": str(root.resolve()),
        "inputs": inputs,
        "environment_digest": digest(dict(os.environ)),
        "tools": tools,
        "python": {"path": sys.executable, "version": sys.version},
        "git_index": None
        if index is None
        else {
            "path": str(index.path),
            "state": file_state(index.path),
        },
    }


def build_state(root: Path) -> dict:
    # ponytail: native Ninja only; other generators need an explicit closure owner.
    if (root / "build/cmake/Makefile").exists():
        raise ValueError("execution context requires Ninja build state")
    paths = set(BUILD_PATHS) | {
        path.relative_to(root).as_posix()
        for path in (root / "build/cmake").rglob("*")
        if path.suffix in {".cmake", ".ninja"}
    }
    return {name: file_state(root / name) for name in sorted(paths)}


def begin(
    root: Path,
    manifest: dict,
    implementation_run_id: str | None,
    participating_targets: list[str] | None = None,
):
    if implementation_run_id is None:
        if participating_targets is not None:
            raise ValueError("participating targets require an implementation run ID")
        return None
    if not isinstance(implementation_run_id, str) or not implementation_run_id.strip():
        raise ValueError("implementation run ID must be nonempty")
    return {
        "schema": SCHEMA,
        "implementation_run_id": implementation_run_id,
        "manifest_digest": manifest["digest"],
        "request_digest": digest(manifest["request"]),
        "adopted_baseline": manifest["workspace_baseline"],
        "initial_state": capture(root, manifest, participating_targets),
        "initial_build": build_state(root),
        "final_state": None,
        "final_build": None,
        "gates": [],
    }


def applied(
    root: Path,
    manifest: dict,
    context,
    changes: dict,
    *,
    deletions: set[str] | None = None,
) -> None:
    if deletions is not None:
        if (
            not isinstance(deletions, set)
            or any(
                not isinstance(name, str) or relative(name) != name for name in changes
            )
            or not set(changes) <= set(manifest["allowed_paths"])
            or {name for name, text in changes.items() if text is None} != deletions
            or any(
                text is not None and not isinstance(text, str)
                for text in changes.values()
            )
        ):
            raise ValueError("execution context requires exact owned deletion opt-in")
        if deletions and context is None:
            raise ValueError("execution context deletions require captured PRE inputs")
    if context is None:
        return
    if deletions is None and any(
        not isinstance(text, str) for text in changes.values()
    ):
        raise ValueError(
            "execution context requires text unless deletions are explicit"
        )
    current = capture(
        root, manifest, context["initial_state"].get("participating_targets")
    )
    expected = {
        **context["initial_state"],
        "inputs": dict(context["initial_state"]["inputs"]),
    }
    for name, text in changes.items():
        if name not in expected["inputs"] or name not in current["inputs"]:
            raise ValueError("execution context intended edit lacks captured input")
        actual = current["inputs"][name]
        if deletions is not None and name in deletions:
            if expected["inputs"][name] is None or actual is not None:
                raise ValueError("execution context intended deletion mismatch")
            expected["inputs"][name] = None
            continue
        if (
            actual is None
            or (
                context["initial_state"]["inputs"][name] is not None
                and actual["mode"] != context["initial_state"]["inputs"][name]["mode"]
            )
            or actual["sha256"] != hashlib.sha256(text.encode()).hexdigest()
        ):
            raise ValueError("execution context intended edit mismatch")
        expected["inputs"][name] = actual
    if current != expected or build_state(root) != context["initial_build"]:
        raise ValueError("execution context input drift during application")
    context["final_state"] = current
    context["final_build"] = context["initial_build"]


def recheck(root: Path, manifest: dict, context) -> None:
    check_deadline()
    verify_writer(root)
    if context is not None and (
        capture(root, manifest, context["initial_state"].get("participating_targets"))
        != context["final_state"]
        or build_state(root) != context["final_build"]
    ):
        raise ValueError("execution context input or build drift")
    check_deadline()


def checked(root: Path, manifest: dict, context, check: dict, receipt: dict) -> None:
    if context is None:
        return
    if (
        capture(root, manifest, context["initial_state"].get("participating_targets"))
        != context["final_state"]
    ):
        raise ValueError("execution context input drift during gate")
    after = build_state(root)
    before = context["final_build"]
    if after != before and check["argv"][0] != "bin/build":
        raise ValueError("execution context unexpected build mutation")
    context["gates"].append(
        {
            "receipt_digest": receipt["digest"],
            "state_digest": digest(context["final_state"]),
            "build_before": before,
            "build_after": after,
        }
    )
    context["final_build"] = after


def validate_context(root: Path, application: dict) -> None:
    if "review_context" not in application:
        return
    _validate_context_history(application)
    recheck(root, application["manifest"], application["review_context"])


def _validate_context_history(application: dict) -> None:
    """Validate retained transitions only; never assert current input validity."""
    context = application["review_context"]
    expected = {
        "schema",
        "implementation_run_id",
        "manifest_digest",
        "request_digest",
        "adopted_baseline",
        "initial_state",
        "initial_build",
        "final_state",
        "final_build",
        "gates",
    }
    if (
        not isinstance(context, dict)
        or set(context) != expected
        or context["schema"] != SCHEMA
    ):
        raise ValueError("invalid execution context schema")
    manifest = application["manifest"]
    if not isinstance(context["gates"], list) or len(context["gates"]) != len(
        manifest["required_checks"]
    ):
        raise ValueError("execution context gate count mismatch")
    for gate in context["gates"]:
        _keys(gate, {"receipt_digest", "state_digest", "build_before", "build_after"})
    if (
        not isinstance(context["implementation_run_id"], str)
        or not context["implementation_run_id"].strip()
        or context["manifest_digest"] != manifest["digest"]
        or context["request_digest"] != digest(manifest["request"])
        or context["adopted_baseline"] != manifest["workspace_baseline"]
        or [g["receipt_digest"] for g in context["gates"]]
        != application["receipt_digests"]
    ):
        raise ValueError("execution context binding mismatch")
    initial = context["initial_state"]
    final = context["final_state"]
    state_keys = {
        "root",
        "inputs",
        "environment_digest",
        "tools",
        "python",
        "git_index",
    }
    for state in (initial, final):
        _keys(
            state,
            state_keys
            | (
                {"participating_targets"} if "participating_targets" in state else set()
            ),
        )
        if "participating_targets" in state:
            _participating_targets(manifest, state["participating_targets"])
        if not isinstance(state["inputs"], dict):
            raise ValueError("invalid execution input map")
        for name, value in state["inputs"].items():
            from harness.common.directory import validate_repo_path

            validate_repo_path(name)
            _file_fact(value)
        _keys(state["python"], {"path", "version"})
        if not isinstance(state["tools"], dict):
            raise ValueError("invalid execution tools")
        for tool in state["tools"].values():
            _keys(tool, {"path", "state"})
            _file_fact(tool["state"])
        if state["git_index"] is not None:
            _keys(state["git_index"], {"path", "state"})
            _file_fact(state["git_index"]["state"])
    if {k: v for k, v in initial.items() if k != "inputs"} != {
        k: v for k, v in final.items() if k != "inputs"
    }:
        raise ValueError("execution environment transition mismatch")
    before, after = initial["inputs"], final["inputs"]
    changed = set(application["changed_paths"])
    if (
        set(before) != set(after)
        or {p for p in before if before[p] != after[p]} != changed
    ):
        raise ValueError("execution input transition mismatch")
    for name in manifest["allowed_paths"]:
        for state, expected in (
            (before, application["pre_state"]),
            (after, application["post_state"]),
        ):
            if (None if state[name] is None else state[name]["sha256"]) != expected[
                name
            ]:
                raise ValueError("execution application state mismatch")
        if (
            before[name] is not None
            and after[name] is not None
            and before[name]["mode"] != after[name]["mode"]
        ):
            raise ValueError("execution input mode transition mismatch")
    prior = context["initial_build"]
    for gate, check in zip(context["gates"], manifest["required_checks"]):
        _keys(gate, {"receipt_digest", "state_digest", "build_before", "build_after"})
        for build in (gate["build_before"], gate["build_after"]):
            if not isinstance(build, dict) or not set(BUILD_PATHS) <= set(build):
                raise ValueError("invalid execution build closure")
            for name in build:
                from harness.common.directory import validate_repo_path

                validate_repo_path(name)
                if not name.startswith("build/cmake/"):
                    raise ValueError("invalid execution build path")
            for fact in build.values():
                _file_fact(fact)
        if (
            gate["build_before"] != prior
            or gate["state_digest"] != digest(final)
            or (gate["build_after"] != prior and check["argv"][0] != "bin/build")
        ):
            raise ValueError("execution gate transition mismatch")
        prior = gate["build_after"]
    if prior != context["final_build"]:
        raise ValueError("execution final build mismatch")


def _keys(value, expected: set[str]) -> None:
    if not isinstance(value, dict) or set(value) != expected:
        raise ValueError("invalid execution context keys")


def _file_fact(value) -> None:
    import re

    if value is None:
        return
    _keys(value, {"sha256", "mode"})
    if (
        not isinstance(value["sha256"], str)
        or not re.fullmatch(r"[0-9a-f]{64}", value["sha256"])
        or type(value["mode"]) is not int
        or not 0 <= value["mode"] <= 0o7777
    ):
        raise ValueError("invalid execution file state")


def publish(root: Path, application: dict, output: str | None) -> None:
    """Publish inside the owner's rollback boundary; failed evidence stays retained."""
    check_deadline()
    verify_writer(root)
    if output is not None:
        validate_context(root, application)
        verify_writer(root)
        check_deadline()
        write_evidence_output(root, output, application)
        validate_context(root, application)
    check_deadline()
