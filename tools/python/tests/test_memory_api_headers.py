from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
REMOVED_HEADERS = (
    "include/bof3/defines.h",
    "include/bof3/memory.h",
    "include/bof3/scratchpad.h",
    "include/bof3/ui/panel_task.h",
    "include/memory/registers.h",
)
REMOVED_INCLUDES = tuple(path.removeprefix("include/") for path in REMOVED_HEADERS)


def test_deprecated_memory_compatibility_headers_are_gone() -> None:
    for path in REMOVED_HEADERS:
        assert not (ROOT / path).exists()


def test_tracked_headers_and_lifts_do_not_include_removed_headers() -> None:
    for root in (ROOT / "include", ROOT / "src"):
        for path in root.rglob("*.[ch]"):
            text = path.read_text(encoding="utf-8")
            for include in REMOVED_INCLUDES:
                assert f'"{include}"' not in text, path
