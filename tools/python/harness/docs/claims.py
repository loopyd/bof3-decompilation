"""Documentation claim patterns and paths for the dialogue drift gate."""

from __future__ import annotations

import re

# Documentation whose claims must agree with the code.
SPEC = "docs/specs/formats/dialogue-text.md"
TOOL_DOCS = (
    "tools/rust/bof3-text/docs/README.md",
    "tools/rust/bof3-text/docs/getting-started.md",
    "tools/rust/bof3-text/docs/commands.md",
    "tools/rust/bof3-text/docs/formats.md",
    "tools/rust/bof3-text/docs/development.md",
)
MODELS = "tools/rust/bof3-text/src/models.rs"
HARNESS_CLI = "tools/python/harness/registry.py"
HARNESS_TEXT = "tools/python/harness/commands/text.py"
# Documented once, in the global table, so a mode's section need not repeat them.
GLOBAL_FLAGS = {
    "--help",
    "--json",
    "--no-color",
    "--verbose",
    "--payloads",
    "--no-payloads",
    "--no-readable",
    "--vocabulary",
    "--no-vocabulary",
}
CRATE = "tools/rust/bof3-text"
BINARY = "build/tools/rust/bof3-text/release/bof3-text"

# Retained evidence is swept too: this is where a survived contradiction hid last time
# (a round-trip document kept asserting the removed TXT codec after the code changed).
EVIDENCE_DIR = "out/text-format"
# `docs-drift.md` *documents* these rules and quotes its own negative control, so it must
# contain the retired patterns by construction and is exempt from that single rule.
# Documents that document this check or record a reconciliation: both must quote the retired
# patterns and control transcripts by construction, so they are exempt from that single rule.
# Everything else — including the agent-facing guide — is swept.
EVIDENCE_EXEMPT = {
    "out/text-format/docs-drift.md",
    "out/text-format/docs-reconciliation.md",
}
# Also swept for retired-codec claims: an agent-facing guide that describes the same tool is
# exactly where a stale claim misleads a reader who trusts it.
EXTRA_RETIRED = ("docs/agents/tool-usage.md",)

# Claims that only a removed codec could support. Each entry is (pattern, why).
RETIRED = (
    (re.compile(r"\bTXT\b"), "the TXT codec was removed; JSON is the only codec"),
    (re.compile(r"--format"), "the --format selector was removed with the TXT codec"),
    (re.compile(r"both codecs", re.I), "there is only one codec"),
    (re.compile(r"line-oriented TXT grammar", re.I), "the TXT grammar was removed"),
    (
        re.compile(r"no US subfile exercises it"),
        "44 US battle subfiles exercise the two-bank layout",
    ),
    (re.compile(r"out/text/\w+\.txt"), "documents are JSON, not .txt"),
    (
        # A document passed to `-o`/`--text` with a .txt name is always stale: JSON is the only
        # codec, and this leaves real repository files such as `symbols.txt` alone.
        re.compile(r"(?:-o|--text)\s+[^\s`]*\.txt"),
        "documents are JSON, not .txt",
    ),
)

COMMAND_CLASS = re.compile(
    r'command_class!\(\s*(\w+)\s*,\s*(0x[0-9A-Fa-f_]+)\s*,\s*"([^"]+)"\s*,\s*(\d+)\s*\)'
)
# A line that *documents the removal* is not drift. Retained evidence legitimately records
# the history ("the TXT codec has since been removed", the negative-control transcript),
# so a retired pattern only counts when the surrounding lines assert the codec is live.
REMOVAL = re.compile(
    r"\bremoved\b|no longer|voided|only codec|retired|Drifted:|superseded|does not exist"
    # A gate result asserting the codec is *absent* is not a claim that it exists
    # ("no TXT, `--format` or contradictory-codec claim remains").
    r"|\bno\b[^|\n]{0,40}(?:TXT|--format)"
    # A defect record quotes the defect it found; "stale" marks such a quote.
    r"|\bstale\b"
    # A record that the index *used* to be refused is a defect record, not a live claim.
    r"|not regenerated|was refused, not",
    re.I,
)
# Claims that contradict how the index now behaves: `fresh_index` regenerates a stale index before
# use, and only `build-index --check` refuses. These are checked separately from RETIRED because the
# word "stale" is one of RETIRED's defect-quote markers and would mask them.
STALE_INDEX_RULES = (
    (
        re.compile(
            r"stale index[^.\n]{0,60}refus|refus\w*[^.\n]{0,30}stale index", re.I
        ),
        "a stale index is regenerated before use; only `build-index --check` refuses",
    ),
    (
        re.compile(r"stale index[^.\n]{0,60}rebuild instruction", re.I),
        "a stale index is regenerated before use, so no rebuild instruction is required",
    ),
)
# A record that the behaviour *used* to be a refusal is a record of the fix, not a live claim.
# A refusal correctly attributed to the refusing mode itself is not a stale claim.
STALE_INDEX_RECORD = re.compile(
    r"not regenerated|was refused, not|used to be refused|build-index --check|`--check`",
    re.I,
)

# Claims that contradict how matching and latching now behave: a hit is anchored on its containing
# string unless `--match-offset` is given, and a classified window is a barrier latching never
# crosses. Checked in their own family so RETIRED's defect-quote markers cannot mask them.
BEHAVIOUR_CLAIMS = (
    (
        re.compile(
            r"reports? the match (?:start )?by default|offset is the match (?:start|offset) by default",
            re.I,
        ),
        "a hit is anchored on its containing string by default; the match start needs --match-offset",
    ),
    (
        re.compile(
            r"latches? may (?:fall|be drawn|overlap)|may be drawn from classified|"
            r"windows? (?:are|is) not a barrier|latches? (?:do not|don't) (?:avoid|skip) classified",
            re.I,
        ),
        "a classified window is a barrier: no heuristic latch is drawn inside one",
    ),
)
# A record of the change is not a live claim.
BEHAVIOUR_RECORD = re.compile(
    r"used to|were removed|removed by|was 311|before the check|replaced|not regenerated|"
    r"injected|produced a finding|negative control|drift control",
    re.I,
)

# Aggregate measurements that later regeneration superseded. A current-state claim carrying one of
# these without saying it is a recorded measurement contradicts the retained artifacts.
SUPERSEDED_COUNTS = re.compile(
    r"\b(?:11,399|11399|13,048|13048|57,456|57456|46,348|46348|36,592|45,258|2,170|2,203|14436|624596)\b"
    r"|\b(?:6\.1 s|6\.4 s|1\.88 s|14\.2 s|1\.75 s|0\.53 s|2\.3 s|4\.4 s)\b"
)
SUPERSEDED_RECORD = re.compile(
    r"recorded|reconstructed|previously|historical|before the reconstruction|used to|were removed|"
    r"superseded|older artifacts",
    re.I,
)

# Claims that contradict the two filtering rules this goal added: a payload family may only be
# excluded when a named record identifies it and its structural check passes, and every reported text
# result must carry a readable word. Checked in their own family so the retired-claim markers cannot
# mask them.
FILTER_CLAIMS = (
    (
        re.compile(
            r"\bmay be excluded without\b|\bis sufficient to exclude\b|"
            r"\bexclud\w+ on the signature alone\b|\bsignature alone is enough to exclude\b",
            re.I,
        ),
        "a payload is excluded only when a named record identifies it and its structural check passes",
    ),
    (
        re.compile(
            r"\bresults? (?:need not|don't need to|do not need to) be readable\b|"
            r"\bwithout (?:a )?readable word\b|\bno word (?:is )?required\b|"
            r"\breadability (?:floor|rule) is optional\b",
            re.I,
        ),
        "every reported text result must carry a readable word; the floor is not optional",
    ),
)

# Claims about where filtering lives that contradict the implementation: the contracts are declared in
# `models.rs`, every implementation is in `filters.rs`, and the two reporting filters really do
# implement `PostFilter`.
FILTER_CLASS_CLAIMS = re.compile(
    r"\bReadable\b[^.]*\b(?:is|does) not implement\b|"
    r"\bPreFilter\b[^.]*\brefuses without (?:a )?check\b|"
    r"\bfilter(?:ing)? logic (?:may|can|does) (?:also )?live outside filters\.rs\b|"
    r"\bno (?:single|one) place (?:for|holds) filter\b",
    re.I,
)

ENUM_VARIANT = re.compile(r"^\s{4}([A-Z]\w+)\s*,\s*$", re.M)
SPEC_COMMAND_ROW = re.compile(
    r"^\|\s*`(0x[0-9A-Fa-f]{2})`\s*\|\s*`([^`]+)`\s*\|\s*([^|]+?)\s*\|", re.M
)
BACKTICK_PATH = re.compile(r"`((?:out|docs|tools|config|src)/[\w./@+-]+)`")
