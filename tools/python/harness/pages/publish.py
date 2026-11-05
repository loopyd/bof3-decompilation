"""Prepare exact-commit pages in isolation and fast-forward the public branch."""

from __future__ import annotations

import fcntl
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

from harness.pages.build import ASSETS


def git(root: Path, *args: str, input: str | None = None, env=None) -> str:
    result = subprocess.run(
        ["git", "-C", str(root), *args],
        input=input,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=env,
    )
    if result.returncode:
        raise RuntimeError(f"git {args[0]} failed: {result.stderr.strip()}")
    return result.stdout.strip()


def build_revision(root: Path, revision: str, output: Path) -> None:
    revision = git(root, "rev-parse", "--verify", f"{revision}^{{commit}}")
    output = output.resolve()
    scratch = root / "out" / "pages"
    scratch.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="build-", dir=scratch) as directory:
        checkout = Path(directory) / "source"
        git(root, "worktree", "add", "--detach", str(checkout), revision)
        try:
            # These are installed tools, never inputs copied into the public site.
            (checkout / "toolchains" / "rizin").symlink_to(
                root / "toolchains" / "rizin", target_is_directory=True
            )
            env = {**os.environ, "PYTHONPATH": str(checkout / "tools/python")}
            command = [
                sys.executable,
                "-m",
                "harness.pages.cli",
                "build",
                "--root",
                str(checkout),
                "--cache",
                str(root),
                "--output",
                str(output),
                "--revision",
                revision,
            ]
            result = subprocess.run(command, cwd=checkout, env=env)
            if result.returncode:
                raise RuntimeError("isolated pages build failed; main push stopped")
        finally:
            git(root, "worktree", "remove", "--force", str(checkout))


def fetch_parent(root: Path, remote: str) -> str | None:
    refs = git(root, "ls-remote", remote, "refs/heads/gh-pages")
    if not refs:
        return None
    git(root, "fetch", "--no-tags", remote, "refs/heads/gh-pages")
    return git(root, "rev-parse", "FETCH_HEAD")


def create_commit(root: Path, output: Path, revision: str, parent: str | None) -> str:
    expected = {*ASSETS, "status.json", ".nojekyll"}
    if {path.name for path in output.iterdir()} != expected:
        raise ValueError("unexpected files in pages output")
    index = output.parent / "git-index"
    env = {**os.environ, "GIT_INDEX_FILE": str(index)}
    git(root, "read-tree", "--empty", env=env)
    for name in sorted(expected):
        path = output / name
        if path.is_symlink() or not path.is_file():
            raise ValueError(f"not a regular site artifact: {name}")
        blob = git(root, "hash-object", "-w", str(path))
        git(
            root,
            "update-index",
            "--add",
            "--cacheinfo",
            f"100644,{blob},{name}",
            env=env,
        )
    tree = git(root, "write-tree", env=env)
    parents = ["-p", parent] if parent else []
    return git(
        root,
        "commit-tree",
        tree,
        *parents,
        input=f"Publish decompilation status for {revision}\n",
    )


def publish(root: Path, remote: str, revision: str) -> None:
    revision = git(root, "rev-parse", "--verify", f"{revision}^{{commit}}")
    scratch = root / "out" / "pages"
    scratch.mkdir(parents=True, exist_ok=True)
    with (scratch / "publish.lock").open("w") as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError as exc:
            raise RuntimeError("another pages publication is running") from exc
        parent = fetch_parent(root, remote)
        if parent:
            previous = json.loads(git(root, "show", f"{parent}:status.json"))
            if previous.get("revision") == revision:
                print("pages: pushed commit is already prepared")
                return
        with tempfile.TemporaryDirectory(prefix="publish-", dir=scratch) as directory:
            output = Path(directory) / "site"
            build_revision(root, revision, output)
            commit = create_commit(root, output, revision, parent)
            git(root, "push", remote, f"{commit}:refs/heads/gh-pages")
            actual = git(root, "ls-remote", remote, "refs/heads/gh-pages").split()[0]
            if actual != commit:
                raise RuntimeError(
                    "remote gh-pages revision changed during publication"
                )
            print(f"pages: prepared gh-pages {commit[:12]} for main {revision[:12]}")


def pre_push(root: Path, remote: str, updates) -> None:
    for update in updates:
        fields = update.split()
        if len(fields) != 4:
            raise ValueError("malformed Git pre-push update")
        _local_ref, revision, remote_ref, _old = fields
        if remote_ref == "refs/heads/main" and set(revision) != {"0"}:
            publish(root, remote, revision)


def install_hook(root: Path) -> None:
    configured = subprocess.run(
        ["git", "-C", str(root), "config", "--get", "core.hooksPath"],
        capture_output=True,
        text=True,
    )
    if configured.returncode not in {0, 1}:
        raise RuntimeError("cannot read core.hooksPath")
    hooks_path = configured.stdout.strip()
    if hooks_path == ".githooks":
        (root / ".githooks" / "pre-push").chmod(0o755)
        print("pages: hook is already installed")
        return
    if hooks_path:
        raise ValueError("existing core.hooksPath must be integrated explicitly")
    hook = Path(git(root, "rev-parse", "--git-path", "hooks/pre-push"))
    if not hook.is_absolute():
        hook = root / hook
    if hook.exists():
        raise ValueError("existing pre-push hook must be integrated explicitly")
    (root / ".githooks" / "pre-push").chmod(0o755)
    git(root, "config", "--local", "core.hooksPath", ".githooks")
    print("pages: installed .githooks/pre-push")
