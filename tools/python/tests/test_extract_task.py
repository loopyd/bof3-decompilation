from __future__ import annotations

from pathlib import Path

from rebof3.paths import repo_layout
from rebof3.setup.models import SetupContext, SetupOptions
from rebof3.setup.tasks import extract


def test_extract_uses_explicit_disc_input_path(monkeypatch, tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    layout.disc_dir.mkdir(parents=True, exist_ok=True)
    (layout.disc_dir / "game.cue").write_text(
        'FILE "game.bin" BINARY\n', encoding="utf-8"
    )
    (layout.disc_dir / "game.bin").write_bytes(b"fake")

    calls: list[list[str]] = []

    def fake_disk_extract(**kwargs) -> Path:
        calls.append([str(kwargs["tool_path"]), str(kwargs["output_dir"])])
        return layout.disc_dir / "game.cue"

    monkeypatch.setattr("rebof3.setup.tasks.extract.disk_extract", fake_disk_extract)

    extract.run(SetupContext(layout=layout, options=SetupOptions()))

    assert calls == [
        [
            str(layout.bof3_disk_bin),
            str(layout.extracted_dir),
        ]
    ]
