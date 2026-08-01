"""Tests for the opt-in decomp.me scratch publisher."""

from __future__ import annotations

import io
import json
from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.domain import parse_function_id
from harness.c_context import public_declaration_context
from harness.toolchain.decompme import (
    DecompMeScratchpadToolchain,
    ScratchpadPayload,
    _remote_compiler_id,
)


class _Response(io.BytesIO):
    status = 201

    def __enter__(self) -> "_Response":
        return self

    def __exit__(self, *exc: object) -> None:
        self.close()


def test_remote_compiler_id_uses_decompme_ps1_spelling() -> None:
    assert _remote_compiler_id("gcc-2.7.2-psx") == "gcc2.7.2-psx"
    with pytest.raises(ValueError, match="unsupported"):
        _remote_compiler_id("psyq4.7")


def test_publish_posts_json_and_returns_public_scratch_url(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    from harness.toolchain import decompme

    captured: dict[str, object] = {}

    def fake_urlopen(request: object, timeout: int) -> _Response:
        captured["url"] = request.full_url  # type: ignore[attr-defined]
        captured["method"] = request.method  # type: ignore[attr-defined]
        captured["payload"] = json.loads(request.data)  # type: ignore[attr-defined]
        captured["timeout"] = timeout
        return _Response(b'{"slug":"Ab123"}')

    monkeypatch.setattr(decompme.urllib.request, "urlopen", fake_urlopen)
    toolchain = DecompMeScratchpadToolchain(SimpleNamespace(root=tmp_path))
    payload = ScratchpadPayload(
        name="func_80100000",
        platform="ps1",
        compiler="gcc2.7.2-psx",
        compiler_flags="-O2",
        diff_label="func_80100000",
        target_asm=".text\nglabel func_80100000\n",
        context="typedef unsigned int u32;\n",
        source_code="void func_80100000(void) {}\n",
    )

    assert toolchain.publish(payload) == "https://decomp.me/scratch/Ab123"
    assert captured == {
        "url": "https://decomp.me/api/scratch",
        "method": "POST",
        "payload": payload.as_api_data(),
        "timeout": 30,
    }


def test_publish_rejects_malformed_response(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    from harness.toolchain import decompme

    monkeypatch.setattr(
        decompme.urllib.request,
        "urlopen",
        lambda request, timeout: _Response(b"not json"),
    )
    payload = ScratchpadPayload(
        name="func_80100000", platform="ps1", compiler="gcc2.7.2-psx",
        compiler_flags="-O2", diff_label="func_80100000", target_asm=".text\n",
        context="", source_code="void func_80100000(void) {}\n",
    )
    with pytest.raises(RuntimeError, match="no scratch slug"):
        DecompMeScratchpadToolchain(SimpleNamespace(root=tmp_path)).publish(payload)


def test_payload_for_battle_range_lift_uses_ps1_and_preprocessed_source() -> None:
    from harness.io import repo_layout

    payload = DecompMeScratchpadToolchain(repo_layout()).payload(
        parse_function_id("emi/battle/battle/15@0x800AF66C"),
        compiler="gcc-2.7.2-psx",
    )
    assert payload.platform == "ps1"
    assert payload.compiler == "gcc2.7.2-psx"
    assert payload.diff_label == "func_800AF66C"
    assert '#include "internal.h"' not in payload.source_code
    assert "#define g_battle_work" not in payload.context
    assert "extern u8* volatile g_battle_work;" in payload.context
    assert "typedef struct BattleWork" not in payload.context
    assert "typedef struct BattleRange" in payload.context
    assert "range_axis_34" in payload.context
    assert "(* ((   u32" not in payload.source_code
    assert "work->range_axis_34" in payload.source_code
    assert "glabel func_800AF66C" in payload.target_asm
    exported = json.dumps(payload.as_api_data())
    assert "inputs/" not in exported and "/toolchains/" not in exported
    assert "OpenEvent" not in payload.context and "SVECTOR" not in payload.context
    assert payload.context.count("extern ") == 1


def test_payload_rejects_data_leading_non_function_range() -> None:
    from harness.io import repo_layout

    with pytest.raises(ValueError, match="not a reviewed function boundary"):
        DecompMeScratchpadToolchain(repo_layout()).payload(
            parse_function_id("emi/battle/battle/03@0x801D0C00"),
            compiler="gcc-2.7.2-psx",
        )


def test_public_context_closes_type_dependencies_from_any_header() -> None:
    context = public_declaration_context(
        """
        typedef struct Shared { u32 value; } Shared;
        typedef struct Local { Shared member; } Local;
        extern Local *g_local;
        extern u32 ignored;
        """,
        "void func(void) { g_local->member.value = 0; }",
        base="typedef unsigned int u32;\n",
    )

    assert "typedef struct Shared" in context
    assert "typedef struct Local" in context
    assert "extern Local *g_local;" in context
    assert "ignored" not in context


def test_public_context_keeps_function_pointer_typedefs() -> None:
    context = public_declaration_context(
        "typedef void (*BattleSelectionHandler)(void);\n",
        "void func(void) { BattleSelectionHandler handler; handler(); }",
        base="",
    )

    assert "typedef void (*BattleSelectionHandler)(void);" in context


def test_payload_allows_local_names_that_collide_with_ignored_headers() -> None:
    from harness.io import repo_layout

    payload = DecompMeScratchpadToolchain(repo_layout()).payload(
        parse_function_id("emi/battle/battle/15@0x800AF66C"),
        compiler="gcc-2.7.2-psx",
    )

    assert "REGISTER_PIN" not in payload.source_code
    assert "result" in payload.source_code


def test_payload_rejects_ignored_psyq_declarations() -> None:
    from harness.io import repo_layout

    with pytest.raises(ValueError, match="ignored PsyQ declarations: DR_MODE"):
        DecompMeScratchpadToolchain(repo_layout()).payload(
            parse_function_id("emi/battle/battle/03@0x801D9900"),
            compiler="gcc-2.7.2-psx",
        )
