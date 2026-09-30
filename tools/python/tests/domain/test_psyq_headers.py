from __future__ import annotations

from pathlib import Path

from harness.psyq.headers import declaration_for, parse_headers
from harness.psyq.signatures import scan


def test_header_catalog_and_signature_evidence_are_exact_name_only(
    tmp_path: Path,
) -> None:
    include = tmp_path / "toolchains/psyq/4.7/include"
    include.mkdir(parents=True)
    (include / "libcd.h").write_text(
        "#define CdlModeSpeed 0x80\n"
        "typedef struct CdlFILE { int pos; } CdlFILE;\n"
        "extern int CdReadSync(int mode, unsigned char *result);\n",
        encoding="utf-8",
    )
    catalog = parse_headers(include)

    assert declaration_for(catalog, "CdReadSync") == {
        "name": "CdReadSync",
        "kind": "function",
        "header": "libcd.h",
        "line": 3,
        "declaration": "extern int CdReadSync(int mode, unsigned char *result);",
    }
    assert declaration_for(catalog, "CdlModeSpeed") == {
        "name": "CdlModeSpeed",
        "kind": "macro",
        "header": "libcd.h",
        "line": 1,
        "declaration": "#define CdlModeSpeed 0x80",
    }
    assert declaration_for(catalog, "CdlFILE")["kind"] == "type"
    assert declaration_for(catalog, "CdReadSync2") is None

    manifest = tmp_path / "config/targets/exe/logo/target.toml"
    manifest.parent.mkdir(parents=True)
    manifest.write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/logo"\nkind = "executable"\n'
        'source_dir = "src/exe/logo"\n'
        'binary = "out/binaries/exe/logo.bin"\n'
        'splat = "config/targets/exe/logo/splat.yaml"\n',
        encoding="utf-8",
    )
    binary = tmp_path / "out/binaries/exe/logo.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\x10\x20\x30\x40")
    signature = tmp_path / "toolchains/psx_psyq_signatures/470/LIBCD.LIB.json"
    signature.parent.mkdir(parents=True)
    signature.write_text(
        '[{"name":"READ.OBJ","sig":"10 20 30 40","labels":['
        '{"name":"CdReadSync","offset":0},{"name":"CdReadSync2","offset":0}]}]',
        encoding="utf-8",
    )
    (tmp_path / "toolchains/psx_psyq_signatures/.git").write_text(
        "gitdir: fake\n", encoding="utf-8"
    )

    labels = scan(tmp_path)["matches"][0]["labels"]
    assert labels[0]["declaration"]["kind"] == "function"
    assert labels[0]["declaration"]["header"] == "libcd.h"
    assert "declaration" not in labels[1]


def test_header_catalog_keeps_c_linkage_prototypes_and_gte_function_macros(
    tmp_path: Path,
) -> None:
    include = tmp_path / "include"
    include.mkdir()
    (include / "libetc.h").write_text(
        '#if defined(__cplusplus)\nextern "C" {\n#endif\n'
        "unsigned long PadRead(int id);\n"
        "#if defined(__cplusplus)\n}\n#endif\n",
        encoding="utf-8",
    )
    (include / "inline_c.h").write_text(
        '#define gte_rtps() __asm__ volatile ( \\\n    "nop;" \\\n    : : : )\n',
        encoding="utf-8",
    )

    catalog = parse_headers(include)
    assert declaration_for(catalog, "PadRead") == {
        "name": "PadRead",
        "kind": "function",
        "header": "libetc.h",
        "line": 4,
        "declaration": "unsigned long PadRead(int id);",
    }
    assert declaration_for(catalog, "gte_rtps") == {
        "name": "gte_rtps",
        "kind": "macro",
        "header": "inline_c.h",
        "line": 1,
        "declaration": "#define gte_rtps() __asm__ volatile ( \\",
    }
