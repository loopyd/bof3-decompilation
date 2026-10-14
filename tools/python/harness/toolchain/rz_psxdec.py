"""Build and verify the sibling RetDec forks as one explicitly loaded DSO."""

from __future__ import annotations

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile

from harness.common.digests import file_record
from harness.common.process import capture
from harness.common.resources import select_build_jobs
from harness.io import RepoLayout
from .base import Toolchain
from .nativebuild import (
    PREREQUISITES,
    archive_manifest,
    compiler_paths,
    link_paths,
    fork_record,
    read_native_build,
    source_changes,
    tree_manifest,
)
from .releases import extract_archive
from .rizin import RizinToolchain


def _records(paths) -> list[dict]:
    return [file_record(path) for path in sorted(set(paths))]


class RzPsxdecToolchain(Toolchain):
    label = "rz-psxdec"

    def __init__(self, layout: RepoLayout) -> None:
        super().__init__(layout)
        self.directory = layout.build_dir / "third_party/rz-psxdec"
        self.receipt: Path | None = None
        self.inputs: dict | None = None

    def _sources(self) -> dict:
        return {
            "plugin": fork_record(self.layout.third_party_dir / "rz-psxdec"),
            "retdec": fork_record(self.layout.third_party_dir / "retdec-psx"),
        }

    def _tools(self) -> dict:
        paths = {
            name: Path("/usr/bin") / name
            for name in (
                "cmake",
                "make",
                "ar",
                "ranlib",
                "ld",
                "autoconf",
                "automake",
                "aclocal",
                "libtoolize",
                "pkg-config",
                "git",
                "ldd",
            )
        }
        paths.update(
            cc=Path("/usr/bin/cc"),
            cxx=Path("/usr/bin/c++"),
            python=Path(sys.executable).resolve(),
            rizin=RizinToolchain(self.layout).executable,
            driver=Path(__file__).resolve(),
            receipt_reader=Path(__file__).with_name("nativebuild.py").resolve(),
        )
        for role in ("cc1", "cc1plus", "collect2"):
            path = subprocess.check_output(
                [str(paths["cxx"]), f"-print-prog-name={role}"], text=True
            ).strip()
            paths[role] = Path(path).resolve()
        return {name: file_record(path) for name, path in paths.items()}

    def _environment(self, directory: Path) -> dict[str, str]:
        return {
            "PATH": "/usr/bin:/bin",
            "LC_ALL": "C",
            "HOME": str(directory / "home"),
            "RZ_NOPLUGINS": "1",
        }

    def _execute(
        self,
        argv: list[str],
        directory: Path,
        name: str,
        *,
        jobs: int | None = None,
    ) -> dict:
        environment = self._environment(directory)
        if jobs is not None:
            environment.update(
                CMAKE_BUILD_PARALLEL_LEVEL=str(jobs),
                OMP_NUM_THREADS=str(jobs),
                OMP_THREAD_LIMIT=str(jobs),
            )
        for suffix in ("stdout.bin", "stderr.bin", "json"):
            (directory / f"{name}.{suffix}").unlink(missing_ok=True)
        record = capture(
            directory,
            argv,
            environment=environment,
            output_limit=128 << 20,
            name=name,
        )
        result = {key: record[key] for key in ("argv", "cwd", "raw_exit", "failure")}
        result["log"] = file_record(directory / f"{name}.json")
        if record["failure"] is not None or record["raw_exit"] != 0:
            raise RuntimeError(
                f"rz-psxdec {name} failed; retained evidence at {directory / (name + '.json')}"
            )
        return result

    def _select_receipt(self) -> Path:
        path = self.directory / "native-build.json"
        if not path.is_file():
            raise FileNotFoundError(
                "missing rz-psxdec build receipt; run bin/harness setup --component rz-psxdec"
            )
        return path

    def _external_inputs(self, tools: dict) -> list[dict]:
        paths = set()
        prefix = RizinToolchain(self.layout).prefix
        paths.update(
            path for path in (prefix / "include").rglob("*.h") if path.is_file()
        )
        paths.update(
            path for path in (prefix / "lib/cmake").rglob("*") if path.is_file()
        )
        compiler = tools["cxx"]["path"]
        for library in (
            "libstdc++fs.a",
            "libstdc++.so",
            "libgcc.a",
            "libgcc_s.so.1",
            "libc.so",
            "libc.so.6",
            "libm.so",
            "libm.so.6",
            "libz.so",
            "libz.so.1",
        ):
            path = Path(
                subprocess.check_output(
                    [compiler, f"-print-file-name={library}"], text=True
                ).strip()
            )
            if path.is_file():
                paths.add(path.absolute())
        for name in ("rizin", "cmake", "cc", "cxx", "cc1", "cc1plus", "ld", "make"):
            result = subprocess.run(
                ["/usr/bin/ldd", tools[name]["path"]],
                capture_output=True,
                text=True,
                check=True,
            )
            for token in result.stdout.split():
                if token.startswith("/") and Path(token).is_file():
                    paths.add(Path(token))
        return _records(path.resolve() for path in paths)

    def install(self, *, force: bool = False) -> str:
        if not force:
            try:
                receipt = self._select_receipt()
                read_native_build(receipt)
                self.receipt = receipt
                return str(self.receipt)
            except (
                OSError,
                ValueError,
                KeyError,
                TypeError,
                subprocess.SubprocessError,
            ):
                pass
        self.receipt = None
        self.inputs = None
        sources = self._sources()
        tools = self._tools()
        if force:
            for name in ("build", "prerequisites", "prefix"):
                shutil.rmtree(self.directory / name, ignore_errors=True)
        self.directory.mkdir(parents=True, exist_ok=True)
        (self.directory / "home").mkdir(exist_ok=True)
        (self.directory / "native-build.json").unlink(missing_ok=True)
        prerequisites = {}
        for role, (relative, expected, directory) in PREREQUISITES.items():
            archive = Path(
                os.environ.get(f"{role}_ARCHIVE", str(self.layout.root / relative))
            ).resolve()
            before = file_record(archive)
            if before["sha256"] != expected:
                raise ValueError(f"{role} archive differs from the pinned dependency")
            pristine = archive_manifest(archive, directory)
            extracted = self.directory / "prerequisites" / role.lower()
            root = extracted / directory
            if not root.is_dir():
                shutil.rmtree(extracted, ignore_errors=True)
                extract_archive(archive, extracted)
                with zipfile.ZipFile(archive) as contents:
                    for member in contents.infolist():
                        if not member.is_dir() and (member.external_attr >> 16) & 0o111:
                            (extracted / member.filename).chmod(0o755)
            if role != "YARA" and tree_manifest(root) != pristine:
                raise ValueError(
                    f"{role} cached source differs; rerun setup with --force"
                )
            if file_record(archive) != before:
                raise ValueError(f"{role} archive changed while preparing its source")
            prerequisites[role] = {
                "root": str(root),
                "archive": before,
                "before": pristine,
                "preparation": {
                    "operation": "harness.toolchain.releases.extract_archive",
                    "executable_modes": "ZIP external_attr",
                },
            }
        self.inputs = {
            "sources": sources,
            "tools": tools,
            "external": self._external_inputs(tools),
            "prerequisites": prerequisites,
        }
        (self.directory / "staged-inputs.json").write_text(
            json.dumps(self.inputs, indent=2) + "\n"
        )
        return str(self.directory)

    def _probe(self, dso: Path, directory: Path) -> dict:
        directory.mkdir(parents=True, exist_ok=True)
        (directory / "home").mkdir(exist_ok=True)
        return self._execute(
            [
                str(RizinToolchain(self.layout).executable),
                "-N",
                "-n",
                "-q",
                "-l",
                str(dso),
                "-c",
                "pdza?",
            ],
            directory,
            "probe",
        )

    def build(self) -> str:
        if self.receipt is not None:
            return str(self.receipt)
        if self.inputs is None:
            raise RuntimeError("rz-psxdec inputs have not been staged")
        jobs = select_build_jobs()
        directory, inputs = self.directory, self.inputs
        build = directory / "build"
        tools = inputs["tools"]
        cxx = tools["cxx"]["path"]
        filesystem = Path(
            subprocess.check_output(
                [cxx, "-print-file-name=libstdc++fs.a"], text=True
            ).strip()
        )
        if not filesystem.is_file():
            raise FileNotFoundError("the system compiler cannot locate stdc++fs")
        command = [
            tools["cmake"]["path"],
            "-S",
            inputs["sources"]["plugin"]["root"],
            "-B",
            str(build),
            "-G",
            "Unix Makefiles",
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_INSTALL_PREFIX={directory / 'prefix'}",
            f"-DCMAKE_C_COMPILER={tools['cc']['path']}",
            f"-DCMAKE_CXX_COMPILER={cxx}",
            f"-DCMAKE_MAKE_PROGRAM={tools['make']['path']}",
            f"-DCMAKE_PREFIX_PATH={RizinToolchain(self.layout).prefix}",
            f"-DCMAKE_LIBRARY_PATH={filesystem.parent}",
            "-DBUILD_BUNDLED_RETDEC=ON",
            "-DBUILD_CUTTER_PLUGIN=OFF",
            "-DRETDEC_ENABLE_ALL=OFF",
            "-DRETDEC_ENABLE_RETDEC=ON",
            "-DRETDEC_TESTS=OFF",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
            f"-DRETDEC_SOURCE_DIR={inputs['sources']['retdec']['root']}",
            f"-DRZ_PSXDEC_REVISION={inputs['sources']['plugin']['commit']}",
            f"-DRETDEC_PSX_REVISION={inputs['sources']['retdec']['commit']}",
        ]
        command += [
            f"-D{role}_LOCAL_DIR={row['root']}"
            for role, row in inputs["prerequisites"].items()
        ]
        configure = self._execute(command, directory, "configure", jobs=jobs)
        configured = link_paths(build, Path(cxx))
        configured = {path for path in configured if not path.is_relative_to(build)}
        inputs["external"] = _records(
            [*(Path(row["path"]) for row in inputs["external"]), *configured]
        )
        (directory / "external-inputs-before-build.json").write_text(
            json.dumps(inputs["external"], indent=2) + "\n"
        )
        execution = self._execute(
            [
                command[0],
                "--build",
                str(build),
                "--parallel",
                str(jobs),
                "--target",
                "rz-psxdec",
            ],
            directory,
            "build",
            jobs=jobs,
        )
        sources_after = self._sources()
        if sources_after != inputs["sources"] or self._tools() != tools:
            raise ValueError("owned sources or native tools changed during the build")
        external_after = _records(Path(row["path"]) for row in inputs["external"])
        if external_after != inputs["external"]:
            raise ValueError("external native inputs changed during the build")
        dso = file_record(build / "src/rz-plugin/rz-psxdec.so")
        prerequisites = inputs["prerequisites"]
        for role, row in prerequisites.items():
            row["after"] = tree_manifest(Path(row["root"]))
            row["mutations"] = source_changes(row["before"], row["after"])
            if role != "YARA" and row["mutations"]:
                raise ValueError(f"unexpected {role} source mutation")
        sources = {
            role: {
                "root": before["root"],
                "commit": before["commit"],
                "commit_after": sources_after[role]["commit"],
                "before": before["files"],
                "after": sources_after[role]["files"],
            }
            for role, before in inputs["sources"].items()
        }
        probe = self._probe(Path(dso["path"]), directory / "probe")
        roots = tuple(Path(row["root"]) for row in sources.values())
        roots += tuple(Path(row["root"]) for row in prerequisites.values())
        consumed = _records(compiler_paths(build, roots))
        links = _records(link_paths(build, Path(cxx)))
        document = {
            "schema": "bof3.rz-psxdec-build/v1",
            "build_root": str(build),
            "sources": sources,
            "configure": configure,
            "build": execution,
            "probe": probe,
            "prerequisites": prerequisites,
            "tools_before": tools,
            "tools_after": self._tools(),
            "external_inputs_before": inputs["external"],
            "external_inputs_after": external_after,
            "compiler_inputs": consumed,
            "compiler_inputs_observation": "post-build CMake/YARA depfiles",
            "link_inputs": links,
            "build_files": tree_manifest(build),
            "dso": dso,
        }
        pending = directory / "native-build.pending.json"
        pending.write_text(json.dumps(document, indent=2, sort_keys=True) + "\n")
        read_native_build(pending)
        receipt = directory / "native-build.json"
        os.replace(pending, receipt)
        self.receipt = receipt
        return str(receipt)

    def verify(self) -> str:
        receipt = self.receipt or self._select_receipt()
        native = read_native_build(receipt)
        if self.inputs is None:
            directory = receipt.parent / "verify"
            execution = self._probe(native.dso.path, directory)
            streams = json.loads(Path(execution["log"]["path"]).read_bytes())["streams"]
            output = b"".join(
                Path(row["path"]).read_bytes() for row in streams.values()
            )
            if b"pdzar" not in output:
                raise ValueError(
                    "explicit DSO load did not expose the original-range command"
                )
            read_native_build(receipt)
        return f"{self.label}; DSO={native.dso.path}; receipt={receipt}"
