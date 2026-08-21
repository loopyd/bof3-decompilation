"""Tests for the opt-in decomp.me scratch publisher."""

from __future__ import annotations

import io
import json
import sqlite3
from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.analysis import type_context as type_context_module
from harness.domain import parse_function_id
from harness.domain.c_context import (
    public_declaration_context,
    scalar_declaration_context,
)
from harness.toolchain.decompme import (
    DecompMeScratchpadToolchain,
    ScratchpadPayload,
    _decompme_compiler_flags,
    _remote_compiler_id,
)


class _Response(io.BytesIO):
    status = 201

    def __enter__(self) -> "_Response":
        return self

    def __exit__(self, *exc: object) -> None:
        self.close()


@pytest.fixture
def isolated_registry_context(monkeypatch: pytest.MonkeyPatch) -> None:
    """Keep payload tests independent of the disposable repository index.

    The toolchain still exercises declaration closure over preprocessed target
    headers. Dedicated freshness tests cover the registry's fail-closed path.
    """

    from harness.io import repo_layout
    from harness.toolchain import decompme

    scalars = scalar_declaration_context(
        (repo_layout().root / "include/base/types.h").read_text(encoding="utf-8")
    )
    monkeypatch.setattr(decompme, "type_context", lambda *_: scalars)


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
        name="func_80100000",
        platform="ps1",
        compiler="gcc2.7.2-psx",
        compiler_flags="-O2",
        diff_label="func_80100000",
        target_asm=".text\n",
        context="",
        source_code="void func_80100000(void) {}\n",
    )
    with pytest.raises(RuntimeError, match="no scratch slug"):
        DecompMeScratchpadToolchain(SimpleNamespace(root=tmp_path)).publish(payload)


def test_payload_requires_fresh_registry_when_index_exists(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    from harness.toolchain import decompme

    (tmp_path / ".git").mkdir()
    source = tmp_path / "source.c"
    source.write_text("void test(void) {}\n", encoding="utf-8")
    index = tmp_path / "out/index/reverse.sqlite"
    index.parent.mkdir(parents=True)
    with sqlite3.connect(index) as connection:
        connection.execute("CREATE TABLE metadata (key TEXT PRIMARY KEY, value TEXT)")
        connection.execute("INSERT INTO metadata VALUES ('schema', 'test-schema')")
    function = parse_function_id("exe/test@0x80100000")
    manifest = SimpleNamespace(id=SimpleNamespace(value="exe/test"))
    resolved = SimpleNamespace(manifest=manifest, source=source, compiled_symbol="test")
    toolchain = DecompMeScratchpadToolchain(SimpleNamespace(root=tmp_path))
    monkeypatch.setattr(decompme, "resolve_function", lambda *_: resolved)
    monkeypatch.setattr(
        decompme, "_preprocess_source", lambda *_: ("", "void test(void) {}\n")
    )

    def stale_index(*_: object) -> object:
        raise ValueError("stale index")

    monkeypatch.setattr(type_context_module, "connect", stale_index)

    with pytest.raises(ValueError, match="stale index"):
        toolchain.payload(function, compiler="gcc-2.7.2-psx")


def test_payload_for_battle_range_lift_uses_ps1_and_preprocessed_source(
    isolated_registry_context: None,
) -> None:
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


def test_payload_uses_registry_function_name(
    monkeypatch: pytest.MonkeyPatch,
    isolated_registry_context: None,
) -> None:
    from harness.io import repo_layout
    from harness.toolchain import decompme

    import dataclasses

    real = decompme.resolve_function

    def named(root, function):
        resolved = real(root, function)
        return dataclasses.replace(resolved, compiled_symbol="drawFrameBorder")

    monkeypatch.setattr(decompme, "resolve_function", named)
    payload = DecompMeScratchpadToolchain(repo_layout()).payload(
        parse_function_id("emi/world00/area008/13@0x801F3D88"),
        compiler="gcc-2.7.2-psx",
    )

    assert payload.name == "drawFrameBorder"
    assert payload.diff_label == "drawFrameBorder"


def test_payload_preserves_required_assembler_flags() -> None:
    assert (
        _decompme_compiler_flags(
            ["bin/cc", "-O2", "-Wa,--expand-div", "-c", "source.c"]
        )
        == "-O2 -G0 -funsigned-char -msoft-float -gcoff -Wa,--expand-div"
    )
    assert (
        _decompme_compiler_flags(["bin/cc", "-O2", "-c", "source.c"])
        == "-O2 -G0 -funsigned-char -msoft-float -gcoff"
    )


def test_payload_rejects_data_leading_non_function_range() -> None:
    from harness.io import repo_layout

    with pytest.raises(ValueError, match="cannot resolve compiled symbol"):
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


def test_public_context_deduplicates_base_typedefs() -> None:
    context = public_declaration_context(
        "typedef unsigned int u32;\nextern u32 g_value;\n",
        "u32 func(void) { return g_value; }",
        base="typedef unsigned int u32;\n",
    )

    assert context.count("typedef unsigned int u32;") == 1
    assert "extern u32 g_value;" in context


def test_public_context_keeps_function_pointer_typedefs() -> None:
    context = public_declaration_context(
        "typedef void (*BattleSelectionHandler)(void);\n",
        "void func(void) { BattleSelectionHandler handler; handler(); }",
        base="",
    )

    assert "typedef void (*BattleSelectionHandler)(void);" in context


def test_public_context_resolves_all_names_from_multi_name_typedef() -> None:
    declaration = "typedef struct Node { u32 value; } Node, *NodePtr;"

    for source in ("Node value;", "NodePtr value;"):
        context = public_declaration_context(
            declaration, source, base="typedef unsigned int u32;\n"
        )
        assert context.count(declaration) == 1


def test_public_context_deduplicates_identical_same_name_declarations() -> None:
    declaration = "extern u32 g_value;"
    context = public_declaration_context(
        declaration + "\n" + declaration,
        "u32 func(void) { return g_value; }",
        base="typedef unsigned int u32;\n",
    )

    assert context.count(declaration) == 1


def test_public_context_keeps_plain_function_prototypes() -> None:
    context = public_declaration_context(
        "s32 GetGraphType(void);\n",
        "s32 func(void) { return GetGraphType(); }",
        base="typedef signed int s32;\n",
    )

    assert "s32 GetGraphType(void);" in context


def test_public_context_rejects_conflicting_same_name_declarations() -> None:
    with pytest.raises(ValueError, match="conflicting declarations for g_value"):
        public_declaration_context(
            "extern u16 g_value;\nextern u32 g_value;\n",
            "u32 func(void) { return g_value; }",
            base="typedef unsigned short u16;\ntypedef unsigned int u32;\n",
        )


def test_payload_allows_local_names_that_collide_with_ignored_headers(
    isolated_registry_context: None,
) -> None:
    from harness.io import repo_layout

    payload = DecompMeScratchpadToolchain(repo_layout()).payload(
        parse_function_id("emi/battle/battle/15@0x800AF66C"),
        compiler="gcc-2.7.2-psx",
    )

    assert "REGISTER_PIN" not in payload.source_code
    assert "result" in payload.source_code


def test_payload_resolves_referenced_psyq_declarations(
    isolated_registry_context: None,
) -> None:
    from harness.io import repo_layout

    payload = DecompMeScratchpadToolchain(repo_layout()).payload(
        parse_function_id("emi/world00/area008/13@0x801F3D88"),
        compiler="gcc-2.7.2-psx",
    )

    assert payload.source_code.lstrip().startswith("void drawTexturedFrame(")
    assert "typedef struct { short x, y; short w, h; } RECT;" in payload.context
    assert "} POLY_FT4;" in payload.context
    assert "} DR_MODE;" in payload.context
    assert "extern u_short GetClut(int x, int y) ;" in payload.context
    assert "extern void SetDrawMode(DR_MODE *p" in payload.context
    assert "s32 GetGraphType(void);" in payload.context
    assert "void func_8014E5A0 (u32 ot_index, u32 primitive_size);" in payload.context
    assert "void func_801AEBA0(s16 arg0, s16 arg1, s16 arg2" in payload.context
