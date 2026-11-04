"""Prove original flow for boundary-promotion batches and stage the Splat edit.

Read-only against production: writes only under the given staging directory.
"""
import argparse
import hashlib
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
INS = re.compile(r"^\s*([0-9a-f]+):\s+([0-9a-f ]+)\s+(\S+)\s*(.*)$")
BRANCH = {"beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez", "bgezal", "bltzal"}


def sha(body):
    return hashlib.sha256(body).hexdigest()


def entries(lines):
    rows = []
    for n, line in enumerate(lines):
        m = re.match(r"\s*-\s*-\s*(\d+)\s*$", line)
        if m is None:
            continue
        kind = lines[n + 1].strip().lstrip("- ").strip() if n + 1 < len(lines) else None
        name = lines[n + 2].strip().lstrip("- ").strip() if n + 2 < len(lines) else None
        source = None
        for k in range(n + 3, min(n + 9, len(lines))):
            if re.match(r"\s*-\s*-\s*\d+\s*$", lines[k]):
                break
            s = re.match(r"\s*-\s*'@source:\s*([^']+)'", lines[k])
            if s is not None:
                source = s.group(1)
                break
        rows.append({"index": n, "offset": int(m.group(1)), "kind": kind, "name": name, "source": source})
    return rows


def flow(objdump, blob, load, entry, nxt):
    va = entry["offset"] + load
    raw = blob[entry["offset"]:nxt]
    directory = STAGE / "disasm"
    directory.mkdir(parents=True, exist_ok=True)
    sample = directory / f"{va:08X}.bin"
    sample.write_bytes(raw)
    run = subprocess.run(
        [objdump, "-D", "-b", "binary", "-m", "mips:3000", "-EL", f"--adjust-vma=0x{va:08X}", str(sample)],
        capture_output=True,
        text=True,
    )
    (directory / f"{va:08X}.txt").write_text(run.stdout)
    ins = []
    for line in run.stdout.splitlines():
        m = INS.match(line)
        if m is not None:
            ins.append((int(m.group(1), 16), m.group(3), m.group(4).strip()))
    end = va + len(raw)
    returns, calls, branches, outside = [], [], [], []
    for pc, op, args in ins:
        target = re.search(r"0x([0-9a-f]+)", args)
        if op == "jr":
            returns.append(pc)
        elif op in ("jal", "j"):
            if target is not None:
                calls.append(int(target.group(1), 16))
                if op == "j" and not va <= int(target.group(1), 16) < end:
                    outside.append({"pc": hex(pc), "op": op, "target": target.group(1)})
        elif op in BRANCH and target is not None:
            t = int(target.group(1), 16)
            branches.append((pc, op, t))
            if not va <= t < end:
                outside.append({"pc": hex(pc), "op": op, "target": hex(t)})
    return {
        "selector": f"{TARGET}@0x{va:08X}",
        "name": entry["name"],
        "current_kind": entry["kind"],
        "range": {"runtime_start": hex(va), "runtime_end_exclusive": hex(end), "size": len(raw), "sha256": sha(raw)},
        "instruction_count": len(ins),
        "returns": [hex(x) for x in returns],
        "jumps": [hex(x) for x in calls],
        "branches_total": len(branches),
        "branches_outside": outside,
        "first_instructions": [f"{op} {a}" for _, op, a in ins[:3]],
        "last_instructions": [f"{op} {a}" for _, op, a in ins[-3:]],
    }


def main():
    global STAGE, TARGET
    parser = argparse.ArgumentParser()
    parser.add_argument("--target", required=True)
    parser.add_argument("--selectors", required=True)
    parser.add_argument("--stage", required=True)
    args = parser.parse_args()
    TARGET = args.target
    STAGE = ROOT / args.stage
    if STAGE.exists():
        raise SystemExit(f"staging directory already exists: {STAGE}")
    STAGE.mkdir(parents=True)
    wanted = {int(s, 16) for s in args.selectors.split(",")}
    toml = (ROOT / f"config/targets/{TARGET}/target.toml").read_text()
    load = int(re.search(r"^load_address\s*=\s*(0x[0-9A-Fa-f]+)", toml, re.M).group(1), 16)
    blob = (ROOT / re.search(r'^binary\s*=\s*"([^"]+)"', toml, re.M).group(1)).read_bytes()
    splat_rel = f"config/targets/{TARGET}/splat.yaml"
    lines = (ROOT / splat_rel).read_text().splitlines()
    rows = entries(lines)
    listed = json.loads(
        subprocess.run(
            [".venv/bin/python", "-m", "harness.cli", "source", "metadata", "list", TARGET, "--limit", "400", "--json"],
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=False,
        ).stdout
    )
    sources = {}
    for record in listed.get("records") or []:
        address = record.get("address")
        address = address if isinstance(address, str) else hex(address)
        sources[address.lower()] = record.get("source")
    objdump = str(ROOT / "toolchains/psyqo/bin/mipsel-none-elf-objdump")
    proofs, staged = [], []
    for i, entry in enumerate(rows):
        va = entry["offset"] + load
        if va not in wanted:
            continue
        nxt = rows[i + 1]["offset"] if i + 1 < len(rows) else len(blob)
        proof = flow(objdump, blob, load, entry, nxt)
        proof["current_source"] = entry["source"]
        proof["planned_source"] = sources.get(f"0x{va:08X}".lower())
        proof["kind_after"] = "c" if entry["kind"] == "asm" else entry["kind"]
        proof["insert_source"] = entry["source"] is None
        proofs.append(proof)
    missing = [hex(a) for a in sorted(wanted) if not any(int(p["range"]["runtime_start"], 16) == a for p in proofs)]
    assert not missing, f"selectors not found: {missing}"
    by_offset = {int(p["range"]["runtime_start"], 16): p for p in proofs}
    row_at = {row["index"]: row for row in rows}
    out_lines, i = [], 0
    while i < len(lines):
        row = row_at.get(i)
        if row is None:
            out_lines.append(lines[i])
            i += 1
            continue
        proof = by_offset.get(row["offset"] + load)
        if i + 2 >= len(lines):
            out_lines.extend(lines[i:])
            break
        out_lines.append(lines[i])
        out_lines.append(lines[i + 1])
        out_lines.append(lines[i + 2])
        i += 3
        if proof is None:
            continue
        indent = re.match(r"(\s*)", lines[i - 2]).group(1)
        out_lines[-2] = f"{indent}- {proof['kind_after']}"
        if proof["insert_source"]:
            assert proof["planned_source"], proof["selector"]
            out_lines.append(f"{indent}- '@source: {proof['planned_source']}'")
        staged.append(
            {
                "selector": proof["selector"],
                "kind_before": proof["current_kind"],
                "kind_after": proof["kind_after"],
                "source": proof["planned_source"],
                "inserted_source": proof["insert_source"],
            }
        )
    new_text = "\n".join(out_lines) + "\n"
    (STAGE / "splat.yaml").write_text(new_text)
    report = {
        "target": TARGET,
        "splat": splat_rel,
        "splat_pre_sha256": sha((ROOT / splat_rel).read_bytes()),
        "splat_staged_sha256": sha(new_text.encode()),
        "members": len(proofs),
        "all_branch_closed": all(not p["branches_outside"] for p in proofs),
        "all_single_return": all(len(p["returns"]) == 1 for p in proofs),
        "proofs": proofs,
        "staged_changes": staged,
    }
    (STAGE / "flow.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({k: report[k] for k in ("target", "members", "all_branch_closed", "all_single_return", "splat_pre_sha256", "splat_staged_sha256")}, indent=1))
    for p in proofs:
        print(f"  {p['selector']} {p['name']} {p['current_kind']}->{p['kind_after']} size={p['range']['size']} rets={len(p['returns'])} outside={len(p['branches_outside'])} src={p['planned_source']}")


if __name__ == "__main__":
    main()
