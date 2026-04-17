from __future__ import annotations

import re
from dataclasses import dataclass


SLUG_CLEAN_RE = re.compile(r"[^0-9A-Za-z]+")
ENTRY_INDEX_RE = re.compile(r"_e(?P<entry>[0-9]+)(?:_|\.)", re.IGNORECASE)
RAW_BIN_NAME_RE = re.compile(r"^(?P<entry>[0-9]+)\.bin(?:\.[0-9]+)?$", re.IGNORECASE)


@dataclass(frozen=True)
class BinProgramPath:
    family: str
    archive: str
    program_name: str
    slot_token: str
    normalized_slot: str

    @property
    def slot_index(self) -> int | None:
        return int(self.slot_token, 10) if self.slot_token.isdigit() else None


def slugify(text: str) -> str:
    lowered = SLUG_CLEAN_RE.sub("_", str(text).strip().lower()).strip("_")
    return lowered or "program"


def normalize_program_selector(text: str) -> str:
    return str(text or "").strip("/")


def selector_matches(value: str | None, selector: str) -> bool:
    if value is None:
        return False
    string_value = str(value)
    if string_value == selector:
        return True
    return normalize_program_selector(string_value) == normalize_program_selector(
        selector
    )


def infer_source_hint(program_path: str, folder: str, program_name: str) -> str | None:
    normalized_folder = str(folder or "").strip("/")
    normalized_program = str(program_name or "")

    if normalized_folder == "boot" and normalized_program == "SLUS_004.22":
        return "build/extracted/SLUS_004.22"
    if normalized_folder == "boot/LOGO" and normalized_program.upper() == "LOGO.EXE":
        return "build/extracted/LOGO/LOGO.EXE"

    for prefix in ("bins/", "overlays/"):
        if not normalized_folder.startswith(prefix):
            continue
        archive_id = normalized_folder[len(prefix) :]
        match = ENTRY_INDEX_RE.search(normalized_program)
        if match is None:
            match = RAW_BIN_NAME_RE.match(normalized_program)
        if match is None or not archive_id:
            return None
        return f"build/extracted/{archive_id}.EMI#{int(match.group('entry'))}"

    _ = program_path
    return None


def classify_program_kind(program_path: str, source_hint: str | None = None) -> str:
    normalized = str(program_path or "")
    normalized_lower = normalized.lower()
    source_hint_lower = str(source_hint or "").lower()
    if normalized == "/boot/SLUS_004.22":
        return "boot"
    if normalized == "/boot/LOGO/LOGO.EXE" or "logo" in source_hint_lower:
        return "logo"
    if normalized_lower.startswith("/bins/bin/"):
        return "bin"
    return "other"


def parse_bin_program_path(program_path: str) -> BinProgramPath:
    parts = str(program_path).split("/")
    if len(parts) < 6 or parts[1] != "bins" or parts[2] != "BIN":
        raise ValueError(f"unsupported program path: {program_path}")
    program_name = parts[5]
    raw_bin_match = RAW_BIN_NAME_RE.match(program_name)
    if raw_bin_match is not None:
        slot_token = raw_bin_match.group("entry")
    else:
        slot_token = program_name.removesuffix(".bin")
    normalized_slot = (
        slot_token.zfill(2) if slot_token.isdigit() else slot_token.lower()
    )
    return BinProgramPath(
        family=parts[3].lower(),
        archive=parts[4].lower(),
        program_name=program_name,
        slot_token=slot_token,
        normalized_slot=normalized_slot,
    )
