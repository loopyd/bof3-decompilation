"""Query the native asset owners for source-qualified physical byte ranges."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory

from harness.commands.audio import build_binary as build_audio
from harness.commands.text import _binary as build_text
from harness.commands.textures import build_binary as build_textures
from harness.domain.psx import binary_offset_for
from harness.io import repo_layout
from harness.pages.binary_map import Span

TEXTURE_KINDS = {
    "sprite-bank",
    "save-icon-source",
    "palette-bank",
    "compressed-bank",
    "texture-upload",
}
TABLE_KINDS = {
    "sprite-definition-table",
    "area-surface-metadata",
    "resident-animation-bank",
}


class AssetCatalogs:
    """One native index/query pass per original source, shared by its entries."""

    def __init__(self, root: Path, media_root: Path):
        self.root = root
        self.media_root = media_root
        layout = repo_layout(root)
        self.text = build_text(layout)
        self.audio = build_audio(root)
        self.textures = build_textures(layout)
        if not self.text or not self.audio:
            raise ValueError(
                "pages requires current BOF3 text, audio and texture tools"
            )
        self.sources = {}
        self.originals = {
            row["path"]: row
            for row in json.loads((root / "config/original-us-disc.json").read_text())[
                "files"
            ]
        }

    def query(self, tool, *arguments, json_output=True, allow_no_text=False):
        process = subprocess.run(
            [str(tool), *map(str, arguments)],
            cwd=self.root,
            capture_output=True,
            text=True,
            check=False,
        )
        if process.returncode:
            if (
                allow_no_text
                and process.stderr.strip()
                == "error: archive has no supported verified text layout"
            ):
                return {"entries": []}
            raise ValueError(
                f"pages asset query failed ({Path(tool).name}): {process.stderr.strip()}"
            )
        return json.loads(process.stdout) if json_output else None

    def source(self, disc_path: str):
        if disc_path not in self.sources:
            self.sources[disc_path] = self.collect(disc_path)
        return self.sources[disc_path]

    def texture_source(self, path: Path, disc_path: str, size: int, digest: str):
        with TemporaryDirectory(prefix="bof3-pages-assets-") as directory:
            catalog = Path(directory) / "textures.json"
            self.query(
                self.textures,
                "index",
                path,
                "--source",
                disc_path,
                "--output",
                catalog,
                json_output=False,
            )
            index = json.loads(catalog.read_text())
            if len(index["files"]) != 1 or any(
                index["files"][0].get(key) != value
                for key, value in {
                    "path": disc_path,
                    "size": size,
                    "sha256": digest,
                }.items()
            ):
                raise ValueError(f"texture source identity differs: {disc_path}")
            resources = self.query(self.textures, "query", catalog, "--json")
            records = self.query(self.textures, "query", catalog, "--records", "--json")
        return resources, records

    def collect(self, disc_path: str):
        path = self.media_root / "out/extracted" / disc_path
        if not path.is_file():
            raise ValueError(
                f"missing original source out/extracted/{disc_path}; run just extract"
            )
        data = path.read_bytes()
        digest = hashlib.sha256(data).hexdigest()
        expected = self.originals.get(disc_path, {})
        if expected.get("sha256") != digest or expected.get("size") != len(data):
            raise ValueError(f"source differs from original US inventory: {disc_path}")
        print(f"pages: classifying {disc_path}", flush=True)
        resources, records = [], []
        if expected["emi_entries"] is not None:
            resources, records = self.texture_source(path, disc_path, len(data), digest)
        ranges = []
        entries = {} if resources else {None: (0, len(data))}
        for row in resources:
            entry = row["entry"]
            entries[entry] = (row["offset"], row["size"])
            category = (
                "texture"
                if row["kind"] in TEXTURE_KINDS
                else "table"
                if row["kind"] in TABLE_KINDS
                else None
            )
            if category:
                ranges.append(
                    (
                        entry,
                        Span(
                            row["offset"],
                            row["offset"] + row["size"],
                            category,
                            f"textures: {row['kind']}",
                        ),
                    )
                )
        for row in records:
            if row["coordinate_space"] != "entry-payload":
                continue
            entry = row["entry"]
            base = entries[entry][0]
            category = (
                "texture"
                if row["kind"] in {"bank-image", "palette-record", "save-icon-frame"}
                else "table"
                if row["kind"] in {"sprite-cell", "sprite-definition", "resident-group"}
                else None
            )
            if category and row["size"]:
                ranges.append(
                    (
                        entry,
                        Span(
                            base + row["offset"],
                            base + row["offset"] + row["size"],
                            category,
                            f"textures record: {row['kind']}",
                            40,
                        ),
                    )
                )
        text_index = self.query(self.text, "index", path, "--json", allow_no_text=True)
        text_query = (
            self.query(self.text, "query", path, "--json")
            if text_index["entries"]
            else {"results": []}
        )
        for row in text_index["entries"]:
            if row["status"] != "ok":
                raise ValueError(f"text layout failed: {disc_path}: {row['status']}")
            if row["class"] in {"dialogue", "battle"}:
                ranges.append(
                    (
                        row["entry"],
                        Span(
                            row["offset"],
                            row["offset"] + row["size"],
                            "text",
                            f"text index: {row['class']} bank",
                        ),
                    )
                )
        for report in text_query["results"]:
            if report.get("schema") != "bof3.literal-query/v1":
                continue
            if report["source_sha256"] != digest:
                raise ValueError(f"text source identity differs: {disc_path}")
            for row in report["matches"]:
                start, length = row["string_offset"], row["string_length"]
                if length:
                    ranges.append(
                        (
                            report.get("entry"),
                            Span(
                                start,
                                start + length,
                                "text",
                                f"text query: {report['source_schema']}",
                            ),
                        )
                    )
        if path.suffix.upper() == ".EMI":
            audio = self.query(
                self.audio, "index", "--mode", "audio", "--archive", path, "--json"
            )
            if len(audio["sources"]) != 1 or audio["sources"][0]["sha256"] != digest:
                raise ValueError(f"audio source identity differs: {disc_path}")
            audio_entries = {
                row["entry"]: row for row in audio["sources"][0]["entries"]
            }
            seen = set()
            for asset in audio["assets"]:
                if asset["entry"] in seen:
                    continue
                seen.add(asset["entry"])
                selected = self.query(
                    self.audio,
                    "query",
                    "--mode",
                    "audio",
                    "--archive",
                    path,
                    "--id",
                    asset["id"],
                    "--json",
                )["asset"]
                entry = selected["entry"]
                if entry is not None:
                    row = audio_entries[entry]
                    ranges.append(
                        (
                            entry,
                            Span(
                                row["offset"],
                                row["offset"] + row["bytes"],
                                "audio",
                                f"audio query: {selected['kind']}",
                            ),
                        )
                    )
        for entry, span in ranges:
            if not 0 <= span.start < span.end <= len(data):
                raise ValueError(
                    f"asset range escapes original source: {disc_path}#{entry}: {span}"
                )
        return data, digest, entries, ranges

    def target(self, manifest, binary: bytes):
        disc_path, separator, slot = manifest.disc_id.partition("#")
        entry = int(slot) if separator else None
        data, digest, entries, ranges = self.source(disc_path)
        if entry not in entries:
            raise ValueError(f"missing catalog entry: {manifest.disc_id}")
        offset, size = entries[entry]
        original = data[offset : offset + size]
        payload_offset = binary_offset_for(original) if entry is None else 0
        if original[payload_offset:] != binary:
            raise ValueError(f"original source differs from target: {manifest.disc_id}")
        spans = [
            Span(
                span.start - offset,
                span.end - offset,
                span.category,
                span.detail,
                span.priority,
            )
            for slot, span in ranges
            if slot == entry
        ]
        return (
            original,
            spans,
            {"id": manifest.disc_id, "sha256": digest, "offset": offset},
        )
