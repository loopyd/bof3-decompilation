"""Self-contained Git metadata copies and resource-bounded read-only queries."""

from __future__ import annotations

import os
import stat
from contextlib import contextmanager
from pathlib import Path
from tempfile import TemporaryDirectory

from harness.common.deadlines import resolve_deadline
from harness.common.inventory import CaptureBudget, capture_file, scan_tree
from harness.common.process import run_bounded

METADATA_EXCLUSIONS = frozenset({"hooks", "logs", "modules", "worktrees"})
CORE_VALUES = frozenset(
    {
        "core.filemode",
        "core.autocrlf",
        "core.eol",
        "core.ignorecase",
        "core.symlinks",
        "core.precomposeunicode",
        "core.repositoryformatversion",
    }
)
INERT_CORE = frozenset(
    {
        "core.bare",
        "core.worktree",
        "core.fsmonitor",
        "core.hookspath",
        "core.logallrefupdates",
        "core.compression",
        "core.packedgitlimit",
        "core.packedgitwindowsize",
        "core.bigfilethreshold",
        "core.trustctime",
        "core.checkstat",
        "core.preloadindex",
        "core.untrackedcache",
        "core.editor",
        "core.pager",
        "core.quotepath",
        "core.sshcommand",
        "core.warnambiguousrefs",
    }
)
INERT_SECTIONS = frozenset(
    {
        "remote",
        "branch",
        "user",
        "credential",
        "color",
        "init",
        "http",
        "https",
        "safe",
        "url",
        "submodule",
        "fetch",
        "push",
        "pull",
        "gc",
        "maintenance",
        "advice",
        "alias",
        "commit",
        "tag",
        "gpg",
        "sendemail",
        "merge",
        "rerere",
    }
)


class RepositoryCopy:
    """Private query files contain no redirects to mutable dependency metadata."""

    def __init__(self, path: Path, budget: CaptureBudget) -> None:
        self.path = path
        self.budget = budget
        self.gitdir = path / "git"
        self.worktree = path / "work"
        self.gitdir.mkdir()
        self.worktree.mkdir()
        (self.gitdir / "objects").mkdir()
        (self.gitdir / "refs").mkdir()
        (self.gitdir / "HEAD").write_text("ref: refs/heads/empty\n")
        self.write_config({})

    def write_config(self, values: dict[str, str]) -> None:
        sections = {"core": {"bare": "false", "fsmonitor": "false"}}
        for key, value in values.items():
            section, name = key.split(".", 1)
            sections.setdefault(section, {})[name] = value
        text = ""
        for section, entries in sections.items():
            text += f"[{section}]\n"
            for name, value in entries.items():
                escaped = value.replace("\\", "\\\\").replace('"', '\\"')
                escaped = escaped.replace("\n", "\\n").replace("\t", "\\t")
                text += f'\t{name} = "{escaped}"\n'
        (self.gitdir / "config").write_text(text)

    def query(
        self,
        arguments: list[str],
        *,
        input_data: bytes | None = None,
        accepted_codes: tuple[int, ...] = (0,),
    ) -> str:
        self.budget.charge_query()
        if input_data is not None and len(input_data) > 2 * 1024 * 1024:
            raise ValueError("isolated Git query input exceeds limit")
        environment = {
            "PATH": os.defpath,
            "HOME": str(self.path),
            "XDG_CONFIG_HOME": str(self.path),
            "LC_ALL": "C.UTF-8",
            "PYTHONPATH": str(Path(__file__).resolve().parents[2]),
            "PYTHONDONTWRITEBYTECODE": "1",
            "GIT_CONFIG_NOSYSTEM": "1",
            "GIT_CONFIG_GLOBAL": "/dev/null",
            "GIT_OPTIONAL_LOCKS": "0",
            "GIT_NO_LAZY_FETCH": "1",
            "GIT_NO_REPLACE_OBJECTS": "1",
            "GIT_TERMINAL_PROMPT": "0",
            "GIT_ATTR_NOSYSTEM": "1",
            "GIT_CONFIG_SYSTEM": "/dev/null",
        }
        command = [
            "/usr/bin/prlimit",
            "--as=1073741824",
            "--cpu=30",
            "--fsize=0",
            "--",
            "git",
            "--no-optional-locks",
            f"--git-dir={self.gitdir}",
            f"--work-tree={self.worktree}",
            "-c",
            "core.fsmonitor=false",
            "-c",
            "core.hooksPath=/dev/null",
            "-c",
            "submodule.recurse=false",
            "-c",
            "diff.ignoreSubmodules=all",
            "-c",
            "core.attributesFile=/dev/null",
            "-c",
            "core.excludesFile=/dev/null",
            "-c",
            "pack.threads=1",
            "-c",
            "core.commitGraph=false",
            "-c",
            "core.multiPackIndex=false",
            *arguments,
        ]
        result = run_bounded(
            self.path,
            command,
            timeout=30,
            output_limit=2 * 1024 * 1024,
            deadline=resolve_deadline(),
            errors="surrogateescape",
            env=environment,
            input_data=input_data,
        )
        if result["failure"] or result["exit_code"] not in accepted_codes:
            raise ValueError(
                "isolated Git query failed: "
                + str(result["failure"] or result["exit_code"])
            )
        return result["stdout"]

    def parse_config(self, content: bytes) -> dict[str, str]:
        path = self.path / "configuration"
        path.write_bytes(content)
        output = self.query(
            ["config", "--file", str(path), "--no-includes", "--null", "--list"]
        )
        values = {}
        for record in output.split("\0"):
            if not record:
                continue
            key, separator, value = record.partition("\n")
            value = value if separator else "true"
            section = key.split(".", 1)[0]
            if key in CORE_VALUES or key == "extensions.objectformat":
                values[key] = value
            elif key in INERT_CORE or section in INERT_SECTIONS:
                if section == "submodule" and key.endswith(".ignore"):
                    raise ValueError(
                        "submodule ignore configuration requires explicit support"
                    )
                if key == "core.bare" and value.lower() not in {
                    "false",
                    "no",
                    "off",
                    "0",
                }:
                    raise ValueError("bare submodule workspaces are unsupported")
            else:
                raise ValueError(f"unsupported submodule status configuration: {key}")
        return values


def _capture_configuration(copy: RepositoryCopy) -> tuple[dict, list[bytes]]:
    """Capture explicit host config inputs without following includes or redirects."""
    allowed_environment = {
        "GIT_CONFIG_NOSYSTEM",
        "GIT_CONFIG_GLOBAL",
        "GIT_OPTIONAL_LOCKS",
        "GIT_PAGER",
    }
    for key, value in os.environ.items():
        if key.startswith("GIT_") and value and key not in allowed_environment:
            raise ValueError(f"Git environment override prevents capture: {key}")
    global_path = os.environ.get("GIT_CONFIG_GLOBAL")
    if global_path not in {None, "/dev/null"}:
        raise ValueError("custom global Git configuration is unsupported")
    home = Path.home()
    xdg = Path(os.environ.get("XDG_CONFIG_HOME", home / ".config"))
    paths = (
        [] if os.environ.get("GIT_CONFIG_NOSYSTEM") == "1" else [Path("/etc/gitconfig")]
    )
    if global_path is None:
        paths += [xdg / "git/config", home / ".gitconfig"]
    controls = {}
    contents = []
    for path in paths:
        name = path.relative_to("/").as_posix()
        state, content = capture_file(Path("/"), name, copy.budget, missing_ok=True)
        controls[str(path)] = state
        if content is not None:
            contents.append(content)
    for path in (
        xdg / "git/ignore",
        xdg / "git/attributes",
        Path("/etc/gitattributes"),
        Path("/usr/etc/gitattributes"),
    ):
        state, content = capture_file(
            Path("/"),
            path.relative_to("/").as_posix(),
            copy.budget,
            missing_ok=True,
        )
        controls[str(path)] = state
        if content:
            raise ValueError(
                "global Git ignore/attribute files require explicit support"
            )
    return controls, contents


def capture_configuration(copy: RepositoryCopy) -> tuple[dict, dict]:
    """Parse the freshly captured configuration once before isolated queries."""
    controls, contents = _capture_configuration(copy)
    values = {}
    for content in contents:
        values.update(copy.parse_config(content))
    return controls, values


def verify_configuration(copy: RepositoryCopy, expected: dict) -> None:
    """Reobserve configuration bytes and controls without reparsing unchanged input."""
    if _capture_configuration(copy)[0] != expected:
        raise ValueError("host Git configuration changed during capture")


def capture_metadata(root: Path, name: str, copy: RepositoryCopy) -> dict:
    namespace = scan_tree(root, name, copy.budget, excluded=METADATA_EXCLUSIONS)
    records = {}
    for child, identity in namespace.items():
        if not child:
            continue
        if (
            child
            in {
                "commondir",
                "config.worktree",
                "objects/info/alternates",
                "objects/info/http-alternates",
                "info/grafts",
            }
            or child.startswith("refs/replace/")
            or child.endswith(".promisor")
        ):
            raise ValueError(f"unsupported Git metadata input: {child}")
        destination = copy.gitdir / child
        if stat.S_ISDIR(identity["mode"]):
            destination.mkdir(parents=True, exist_ok=True)
            continue
        if not stat.S_ISREG(identity["mode"]):
            raise ValueError(
                f"Git metadata must not contain redirects or special files: {child}"
            )
        record, content = capture_file(
            root, f"{name}/{child}", copy.budget, max_bytes=256 * 1024 * 1024
        )
        if {key: record[key] for key in identity} != identity:
            raise ValueError("Git metadata changed during capture")
        if child == "HEAD" or child.startswith("refs/"):
            reference = content.decode("utf-8", "surrogateescape").strip()
            if reference.startswith("ref: "):
                target = reference[5:]
                parts = target.split("/")
                if (
                    not target.startswith("refs/")
                    or any(
                        not part
                        or part.startswith(".")
                        or part.endswith((".", ".lock"))
                        or ".." in part
                        for part in parts
                    )
                    or any(
                        character.isspace()
                        or character in "~^:?*[\\"
                        or ord(character) < 32
                        or ord(character) == 127
                        for character in target
                    )
                    or "@{" in target
                ):
                    raise ValueError("unsafe symbolic Git reference")
            elif len(reference) not in {40, 64} or any(
                character not in "0123456789abcdef" for character in reference
            ):
                raise ValueError("invalid loose Git reference")
        records[child] = record
        if child == "packed-refs" and any(
            line
            and not line.startswith((b"#", b"^"))
            and line.split(maxsplit=1)[-1].startswith(b"refs/replace/")
            for line in content.splitlines()
        ):
            raise ValueError("packed replacement refs are unsupported")
        destination.parent.mkdir(parents=True, exist_ok=True)
        if (
            child != "config"
            and not child.endswith(".lock")
            and not any(
                "multi-pack-index" in part or part in {"commit-graph", "commit-graphs"}
                for part in Path(child).parts
            )
        ):
            destination.write_bytes(content)
    if "HEAD" not in records or "config" not in records:
        raise ValueError("incomplete submodule Git directory")
    return {"namespace": namespace, "files": records}


def verify_metadata(
    root: Path, name: str, budget: CaptureBudget, expected: dict
) -> None:
    if (
        scan_tree(root, name, budget, excluded=METADATA_EXCLUSIONS)
        != expected["namespace"]
    ):
        raise ValueError("Git metadata namespace changed during capture")
    for child, state in expected["files"].items():
        if (
            capture_file(root, f"{name}/{child}", budget, max_bytes=256 * 1024 * 1024)[
                0
            ]
            != state
        ):
            raise ValueError("Git metadata changed during query")


@contextmanager
def isolate_repository(root: Path, gitdir: str, budget: CaptureBudget):
    with TemporaryDirectory(prefix="bof3-git-snapshot-", dir="/tmp") as temporary:
        copy = RepositoryCopy(Path(temporary), budget)
        host, values = capture_configuration(copy)
        metadata = capture_metadata(root, gitdir, copy)
        state, content = capture_file(root, f"{gitdir}/config", budget)
        if state != metadata["files"]["config"]:
            raise ValueError("Git configuration changed during capture")
        values.update(copy.parse_config(content))
        copy.write_config(values)
        yield copy, {"metadata": metadata, "host": host}
        verify_metadata(root, gitdir, budget, metadata)
        verify_configuration(copy, host)
