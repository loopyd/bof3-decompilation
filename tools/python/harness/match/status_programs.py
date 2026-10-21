"""Observe ordinary status producers through their configured native bootstrap."""

from __future__ import annotations

import json
import os
from pathlib import Path
import sys

from harness.build.compilation import configured_command
from harness.build.compiler import COMPILER_INPUTS
from harness.build.invocation import Invocation, Stage
from harness.build.profiles import _SUPPORTED_CC_SHA256
from harness.build.programs import ProgramSnapshot, describe_inputs
from harness.common.digests import digest

from harness.common.inputs import InputBatch
from harness.common.observation import PathWatch
from harness.common.process import run_bounded
from harness.io import file_sha256, repo_layout, unique_object

_PYTHON_ENV_SHA256 = "04eee0b601b47d438b012a33dada20d8dfbd1e31ea6377fbc061ea843a6e75bc"

_ROUTING_ENVIRONMENT = frozenset(
    {
        "BOF3_PRESERVATION_RECORD",
        "BOF3_PRESERVATION_FINGERPRINT",
        "BOF3_PRESERVATION_ROUTES",
        "BOF3_PRESERVATION_ROUTES_SHA256",
    }
)


def _compiler_environment() -> dict[str, str]:
    """Environment minus preservation-routing selection variables.

    Those variables choose a preservation record; they are not part of the compiler
    environment and must not change a status identity or its comparison stages.
    """
    return {
        name: value
        for name, value in os.environ.items()
        if name not in _ROUTING_ENVIRONMENT
    }


_INPUTS = (
    *COMPILER_INPUTS,
    ".venv/pyvenv.cfg",
    "tools/python/harness/__init__.py",
    "tools/python/harness/build/__init__.py",
    "tools/python/harness/common/__init__.py",
    "tools/python/harness/domain/__init__.py",
    "tools/python/harness/toolchain/__init__.py",
    "tools/python/harness/match/__init__.py",
    "tools/python/harness/decomp/__init__.py",
    "tools/python/harness/common/cli.py",
    "tools/python/harness/common/artifacts.py",
    "tools/python/harness/domain/policy.py",
    "tools/python/harness/match/_asm_diff_payload.py",
    "tools/python/harness/match/_asm_diff_run.py",
    "tools/python/harness/match/_asm_disasm.py",
    "tools/python/harness/match/_asm_link.py",
    "tools/python/harness/match/_asm_resolve.py",
    "tools/python/harness/match/asm_diff.py",
    "tools/python/harness/match/status_cache.py",
    "tools/python/harness/match/status_programs.py",
    "tools/python/harness/decomp/preflight.py",
    "tools/python/harness/decomp/status.py",
)


def _helper_inputs(root: Path) -> dict:
    with InputBatch(root) as batch:
        result = {
            name: batch.read(root / name, max_bytes=4 << 20, single_link=True)[0]
            for name in _INPUTS
        }
    if any(value is None for value in result.values()):
        raise ValueError("status compiler helper is missing")
    if (
        result["bin/cc"]["sha256"] != _SUPPORTED_CC_SHA256
        or result["bin/python-env"]["sha256"] != _PYTHON_ENV_SHA256
    ):
        raise ValueError("unsupported status compiler bootstrap")
    return result


def _inspection(root: Path, source: Path) -> dict:
    output = root / "build" / source.relative_to(root).with_suffix(".o")
    arguments, environment = configured_command(
        root, source, output, allow_grouped=True
    )
    result = run_bounded(
        root,
        [arguments[0], "--bof3-inspect", *arguments[1:]],
        env=environment,
        output_limit=1 << 20,
    )
    if result["failure"] or result["exit_code"] != 0:
        raise ValueError("configured compiler inspection failed")
    value = json.loads(result["stdout"], object_pairs_hook=unique_object)
    if (
        not isinstance(value, dict)
        or set(value) != {"schema", "interpreter", "recipe", "program_inputs"}
        or value["schema"] != "bof3.compiler-inspection/v1"
        or value["interpreter"] != sys.executable
        or value["interpreter"] != str(root / ".venv/bin/python")
        or value["recipe"]["mode"] != "c"
        or value["recipe"]["source"] != str(source)
        or value["recipe"]["output"] != str(output)
    ):
        raise ValueError("unsupported status compiler inspection")
    return value


def _comparison_inputs(root: Path) -> dict:
    environment = tuple(_compiler_environment().items())
    tools = repo_layout(root).psyqo_toolchain_root / "bin"
    stages = [
        Stage(
            "bootstrap-shell",
            ("/bin/sh", str(root / "bin/cc")),
            environment,
            root,
            files=(1,),
        ),
        Stage(
            "bootstrap-dirname", ("dirname", str(root / "bin/cc")), environment, root
        ),
        *(
            Stage(name, (str(tools / f"mipsel-none-elf-{name}"),), environment, root)
            for name in ("ld", "nm", "objcopy", "objdump")
        ),
    ]
    return describe_inputs(Invocation("status-tools", None, None, tuple(stages)))


def _supported(description: dict) -> None:
    for request in description["requests"]:
        files = [lookup for lookup in request["lookups"] if lookup["status"] == "file"]
        if request["role"] in {"executable", "script", "supervisor"} and not files:
            raise ValueError("status program is missing")
        if (
            request["role"] == "runtime-source"
            and request["spelling"].endswith("/specs")
            and files
        ):
            raise ValueError("external compiler specs disable status reuse")
    if any(edge["status"] != "candidate-only" for edge in description["runtime_edges"]):
        raise ValueError("unsupported status compiler runtime policy")


class StatusPrograms:
    """Keep deduplicated program observations alive through a complete status pass."""

    def __init__(self, root: Path, inspections: object | None = None):
        self.root = root
        self.inspections = inspections
        self.environment = _compiler_environment()
        self.snapshots: dict[str, tuple[dict, ProgramSnapshot, dict]] = {}
        self.watch = PathWatch({root / name for name in _INPUTS})
        try:
            self.helpers = _helper_inputs(root)
            self.common = self._capture(_comparison_inputs(root))
            self.validate()
        except BaseException:
            self.close()
            raise

    def _capture(self, inputs: dict) -> dict:
        key = digest(inputs)
        if key not in self.snapshots:
            snapshot = ProgramSnapshot.from_inputs(self.root, inputs)
            try:
                description = snapshot.describe()
                _supported(description)
            except BaseException:
                snapshot.close()
                raise
            self.snapshots[key] = (inputs, snapshot, description)
        return self.snapshots[key][2]

    def _inspection(self, source: Path) -> dict:
        """Reuse a recorded inspection while its material inputs are unchanged.

        The inspection is a read-only bootstrap query. Its answer depends on the
        source bytes, the resolved ordered arguments and the captured tool identity,
        all of which enter the key; a miss always recomputes.
        """
        output = self.root / "build" / source.relative_to(self.root).with_suffix(".o")
        arguments, _ = configured_command(self.root, source, output, allow_grouped=True)
        key = digest(
            {
                "source": source.relative_to(self.root).as_posix(),
                "source_sha256": file_sha256(source),
                "arguments": arguments,
                "tools": digest({"helpers": self.helpers, "common": self.common}),
            }
        )
        cached = (
            self.inspections.get_inspection(key)
            if self.inspections is not None
            else None
        )
        if cached is not None:
            self.watch.validate()
            return cached
        value = _inspection(self.root, source)
        if self.inspections is not None:
            self.inspections.put_inspection(key, value)
        return value

    def fingerprint(self, source: Path) -> str:
        self.watch.validate()
        inspection = self._inspection(source)
        programs = self._capture(inspection["program_inputs"])
        self.watch.validate()
        return digest(
            {
                "helpers": self.helpers,
                "inspection": inspection,
                "programs": programs,
                "comparison": self.common,
                "environment": digest(_compiler_environment()),
            }
        )

    def validate(self) -> None:
        self.watch.validate()
        if _compiler_environment() != self.environment:
            raise ValueError("status process environment changed")
        if _helper_inputs(self.root) != self.helpers:
            raise ValueError("status implementation inputs changed")
        for inputs, snapshot, _ in self.snapshots.values():
            snapshot.validate_inputs(inputs)
        self.watch.validate()

    def close(self) -> None:
        for _, snapshot, _ in self.snapshots.values():
            snapshot.close()
        self.watch.close()
