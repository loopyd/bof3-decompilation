from __future__ import annotations

import hashlib
import json
import os
import subprocess

import pytest

from harness.io import repo_layout
from harness.toolchain import pcsx


def git(directory, *arguments):
    environment = {
        key: value for key, value in os.environ.items() if not key.startswith("GIT_")
    }
    environment.update(
        GIT_CONFIG_GLOBAL="/dev/null", GIT_CONFIG_NOSYSTEM="1", GIT_TERMINAL_PROMPT="0"
    )
    return (
        subprocess.run(
            ["git", "-C", str(directory), *arguments],
            check=True,
            capture_output=True,
            timeout=30,
            env=environment,
        )
        .stdout.decode()
        .strip()
    )


def commit(directory):
    git(
        directory,
        "-c",
        "user.name=Source guard fixture",
        "-c",
        "user.email=fixture@example.invalid",
        "-c",
        "commit.gpgsign=false",
        "commit",
        "-qm",
        "fixture",
    )
    return git(directory, "rev-parse", "HEAD")


@pytest.fixture
def source_workspace(tmp_path):
    root = tmp_path.resolve()
    source = pcsx.PcsxReduxToolchain.submodule
    fork = root / source
    children = {"imgui": "imgui.cpp", "luajit": "src/lj_dispatch.c"}
    for directory, tracked in [(root, "tracked.txt"), (fork, "src/tracked.cc")]:
        directory.mkdir(parents=True, exist_ok=True)
        git(directory, "init", "-q")
        git(directory, "config", "core.autocrlf", "false")
        (directory / ".gitignore").write_text(
            "*.cc\n*.h\nMakefile\nignored.lua\nvector\n"
        )
        path = directory / tracked
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("tracked source\n")
        git(directory, "add", "--force", ".gitignore", tracked)
    for name, tracked in children.items():
        directory = fork / "third_party" / name
        directory.mkdir(parents=True)
        git(directory, "init", "-q")
        git(directory, "config", "core.autocrlf", "false")
        (directory / ".gitignore").write_text("*.h\n")
        path = directory / tracked
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("nested tracked source\n")
        git(directory, "add", "--force", ".gitignore", tracked)
        revision = commit(directory)
        git(
            fork,
            "update-index",
            "--add",
            "--cacheinfo",
            f"160000,{revision},third_party/{name}",
        )
    (fork / ".gitmodules").write_text(
        "".join(
            f'[submodule "{name}"]\n\tpath = third_party/{name}\n\turl = fixture-only\n'
            for name in children
        )
    )
    git(fork, "add", ".gitmodules")
    revision = commit(fork)
    (root / ".gitmodules").write_text(
        f'[submodule "fork"]\n\tpath = {source}\n\turl = fixture-only\n'
    )
    git(root, "add", ".gitmodules")
    git(root, "update-index", "--add", "--cacheinfo", f"160000,{revision},{source}")
    commit(root)
    layout = repo_layout(root)
    identity = pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.RUNTIME)
    assert identity["revision"] == revision
    assert identity["generated_sources"] == {}
    assert [row["path"] for row in identity["source_forests"]] == [
        source,
        f"{source}/third_party/imgui",
        f"{source}/third_party/luajit",
    ]
    return layout, source


@pytest.mark.parametrize(
    "name",
    [
        "src/core/extra.cc",
        "src/hidden.h",
        "Makefile",
        "ignored.lua",
        "vector",
        "src/vector",
    ],
)
def test_untracked_build_inputs_reject_even_when_ignored(source_workspace, name):
    layout, source = source_workspace
    fork = layout.root / source
    path = fork / name
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"untracked source fixture")
    assert git(fork, "check-ignore", "--", name) == name
    with pytest.raises(ValueError) as caught:
        pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.OBSERVE)
    assert str(caught.value) == (
        f"Fork has an untracked or ignored build input: {source}/{name}"
    )
    assert path.read_bytes() == b"untracked source fixture"


def test_nested_changes_ignore_git_ignore_configuration(source_workspace):
    layout, source = source_workspace
    fork = layout.root / source
    git(fork, "config", "diff.ignoreSubmodules", "all")
    git(fork, "config", "submodule.imgui.ignore", "all")
    path = fork / "third_party/imgui/imgui.cpp"
    path.write_bytes(b"nested tracked drift")
    assert git(fork, "diff", "--name-only", "HEAD", "--") == ""
    with pytest.raises(ValueError) as caught:
        pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.OBSERVE)
    assert str(caught.value) == "Unsupported committed checkout conversion: imgui.cpp"
    assert path.read_bytes() == b"nested tracked drift"


def write_generated_sources(layout, source):
    child = f"{source}/third_party/luajit"
    expected = {}
    for name in pcsx.GENERATED_LUAJIT:
        path = layout.root / child / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(b"generated fixture")
        expected[f"{child}/{name}"] = hashlib.sha256(b"generated fixture").hexdigest()
    return expected


def test_known_generated_sources_are_hashed_but_unknown_headers_reject(
    source_workspace,
):
    layout, source = source_workspace
    child = f"{source}/third_party/luajit"
    name = "src/luajit.h"
    path = layout.root / child / name
    expected = write_generated_sources(layout, source)
    first = pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.OBSERVE)
    assert first["generated_sources"] == expected
    path.write_bytes(b"changed generated fixture")
    second = pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.OBSERVE)
    assert second["generated_sources"] == {
        **expected,
        f"{child}/{name}": hashlib.sha256(b"changed generated fixture").hexdigest(),
    }
    assert first != second
    assert {
        key: value for key, value in first.items() if key != "generated_sources"
    } == {key: value for key, value in second.items() if key != "generated_sources"}
    unknown = layout.root / child / "src/extra.h"
    unknown.write_bytes(b"unknown generated header")
    assert (
        git(layout.root / child, "check-ignore", "--", "src/extra.h") == "src/extra.h"
    )
    with pytest.raises(ValueError) as caught:
        pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.OBSERVE)
    assert str(caught.value) == (
        f"Fork has an untracked or ignored build input: {child}/src/extra.h"
    )


def test_generated_sources_require_previous_build_provenance(source_workspace):
    layout, source = source_workspace
    path = layout.root / source / "third_party/luajit/src/luajit.h"
    write_generated_sources(layout, source)
    generated = pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.OBSERVE)
    with pytest.raises(ValueError) as caught:
        pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.RUNTIME)
    assert str(caught.value) == (
        "Generated fork sources require the exact prior successful build receipt"
    )
    receipt = layout.out_dir / "setup/pcsx-redux.json"
    receipt.parent.mkdir(parents=True)
    receipt.write_text(
        json.dumps({"schema": "bof3.redux-build/v2", "identity": generated})
    )
    expected_receipt = receipt.read_bytes()
    assert (
        pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.RUNTIME) == generated
    )
    path.write_bytes(b"changed generated fixture")
    with pytest.raises(ValueError) as caught:
        pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.RUNTIME)
    assert str(caught.value) == (
        "Generated fork sources or committed source identity changed"
    )
    assert receipt.read_bytes() == expected_receipt
    assert path.read_bytes() == b"changed generated fixture"
    for name in pcsx.GENERATED_LUAJIT:
        (layout.root / source / "third_party/luajit" / name).unlink()
    assert (
        pcsx._inspect_sources(layout, source, use=pcsx._SourceUse.RUNTIME)[
            "generated_sources"
        ]
        == {}
    )
    assert receipt.read_bytes() == expected_receipt
