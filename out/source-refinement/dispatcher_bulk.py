"""Group every remaining 17-instruction work-slot dispatcher per target.

Usage: python dispatcher_bulk.py TARGET [TARGET ...]

For each target: structurally parse the splat entries, plan the dispatcher members,
write one grouped source behind BOF3_DISPATCH_WORK_SLOT_ONE/TWO, retire the
per-function sources it replaces, patch manifest/symbols/splat, then build and gate
every member. A target whose build or gate fails is rolled back exactly and reported.
"""

from __future__ import annotations

import json
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path("/mnt/PROJECTS/repos/rebof3-simple")
SCAN = json.loads((ROOT / "out/source-refinement/dispatcher-scan.json").read_text())
BLOCK_RE = re.compile(r"(?m)^  - - (\d+)\n((?:    - .*\n)+)")
SOURCE_RE = re.compile(r"^    - '@source: (\S+)'\s*$")


def harness(*args: str, timeout: int = 1800) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [".venv/bin/python", "-m", "harness.cli", *args],
        cwd=ROOT,
        capture_output=True,
        text=True,
        timeout=timeout,
    )


def entries(text: str) -> list[dict]:
    out = []
    for match in BLOCK_RE.finditer(text):
        offset = int(match.group(1))
        body = [line for line in match.group(2).splitlines() if line.strip()]
        kind = body[0].strip().lstrip("- ").strip()
        label = body[1].strip().lstrip("- ").strip()
        source = None
        for line in body[2:]:
            found = SOURCE_RE.match(line)
            if found:
                source = found.group(1)
        out.append(
            {"offset": offset, "kind": kind, "label": label, "source": source,
             "start": match.start(), "end": match.end(), "body": body}
        )
    return out


def plan(target: str) -> dict:
    cfg = ROOT / "config/targets" / target
    manifest = (cfg / "target.toml").read_text()
    load = int(
        next(
            line.split("=")[1].strip()
            for line in manifest.splitlines()
            if line.startswith("load_address")
        ),
        0,
    )
    data = (ROOT / "out/binaries" / f"{target}.bin").read_bytes()
    splat = (cfg / "splat.yaml").read_text()
    rows = []
    by_offset = {entry["offset"]: entry for entry in entries(splat)}
    offsets = sorted(by_offset)
    _, family, area, index = target.split("/")
    token = f"{family.capitalize()}{area.capitalize()}{index}"
    for offset, slot in SCAN[target]:
        address = load + offset
        words = struct.unpack_from("<17I", data, offset)
        high = (words[7] & 0xFFFF) << 16
        low = words[9] & 0xFFFF
        low = low - 0x10000 if low & 0x8000 else low
        table = high + low
        next_offset = next((value for value in offsets if value > offset), None)
        size = next_offset - offset if next_offset is not None else None
        entry = by_offset.get(offset)
        if entry is None:
            return {"skip": f"no splat entry at 0x{address:08X}"}
        if size != 68:
            return {"skip": f"0x{address:08X} spans {size} bytes, not 68"}
        rows.append(
            {
                "offset": offset,
                "address": address,
                "slot": slot,
                "table": table,
                "kind": entry["kind"],
                "label": entry["label"],
                "source": entry["source"],
                "size": size,
                "name": entry["label"],
            }
        )
    manifest_sources = re.findall(r'"(src/bof3/[^"]+\.c)"', manifest)
    for row in rows:
        if row["kind"] == "c" and not row["source"]:
            marker = f"0x{row['address']:08X}".lower()
            for source in manifest_sources:
                path = ROOT / source
                if not path.is_file():
                    continue
                text = path.read_text()
                if f"@source {marker}" in text.lower() and "@behavior" in text:
                    row["source"] = source
                    break
    for row in rows:
        if row["kind"] == "asm":
            row["name"] = (
                f"dispatchStateByte{row['slot']}Table{row['table'] & 0xFFFF:04x}"
                f"{token}_{row['address']:08X}"
            )
    names = {}
    for line in (cfg / "symbols.txt").read_text().splitlines():
        found = re.match(r"(\w+) = 0x([0-9a-fA-F]+);", line)
        if found:
            names[int(found.group(2), 16)] = found.group(1)
    renames = {}
    for row in rows:
        owner = names.get(row["table"])
        if owner and owner.startswith("func_"):
            renames[owner] = f"dispatchHandlerTable{row['table']:08X}"
            row["table_symbol"] = renames[owner]
        else:
            row["table_symbol"] = owner or f"D_{row['table']:08X}"
    return {"cfg": cfg, "load": load, "rows": rows, "splat": splat,
            "token": token, "renames": renames}


def block_text(row: dict) -> str:
    return (
        f"/* @source 0x{row['address']:08X}\n"
        f" * @behavior Overlay step dispatch: reads the state byte at +{row['slot']} of\n"
        f" *           the work record published at the scratchpad pointer cell\n"
        f" *           0x1F800044 and calls the handler pointer at that index of the\n"
        f" *           local handler table based at 0x{row['table']:08X}. Takes no\n"
        f" *           arguments and returns nothing.\n"
        f" * @status exact\n"
        f" * @match 100.00\n"
        f" * @residual none\n"
        f" */"
    )


def apply(target: str, planned: dict) -> dict:
    cfg = planned["cfg"]
    rows = planned["rows"]
    _, family, area, index = target.split("/")
    existing = re.findall(r'"(src/bof3/[^"]+\.c)"', (cfg / "target.toml").read_text())
    source_dir = str(Path(existing[0]).parent) if existing else "src/bof3/world"
    grouped = f"{source_dir}/{area}{index}DispatchTables.c"
    journal = {"created": [grouped], "deleted": [], "config": [
        str((cfg / name).relative_to(ROOT)) for name in ("target.toml", "symbols.txt", "splat.yaml")
    ]}
    blocks = []
    for row in rows:
        if row["kind"] == "c" and row["source"]:
            path = ROOT / row["source"]
            text = path.read_text()
            if len(re.findall(r"@behavior", text)) > 1:
                return {"skip": f"{row['source']} holds several functions"}
            marker = f"0x{row['address']:08X}"
            found = [
                b
                for b in re.findall(r"/\*.*?\*/", text, re.S)
                if "@source" in b and marker.lower() in b.lower()
            ]
            if not found:
                return {"skip": f"{row['source']} has no block for {marker}"}
            blocks.append({"row": row, "block": found[-1].strip()})
        else:
            blocks.append({"row": row, "block": block_text(row)})
    lines = ['#include "bof3/emi/dispatchers.h"', ""]
    lines.append(
        "\n".join(
            f"extern void (*{row['table_symbol']}[])(void); "
            f"/* @source 0x{row['table']:08X} @kind data */"
            for row in rows
        )
    )
    for item in blocks:
        row = item["row"]
        variant = "ONE" if row["slot"] == 1 else "TWO"
        lines += [
            "",
            item["block"],
            f"EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_{row['slot']}, "
            f"{row['name']}, {row['table_symbol']});",
        ]
    (ROOT / grouped).parent.mkdir(parents=True, exist_ok=True)
    (ROOT / grouped).write_text("\n".join(lines).rstrip() + "\n")
    for row in rows:
        if row["kind"] == "c" and row["source"] and (ROOT / row["source"]).is_file():
            (ROOT / row["source"]).unlink()
            journal["deleted"].append(row["source"])
    manifest = cfg / "target.toml"
    text = manifest.read_text()
    for source in journal["deleted"]:
        text = text.replace(f'    "{source}",\n', "", 1)
    if grouped not in text:
        first = next(line for line in text.splitlines(True) if line.strip().startswith('"src/bof3/'))
        text = text.replace(first, first + f'    "{grouped}",\n', 1)
    manifest.write_text(text)
    symbols = cfg / "symbols.txt"
    text = symbols.read_text()
    for old_name, new_name in planned.get("renames", {}).items():
        text = text.replace(f"{old_name} = ", f"{new_name} = ")
    for row in rows:
        if row["kind"] == "asm":
            text = text.replace(f"{row['label']} = ", f"{row['name']} = ")
    symbols.write_text(text)
    splat = cfg / "splat.yaml"
    text = splat.read_text()
    out = []
    cursor = 0
    by_offset = {row["offset"]: row for row in rows}
    table_pointer_offsets = {
        row["table"] - planned["load"]: row["table_symbol"] for row in rows
    }
    for entry in entries(text):
        if entry["offset"] in table_pointer_offsets:
            out.append(text[cursor:entry["start"]])
            cursor = entry["end"]
            out.append(
                f"  - - {entry['offset']}\n    - asm\n    - "
                f"{table_pointer_offsets[entry['offset']]}\n"
            )
            continue
        out.append(text[cursor:entry["start"]])
        cursor = entry["end"]
        row = by_offset.get(entry["offset"])
        if row is None:
            out.append(text[entry["start"]:entry["end"]])
            continue
        label = row["name"]
        head = f"  - - {entry['offset']}\n    - c\n    - {label}\n"
        tail = ""
        for line in entry["body"][2:]:
            if SOURCE_RE.match(line):
                continue
            tail += line + "\n"
        out.append(head + f"    - '@source: {grouped}'\n" + tail)
    out.append(text[cursor:])
    splat.write_text("".join(out))
    return {"groups": blocks, "journal": journal, "grouped": grouped}


def rollback(target: str, journal: dict) -> None:
    for path in journal["config"]:
        subprocess.run(["git", "checkout", "--", path], cwd=ROOT, check=False)
    for path in journal["deleted"]:
        subprocess.run(["git", "checkout", "--", path], cwd=ROOT, check=False)
    for path in journal["created"]:
        full = ROOT / path
        if full.is_file():
            full.unlink()


def gate(target: str, rows: list[dict]) -> list[str]:
    built = harness("build", target)
    if built.returncode:
        return [f"build failed: {built.stdout[-200:]}"]
    problems = []
    for row in rows:
        result = harness("lift", "gate", "--target", target, f"{target}@0x{row['address']:08X}")
        if "gate" not in result.stdout or f"@0x{row['address']:08X}: PASS" not in result.stdout:
            detail = [line for line in result.stdout.splitlines() if line.startswith(("DIFF", "error"))]
            lines = [line for line in result.stdout.splitlines()
                     if line.startswith(("DIFF", "error", "metadata"))]
            problems.append(f"0x{row['address']:08X}: {lines[:2]}")
    return problems


def run(targets: list[str]) -> None:
    good, skipped, failed = [], [], []
    for target in targets:
        planned = plan(target)
        if "skip" in planned:
            skipped.append((target, planned["skip"]))
            continue
        applied = apply(target, planned)
        if "skip" in applied:
            rollback(target, {"config": [str((ROOT / "config/targets" / target / n).relative_to(ROOT))
                                         for n in ("target.toml", "symbols.txt", "splat.yaml")],
                              "deleted": [], "created": []})
            skipped.append((target, applied["skip"]))
            continue
        problems = gate(target, planned["rows"])
        if problems:
            rollback(target, applied["journal"])
            failed.append((target, problems))
        else:
            good.append((target, len(planned["rows"])))
    print(json.dumps({"done": good, "skipped": skipped, "failed": failed}, indent=1))


if __name__ == "__main__":
    run(sys.argv[1:])
