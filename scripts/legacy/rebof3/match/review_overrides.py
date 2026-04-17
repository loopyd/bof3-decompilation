from __future__ import annotations

LIKELY_NONCODE_EMI_ENTRY_PATHS: dict[str, str] = {
    "build/extracted/BIN/ETC/AFLDKWA.EMI#0": "CPU-RAM text/table pack, not a code overlay",
    "build/extracted/BIN/ETC/DEMO.EMI#7": "title-side CLUT/data block, not a code overlay",
    "build/extracted/BIN/ETC/FIRST.EMI#8": "small title/menu resource blob",
    "build/extracted/BIN/ETC/FIRST.EMI#9": "small title/menu resource blob duplicated by BATE#3",
    "build/extracted/BIN/ETC/FIRST.EMI#10": "small title/menu resource blob duplicated by BATE#4",
    "build/extracted/BIN/ETC/FIRST.EMI#11": "CPU-RAM text/table block duplicated by AFLDKWA#0",
    "build/extracted/BIN/ETC/FIRST.EMI#13": "title/menu CLUT block, not a code overlay",
}


def likely_noncode_reason(source_hint: str | None) -> str | None:
    if source_hint is None:
        return None
    return LIKELY_NONCODE_EMI_ENTRY_PATHS.get(str(source_hint))
