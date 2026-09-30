"""PCSX-Redux setup refuses unexpected source inputs and generated-source drift."""

from __future__ import annotations

import json

import pytest

from harness.io import repo_layout
from harness.runtime import patches


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
def test_untracked_build_inputs_reject_even_when_ignored(tmp_path, monkeypatch, name):
    def git(root, *args):
        assert args[2:] == ("ls-files", "--others", "-z")
        return name + "\0"

    monkeypatch.setattr(patches, "git_text", git)
    with pytest.raises(ValueError, match="untracked build input"):
        patches.inspect_redux_sources(repo_layout(tmp_path), [], set())


def test_nested_changes_ignore_git_ignore_configuration(tmp_path, monkeypatch):
    child = f"{patches.SOURCE}/third_party/imgui"

    def git(root, *args):
        if args[1] == patches.SOURCE:
            return ""
        assert args[1] == child and "--ignore-submodules=none" in args
        return "imgui.cpp"

    monkeypatch.setattr(patches, "git_text", git)
    with pytest.raises(ValueError, match="nested source has tracked changes"):
        patches.inspect_redux_sources(repo_layout(tmp_path), [child], set())


def test_known_generated_sources_are_hashed_but_unknown_headers_reject(
    tmp_path, monkeypatch
):
    layout = repo_layout(tmp_path)
    child = f"{patches.SOURCE}/third_party/luajit"
    name = "src/luajit.h"
    path = tmp_path / child / name
    path.parent.mkdir(parents=True)
    path.write_bytes(b"generated fixture")

    def git(root, *args):
        if args[2] == "diff" or args[1] == patches.SOURCE:
            return ""
        return name + "\0"

    monkeypatch.setattr(patches, "git_text", git)
    first = patches.inspect_redux_sources(layout, [child], set())
    path.write_bytes(b"changed generated fixture")
    assert first != patches.inspect_redux_sources(layout, [child], set())
    name = "src/extra.h"
    with pytest.raises(ValueError, match="untracked build input"):
        patches.inspect_redux_sources(layout, [child], set())


def test_generated_sources_require_previous_build_provenance(tmp_path):
    layout = repo_layout(tmp_path)
    generated = {"third_party/pcsx-redux/third_party/luajit/src/luajit.h": "known"}
    with pytest.raises(ValueError, match="lack a prior build receipt"):
        patches.validate_redux_generation(layout, generated)
    receipt = layout.out_dir / "setup/pcsx-redux.json"
    receipt.parent.mkdir(parents=True)
    receipt.write_text(
        json.dumps(
            {
                "schema": "bof3.redux-build/v1",
                "identity": {"generated_sources": generated},
            }
        )
    )
    patches.validate_redux_generation(layout, generated)
    with pytest.raises(ValueError, match="will not bless or overwrite"):
        patches.validate_redux_generation(layout, {next(iter(generated)): "changed"})
    patches.validate_redux_generation(layout, {})
