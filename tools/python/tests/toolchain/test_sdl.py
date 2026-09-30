"""SDL source publication and build identity at the harness setup boundary."""

from __future__ import annotations

import hashlib
import io
import json
import tarfile

import pytest

from harness.io import repo_layout
from harness.toolchain import sdl


@pytest.fixture
def release(tmp_path, monkeypatch):
    archive = io.BytesIO()
    with tarfile.open(fileobj=archive, mode="w:gz") as tar:
        data = b"cmake_minimum_required(VERSION 3.16)\n"
        member = tarfile.TarInfo(f"SDL3-{sdl.VERSION}/CMakeLists.txt")
        member.size = len(data)
        tar.addfile(member, io.BytesIO(data))
    payload = archive.getvalue()
    monkeypatch.setattr(sdl, "ARCHIVE_SHA256", hashlib.sha256(payload).hexdigest())
    monkeypatch.setattr(
        sdl.urllib.request, "urlopen", lambda *a, **kw: io.BytesIO(payload)
    )
    return sdl.SdlToolchain(repo_layout(tmp_path)), payload


def test_download_verifies_archive_and_reuses_unchanged_source(release, monkeypatch):
    tool, _ = release
    expected = tool.install()
    monkeypatch.setattr(
        sdl.urllib.request,
        "urlopen",
        lambda *a, **kw: pytest.fail("unexpected download"),
    )
    assert tool.install() == expected
    (tool.source / "CMakeLists.txt").write_text("changed")
    with pytest.raises(ValueError, match="source differs"):
        tool.install()
    assert (tool.source / "CMakeLists.txt").read_text() == "changed"


def test_invalid_download_never_publishes_cache(release, monkeypatch):
    tool, _ = release
    monkeypatch.setattr(
        sdl.urllib.request, "urlopen", lambda *a, **kw: io.BytesIO(b"bad archive")
    )
    with pytest.raises(ValueError, match="SHA-256"):
        tool.install()
    assert not (tool.layout.downloads_dir / f"SDL3-{sdl.VERSION}.tar.gz").exists()
    assert not tool.source.exists()


def test_doctor_verifies_source_and_binary_without_repair(release, monkeypatch):
    tool, _ = release
    source = tool.install()
    tool.library.parent.mkdir(parents=True)
    tool.library.write_bytes(b"fixture library")
    tool.receipt.parent.mkdir(parents=True)
    tool.receipt.write_text(
        json.dumps(
            {
                "version": sdl.VERSION,
                "archive_sha256": sdl.ARCHIVE_SHA256,
                "source_sha256": source,
                "library_sha256": hashlib.sha256(b"fixture library").hexdigest(),
            }
        )
    )
    monkeypatch.setattr(
        sdl.urllib.request, "urlopen", lambda *a, **kw: pytest.fail("doctor downloaded")
    )
    assert "verified" in tool.verify()
    before = tool.receipt.read_bytes()
    tool.library.write_bytes(b"damaged")
    with pytest.raises(ValueError, match="binary differs"):
        tool.verify()
    assert tool.receipt.read_bytes() == before
