"""Typed matching experiment ledger and lane state evaluator.

Owns the mechanical shape and legality of one selector's matching
experiments: typed rows (hypothesis, rung, expected/actual effect,
retained/reverted state, score, stall accounting), the durable JSONL
store under ``out/lift-loop/experiment-ledgers/`` (lane-worktree's
``record``/``ledger`` commands stay the write path; this module is the
typed reader/evaluator), and terminal verdicts.  Agents propose
experiments; the ledger owns their recorded state — an agent never
claims exhaustion, the evaluator does.

Rungs are the canonical ladder; a row may only sit on the current or
a later rung, scores never exceed the recorded best without a
baseline, and the stall rule (three non-improving queues advance one
rung) is enforced here, not in agent prose.
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

LADDER = (
    "clean-c",
    "static-allocation",
    "compiler-profile",
    "permuter",
    "compiler-ceiling",
)
STALL_LIMIT = 3
LEAD_LEVERS = ("baseline", "ladder advance", "interruption recovery")
RETAINED_STATES = {
    "retained",
    "restored",
    "restored best coherent baseline",
    "restored best coherent baseline and live validated",
    "no variant retained",
    "no queue variant retained; restored best coherent baseline and live validated",
}


class LedgerError(ValueError):
    """One ledger row violates the typed schema or ladder invariants."""


def _number(value: object, field: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise LedgerError(f"{field} must be a number, got {value!r}")
    return float(value)


@dataclass(frozen=True)
class LedgerRow:
    """One recorded experiment pass (baseline, attempt, or rung advance)."""

    selector: str
    lane_key: str
    attempt: int
    score: float
    rung: str
    lever: str
    hypothesis_id: str
    expected_effect: str
    actual_effect: str
    retained_status: str
    variants: tuple[str, ...]

    @classmethod
    def from_entry(cls, entry: dict[str, Any], lane_key: str | None) -> "LedgerRow":
        row = entry.get("row")
        if not isinstance(row, dict):
            raise LedgerError("ledger entry requires a row object")
        selector = entry.get("selector")
        if not isinstance(selector, str) or selector.count("@") != 1:
            raise LedgerError(f"ledger entry requires a TARGET@0xADDRESS selector: {selector!r}")
        attempt = row.get("attempt")
        if not isinstance(attempt, int) or isinstance(attempt, bool):
            raise LedgerError(f"{selector} ledger row requires an integer attempt")
        if attempt < 0:
            raise LedgerError(f"{selector} ledger attempt is negative")
        score = _number(row.get("score"), f"{selector} ledger score")
        rung = row.get("rung")
        if rung not in LADDER:
            raise LedgerError(f"{selector} ledger row has unknown rung {rung!r}; ladder is {list(LADDER)}")
        lever = row.get("lever")
        hypothesis = row.get("hypothesis_id")
        if not isinstance(lever, str) or not isinstance(hypothesis, str) or not hypothesis.strip():
            raise LedgerError(f"{selector} ledger row requires a lever and a hypothesis_id")
        expected = row.get("expected_effect", row.get("predicted", ""))
        actual = row.get("actual_effect", "")
        retained = row.get("retained_status", "retained")
        if not isinstance(expected, str) or not isinstance(actual, str) or not isinstance(retained, str):
            raise LedgerError(f"{selector} ledger row effects must be strings")
        if retained and retained not in RETAINED_STATES and retained != "retained":
            raise LedgerError(f"{selector} ledger retained_status must be one of {sorted(RETAINED_STATES)}")
        variants = tuple(str(item) for item in row.get("variants", ()))
        key = lane_key or entry.get("lane_key")
        if not isinstance(key, str):
            raise LedgerError("ledger entry requires lane_key")
        return cls(
            selector,
            key,
            row["attempt"],
            score,
            rung,
            lever,
            hypothesis,
            expected,
            actual,
            retained,
            variants,
        )


@dataclass(frozen=True)
class Ledger:
    """Typed view over one selector's durable experiment rows."""

    selector: str
    rows: tuple[LedgerRow, ...]

    @classmethod
    def read(cls, path: Path, selector: str) -> "Ledger":
        rows: list[LedgerRow] = []
        if path.is_file():
            for line in path.read_text(encoding="utf-8").splitlines():
                if line.strip():
                    entry = json.loads(line)
                    if entry.get("selector") == selector:
                        rows.append(LedgerRow.from_entry(entry, entry.get("lane_key")))
        return cls(selector, tuple(rows))

    def entries(self) -> list[dict[str, Any]]:
        """Re-emit durable entries in the lane-worktree record shape."""
        return [
            {
                "lane_key": row.lane_key,
                "selector": self.selector,
                "row": {
                    "attempt": row.attempt,
                    "score": row.score,
                    "rung": row.rung,
                    "lever": row.lever,
                    "hypothesis_id": row.hypothesis_id,
                    "expected_effect": row.expected_effect,
                    "actual_effect": row.actual_effect,
                    "retained_status": row.retained_status,
                    "variants": list(row.variants),
                },
            }
            for row in self.rows
        ]

    def variants_tried(self) -> set[str]:
        """Distinct (hypothesis, expected-effect) keys already spent."""
        keys: set[str] = set()
        for row in self.rows:
            if row.lever in LEAD_LEVERS:
                continue
            key = json.dumps([row.hypothesis_id, row.expected_effect], sort_keys=True)
            if key != '["",""]':
                keys.add(key)
        return keys

    def evaluate(self) -> dict[str, Any]:
        """Typed state evaluator: ladder legality, stalls, terminal verdict."""
        if not self.rows:
            return {"selector": self.selector, "verdict": "open", "reasons": [], "rung_index": 0}
        if self.rows[0].lever not in ("baseline", "interruption recovery"):
            raise LedgerError(f"{self.selector} ledger must start with the baseline row")
        problems: list[str] = []
        best = self.rows[0].score
        rung_index = LADDER.index(self.rows[0].rung)
        stalls = 0
        for index, row in enumerate(self.rows[1:], start=1):
            improved = row.score > best
            if improved:
                best = row.score
                stalls = 0
            else:
                stalls += 1
            row_rung = LADDER.index(row.rung)
            if row_rung < rung_index:
                problems.append(f"row {row.attempt} regressed the ladder")
                row_rung = rung_index
            one_shot = row.rung in ("compiler-profile", "permuter", "compiler-ceiling")
            limit = 1 if one_shot else STALL_LIMIT
            if not improved and stalls >= limit and row_rung > rung_index and row.lever not in LEAD_LEVERS:
                problems.append(f"row {row.attempt} stayed on an exhausted rung")
            rung_index = row_rung
            if row.score > 100.0:
                problems.append(f"row {row.attempt} score exceeds 100")
        terminal = "exact" if best >= 100.0 else (
            "ladder-exhausted" if rung_index == len(LADDER) - 1 else "open"
        )
        reasons = problems + (
            [] if terminal == "open" else [f"terminal: {terminal} at best {best}"]
        )
        return {
            "selector": self.selector,
            "verdict": terminal,
            "reasons": reasons,
            "best_score": best,
            "rung_index": rung_index,
            "stalls": stalls,
        }


def evaluate_lane_state(state: dict[str, Any]) -> dict[str, Any]:
    """Terminal-state evaluator for one rendered lane's final measurement.

    Mirrors the renderer's final-measure semantics so a host gate can
    re-derive the lane verdict without rerunning the workflow: exact at
    100, restored when the final score regressed below best, improved
    partial otherwise.
    """

    if not isinstance(state, dict):
        raise LedgerError("lane state must be an object")
    selector = state.get("selector")
    if not isinstance(selector, str) or selector.count("@") != 1:
        raise LedgerError("lane state requires a TARGET@0xADDRESS selector")
    final = _number(state.get("finalScore"), "finalScore")
    best = _number(state.get("bestScore", 0.0), "bestScore")
    baseline = _number(state.get("baselineScore", best), "baselineScore")
    verdict: str
    if final >= 100.0:
        verdict = "exact"
    elif final < best or final <= baseline:
        verdict = "restored"
    else:
        verdict = "improved-partial"
    return {"selector": selector, "verdict": verdict, "final_score": final, "best_score": best}


__all__ = [
    "LADDER",
    "Ledger",
    "LedgerError",
    "LedgerRow",
    "STALL_LIMIT",
    "evaluate_lane_state",
]
