"""Audio reference setup and filename-independent BIOS doctor validation."""

from __future__ import annotations

import hashlib
import io
import shutil
import subprocess

import pytest

from harness.commands import doctor, setup
from harness.io import repo_layout
from harness.toolchain import bios
from harness.toolchain.pcsx import PcsxReduxToolchain


@pytest.fixture
def collection(tmp_path, monkeypatch):
    if not shutil.which("7z"):
        pytest.skip("7z is needed for the BIOS archive integration fixture")
    rom = tmp_path / "ps-30a.bin"
    rom.write_bytes(bytes(range(256)) * 2048)
    archive = tmp_path / "ps-30a.7z"
    subprocess.run(["7z", "a", str(archive), str(rom)], check=True, capture_output=True)
    data = archive.read_bytes()
    monkeypatch.setattr(bios, "ARCHIVE_SIZE", len(data))
    monkeypatch.setattr(bios, "ARCHIVE_SHA1", hashlib.sha1(data).hexdigest())
    monkeypatch.setattr(
        bios, "BIOS_SHA256", hashlib.sha256(rom.read_bytes()).hexdigest()
    )
    return bios.BiosToolchain(repo_layout(tmp_path / "repo")), data


def test_bios_download_names_scph_model_and_reuses_offline(collection, monkeypatch):
    tool, data = collection
    calls = []

    def download(url, timeout):
        calls.append((url, timeout))
        return io.BytesIO(data)

    monkeypatch.setattr(bios.urllib.request, "urlopen", download)
    assert "SHA-256 verified" in tool.run()
    assert calls == [(bios.ARCHIVE_URL, 60)]
    assert [p.name for p in tool.directory.iterdir()] == ["scph5501.bin"]
    assert "SHA-256 verified" in tool.run()
    assert len(calls) == 1


@pytest.mark.parametrize("payload", [b"truncated", b"wrong-hash"])
def test_bios_failed_download_publishes_nothing(collection, monkeypatch, payload):
    tool, data = collection
    if payload == b"wrong-hash":
        payload = bytes(len(data))
    monkeypatch.setattr(
        bios.urllib.request, "urlopen", lambda *a, **kw: io.BytesIO(payload)
    )
    with pytest.raises(ValueError, match="mismatch"):
        tool.run()
    assert not list(tool.directory.iterdir())
    assert not list(tool.layout.downloads_dir.rglob(".download-*"))


def test_bios_failed_refresh_preserves_existing_rom(collection, monkeypatch):
    tool, data = collection
    monkeypatch.setattr(
        bios.urllib.request, "urlopen", lambda *a, **kw: io.BytesIO(data)
    )
    tool.run()
    target = tool.directory / bios.BIOS_FILENAME
    before = target.read_bytes()
    monkeypatch.setattr(
        bios.urllib.request, "urlopen", lambda *a, **kw: io.BytesIO(b"bad")
    )
    with pytest.raises(ValueError, match="mismatch"):
        tool.run(force=True)
    assert target.read_bytes() == before
    assert "SHA-256 verified" in tool.verify()


def test_bios_failed_publication_leaves_no_rom(collection, monkeypatch):
    tool, data = collection
    monkeypatch.setattr(
        bios.urllib.request, "urlopen", lambda *a, **kw: io.BytesIO(data)
    )

    def fail(*args):
        raise OSError("publication fixture failure")

    monkeypatch.setattr(bios.os, "link", fail)
    with pytest.raises(OSError, match="publication fixture failure"):
        tool.run()
    assert not list(tool.directory.iterdir())


def test_setup_rejects_conflicting_canonical_file_without_download(
    collection, monkeypatch
):
    tool, _ = collection
    tool.directory.mkdir(parents=True)
    target = tool.directory / bios.BIOS_FILENAME
    target.write_bytes(b"user content")
    monkeypatch.setattr(
        bios.urllib.request,
        "urlopen",
        lambda *a, **kw: pytest.fail("unexpected download"),
    )
    with pytest.raises(ValueError, match="refusing to overwrite"):
        tool.run(force=True)
    assert target.read_bytes() == b"user content"


@pytest.mark.parametrize(
    "name", ["scph5501.bin", "custom.rom", "nested/renamed.BIN", "no-extension"]
)
def test_doctor_accepts_content_under_any_name_without_inventory(
    collection, monkeypatch, capsys, name
):
    tool, data = collection
    monkeypatch.setattr(
        bios.urllib.request, "urlopen", lambda *a, **kw: io.BytesIO(data)
    )
    tool.run()
    target = tool.directory / name
    target.parent.mkdir(parents=True, exist_ok=True)
    (tool.directory / bios.BIOS_FILENAME).rename(target)
    (tool.directory / "inventory.json").write_text("not a manifest")
    (tool.directory / "wrong.bin").write_bytes(bytes(bios.BIOS_SIZE))
    monkeypatch.setattr(
        bios.urllib.request,
        "urlopen",
        lambda *a, **kw: pytest.fail("doctor downloaded"),
    )
    monkeypatch.setattr(
        doctor, "TASKS", [task for task in doctor.TASKS if task.run is doctor._bios]
    )
    assert doctor.main(["--root", str(tool.layout.root)]) == 0
    assert str(target) in capsys.readouterr().out


@pytest.mark.parametrize("damage", ["missing", "changed", "permission", "symlink"])
def test_doctor_reports_missing_matching_hash_without_repair(
    collection, monkeypatch, capsys, damage
):
    tool, data = collection
    monkeypatch.setattr(
        bios.urllib.request, "urlopen", lambda *a, **kw: io.BytesIO(data)
    )
    tool.run()
    rom = tool.directory / bios.BIOS_FILENAME
    if damage == "missing":
        rom.unlink()
    elif damage == "changed":
        rom.write_bytes(bytes(bios.BIOS_SIZE))
    elif damage == "symlink":
        outside = tool.directory.parent / "outside.bin"
        rom.rename(outside)
        rom.symlink_to(outside)
    else:

        def denied(path):
            raise PermissionError("fixture access denied")

        monkeypatch.setattr(bios, "file_sha256", denied)
    monkeypatch.setattr(
        bios.urllib.request,
        "urlopen",
        lambda *a, **kw: pytest.fail("doctor downloaded"),
    )
    monkeypatch.setattr(
        doctor, "TASKS", [task for task in doctor.TASKS if task.run is doctor._bios]
    )
    assert doctor.main(["--root", str(tool.layout.root)]) == 2
    assert "[FAIL] US BIOS" in capsys.readouterr().out


@pytest.mark.parametrize(
    "component,class_name",
    [("bios", "BiosToolchain"), ("pcsx-redux", "PcsxReduxToolchain")],
)
def test_focused_setup_does_not_run_unrelated_tasks(
    tmp_path, monkeypatch, component, class_name
):
    calls = []

    class RecordingToolchain:
        def __init__(self, layout):
            assert layout.root == tmp_path

        def run(self, *, force):
            calls.append(force)
            return "fixture prepared"

    monkeypatch.setattr(setup, class_name, RecordingToolchain)
    assert (
        setup.main(["--root", str(tmp_path), "--component", component, "--force"]) == 0
    )
    assert calls == [True]


def test_redux_refuses_changed_checkout_without_update(tmp_path, monkeypatch):
    tool = PcsxReduxToolchain(repo_layout(tmp_path))
    source = tmp_path / tool.submodule
    source.mkdir(parents=True)
    (source / ".git").write_text("gitdir: fixture")
    calls = []

    def git(*args):
        calls.append(args)
        if args[0] == "ls-files":
            return f"160000 {'a' * 40} 0\t{tool.submodule}"
        if args[-1] == "HEAD":
            return "b" * 40
        pytest.fail("setup must not mutate a changed checkout")

    monkeypatch.setattr(tool, "_git", git)
    with pytest.raises(ValueError, match="differs from the gitlink"):
        tool.install(force=True)
    assert len(calls) == 2
