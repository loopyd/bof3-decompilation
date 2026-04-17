from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import normalize_repo_path, write_json_output
from . import pipeline_ready, workspace as workspace_lib


COMMUTATIVE_RTYPE_MNEMONICS = {"add", "addu", "and", "or", "xor"}
MEMORY_MNEMONICS = {
    "lb",
    "lbu",
    "lh",
    "lhu",
    "lw",
    "sb",
    "sh",
    "sw",
}
ADDRESS_IMM_MNEMONICS = {"lui", "addiu", "ori", *MEMORY_MNEMONICS}
CATEGORY_ORDER = (
    "move_zero_sugar",
    "li_zero_sugar",
    "branch_zero_sugar",
    "commutative_swap",
    "call_target_reloc",
    "address_materialization",
)
MEMORY_OPERAND_RE = re.compile(
    r"^(?P<dst>[^,]+),\s*(?P<imm>-?(?:0x[0-9a-fA-F]+|[0-9]+))\((?P<base>[^)]+)\)$"
)


def maybe_load_json(text: str) -> dict[str, Any] | None:
    payload = text.strip()
    if not payload:
        return None
    try:
        loaded = json.loads(payload)
    except json.JSONDecodeError:
        return None
    if not isinstance(loaded, dict):
        return None
    return loaded


def normalize_register(text: str) -> str:
    return text.strip().lstrip("$")


def normalize_instruction_text(text: str | None) -> str | None:
    if text is None:
        return None
    return " ".join(text.strip().split())


def parse_instruction(text: str | None) -> tuple[str, list[str]] | None:
    normalized = normalize_instruction_text(text)
    if not normalized or normalized.endswith(":") or normalized.startswith("."):
        return None
    parts = normalized.split(None, 1)
    if not parts:
        return None
    mnemonic = parts[0]
    operands = [] if len(parts) == 1 else [part.strip() for part in parts[1].split(",")]
    return mnemonic, operands


def match_memory_operands(operands: list[str]) -> tuple[str, str, str] | None:
    if len(operands) != 2:
        return None
    match = MEMORY_OPERAND_RE.fullmatch(", ".join(operands))
    if match is None:
        return None
    return (
        normalize_register(match.group("dst")),
        normalize_register(match.group("base")),
        match.group("imm"),
    )


def normalized_move_signature(parsed: tuple[str, list[str]] | None) -> tuple[str, str] | None:
    if parsed is None:
        return None
    mnemonic, operands = parsed
    if mnemonic == "move" and len(operands) == 2:
        return normalize_register(operands[0]), normalize_register(operands[1])
    if mnemonic not in {"or", "addu", "add"} or len(operands) != 3:
        return None
    dst = normalize_register(operands[0])
    left = normalize_register(operands[1])
    right = normalize_register(operands[2])
    if left == "zero":
        return dst, right
    if right == "zero":
        return dst, left
    return None


def normalized_li_signature(parsed: tuple[str, list[str]] | None) -> tuple[str, str] | None:
    if parsed is None:
        return None
    mnemonic, operands = parsed
    if mnemonic == "li" and len(operands) == 2:
        return normalize_register(operands[0]), operands[1]
    if mnemonic in {"addiu", "ori"} and len(operands) == 3:
        if normalize_register(operands[1]) == "zero":
            return normalize_register(operands[0]), operands[2]
    return None


def normalized_branch_signature(parsed: tuple[str, list[str]] | None) -> tuple[str, str | None, str] | None:
    if parsed is None:
        return None
    mnemonic, operands = parsed
    if mnemonic == "b" and len(operands) == 1:
        return "b", None, operands[0]
    if mnemonic in {"beqz", "bnez"} and len(operands) == 2:
        return mnemonic, normalize_register(operands[0]), operands[1]
    if mnemonic == "beq" and len(operands) == 3:
        left = normalize_register(operands[0])
        right = normalize_register(operands[1])
        if left == right:
            return "b", None, operands[2]
        if left == "zero":
            return "beqz", right, operands[2]
        if right == "zero":
            return "beqz", left, operands[2]
    if mnemonic == "bne" and len(operands) == 3:
        left = normalize_register(operands[0])
        right = normalize_register(operands[1])
        if left == "zero":
            return "bnez", right, operands[2]
        if right == "zero":
            return "bnez", left, operands[2]
    return None


def is_commutative_swap(
    left: tuple[str, list[str]] | None, right: tuple[str, list[str]] | None
) -> bool:
    if left is None or right is None:
        return False
    left_mnemonic, left_operands = left
    right_mnemonic, right_operands = right
    if left_mnemonic != right_mnemonic or left_mnemonic not in COMMUTATIVE_RTYPE_MNEMONICS:
        return False
    if len(left_operands) != 3 or len(right_operands) != 3:
        return False
    left_dst = normalize_register(left_operands[0])
    right_dst = normalize_register(right_operands[0])
    if left_dst != right_dst:
        return False
    left_pair = tuple(normalize_register(item) for item in left_operands[1:])
    right_pair = tuple(normalize_register(item) for item in right_operands[1:])
    if "zero" in left_pair or "zero" in right_pair:
        return False
    return left_pair != right_pair and set(left_pair) == set(right_pair)


def is_call_target_reloc(
    left: tuple[str, list[str]] | None, right: tuple[str, list[str]] | None
) -> bool:
    if left is None or right is None:
        return False
    left_mnemonic, left_operands = left
    right_mnemonic, right_operands = right
    if left_mnemonic != right_mnemonic or left_mnemonic not in {"jal", "j"}:
        return False
    if len(left_operands) != 1 or len(right_operands) != 1:
        return False
    return left_operands[0] != right_operands[0]


def is_address_materialization(
    left: tuple[str, list[str]] | None, right: tuple[str, list[str]] | None
) -> bool:
    if left is None or right is None:
        return False
    left_mnemonic, left_operands = left
    right_mnemonic, right_operands = right
    if left_mnemonic != right_mnemonic or left_mnemonic not in ADDRESS_IMM_MNEMONICS:
        return False
    if left_mnemonic == "lui" and len(left_operands) == 2 and len(right_operands) == 2:
        return normalize_register(left_operands[0]) == normalize_register(right_operands[0])
    if left_mnemonic in {"addiu", "ori"} and len(left_operands) == 3 and len(right_operands) == 3:
        return (
            normalize_register(left_operands[0]) == normalize_register(right_operands[0])
            and normalize_register(left_operands[1]) == normalize_register(right_operands[1])
        )
    if left_mnemonic in MEMORY_MNEMONICS:
        left_memory = match_memory_operands(left_operands)
        right_memory = match_memory_operands(right_operands)
        if left_memory is None or right_memory is None:
            return False
        return left_memory[:2] == right_memory[:2]
    return False


def classify_pair(
    left_text: str | None,
    right_text: str | None,
    diff_kind: str | None,
) -> str | None:
    if not diff_kind or diff_kind == "DIFF_NONE":
        return None
    left = parse_instruction(left_text)
    right = parse_instruction(right_text)
    if normalized_move_signature(left) is not None and normalized_move_signature(left) == normalized_move_signature(right):
        return "move_zero_sugar"
    if normalized_li_signature(left) is not None and normalized_li_signature(left) == normalized_li_signature(right):
        return "li_zero_sugar"
    if normalized_branch_signature(left) is not None and normalized_branch_signature(left) == normalized_branch_signature(right):
        return "branch_zero_sugar"
    if is_commutative_swap(left, right):
        return "commutative_swap"
    if is_call_target_reloc(left, right):
        return "call_target_reloc"
    if is_address_materialization(left, right):
        return "address_materialization"
    return None


def select_symbol_payload(
    stdout_json: dict[str, Any],
    symbol_name: str | None,
) -> tuple[dict[str, Any] | None, dict[str, Any] | None, str | None]:
    left_symbols = (stdout_json.get("left") or {}).get("symbols") or []
    right_symbols = (stdout_json.get("right") or {}).get("symbols") or []
    candidate_name = symbol_name
    if candidate_name is None:
        for symbol in left_symbols:
            if symbol.get("kind") == "SYMBOL_FUNCTION" and symbol.get("instructions"):
                candidate_name = symbol.get("name")
                break
    if candidate_name is None:
        return None, None, None
    left = next((item for item in left_symbols if item.get("name") == candidate_name), None)
    right = next((item for item in right_symbols if item.get("name") == candidate_name), None)
    return left, right, candidate_name


def classify_objdiff_payload(
    stdout_json: dict[str, Any],
    *,
    symbol_name: str | None = None,
    asm_score: int | None = None,
) -> dict[str, Any]:
    left_symbol, right_symbol, resolved_symbol = select_symbol_payload(stdout_json, symbol_name)
    left_instructions = (left_symbol or {}).get("instructions") or []
    right_instructions = (right_symbol or {}).get("instructions") or []
    total_mismatches = 0
    category_counts = {name: 0 for name in CATEGORY_ORDER}
    category_examples: dict[str, list[dict[str, str]]] = {name: [] for name in CATEGORY_ORDER}
    unclassified_examples: list[dict[str, str]] = []

    for index, left_instruction in enumerate(left_instructions):
        right_instruction = right_instructions[index] if index < len(right_instructions) else {}
        diff_kind = left_instruction.get("diff_kind") or right_instruction.get("diff_kind")
        if not diff_kind or diff_kind == "DIFF_NONE":
            continue
        total_mismatches += 1
        left_text = ((left_instruction.get("instruction") or {}).get("formatted"))
        right_text = ((right_instruction.get("instruction") or {}).get("formatted"))
        category = classify_pair(left_text, right_text, diff_kind)
        example = {
            "left": normalize_instruction_text(left_text) or "",
            "right": normalize_instruction_text(right_text) or "",
            "diff_kind": str(diff_kind),
        }
        if category is None:
            if len(unclassified_examples) < 8:
                unclassified_examples.append(example)
            continue
        category_counts[category] += 1
        if len(category_examples[category]) < 8:
            category_examples[category].append(example)

    classified_count = sum(category_counts.values())
    unclassified_count = total_mismatches - classified_count
    asm_view_only_noise = total_mismatches == 0 and int(asm_score or 0) > 0
    if asm_view_only_noise:
        semantic_status = "asm_view_only_noise"
    elif total_mismatches == 0:
        semantic_status = "exact"
    elif unclassified_count == 0:
        sugar_count = (
            category_counts["move_zero_sugar"]
            + category_counts["li_zero_sugar"]
            + category_counts["branch_zero_sugar"]
            + category_counts["commutative_swap"]
        )
        reloc_count = (
            category_counts["call_target_reloc"]
            + category_counts["address_materialization"]
        )
        if sugar_count and not reloc_count:
            semantic_status = "sugar_only"
        elif reloc_count and not sugar_count:
            semantic_status = "relocation_only"
        else:
            semantic_status = "mixed_low_level"
    else:
        semantic_status = "structural"

    active_categories = [name for name in CATEGORY_ORDER if category_counts[name] > 0]
    return {
        "symbol_name": resolved_symbol,
        "semantic_status": semantic_status,
        "asm_view_only_noise": asm_view_only_noise,
        "total_mismatch_count": total_mismatches,
        "classified_mismatch_count": classified_count,
        "unclassified_mismatch_count": unclassified_count,
        "category_counts": category_counts,
        "active_categories": active_categories,
        "category_examples": {
            name: examples
            for name, examples in category_examples.items()
            if examples
        },
        "unclassified_examples": unclassified_examples,
    }


def backend_layout(workspace_dir: Path) -> dict[str, Path]:
    backend_dir = workspace_dir / "semantic_diff"
    return {
        "backend_dir": backend_dir,
        "report": backend_dir / "backend.json",
    }


def build_backend_report(
    workspace_dir: Path,
    workspace_payload: dict[str, Any],
    *,
    asm_backend_report: dict[str, Any],
    obj_backend_report: dict[str, Any],
) -> dict[str, Any]:
    stdout_path = normalize_repo_path(obj_backend_report.get("stdout_path"))
    if stdout_path is None or not stdout_path.exists():
        raise FileNotFoundError("semantic-diff requires objdiff stdout json")
    stdout_json = maybe_load_json(stdout_path.read_text(encoding="utf-8"))
    if stdout_json is None:
        raise ValueError("semantic-diff could not parse objdiff stdout json")
    source_mapping = workspace_payload.get("source_mapping") or {}
    symbol_name = source_mapping.get("source_function") or workspace_payload.get("name")
    summary = classify_objdiff_payload(
        stdout_json,
        symbol_name=str(symbol_name) if symbol_name else None,
        asm_score=((asm_backend_report.get("diff_summary") or {}).get("current_score")),
    )
    layout = backend_layout(workspace_dir)
    layout["backend_dir"].mkdir(parents=True, exist_ok=True)
    report = {
        "backend": "semantic-diff",
        "backend_dir": workspace_lib.relative_to_root(layout["backend_dir"]),
        "report_path": workspace_lib.relative_to_root(layout["report"]),
        "objdiff_stdout_path": obj_backend_report.get("stdout_path"),
        "asm_differ_report_path": asm_backend_report.get("report_path"),
        "workspace_dir": workspace_payload.get("workspace_dir"),
        "symbol_name": summary.get("symbol_name"),
        "succeeded": True,
        "diff_summary": summary,
    }
    write_json_output(layout["report"], report)
    return report


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "semantic-diff"),
        description=(
            "Classify remaining objdiff mismatches into sugar, relocation, and "
            "structural buckets."
        ),
    )
    add_logging_args(parser)
    pipeline_ready.add_workspace_resolver_args(parser)
    parser.add_argument(
        "--objdiff-json",
        default=None,
        help="Objdiff stdout json path, or '-' to read from stdin.",
    )
    parser.add_argument(
        "--asm-differ-json",
        default=None,
        help="Optional asm-differ stdout json path for asm-view-only-noise checks.",
    )
    parser.add_argument("--symbol", default=None)
    parser.add_argument("-o", "--output-json", type=Path, default=None)
    parser.add_argument("--dry-run", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def workspace_requested(args: argparse.Namespace) -> bool:
    return bool(getattr(args, "workspace_json", None)) or bool(
        getattr(args, "program", None) and getattr(args, "entry", None)
    )


def resolve_workspace_json_inputs(
    args: argparse.Namespace, logger: object
) -> tuple[dict[str, Any], str | None, int | None] | None:
    resolved = pipeline_ready.resolve_workspace(args, logger)
    if resolved is None:
        return None
    workspace_json, workspace_payload = resolved
    workspace_dir = workspace_json.parent
    objdiff_path = workspace_dir / "objdiff" / "diff.stdout.json"
    if not objdiff_path.exists():
        logger.error(f"objdiff json not found: {objdiff_path}")
        return None
    asm_score = None
    asm_report_path = workspace_dir / "asm_differ" / "backend.json"
    if asm_report_path.exists():
        asm_report = maybe_load_json(asm_report_path.read_text(encoding="utf-8")) or {}
        asm_score = ((asm_report.get("diff_summary") or {}).get("current_score"))
    objdiff_json = maybe_load_json(objdiff_path.read_text(encoding="utf-8"))
    if objdiff_json is None:
        logger.error(f"failed to parse objdiff json: {objdiff_path}")
        return None
    symbol_name = args.symbol
    if symbol_name is None:
        source_mapping = workspace_payload.get("source_mapping") or {}
        symbol_name = source_mapping.get("source_function") or workspace_payload.get("name")
    return objdiff_json, str(symbol_name) if symbol_name else None, asm_score


def resolve_direct_json_inputs(
    args: argparse.Namespace, logger: object
) -> tuple[dict[str, Any], str | None, int | None] | None:
    if not args.objdiff_json:
        logger.error("pass --workspace-json/--program+--entry or --objdiff-json")
        return None
    if args.objdiff_json == "-":
        text = sys.stdin.read()
    else:
        path = Path(args.objdiff_json)
        if not path.exists():
            logger.error(f"objdiff json not found: {path}")
            return None
        text = path.read_text(encoding="utf-8")
    objdiff_json = maybe_load_json(text)
    if objdiff_json is None:
        logger.error("failed to parse objdiff json")
        return None
    asm_score = None
    if args.asm_differ_json:
        asm_path = Path(args.asm_differ_json)
        if not asm_path.exists():
            logger.error(f"asm-differ json not found: {asm_path}")
            return None
        asm_json = maybe_load_json(asm_path.read_text(encoding="utf-8")) or {}
        asm_score = asm_json.get("current_score")
    return objdiff_json, args.symbol, asm_score


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_semantic_diff")

    if args.dry_run:
        logger.summary(
            f"workspace_mode={workspace_requested(args)} output_json={args.output_json or '-'}"
        )
        return 0

    if workspace_requested(args):
        resolved = resolve_workspace_json_inputs(args, logger)
    else:
        resolved = resolve_direct_json_inputs(args, logger)
    if resolved is None:
        return 1
    objdiff_json, symbol_name, asm_score = resolved
    summary = classify_objdiff_payload(
        objdiff_json,
        symbol_name=symbol_name,
        asm_score=asm_score,
    )
    if args.output_json is not None:
        write_json_output(args.output_json, summary)
        logger.summary(
            f"status={summary['semantic_status']} output_json={args.output_json}"
        )
        return 0
    sys.stdout.write(json.dumps(summary, indent=2, sort_keys=True) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
