"""Capture this naming batch's 23 partials after the parent payload finishes."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/python"))

from harness.common.lease import acquire_writer
from harness.common.process import run_bounded
from harness.domain.functions import parse_function_records
from harness.match.extraction import read_function_bytes
from harness.match.members import capture_members
from harness.match.validation import _evidence, _exact_evidence

READY = ROOT / "out/source-refinement/first-naming-ready.json"


def sha(content):
    return hashlib.sha256(content).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def confined(path):
    path = Path(path)
    path = path if path.is_absolute() else ROOT / path
    if not path.resolve().is_relative_to(ROOT):
        raise ValueError(f"path leaves repository: {path}")
    return path


def verify_pins(pins):
    for name, expected in pins.items():
        path = confined(name)
        observed = sha(path.read_bytes()) if path.is_file() else None
        if observed != expected:
            raise ValueError(f"input changed: {name}")


def capture_tag_free(ready, payload, output):
    image = confined(payload["output"]).read_bytes()
    if (payload["schema"] != "harness.build-payload/v1"
            or payload["target"] != "emi/etc/game/00"
            or sha(image) != payload["sha256"]):
        raise ValueError("Game00 payload identity or image changed")
    (output / "payload-before.bin").write_bytes(image)
    records = []
    comparisons = {row["source"]: row["comparison"]
                   for row in payload["source_comparisons"]}
    for member in ready["unrecorded_progress_members"]:
        source = member["source"]
        comparison = comparisons[source]
        captured = capture_members(ROOT, source)
        original = captured["functions"][0]
        selected = [row for row in payload["compiled"]
                    if row["source"] == source
                    and row["symbol"] == member["spelling"]
                    and row["owners"] == [member["selector"]]]
        if len(selected) != 1:
            raise ValueError(f"tag-free compiled range missing: {source}")
        selected = selected[0]
        current = image[selected["start"]:selected["end"]]
        evidence = comparison["function_image"]
        if (comparison["status"] != "exact_match"
                or comparison["byte_match"] is not True
                or comparison["fresh_compilation"] is not True
                or comparison["source_image"]["sha256"]
                != ready["source_config_sha256"][source]
                or selected["address"] != original["address"]
                or comparison["original_size"] != len(original["original"])
                or comparison["current_size"] != len(current)
                or sha(current) != selected["sha256"]
                or sha(current) != evidence["current_sha256"]
                or sha(original["original"]) != evidence["original_sha256"]
                or current != original["original"]):
            raise ValueError(f"tag-free member needs review: {source}")
        for field in ("native_inputs", "include_inputs"):
            verify_pins({name: state["sha256"] if state else None
                         for name, state in comparison[field].items()})
        directory = output / ("tag-free-" + member["selector"].split("@")[-1])
        directory.mkdir()
        (directory / "original.bin").write_bytes(original["original"])
        (directory / "current.bin").write_bytes(current)
        save(directory / "payload-comparison.json", comparison)
        records.append({"selector": member["selector"], "source": source,
                        "status": "native_exact_source_progress_tag_free",
                        "size": len(current), "sha256": sha(current),
                        "images": str(directory.relative_to(ROOT))})
    return records


def preserve_payload_partial_images(ready, payload, output):
    comparisons = {row["source"]: row["comparison"]
                   for row in payload["source_comparisons"]}
    preserved = []
    for member in ready["retained_partial_members"]:
        if member["target"] != "emi/etc/game/00":
            continue
        source = member["source"]
        comparison = comparisons[source]
        captured = capture_members(ROOT, source)
        original = captured["functions"][0]["original"]
        linked = confined(comparison["current_object"]).with_suffix(".linked.o")
        elf = linked.read_bytes()
        native = comparison["native_outputs"][linked.relative_to(ROOT).as_posix()]
        current = read_function_bytes(elf, function_name=member["spelling"],
                                      address=int(member["selector"].split("@")[1], 16)).content
        evidence = comparison["function_image"]
        if (sha(elf) != native["sha256"]
                or sha(original) != evidence["original_sha256"]
                or sha(current) != evidence["current_sha256"]
                or len(original) != comparison["original_size"]
                or len(current) != comparison["current_size"]
                or comparison["source_image"]["sha256"]
                != ready["source_config_sha256"][source]):
            raise ValueError(f"payload partial image changed before preservation: {source}")
        directory = output / ("payload-before-" + member["selector"].split("@")[-1])
        directory.mkdir()
        (directory / "original.bin").write_bytes(original)
        (directory / "current.bin").write_bytes(current)
        (directory / "current.linked.o").write_bytes(elf)
        save(directory / "payload-comparison.json", comparison)
        preserved.append({"selector": member["selector"], "directory": str(directory.relative_to(ROOT)),
                          "function_image": evidence})
    return preserved


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ready", default=str(READY))
    parser.add_argument("--payload-report", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()
    output = confined(args.out)
    if not output.resolve().is_relative_to(ROOT / "out/source-refinement"):
        raise ValueError("output must be under out/source-refinement")
    output.mkdir(parents=True, exist_ok=False)
    state = {"schema": "disposable.first-naming-partials/v1",
             "status": "preparing", "partials": [], "tag_free": [],
             "active_selector": None, "acceptance": "diagnostics only",
             "canonical_artifact_paths_are_historical_after_diagnostics": True}
    active = None
    standard = None
    try:
        ready_bytes = confined(args.ready).read_bytes()
        ready = json.loads(ready_bytes)
        partials = ready["retained_partial_members"]
        if (len(partials) != 23 or len(ready["unrecorded_progress_members"]) != 3
                or ready["closure_total"]["translation_units"] != 255
                or ready["closure_total"]["members"] != 281):
            raise ValueError("expected fixed first-naming closure and 23+3 members")
        pins = {name: digest for name, digest in ready["source_config_sha256"].items()
                if name.startswith(("src/", "include/", "config/", "tools/"))
                or name == "CMakeLists.txt"}
        payload_path = confined(args.payload_report)
        payload_bytes = payload_path.read_bytes()
        payload = json.loads(payload_bytes)
        (output / "ready.json").write_bytes(ready_bytes)
        (output / "payload-report.json").write_bytes(payload_bytes)
        state["ready_sha256"] = sha(ready_bytes)
        state["payload_report_sha256"] = sha(payload_bytes)
        state["payload_report_original_path"] = str(payload_path.relative_to(ROOT))
        state["source_config_sha256"] = pins
        with acquire_writer(ROOT):
            verify_pins(pins)
            state["tag_free"] = capture_tag_free(ready, payload, output)
            state["payload_partial_images_before_diagnostics"] = preserve_payload_partial_images(ready, payload, output)
            state["status"] = "running"
            save(output / "summary.json", state)
            for member in partials:
                verify_pins(pins)
                source, selector = member["source"], member["selector"]
                current_records = parse_function_records((ROOT / source).read_text())
                record = next(row for row in current_records
                              if row.address == int(selector.split("@")[1], 16))
                if record.status != "partial" or "@status partial" not in record.metadata:
                    raise ValueError(f"explicit partial changed: {selector}")
                active = output / selector.replace("/", "_").replace("@", "-")
                active.mkdir()
                standard = ROOT / "out/asm-diff" / member["target"].replace("/", "_") / member["spelling"]
                argv = ["bin/harness", "lift", "asm-diff", selector,
                        "--json", "--detail", "full", "--image-evidence"]
                state["active_selector"] = selector
                save(output / "summary.json", state)
                print(f"Measuring {selector}", flush=True)
                with (active / "stdout.txt").open("wb") as stdout, (active / "stderr.txt").open("wb") as stderr:
                    def retain(stream, chunk):
                        handle = stdout if stream == "stdout" else stderr
                        handle.write(chunk)
                        handle.flush()
                    execution = run_bounded(ROOT, argv, output_limit=16 * 1024 * 1024,
                                            on_output=retain)
                save(active / "execution.json", execution)
                if standard.is_dir():
                    shutil.copytree(standard, active / "diagnostics")
                if execution["failure"]:
                    raise ValueError(f"command infrastructure failed: {execution['failure']}")
                check = {"argv": argv, "target": member["target"],
                         "selector": selector, "function": member["spelling"], "source": source}
                code = execution["exit_code"]
                if code == 1:
                    _evidence(check, execution["stdout"], ROOT)
                elif code != 0 or not _exact_evidence(check, execution["stdout"], ROOT):
                    raise ValueError(f"unexpected command failure or exact schema: {selector}")
                measured = json.loads(execution["stdout"])
                linked = confined(measured["current_object"]).with_suffix(".linked.o")
                elf = linked.read_bytes()
                current = read_function_bytes(elf, function_name=member["spelling"],
                                              address=int(selector.split("@")[1], 16)).content
                original = confined(measured["outputs"]["original_bytes"]).read_bytes()
                evidence = measured["function_image"]
                if (sha(current) != evidence["current_sha256"]
                        or sha(original) != evidence["original_sha256"]
                        or len(current) != measured["current_size"]
                        or len(original) != measured["original_size"]):
                    raise ValueError(f"function image changed: {selector}")
                (active / "original.bin").write_bytes(original)
                (active / "current.bin").write_bytes(current)
                (active / "current.linked.o").write_bytes(elf)
                save(active / "measurement.json", measured)
                state["partials"].append({"selector": selector, "source": source,
                    "exit_code": code, "byte_match": measured["byte_match"],
                    "original_size": len(original), "current_size": len(current),
                    "instruction_count": measured["instruction_count"],
                    "first_mismatch": measured["first_mismatch"],
                    "function_image": evidence, "directory": str(active.relative_to(ROOT))})
                state["active_selector"] = None
                save(output / "summary.json", state)
            verify_pins(pins)
        state["status"] = "complete"
        save(output / "summary.json", state)
        print(f"Saved 23 measurements and 3 tag-free image comparisons to {output}")
        return 0
    except BaseException as error:
        state["status"] = "stopped"
        state["failure"] = f"{type(error).__name__}: {error}"
        if getattr(error, "observed", None) is not None:
            state["process_failure_observed"] = error.observed
        save(output / "summary.json", state)
        if active is not None and standard is not None and standard.is_dir() and not (active / "diagnostics").exists():
            try:
                shutil.copytree(standard, active / "diagnostics")
            except OSError as copy_error:
                state["failure_artifact_copy_error"] = str(copy_error)
                save(output / "summary.json", state)
        raise


if __name__ == "__main__":
    raise SystemExit(main())
