"""Validate function-owned original-byte contracts from one native invocation."""

from __future__ import annotations

from copy import deepcopy
from dataclasses import dataclass
import re

from harness.domain.mips import word_fields
from harness.reconstruction import memorycontracts, pathcontracts
from harness.reconstruction.proofvalues import (
    _complete,
    _integer,
    _keys,
    _references,
    _require,
    _rows,
    _reasons,
)


def validate_return(
    row: dict,
    selected: dict,
    *,
    revised: bool = False,
    entry_observation: bool | None = None,
) -> None:
    _require(
        entry_observation is None or type(entry_observation) is bool,
        "native entry observation context",
    )
    native = _keys(
        row.get("returnDisposition"),
        {"rule", "mode", "kind", "origin", "evidence"},
        "native return",
    )
    kind = native["kind"]
    _require(
        native["rule"]
        == ("mips-all-normal-exits/v2" if revised else "mips-all-normal-exits/v1")
        and native["mode"] == "original-only-mips32"
        and type(kind) is str
        and kind in {"unknown", "value"}
        and native["origin"]
        == ("native-must-definition" if kind == "value" else "none"),
        "native return rule/mode/origin",
    )
    evidence = _keys(
        native["evidence"],
        set(
            "symbol start end selection decodeComplete analysisComplete instructions edges definitions exits calls obligations".split()
        ),
        "native return evidence",
    )
    _require(
        evidence["symbol"] == row["name"]
        and evidence["start"] == selected["start"]
        and evidence["end"] == selected["end"]
        and evidence["selection"] == selected.get("selection", "selected-root"),
        "native return selection differs",
    )
    for key in ("decodeComplete", "analysisComplete"):
        _require(type(evidence[key]) is bool, "native return completeness Boolean")
    shapes = {
        "instructions": "pc word owner bytes mapped undef fullWidthRegisters",
        "edges": "from to delayOwner kind",
        "definitions": "id pc register width operation delayed inputs formals bad",
        "exits": "pc delaySlot missing definitions formals pendingDefinitions bad",
        "calls": "pc target required qualified unresolved",
        "obligations": "pc domain reason",
    }
    if revised:
        shapes["definitions"] += " entries origin equalEntry constant equalityEvidence"
        shapes["calls"] += " contract"
    integers = dict.fromkeys("pc from to delayOwner delaySlot target".split(), 1 << 32)
    integers.update(word=(1 << 32) - 1, bytes=16, id=4096, register=33, width=32)
    booleans = set("mapped undef delayed missing required qualified".split())
    references = {"fullWidthRegisters": (0, 33), "formals": (4, 7)}
    references.update(
        dict.fromkeys("inputs definitions pendingDefinitions".split(), (1, 4096))
    )
    reference_count = 0
    for category, fields in shapes.items():
        for item in _rows(evidence[category], 4096, "native " + category):
            expected_fields = set(fields.split())
            if revised and category == "calls" and "entryPath" in item:
                expected_fields.add("entryPath")
            _keys(item, expected_fields, "native " + category)
            for key, value in item.items():
                if revised and category == "calls" and key == "entryPath":
                    continue
                if revised and key in {
                    "origin",
                    "contract",
                    "equalEntry",
                    "constant",
                    "equalityEvidence",
                    "entries",
                }:
                    continue
                if key in integers:
                    _require(
                        (value is None and key in {"target", "delayOwner"})
                        or (type(value) is int and 0 <= value <= integers[key]),
                        "native evidence integer",
                    )
                elif key in booleans:
                    _require(type(value) is bool, "native evidence Boolean")
                elif key in references or key in {"bad", "unresolved"}:
                    values = _rows(value, 4096, "native evidence dependency")
                    reference_count += len(values)
                    bounds = references.get(key)
                    _require(
                        reference_count <= 8192
                        and all(
                            (type(entry) is int and bounds[0] <= entry <= bounds[1])
                            if bounds
                            else (
                                type(entry) is str and len(entry.encode("utf-8")) <= 128
                            )
                            for entry in values
                        ),
                        "native evidence dependency",
                    )
                    if bounds:
                        _require(
                            len(set(values)) == len(values),
                            "duplicate native register/dependency",
                        )
                else:
                    _require(
                        type(value) is str and len(value.encode("utf-8")) <= 4096,
                        "native evidence text",
                    )
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    definitions = {item["id"]: item for item in evidence["definitions"]}
    _require(
        len(instructions) == len(evidence["instructions"])
        and len(definitions) == len(evidence["definitions"])
        and 0 not in definitions,
        "duplicate native instruction/definition",
    )
    for item in evidence["definitions"]:
        _require(
            item["pc"] in instructions
            and item["width"] in {0, 32}
            and (
                item["width"] == 0
                or (revised and item["origin"]["kind"] == "call")
                or item["register"] in instructions[item["pc"]]["fullWidthRegisters"]
            )
            and all(reference in definitions for reference in item["inputs"]),
            "native definition ownership/reference",
        )
    for item in evidence["exits"]:
        _require(
            item["pc"] in instructions
            and item["delaySlot"] in instructions
            and all(
                reference in definitions
                for reference in item["definitions"] + item["pendingDefinitions"]
            ),
            "native exit ownership/reference",
        )
    if kind == "unknown":
        if is_stack_word_body_candidate(row) and (
            "returnType" in row or "returnStorage" in row
        ):
            _validate_stack_word_body_candidate(row)
        else:
            _require(
                "returnType" not in row and "returnStorage" not in row,
                "unknown native return has concrete fields",
            )
    else:
        _require(
            evidence["decodeComplete"]
            and evidence["analysisComplete"]
            and evidence["instructions"]
            and evidence["exits"],
            "native return evidence incomplete",
        )
        _require(
            type(row.get("returnType")) is dict
            and type(row.get("returnStorage")) is dict
            and row["returnType"].get("llvmIr") == "i32"
            and row["returnStorage"].get("type") == "register"
            and row["returnStorage"].get("value") == "v0"
            and type(row["returnStorage"].get("registerNumber")) is int
            and row["returnStorage"]["registerNumber"] == 4,
            "native i32/V0 return missing",
        )
        _require(
            all(
                not exit["missing"]
                and exit["definitions"]
                and not exit["bad"]
                and not exit["pendingDefinitions"]
                and all(
                    definitions[reference]["register"] == 2
                    and definitions[reference]["width"] == 32
                    and not definitions[reference]["bad"]
                    for reference in exit["definitions"]
                )
                for exit in evidence["exits"]
            ),
            "native all-exit definition incomplete",
        )
        _require(
            all(
                item["mapped"]
                and item["owner"] == row["name"]
                and not item["undef"]
                and item["bytes"] == 4
                for item in evidence["instructions"]
            )
            and not any(
                item["domain"] == "architectural" for item in evidence["obligations"]
            ),
            "native architectural evidence incomplete",
        )
        if any(item["domain"] == "entry" for item in evidence["obligations"]):
            _require(
                revised and entry_observation is False,
                "native entry evidence incomplete",
            )
            _validate_entry_covered_return(row, instructions, definitions)


def _declaration_demand(use):
    """One entry demand a projected i32 declaration may rest on.

    O32 passes arguments in a0-a3 only. The stack pointer is the frame base and
    never an argument; a demanded callee-saved or context register is the caller's
    own context, which the function only saves or passes through.
    """
    register, role = use["register"], use["role"]
    if 4 <= register <= 7 or register == 29:
        return True
    context = register in {28, 30, 31} or 16 <= register <= 23
    return context and role in {"stored-value", "call-input", "continuation"}


def _validate_entry_covered_return(row, instructions, definitions):
    evidence = row["returnDisposition"]["evidence"]
    summary = row.get("originalCallSummary")
    _require(type(summary) is dict, "native entry coverage lacks function summary")
    start, end = evidence["start"], evidence["end"]
    root = evidence.get("selection") == "selected-root"
    _require(
        evidence["decodeComplete"] is evidence["analysisComplete"] is True
        and _complete(summary["entry"])
        and (root or _complete(summary["control"]))
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
        and 0 <= start < end <= 1 << 32
        and start % 4 == end % 4 == 0
        and len(evidence["instructions"]) == (end - start) // 4
        and all(
            item["pc"] == start + 4 * index
            for index, item in enumerate(evidence["instructions"])
        ),
        "native entry coverage or exact instruction inventory incomplete",
    )

    def settled(transfers):
        _require(
            type(transfers) is list and len(transfers) == 34,
            "native entry coverage lacks register inventory",
        )
        transfer = _keys(transfers[2], TRANSFER_FIELDS, "covered V0 transfer")
        _require(
            transfer["register"] == 2
            and transfer["kind"] == "assigned"
            and _complete(transfer["proof"])
            and transfer["definitions"]
            and not transfer["bad"]
            and transfer["pending"] is False
            and transfer["mayNoPending"] is True
            and not transfer["pendingDefinitions"]
            and all(
                ref in definitions
                and definitions[ref]["pc"] in instructions
                and definitions[ref]["register"] == 2
                and definitions[ref]["width"] == 32
                and not definitions[ref]["bad"]
                for ref in transfer["definitions"]
            ),
            "native entry coverage lacks settled full-width V0",
        )
        return transfer

    if root:
        returns = sorted(
            pc for pc, item in instructions.items() if item["word"] == 0x03E00008
        )
        _require(
            returns == sorted(exit["pc"] for exit in evidence["exits"]),
            "root return without complete jr-ra exit census",
        )
    settled(summary["transfers"])
    exits = _rows(summary["exits"], 4096, "covered normal exits")
    _require(
        exits and len(exits) == len(evidence["exits"]),
        "native entry coverage lacks normal exit inventory",
    )
    for normal, exit in zip(exits, evidence["exits"], strict=True):
        transfer = settled(_exit_transfers(summary, normal))
        _require(
            normal["pc"] == exit["pc"]
            and instructions[exit["pc"]]["word"] == 0x03E00008
            and normal["delay"] == exit["delaySlot"] == exit["pc"] + 4
            and normal["delay"] in instructions
            and (root or _complete(normal["incomingLink"]))
            and transfer["definitions"] == exit["definitions"]
            and transfer["pendingDefinitions"] == exit["pendingDefinitions"],
            "native entry coverage lacks owned settled normal exit",
        )


DIMENSIONS = "identity inputs transfers result control effects declaration".split()
TRANSFER_FIELDS = set(
    "register kind proof definitions entries bad equalEntry constant equalityEvidence pending mayNoPending pendingDefinitions".split()
)
BINDING_FIELDS = set(
    "register missing definitions entries uses bad equalEntry constant".split()
)
CONTRACT_FIELDS = set(
    "kind callee delay continuation actualIncomingLink bindings calleeExits registerTransfers remaining".split()
) | set(DIMENSIONS)
SUMMARY_FIELDS = set(
    "function pendingEntry entry result control effects declaration parameterRegisters demands transferEncoding transfers exits accesses".split()
)
EXIT_FIELDS = set(
    "pc delay incomingLink sampledEntry sampledConstant sampledDefinitions equalityEvidence transferSource".split()
)
TRANSFER_ENCODING = "summary-or-inline/v1"
CALL_TRANSFER_ENCODING = "callee-or-inline/v1"


def _same_transfers(left, right):
    if type(left) is not type(right):
        return False
    if type(left) is dict:
        return left.keys() == right.keys() and all(
            _same_transfers(left[key], right[key]) for key in left
        )
    if type(left) is list:
        return len(left) == len(right) and all(
            _same_transfers(a, b) for a, b in zip(left, right, strict=True)
        )
    return left == right


def decode_configuration(configuration):
    decoded = deepcopy(configuration)
    functions = _rows(decoded.get("functions", []), 4096, "native functions")
    summaries = {}
    for function in functions:
        _require(type(function) is dict, "native function object")
        if "originalCallSummary" not in function:
            continue
        summary = function["originalCallSummary"]
        _require(type(summary) is dict, "function summary object")
        key = summary.get("function")
        _require(
            type(key) is str and key and key not in summaries,
            "duplicate or invalid transfer summary identity",
        )
        summaries[key] = _rows(summary.get("transfers"), 34, "summary transfers")
    for function in functions:
        native = function.get("returnDisposition", {})
        _require(type(native) is dict, "native return object")
        evidence = native.get("evidence", {})
        _require(type(evidence) is dict, "native evidence object")
        for call in _rows(evidence.get("calls", []), 4096, "native calls"):
            _require(type(call) is dict, "native call object")
            if "contract" not in call:
                continue
            value = call["contract"]
            fields = (CONTRACT_FIELDS - {"registerTransfers"}) | {
                "registerTransferSource"
            }
            if type(value) is dict and "noMemoryWrites" in value:
                fields.add("noMemoryWrites")
            contract = _keys(value, fields, "encoded call contract")
            _require(type(contract["callee"]) is str, "call transfer callee")
            callee = summaries.get(contract["callee"])
            source = contract.pop("registerTransferSource")
            _require(type(source) is dict, "call transfer source")
            _require(
                source.get("encoding") == CALL_TRANSFER_ENCODING,
                "call transfer encoding",
            )
            if source.get("kind") == "callee-summary":
                _keys(source, {"encoding", "kind"}, "callee transfer source")
                _require(callee is not None and callee, "missing callee transfers")
                rows = callee
            else:
                _keys(source, {"encoding", "kind", "rows"}, "inline call transfers")
                _require(source["kind"] == "inline", "call transfer source kind")
                rows = _rows(source["rows"], 34, "inline call transfers")
                _require(
                    not rows or callee is None or not _same_transfers(rows, callee),
                    "redundant inline call transfers",
                )
            contract["registerTransfers"] = rows
    return decoded


def _exit_transfers(summary, exit):
    _require(
        summary.get("transferEncoding") == TRANSFER_ENCODING,
        "function transfer encoding",
    )
    _keys(exit, EXIT_FIELDS, "normal exit")
    source = exit["transferSource"]
    _require(type(source) is dict, "exit transfer source")
    if source.get("kind") == "summary":
        _keys(source, {"kind"}, "summary transfer source")
        return summary["transfers"]
    _keys(source, {"kind", "rows"}, "inline transfer source")
    _require(source["kind"] == "inline", "exit transfer source kind")
    rows = _rows(source["rows"], 34, "exit register transfers")
    _require(
        not _same_transfers(rows, summary["transfers"]),
        "redundant inline exit transfers",
    )
    return rows


ENTRY_ROLES = set("continuation control address stored-value result call-input".split())
MEMORY_OPERATIONS = {
    32: ("read", 8, "sign", True),
    33: ("read", 16, "sign", True),
    34: ("read", 32, "none", True),
    35: ("read", 32, "none", True),
    36: ("read", 8, "zero", True),
    37: ("read", 16, "zero", True),
    38: ("read", 32, "none", True),
    40: ("write", 8, "none", False),
    41: ("write", 16, "none", False),
    42: ("write", 32, "none", False),
    43: ("write", 32, "none", False),
    46: ("write", 32, "none", False),
}


def _definition_extras(definition, instructions):
    _references(definition["entries"], 33, 1)
    _integer(definition["equalEntry"], 33, optional=True)
    _integer(definition["constant"], (1 << 32) - 1, optional=True)
    _require(
        all(
            pc in instructions
            for pc in _references(definition["equalityEvidence"], (1 << 32))
        ),
        "definition equality ownership",
    )
    origin = definition["origin"]
    _require(
        type(origin) is dict and type(origin.get("kind")) is str, "definition origin"
    )
    if origin["kind"] == "instruction":
        _keys(origin, {"kind"}, "instruction definition origin")
    else:
        _keys(
            origin,
            set(
                "kind callee available delay calleeDefinitions substitutedEntries".split()
            ),
            "call definition origin",
        )
        _require(
            origin["kind"] == "call"
            and type(origin["callee"]) is str
            and bool(origin["callee"]),
            "call definition callee",
        )
        _integer(origin["available"])
        _integer(origin["delay"])
        _references(origin["calleeDefinitions"], 4096, 1)
        _references(origin["substitutedEntries"], 33, 1)


def _transfers(rows, definitions, instructions, *, covered):
    rows = _rows(rows, 34, "register transfers")
    _require(
        not rows or [row.get("register") for row in rows] == list(range(34)),
        "complete register inventory",
    )
    for row in rows:
        _keys(row, TRANSFER_FIELDS, "register transfer")
        _integer(row["register"], 33)
        complete = _complete(row["proof"])
        _require(
            type(row["kind"]) is str
            and row["kind"] in {"unknown", "preserved", "assigned", "clobbered"},
            "register transfer kind",
        )
        for field in ("definitions", "pendingDefinitions"):
            refs = _references(row[field], 4096, 1)
            _require(
                all(
                    ref in definitions
                    and definitions[ref]["register"] == row["register"]
                    for ref in refs
                ),
                "register transfer definition ownership",
            )
        _references(row["entries"], 33, 1)
        _reasons(row["bad"])
        _integer(row["equalEntry"], 33, optional=True)
        _integer(row["constant"], (1 << 32) - 1, optional=True)
        _require(
            all(
                pc in instructions
                for pc in _references(row["equalityEvidence"], 1 << 32)
            ),
            "register equality ownership",
        )
        _require(
            type(row["pending"]) is bool
            and type(row["mayNoPending"]) is bool
            and (row["pending"] or not row["pendingDefinitions"]),
            "pending register facts",
        )
        if complete:
            _require(
                covered
                and not row["pending"]
                and row["mayNoPending"]
                and row["kind"] != "unknown",
                "register transfer coverage/timing",
            )
            if row["kind"] == "preserved":
                _require(
                    row["register"] == 0 or row["equalEntry"] == row["register"],
                    "preservation needs value equality",
                )
                _require(
                    all(
                        definitions[ref]["equalEntry"] == row["register"]
                        for ref in row["definitions"]
                    ),
                    "preserved definition equality",
                )
            if row["kind"] in {"assigned", "clobbered"}:
                _require(
                    bool(row["definitions"]),
                    "assigned/clobbered register has no definition",
                )
            if row["kind"] == "assigned":
                _require(
                    not row["bad"]
                    and all(
                        definitions[ref]["width"] == 32 and not definitions[ref]["bad"]
                        for ref in row["definitions"]
                    ),
                    "assigned full-width register proof",
                )
    return rows


def _validate_entry_arithmetic(row, entry_flow):
    if entry_flow is None:
        return
    entry_offset, reaches, claims = entry_flow
    definitions = {
        item["id"]: item for item in row["returnDisposition"]["evidence"]["definitions"]
    }
    recovered = {}
    for claim in claims:
        entry, offset, pcs, adjusted = entry_offset(claim["id"])
        _require(
            offset == 0
            and adjusted
            and claim["equalEntry"] == entry
            and claim["equalityEvidence"] == sorted(pcs),
            "arithmetic identity modular equality/provenance differs",
        )
        recovered[claim["id"]] = (entry, pcs)
    summary = row["originalCallSummary"]
    for exit in summary["exits"]:
        for transfer in _exit_transfers(summary, exit):
            refs = transfer["definitions"]
            if transfer["equalEntry"] is None or not recovered.keys() & set(refs):
                continue
            _require(
                len(refs) == 1 and not transfer["pending"] and transfer["mayNoPending"],
                "arithmetic exit identity ambiguous/pending",
            )
            entry, pcs = recovered[refs[0]]
            _require(
                transfer["equalEntry"] == entry
                and transfer["equalityEvidence"] == sorted(pcs),
                "arithmetic exit identity evidence differs",
            )
            reaches(
                exit["delay"],
                transfer["register"],
                definitions[refs[0]]["pc"],
                after=True,
            )
    for transfer in summary["transfers"]:
        if transfer["equalEntry"] is None or not recovered.keys() & set(
            transfer["definitions"]
        ):
            continue
        witnesses = [
            _exit_transfers(summary, exit)[transfer["register"]]
            for exit in summary["exits"]
        ]
        _require(
            witnesses
            and all(item["equalEntry"] == transfer["equalEntry"] for item in witnesses)
            and transfer["definitions"]
            == sorted({ref for item in witnesses for ref in item["definitions"]})
            and transfer["equalityEvidence"]
            == sorted({pc for item in witnesses for pc in item["equalityEvidence"]}),
            "arithmetic joined exit identity evidence differs",
        )


def _observed_entry_context(summary, evidence):
    value = _keys(
        summary["entryContext"],
        {"kind", "observation", "stateHash", "entryPc", "cpu"},
        "observed entry context",
    )
    _require(
        value["kind"] == "psx-observed-entry-context/v1"
        and value["entryPc"] == evidence["start"]
        and all(
            type(value[key]) is str and re.fullmatch(r"[0-9a-f]{64}", value[key])
            for key in ("observation", "stateHash")
        ),
        "observed entry context identity",
    )
    cpu = _keys(value["cpu"], {"inIsr", "status", "cause", "epc"}, "entry CPU context")
    _require(type(cpu["inIsr"]) is bool, "entry CPU ISR marker")
    for key in ("status", "cause", "epc"):
        _integer(cpu[key], 0xFFFFFFFF)
    _require(not cpu["status"] & 0x10000, "entry CPU cache isolation")
    if cpu["inIsr"]:
        _require(
            not cpu["status"] & 3
            and not evidence["decodeComplete"]
            and all(
                not _complete(summary[key])
                for key in (
                    "entry",
                    "result",
                    "control",
                    "effects",
                    "declaration",
                    "noMemoryWrites",
                )
            )
            and "scopedRestoration" not in summary
            and "symbolicFrame" not in summary
            and {
                "pc": value["entryPc"],
                "domain": "architectural",
                "reason": "observed-isr-context-preservation-unresolved",
            }
            in evidence["obligations"],
            "observed ISR context cannot establish a generic contract",
        )


def _scoped_restoration(value, row):
    evidence = row["returnDisposition"]["evidence"]
    _keys(
        value,
        {
            "kind",
            "observation",
            "stateHash",
            "entryPc",
            "entrySp",
            "entryBindings",
            "frameBytes",
            "conditions",
            "registers",
        },
        "scoped restoration",
    )
    _require(
        value["kind"] == "mips-observed-frame-restoration/v2"
        and value["entryPc"] == evidence["start"]
        and value["conditions"]
        == [
            "captured-entry-registers-and-settled-load-state",
            "ordinary-ram-mapping-and-cache-mode-remain-active",
            "original-instructions-remain-active",
            "accesses-and-instruction-fetches-complete-normally",
            "no-asynchronous-or-external-state-interference",
            "mips-i-load-and-branch-delays",
        ]
        and all(
            type(value[key]) is str and re.fullmatch(r"[0-9a-f]{64}", value[key])
            for key in ("observation", "stateHash")
        ),
        "scoped restoration conditions or identity",
    )
    _integer(value["entrySp"], 0xFFFFFFFF)
    bindings = _rows(value["entryBindings"], 30, "scoped entry bindings")
    for binding in bindings:
        _keys(binding, {"register", "value"}, "scoped entry binding")
        _integer(binding["register"], 31)
        _integer(binding["value"], 0xFFFFFFFF)
        _require(binding["register"] not in {0, 29}, "scoped entry binding register")
    _require(
        [item["register"] for item in bindings]
        == sorted({item["register"] for item in bindings}),
        "scoped entry binding order",
    )
    _integer(value["frameBytes"], 32768)
    sp, frame = value["entrySp"], value["frameBytes"]
    _require(
        sp % 4 == 0
        and frame > 0
        and frame % 4 == 0
        and sp >= frame
        and sp & 0xE0000000 in {0, 0x80000000, 0xA0000000}
        and frame <= (sp & 0x1FFFFFFF) < 0x200000,
        "scoped frame placement",
    )
    instructions = {item["pc"]: item["word"] for item in evidence["instructions"]}
    _require(
        instructions.get(evidence["start"]) == (0x27BD0000 | ((-frame) & 0xFFFF)),
        "scoped frame allocation",
    )
    registers = _rows(value["registers"], 31, "scoped restored registers")
    _require(
        registers
        and [item["register"] for item in registers]
        == sorted(set(item["register"] for item in registers)),
        "scoped restoration register order",
    )
    calls = {item["pc"]: item for item in evidence["calls"]}
    exits = sorted(item["pc"] for item in evidence["exits"])
    for restored in registers:
        _keys(
            restored,
            {
                "register",
                "entrySpOffset",
                "physicalWord",
                "storePc",
                "loadPcs",
                "callPcs",
                "exitPcs",
            },
            "scoped register",
        )
        reg, offset = restored["register"], restored["entrySpOffset"]
        _integer(reg, 31)
        _require(
            reg > 0
            and type(offset) is int
            and -frame <= offset <= -4
            and offset % 4 == 0
            and restored["physicalWord"] == ((sp + offset) & 0x1FFFFFFF)
            and restored["exitPcs"] == exits
            and exits,
            "scoped saved word placement or exit coverage",
        )
        store = instructions.get(restored["storePc"])
        _require(
            store is not None
            and store >> 26 == 43
            and (store >> 21) & 31 == 29
            and word_fields(store).signed_immediate - frame == offset,
            "scoped save instruction",
        )
        _require(restored["loadPcs"], "scoped load evidence absent")
        for key in ("loadPcs", "callPcs", "exitPcs"):
            _references(restored[key], 0xFFFFFFFF)
        for pc in restored["loadPcs"]:
            word = instructions.get(pc)
            _require(
                word is not None
                and word >> 26 == 35
                and (word >> 21) & 31 == 29
                and word_fields(word).signed_immediate - frame == offset,
                "scoped restore instruction",
            )
        for pc in restored["callPcs"]:
            contract = calls.get(pc, {}).get("contract", {})
            _require(
                type(contract) is dict
                and "noMemoryWrites" in contract
                and _complete(contract["noMemoryWrites"]),
                "scoped call writes remain unresolved",
            )
    memorycontracts._observed_restoration(row, value)


FRAME_CONDITIONS = [
    "settled-entry-loads-and-mips-i-delays",
    "aligned-whole-frame-in-ordinary-ram",
    "frame-writes-disjoint-from-fetched-code",
    "ordinary-ram-mapping-and-cache-mode-remain-active",
    "original-instructions-and-call-bindings-remain-active",
    "accesses-and-instruction-fetches-complete-normally",
    "no-asynchronous-or-external-state-interference",
]
FRAME_OBLIGATION = "conditional-symbolic-frame-restoration-unresolved"


def _symbolic_frame(summary, evidence):
    value = _keys(
        summary["symbolicFrame"],
        {"kind", "entryPc", "frameBytes", "conditions", "registers"},
        "symbolic frame",
    )
    _integer(value["frameBytes"], 32768)
    frame = value["frameBytes"]
    _require(
        value["kind"] == "mips-symbolic-frame-restoration/v1"
        and value["entryPc"] == evidence["start"]
        and value["conditions"] == FRAME_CONDITIONS
        and frame > 0
        and frame % 4 == 0
        and evidence["decodeComplete"]
        and evidence["analysisComplete"]
        and _complete(summary["entry"])
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
        and "scopedRestoration" not in summary,
        "symbolic frame identity or remaining conditions",
    )
    instructions = {item["pc"]: item["word"] for item in evidence["instructions"]}
    definitions = {item["id"]: item for item in evidence["definitions"]}
    accesses = {item["pc"]: item for item in summary["accesses"]}
    calls = {item["pc"]: item for item in evidence["calls"]}
    exits = sorted(item["pc"] for item in evidence["exits"])
    _require(
        instructions.get(evidence["start"]) == (0x27BD0000 | ((-frame) & 0xFFFF))
        and exits
        and len(summary["transfers"]) == 34
        and summary["transfers"][29]["kind"] == "preserved"
        and summary["transfers"][29]["equalEntry"] == 29
        and _complete(summary["transfers"][29]["proof"])
        and not summary["transfers"][29]["pending"]
        and not _complete(summary["noMemoryWrites"]),
        "symbolic frame allocation or physical-write facts",
    )
    registers = _rows(value["registers"], 31, "symbolic restored registers")
    _require(
        [item["register"] for item in registers]
        == sorted(set(item["register"] for item in registers))
        and any(item["register"] == 31 for item in registers),
        "symbolic frame register order or return relation",
    )
    for restored in registers:
        _keys(
            restored,
            {"register", "entrySpOffset", "storePc", "loadPcs", "callPcs", "exitPcs"},
            "symbolic restored register",
        )
        reg, offset = restored["register"], restored["entrySpOffset"]
        _integer(reg, 31)
        _require(
            reg > 0
            and type(offset) is int
            and -frame <= offset <= -4
            and offset % 4 == 0
            and _references(restored["exitPcs"], 0xFFFFFFFF) == exits
            and _references(restored["callPcs"], 0xFFFFFFFF) == sorted(calls)
            and restored["loadPcs"],
            "symbolic frame slot or exit/call coverage",
        )
        store = instructions.get(restored["storePc"])
        access = accesses.get(restored["storePc"], {})
        _require(
            store is not None
            and word_fields(store).opcode == 43
            and word_fields(store).rs == 29
            and word_fields(store).rt == reg
            and word_fields(store).signed_immediate - frame == offset
            and access.get("kind") == "write"
            and access.get("width") == 32
            and access.get("valueEntries") == [reg]
            and all(
                ref in definitions and definitions[ref]["equalEntry"] == reg
                for ref in access.get("valueDefinitions", [])
            ),
            "symbolic canonical entry save",
        )
        loads = _references(restored["loadPcs"], 0xFFFFFFFF)
        for pc in loads:
            word = instructions.get(pc)
            _require(
                word is not None
                and word_fields(word).opcode == 35
                and word_fields(word).rs == 29
                and word_fields(word).rt == reg
                and word_fields(word).signed_immediate - frame == offset
                and accesses.get(pc, {}).get("kind") == "read"
                and accesses[pc]["width"] == 32
                and accesses[pc]["delayed"],
                "symbolic canonical restore",
            )
        for exit in summary["exits"]:
            transfer = _exit_transfers(summary, exit)[reg]
            refs = exit["sampledDefinitions"] if reg == 31 else transfer["definitions"]
            _require(
                not transfer["pending"]
                and transfer["mayNoPending"]
                and not transfer["pendingDefinitions"]
                and refs
                and all(
                    ref in definitions
                    and definitions[ref]["register"] == reg
                    and definitions[ref]["origin"] == {"kind": "instruction"}
                    and definitions[ref]["width"] == 32
                    and definitions[ref]["delayed"]
                    and definitions[ref]["pc"] in loads
                    for ref in refs
                ),
                "symbolic restored exit reaching loads",
            )
        for pc in restored["callPcs"]:
            contract = calls[pc]["contract"]
            word = instructions[pc]
            _require(
                word >> 26 == 3
                and calls[pc]["target"]
                == (((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2))
                and all(
                    _complete(contract[key])
                    for key in (
                        "identity",
                        "inputs",
                        "control",
                        "transfers",
                        "noMemoryWrites",
                    )
                )
                and contract["delay"] == pc + 4
                and contract["continuation"] == contract["actualIncomingLink"] == pc + 8
                and len(contract["registerTransfers"]) == 34
                and all(
                    item["register"] == index
                    and _complete(item["proof"])
                    and not item["pending"]
                    and item["mayNoPending"]
                    and not item["pendingDefinitions"]
                    for index, item in enumerate(contract["registerTransfers"])
                ),
                "symbolic nested no-write call binding",
            )
    return value


def _frame_binding(call, caller, callee):
    contract = call["contract"]
    pc = call["pc"]
    evidence = caller["returnDisposition"]["evidence"]
    summary = callee["originalCallSummary"]
    bound = "symbolicFrame" in call.get("entryPath", {})
    restoration = (
        call["entryPath"]["symbolicFrame"] if bound else summary["symbolicFrame"]
    )
    condition = f"conditional-callee-preservation-{pc}"
    words = {item["pc"]: item["word"] for item in evidence["instructions"]}
    obligations = [
        item
        for item in evidence["obligations"]
        if item["pc"] == pc
        and item["domain"] == "architectural"
        and item["reason"] == FRAME_OBLIGATION
    ]
    links = [item for item in restoration["registers"] if item["register"] == 31]
    _require(
        len(obligations) == 1
        and contract["kind"] == "partial"
        and not call["qualified"]
        and _complete(contract["identity"])
        and not _complete(contract["control"])
        and not _complete(contract["transfers"])
        and contract["callee"] == summary["function"]
        and words.get(pc, 0) >> 26 == 3
        and call["target"]
        == restoration["entryPc"]
        == (((pc + 4) & 0xF0000000) | ((words[pc] & 0x03FFFFFF) << 2))
        and contract["delay"] == pc + 4
        and contract["continuation"] == contract["actualIncomingLink"] == pc + 8
        and pc + 4 in words
        and memorycontracts._memory_word(words[pc + 4])[0] != "load"
        and len(links) == 1
        and links[0]["exitPcs"]
        == ([call["entryPath"]["terminalPc"]] if bound else contract["calleeExits"])
        and _same_transfers(contract["registerTransfers"], summary["transfers"])
        and condition in contract["inputs"]["remaining"]
        and condition in contract["transfers"]["remaining"]
        and not {
            "pending-call-boundary-load-unresolved",
            "pending-return-load-boundary-unresolved",
        }.intersection(contract["remaining"]),
        "conditional symbolic frame lacks actual settled call binding",
    )
    if bound:
        _require(
            call["entryPath"]["callee"] == contract["callee"]
            and restoration["entryPc"]
            == callee["returnDisposition"]["evidence"]["start"]
            and "entryContext" not in summary,
            "bound symbolic frame callee differs",
        )
        preserved = pathcontracts.bound_frame_registers(call, FRAME_CONDITIONS)
        return {
            "call": call,
            "entryPc": restoration["entryPc"],
            "continuation": contract["continuation"],
            "registers": preserved,
        }
    preserved = {item["register"] for item in restoration["registers"]}
    preserved.update(
        item["register"]
        for item in summary["transfers"]
        if item["kind"] == "preserved"
        and item["equalEntry"] == item["register"]
        and _complete(item["proof"])
        and not item["pending"]
        and item["mayNoPending"]
        and not item["pendingDefinitions"]
    )
    return {
        "call": call,
        "entryPc": restoration["entryPc"],
        "continuation": contract["continuation"],
        "registers": preserved - {31},
    }


def _summary(row, identity, payload, load, entry_flow, context, images, frame_uses):
    value = row.get("originalCallSummary")
    fields = SUMMARY_FIELDS | (
        {"noMemoryWrites"}
        if type(value) is dict and "noMemoryWrites" in value
        else set()
    )
    if type(value) is dict and "scopedRestoration" in value:
        fields.add("scopedRestoration")
    if type(value) is dict and "symbolicFrame" in value:
        fields.add("symbolicFrame")
    if type(value) is dict and "entryContext" in value:
        fields.add("entryContext")
    summary = _keys(value, fields, "function summary")
    _require(
        summary["function"] == identity["key"]
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
        and summary["transferEncoding"] == TRANSFER_ENCODING,
        "function summary identity/precondition",
    )
    evidence = row["returnDisposition"]["evidence"]
    if "entryContext" in summary:
        _observed_entry_context(summary, evidence)
    if "scopedRestoration" in summary:
        _scoped_restoration(summary["scopedRestoration"], row)
    if "symbolicFrame" in summary:
        _symbolic_frame(summary, evidence)
    instructions = {item["pc"]: item for item in evidence["instructions"]}
    definitions = {item["id"]: item for item in evidence["definitions"]}
    facts = {
        key: _complete(summary[key])
        for key in ("entry", "result", "control", "effects", "declaration")
    }
    covered = (
        evidence["decodeComplete"]
        and evidence["analysisComplete"]
        and bool(evidence["exits"])
    )
    _require(not any(facts.values()) or covered, "summary dimension without coverage")
    if "noMemoryWrites" in summary and _complete(summary["noMemoryWrites"]):
        _require(
            covered and facts["entry"] and facts["control"],
            "memory-write absence without complete original entry/control",
        )
    _require(
        facts["result"] == (row["returnDisposition"]["kind"] == "value"),
        "summary return disagreement",
    )
    demands = _rows(summary["demands"], 4096, "entry demands")
    unique = set()
    for demand in demands:
        _keys(demand, {"pc", "register", "role"}, "entry use")
        _integer(demand["register"], 33)
        _integer(demand["pc"])
        _require(
            demand["register"] > 0
            and demand["pc"] in instructions
            and demand["role"] in ENTRY_ROLES,
            "original-byte entry use",
        )
        use = (demand["pc"], demand["register"], demand["role"])
        _require(use not in unique, "duplicate entry demand")
        unique.add(use)
    expected_lanes = sorted(
        {use["register"] for use in demands if 4 <= use["register"] <= 7}
    )
    _require(
        _references(summary["parameterRegisters"], 7, 4) == expected_lanes,
        "entry parameter projection",
    )
    _transfers(summary["transfers"], definitions, instructions, covered=covered)
    exits = _rows(summary["exits"], 4096, "summary exits")
    _require(
        [item["pc"] for item in exits] == [item["pc"] for item in evidence["exits"]],
        "summary normal exit inventory",
    )
    for exit, original_exit in zip(exits, evidence["exits"], strict=True):
        rows = _exit_transfers(summary, exit)
        complete = _complete(exit["incomingLink"])
        _integer(exit["sampledEntry"], 33, optional=True)
        _integer(exit["sampledConstant"], (1 << 32) - 1, optional=True)
        _require(
            exit["pc"] in instructions
            and instructions[exit["pc"]]["word"] == 0x03E00008
            and exit["delay"] == exit["pc"] + 4 == original_exit["delaySlot"]
            and exit["delay"] in instructions,
            "JR sampled target and delay ownership",
        )
        refs = _references(exit["sampledDefinitions"], 4096, 1)
        _require(
            all(
                ref in definitions and definitions[ref]["register"] == 31
                for ref in refs
            ),
            "sampled target definitions",
        )
        equality = _references(exit["equalityEvidence"], 1 << 32)
        _require(
            all(pc in instructions for pc in equality), "sampled equality ownership"
        )
        if complete:
            _require(
                covered
                and exit["sampledEntry"] == 31
                and exit["pc"] in equality
                and all(definitions[ref]["equalEntry"] == 31 for ref in refs),
                "sampled incoming-link value equality",
            )
        transfers = _transfers(rows, definitions, instructions, covered=covered)
        if transfers:
            _require(
                transfers[2]["definitions"] == original_exit["definitions"]
                and transfers[2]["pendingDefinitions"]
                == original_exit["pendingDefinitions"],
                "exit V0 transfer disagreement",
            )
    accesses = _rows(summary["accesses"], 4096, "memory accesses")
    seen = set()
    for access in accesses:
        _keys(
            access,
            set(
                "pc kind extension width delayed addressDefinitions addressEntries valueDefinitions valueEntries obligation address".split()
            )
            | memorycontracts.DIMENSIONS,
            "memory access",
        )
        _require(
            access["pc"] in instructions and access["pc"] not in seen,
            "memory access ownership/uniqueness",
        )
        seen.add(access["pc"])
        opcode = instructions[access["pc"]]["word"] >> 26
        _require(opcode in MEMORY_OPERATIONS, "memory operation provenance")
        actual = tuple(
            access[field] for field in ("kind", "width", "extension", "delayed")
        )
        _require(
            type(access["delayed"]) is bool and actual == MEMORY_OPERATIONS[opcode],
            "memory width/extension/delay",
        )
        for field in ("addressDefinitions", "valueDefinitions"):
            _require(
                all(ref in definitions for ref in _references(access[field], 4096, 1)),
                "memory definition reference",
            )
        for field in ("addressEntries", "valueEntries"):
            _references(access[field], 33, 1)
        _require(not _complete(access["obligation"]), "unsupported memory closure")
        memorycontracts.validate_access(
            row,
            access,
            payload,
            load,
            entry_flow,
            context=context,
            images=images,
            frame_uses=frame_uses,
        )
    if covered:
        reachable = {edge["from"] for edge in evidence["edges"]} | {
            pc for exit in evidence["exits"] for pc in (exit["pc"], exit["delaySlot"])
        }
        _require(
            seen
            == {
                pc
                for pc in reachable
                if pc in instructions
                and instructions[pc]["word"] >> 26 in MEMORY_OPERATIONS
            },
            "memory access inventory incomplete",
        )
    if facts["control"]:
        _require(
            exits
            and all(_complete(exit["incomingLink"]) for exit in exits)
            and all(
                _complete(call["contract"]["control"]) for call in evidence["calls"]
            ),
            "nonempty control closure",
        )
    if facts["effects"]:
        _require(
            not accesses
            and all(
                _complete(call["contract"]["effects"]) for call in evidence["calls"]
            ),
            "effect closure inventory",
        )
    if facts["declaration"]:
        _require(
            facts["entry"]
            and facts["result"]
            and all(_declaration_demand(use) for use in demands),
            "supported declaration facts",
        )
    return summary


def _call_links(call, caller, graph):
    contract = call["contract"]
    facts = {key: _complete(contract[key]) for key in DIMENSIONS}
    no_writes = "noMemoryWrites" in contract and _complete(contract["noMemoryWrites"])
    _require(
        not no_writes or all(facts[key] for key in ("identity", "inputs", "control")),
        "composed memory-write absence lacks qualified call",
    )
    remaining = sorted(
        {reason for key in DIMENSIONS for reason in contract[key]["remaining"]}
    )
    _require(
        contract["remaining"] == remaining and call["unresolved"] == remaining,
        "derived call remainders",
    )
    _require(
        call["qualified"] == (contract["kind"] == "complete")
        and (bool(remaining) != call["qualified"]),
        "derived complete call",
    )
    _require(
        not call["qualified"] or all(facts.values()),
        "complete call has partial dimension",
    )
    row, _, instructions, definitions = caller
    pc = call["pc"]
    _require(
        pc in instructions
        and contract["delay"] == pc + 4
        and contract["continuation"] == pc + 8
        and pc + 4 in instructions,
        "caller call/delay ownership",
    )
    word = instructions[pc]["word"]
    if word >> 26 == 3:
        _require(
            call["target"] == ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2),
            "direct JAL original target",
        )
    else:
        _require(
            word & 63 == 9
            and word >> 26 == 0
            and call["target"] is None
            and not facts["identity"],
            "unsupported indirect call remains partial",
        )
    if not contract["callee"]:
        _require(
            not any(facts.values())
            and not contract["bindings"]
            and not contract["calleeExits"]
            and not contract["registerTransfers"],
            "unknown call acquired proof references",
        )
        return
    _require(
        contract["callee"] in graph and facts["identity"],
        "callee belongs to current invocation",
    )
    callee_row, summary, _, _ = graph[contract["callee"]]
    _require(
        not no_writes
        or ("noMemoryWrites" in summary and _complete(summary["noMemoryWrites"])),
        "composed memory-write absence lacks callee fact",
    )
    _require(
        call["target"] == callee_row["returnDisposition"]["evidence"]["start"]
        and contract["calleeExits"] == [exit["pc"] for exit in summary["exits"]],
        "callee entry/normal exits",
    )
    _require(
        _same_transfers(contract["registerTransfers"], summary["transfers"]),
        "callee transfer substitution source",
    )
    for dimension in ("result", "effects", "declaration"):
        _require(
            contract[dimension] == summary[dimension], "callee dimension disagreement"
        )
    bindings = {item["register"]: item for item in contract["bindings"]}
    demands = {}
    for use in summary["demands"]:
        demands.setdefault(use["register"], set()).add(use["pc"])
    for transfer in summary["transfers"]:
        if (
            transfer["kind"] == "assigned"
            and _complete(transfer["proof"])
            and not transfer["pending"]
        ):
            for entry in transfer["entries"]:
                demands.setdefault(entry, set())
    _require(
        len(bindings) == len(contract["bindings"]) and set(bindings) == set(demands),
        "all live-in bindings",
    )
    for reg, binding in bindings.items():
        _keys(binding, BINDING_FIELDS, "actual input binding")
        _require(
            type(binding["missing"]) is bool
            and _references(binding["uses"], 1 << 32) == sorted(demands[reg]),
            "bound original entry uses",
        )
        _require(
            all(
                ref in definitions
                for ref in _references(binding["definitions"], 4096, 1)
            ),
            "caller input definition references",
        )
        _references(binding["entries"], 33, 1)
        _reasons(binding["bad"])
        _integer(binding["equalEntry"], 33, optional=True)
        _integer(binding["constant"], (1 << 32) - 1, optional=True)
        if binding["constant"] is not None:
            _require(
                all(
                    definitions[ref]["constant"] == binding["constant"]
                    for ref in binding["definitions"]
                ),
                "actual input constant proof",
            )
    if facts["inputs"]:
        _require(
            _complete(summary["entry"])
            and all(
                not item["missing"] and not item["bad"] for item in bindings.values()
            ),
            "complete actual input bindings",
        )
    if facts["control"]:
        _require(
            _complete(summary["control"])
            and 31 in bindings
            and bindings[31]["constant"] == contract["actualIncomingLink"] == pc + 8,
            "actual bound incoming link equals caller continuation",
        )
    if facts["transfers"]:
        _require(
            facts["control"]
            and len(summary["transfers"]) == 34
            and all(
                _complete(item["proof"]) and not item["pending"]
                for item in summary["transfers"]
            ),
            "all-register transfer closure",
        )
    call_definitions = [
        definition
        for definition in definitions.values()
        if definition["origin"]["kind"] == "call" and definition["pc"] == pc
    ]
    successors = {}
    for edge in row["returnDisposition"]["evidence"]["edges"]:
        successors.setdefault(edge["from"], set()).add(edge["to"])
    for source in row["returnDisposition"]["evidence"]["obligations"]:
        if source["domain"] != "architectural" or source["reason"] != FRAME_OBLIGATION:
            continue
        pending, visited = [source["pc"] + 8], set()
        while pending:
            point = pending.pop()
            if point in visited:
                continue
            visited.add(point)
            _require(len(visited) <= 4096, "conditional frame dependent-use bound")
            pending.extend(successors.get(point, set()))
        if pc in visited:
            condition = f"conditional-callee-preservation-{source['pc']}"
            _require(
                condition in contract["inputs"]["remaining"]
                and condition in contract["transfers"]["remaining"],
                "dependent call discarded conditional frame obligation",
            )
    obligations = row["returnDisposition"]["evidence"]["obligations"]
    frame_obligations = [
        item
        for item in obligations
        if item["pc"] == pc
        and item["domain"] == "architectural"
        and item["reason"] == FRAME_OBLIGATION
    ]
    frame = bool(frame_obligations)
    if frame:
        _frame_binding(call, row, callee_row)
        _require(
            "symbolicFrame" not in call.get("entryPath", {}) or not call_definitions,
            "bound frame cannot supply a call-produced result",
        )
    scoped_r2 = frame and bool(call_definitions)
    if scoped_r2:
        restoration = summary.get("symbolicFrame", {})
        links = [
            item for item in restoration.get("registers", []) if item["register"] == 31
        ]
        _require(
            len(summary["transfers"]) == 34,
            "conditional register-2 transfer source incomplete",
        )
        transfer = summary["transfers"][2]
        condition = f"conditional-callee-preservation-{pc}"
        _require(
            contract["kind"] == "partial"
            and not call["qualified"]
            and not facts["control"]
            and not facts["transfers"]
            and _complete(summary["entry"])
            and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
            and restoration.get("entryPc") == call["target"]
            and len(links) == 1
            and bool(links[0]["loadPcs"])
            and links[0]["exitPcs"] == contract["calleeExits"]
            and bool(contract["calleeExits"])
            and contract["actualIncomingLink"] == pc + 8
            and condition in contract["inputs"]["remaining"]
            and condition in contract["transfers"]["remaining"]
            and not {
                "pending-call-boundary-load-unresolved",
                "pending-return-load-boundary-unresolved",
            }.intersection(remaining)
            and transfer["register"] == 2
            and transfer["kind"] == "assigned"
            and _complete(transfer["proof"])
            and bool(transfer["definitions"])
            and not transfer["bad"]
            and not transfer["pending"]
            and transfer["mayNoPending"]
            and not transfer["pendingDefinitions"]
            and all(
                entry in bindings
                and not bindings[entry]["missing"]
                and not bindings[entry]["bad"]
                for entry in transfer["entries"]
            )
            and len(call_definitions) == 1
            and call_definitions[0]["register"] == 2
            and not call_definitions[0]["bad"]
            and call_definitions[0]["entries"]
            == sorted(
                {
                    reg
                    for entry in transfer["entries"]
                    for reg in bindings[entry]["entries"]
                }
            ),
            "conditional register-2 transfer lacks scoped callee binding",
        )
    for definition in call_definitions:
        origin = definition["origin"]
        reg = definition["register"]
        transfer = summary["transfers"][reg]
        _require(
            origin["callee"] == contract["callee"]
            and origin["available"] == pc + 8
            and origin["delay"] == pc + 4
            and origin["calleeDefinitions"] == transfer["definitions"]
            and origin["substitutedEntries"] == transfer["entries"],
            "call-produced definition composition",
        )
        _require(
            (facts["control"] or (scoped_r2 and reg == 2))
            and _complete(transfer["proof"])
            and transfer["kind"] == "assigned"
            and not transfer["pending"]
            and definition["width"] == 32
            and not definition["delayed"]
            and bool(origin["calleeDefinitions"]),
            "supported call-produced transfer",
        )
        _require(
            set(origin["substitutedEntries"]) <= set(bindings)
            and definition["inputs"]
            == sorted(
                {
                    ref
                    for entry in origin["substitutedEntries"]
                    for ref in bindings[entry]["definitions"]
                }
            ),
            "actual substituted caller inputs",
        )


def validate_functions(
    rows,
    identities,
    *,
    payload,
    load,
    images=None,
    context=None,
    entry_observation=None,
):
    _require(type(payload) is bytes and payload, "complete original memory payload")
    _integer(load, 0xFFFFFFFF)
    _require(
        context is None or images is not None, "mounted context lacks verified images"
    )
    frames = {}
    for key, row in rows.items():
        summary = row.get("originalCallSummary", {})
        _require(
            "entryContext" not in summary or entry_observation is True,
            "observed entry context lacks captured input",
        )
        if "symbolicFrame" in summary:
            _keys(
                summary,
                SUMMARY_FIELDS | {"noMemoryWrites", "symbolicFrame"},
                "symbolic function summary",
            )
            _symbolic_frame(summary, row["returnDisposition"]["evidence"])
            frames[key] = row
    graph = {}
    for key, row in rows.items():
        identity = identities[key]
        evidence = row["returnDisposition"]["evidence"]
        instructions = {item["pc"]: item for item in evidence["instructions"]}
        definitions = {item["id"]: item for item in evidence["definitions"]}
        for definition in definitions.values():
            _definition_extras(definition, instructions)
        for call in evidence["calls"]:
            value = call["contract"]
            fields = CONTRACT_FIELDS | (
                {"noMemoryWrites"}
                if type(value) is dict and "noMemoryWrites" in value
                else set()
            )
            contract = _keys(value, fields, "call contract")
            _require(
                contract["kind"] in {"partial", "complete"}
                and type(contract["callee"]) is str,
                "call contract variant",
            )
            for dimension in DIMENSIONS:
                _complete(contract[dimension])
            if "noMemoryWrites" in contract:
                _complete(contract["noMemoryWrites"])
            _integer(contract["actualIncomingLink"], (1 << 32) - 1, optional=True)
            _rows(contract["bindings"], 34, "live-in bindings")
            _references(contract["calleeExits"], 1 << 32)
            _rows(contract["registerTransfers"], 34, "call register transfers")
            _reasons(contract["remaining"])
        validate_return(
            row,
            {
                "start": identity["start"],
                "end": identity["end"],
                "selection": "selected-root" if identity["root"] else "analysis-callee",
            },
            revised=True,
            entry_observation=(
                "entryContext" in row.get("originalCallSummary", {})
                if entry_observation is True
                else entry_observation
            ),
        )
        frame_uses = {}
        for call in evidence["calls"]:
            if any(
                item["pc"] == call["pc"]
                and item["domain"] == "architectural"
                and item["reason"] == FRAME_OBLIGATION
                for item in evidence["obligations"]
            ):
                _require(
                    call["contract"]["callee"] in frames
                    or "symbolicFrame" in call.get("entryPath", {})
                    and call["contract"]["callee"] in rows,
                    "conditional frame source absent",
                )
                frame_uses[call["pc"]] = _frame_binding(
                    call, row, rows[call["contract"]["callee"]]
                )
        entry_flow = memorycontracts._entry_flow(row, frame_uses=frame_uses)
        original_payload, original_load = (
            (payload, load) if images is None else images[identity["image_id"]]
        )
        graph[key] = (
            row,
            _summary(
                row,
                identity,
                original_payload,
                original_load,
                entry_flow,
                context,
                images,
                frame_uses,
            ),
            instructions,
            definitions,
        )
        _validate_entry_arithmetic(row, entry_flow)
    for value in graph.values():
        row, _, _, definitions = value
        calls = {
            call["pc"]: call for call in row["returnDisposition"]["evidence"]["calls"]
        }
        _require(
            len(calls) == len(row["returnDisposition"]["evidence"]["calls"]),
            "duplicate call proof",
        )
        for definition in definitions.values():
            if definition["origin"]["kind"] == "call":
                _require(
                    definition["pc"] in calls, "call-produced definition has no call"
                )
        for call in calls.values():
            _call_links(call, value, graph)
            if "entryPath" in call:
                pathcontracts.validate_entry_path(
                    call,
                    value,
                    graph,
                    context=context,
                    entry_observation=entry_observation,
                )
                if "symbolicFrame" in call["entryPath"]:
                    pathcontracts.bound_frame_registers(call, FRAME_CONDITIONS)

    pending = set(graph)
    while pending:
        ready = {
            key
            for key in pending
            if not any(
                call["contract"]["callee"] in pending
                for call in graph[key][0]["returnDisposition"]["evidence"]["calls"]
            )
        }
        _require(ready, "cyclic call proof references")
        pending -= ready


def _validate_ordinary_body_candidate(row: dict):
    native = row["returnDisposition"]
    evidence = native["evidence"]
    summary = row["originalCallSummary"]
    rows = evidence["instructions"]
    start, end = evidence["start"], evidence["end"]
    _require(
        native["kind"] == "unknown"
        and evidence["selection"] == "selected-root"
        and evidence["analysisComplete"] is True
        and not _complete(summary["result"])
        and not _complete(summary["declaration"])
        and not row.get("parameters", [])
        and "returnType" not in row
        and "returnStorage" not in row
        and 2 <= len(rows) <= 4096
        and end == start + 4 * len(rows)
        and rows[-2]["word"] == 0x03E00008
        and len(evidence["exits"]) == 1
        and not any(
            item["domain"] == "architectural"
            and item["reason"]
            in {
                "conditional-callee-preservation-unresolved",
                FRAME_OBLIGATION,
            }
            for item in evidence["obligations"]
        ),
        "native ordinary-body proposal scope or contract differs",
    )
    immediate = rows[0]["word"] & 0xFFFF
    immediate -= 0x10000 if immediate & 0x8000 else 0
    frame = -immediate if rows[0]["word"] >> 16 == 0x27BD else 0
    slot_restore = frame != 0 and rows[-1]["word"] == (0x27BD0000 | frame)
    early_restore = (
        frame != 0
        and len(rows) >= 3
        and rows[-1]["word"] == 0
        and rows[-3]["word"] == (0x27BD0000 | frame)
    )
    _require(
        not frame
        or (8 <= frame <= 32767 and frame % 8 == 0 and (slot_restore or early_restore)),
        "native ordinary-body proposal frame differs",
    )
    edges, calls = [], []
    call_indices = [index for index, item in enumerate(rows) if item["word"] >> 26 == 3]
    last_call = call_indices[-1] if call_indices else None
    saved_link = None
    save_pc = restore_pc = None
    saved_count = loaded_count = branch_count = 0
    pending_load = None
    for index, instruction in enumerate(rows):
        pc, word = instruction["pc"], instruction["word"]
        op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
        call, terminal = op == 3, word == 0x03E00008
        branch = op in {4, 5}
        load, store = op in {32, 35, 36}, op in {40, 41, 43}
        writes_register = op in {9, 15} or load
        previous = rows[index - 1]["word"] if index else 0
        _require(
            pc == start + 4 * index
            and instruction["owner"] == evidence["symbol"]
            and instruction["bytes"] == 4
            and instruction["mapped"] is True
            and instruction["undef"] is False
            and (word == 0 or writes_register or store or call or terminal or branch)
            and (op != 15 or rs == 0)
            and (not terminal or index == len(rows) - 2)
            and (op != 32 or (last_call is not None and index > last_call))
            and not (
                (previous >> 26 in {3, 4, 5} or previous == 0x03E00008)
                and (call or terminal or branch or (load and rt))
            ),
            "native ordinary-body proposal instruction lowering differs",
        )
        if branch:
            immediate = word & 0xFFFF
            immediate -= 0x10000 if immediate & 0x8000 else 0
            _require(
                not branch_count
                and frame
                and saved_count == 1
                and not loaded_count
                and (last_call is None or index > last_call)
                and index + 3 < len(rows)
                and pc + 4 + 4 * immediate == pc + 12
                and rows[index + 1]["word"] >> 16 == 0x2402
                and rows[index + 2]["word"] >> 16 == 0x2402,
                "native ordinary-body proposal conditional assignment differs",
            )
            branch_count += 1
        reads = {rs} if op == 9 or load or store or branch else set()
        if store or branch:
            reads.add(rt)
        if terminal:
            reads.add(31)
        _require(
            pending_load not in reads
            and (
                not writes_register or not rt or rt in instruction["fullWidthRegisters"]
            )
            and (not call or 31 in instruction["fullWidthRegisters"]),
            "native ordinary-body proposal register timing or width differs",
        )
        pending_load = rt if load and rt else None
        adjustment = (
            frame
            and op == 9
            and rs == rt == 29
            and (
                index == 0
                or (slot_restore and index == len(rows) - 1)
                or (early_restore and index == len(rows) - 3)
            )
        )
        link_access = frame and rs == 29 and rt == 31 and op in {35, 43}
        if link_access:
            offset = word & 0xFFFF
            offset -= 0x10000 if offset & 0x8000 else 0
            _require(
                0 <= offset and offset % 4 == 0 and offset + 4 <= frame,
                "native ordinary-body proposal stack access escapes frame",
            )
            if op == 43:
                _require(
                    not calls and not loaded_count and not saved_count,
                    "native proposal link save",
                )
                saved_link, saved_count = offset, saved_count + 1
                save_pc = pc
            else:
                _require(
                    saved_link == offset and not loaded_count and index < len(rows) - 2,
                    "native proposal link load",
                )
                loaded_count += 1
                restore_pc = pc
        _require(
            adjustment
            or link_access
            or not (
                29 in reads
                or (writes_register and rt in {29, 31})
                or (not terminal and 31 in reads)
            ),
            "native ordinary-body proposal stack or link escape",
        )
        if call:
            _require(
                not loaded_count and index + 2 < len(rows),
                "native ordinary-body proposal call continuation differs",
            )
            calls.append((pc, ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)))
        if index + 1 < len(rows):
            edges.append(
                dict(
                    **{"from": pc, "to": pc + 4},
                    delayOwner=pc if call or terminal or branch else None,
                    kind="delay-slot"
                    if call or terminal or branch
                    else "call-continuation"
                    if previous >> 26 == 3
                    else "fallthrough",
                )
            )
            if previous >> 26 in {4, 5}:
                edges.append(
                    {"from": pc, "to": pc + 8, "delayOwner": None, "kind": "branch"}
                )
    _require(
        evidence["edges"] == edges
        and [(call["pc"], call["target"]) for call in evidence["calls"]] == calls
        and (not frame or saved_count == loaded_count == 1)
        and (not calls or frame),
        "native ordinary-body proposal control closure differs",
    )
    exit_row = evidence["exits"][0]
    _require(
        exit_row["pc"] == end - 8
        and exit_row["delaySlot"] == end - 4
        and exit_row["missing"] is False
        and exit_row["definitions"]
        and not any(exit_row[key] for key in ("formals", "pendingDefinitions", "bad")),
        "native ordinary-body proposal fresh V0 exit differs",
    )
    definitions = {
        definition["id"]: definition for definition in evidence["definitions"]
    }
    _require(
        len(definitions) == len(evidence["definitions"])
        and all(definitions[ref]["register"] == 2 for ref in exit_row["definitions"]),
        "native ordinary-body proposal V0 ownership differs",
    )
    pending, visited = list(exit_row["definitions"]), set()
    while pending:
        reference = pending.pop()
        if reference in visited:
            continue
        visited.add(reference)
        definition = definitions[reference]
        pc = definition["pc"]
        _require(
            definition["width"] == 32
            and definition["origin"] == {"kind": "instruction"}
            and not any(definition[key] for key in ("bad", "entries", "formals"))
            and start <= pc < end
            and (pc - start) % 4 == 0
            and definition["register"] in rows[(pc - start) // 4]["fullWidthRegisters"]
            and all(ref < reference for ref in definition["inputs"]),
            "native ordinary-body proposal fresh definition closure differs",
        )
        pending.extend(definition["inputs"])
    return (save_pc, restore_pc, frame, saved_link - frame) if frame else None


STACK_WORD_BODY_KIND = "mips-entry-stack-word-body/v1"


def is_stack_word_body_candidate(row: dict) -> bool:
    candidate = row.get("originalBodyCandidate")
    return type(candidate) is dict and candidate.get("kind") == STACK_WORD_BODY_KIND


def _stack_word_inputs(word):
    fields = word_fields(word)
    op, fn = fields.opcode, fields.funct
    if op == 0:
        if fn in {0, 2, 3}:
            return (fields.rt,)
        if fn in {16, 18}:
            return (32 if fn == 16 else 33,)
        if fn == 8:
            return (fields.rs,)
        return (fields.rs, fields.rt)
    if op in {2, 15}:
        return ()
    if op in {4, 5, 40, 41, 43}:
        return (fields.rs, fields.rt)
    return (fields.rs,)


def _validate_stack_word_body_candidate(row: dict) -> None:
    candidate = _keys(
        row["originalBodyCandidate"],
        {"kind", "input", "condition"},
        "stack word proposal",
    )
    witness = _keys(candidate["input"], {"pc", "definition"}, "stack word input")
    _integer(witness["pc"])
    _integer(witness["definition"], 4096)
    _require(
        candidate["kind"] == STACK_WORD_BODY_KIND
        and candidate["condition"] == "o32-entry-sp16-word-stable-until-load",
        "conditional stack word kind or condition differs",
    )
    native, summary = row["returnDisposition"], row["originalCallSummary"]
    evidence = native["evidence"]
    start, end = evidence["start"], evidence["end"]
    rows = evidence["instructions"]
    _require(
        (
            native["kind"] == "unknown"
            and summary["result"]["kind"] == "partial"
            and not _complete(summary["result"])
            or native["kind"] == "value"
            and _complete(summary["result"])
        )
        and evidence["selection"] == "selected-root"
        and evidence["decodeComplete"] is evidence["analysisComplete"] is True
        and 3 <= len(rows) <= 4096
        and end == start + 4 * len(rows)
        and not evidence["calls"]
        and not row.get("originalJumpTables", [])
        and _complete(summary["entry"])
        and _complete(summary["control"])
        and all(
            summary[key]["kind"] == "partial" and not _complete(summary[key])
            for key in ("declaration", "effects")
        )
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads"
        and len(evidence["exits"]) == len(summary["exits"]) == 1,
        "conditional stack word proof scope differs",
    )
    instructions = {item["pc"]: item for item in rows}
    definitions = {item["id"]: item for item in evidence["definitions"]}
    _require(
        len(instructions) == len(rows)
        and len(definitions) == len(evidence["definitions"]),
        "conditional stack word duplicate instruction or definition",
    )
    stack_uses, returns = [], []
    prefix = True
    for index, instruction in enumerate(rows):
        pc, word = instruction["pc"], instruction["word"]
        fields = word_fields(word)
        kind, destinations = memorycontracts._memory_word(word)
        _require(
            pc == start + 4 * index
            and pc % 4 == 0
            and instruction["bytes"] == 4
            and instruction["mapped"] is True
            and instruction["undef"] is False
            and instruction["owner"] == row["name"]
            and kind not in {"unsupported", "call", "trap", "indirect-branch"}
            and fields.opcode not in {34, 38, 42, 46}
            and not {29, 31} & set(destinations)
            and set(instruction["fullWidthRegisters"])
            == {reg for reg in destinations if reg}
            and all(
                not reg or instruction["fullWidthRegisters"].count(reg) == 1
                for reg in destinations
            ),
            "conditional stack word unsupported physical instruction",
        )
        if 29 in _stack_word_inputs(word):
            _require(
                prefix
                and fields.opcode == 35
                and fields.rs == 29
                and word & 0xFFFF == 16
                and fields.rt != 0,
                "conditional stack word SP escape or input shape differs",
            )
            stack_uses.append(pc)
        if kind == "return":
            _require(word == 0x03E00008, "conditional stack word return shape differs")
            returns.append(pc)
        if kind in {"branch", "store", "return"}:
            prefix = False
    _require(
        stack_uses == [witness["pc"]], "conditional stack word input is not unique"
    )
    memorycontracts._memory_prefix(row, {"pc": witness["pc"]})
    edges = evidence["edges"]
    outgoing, incoming, delay_owners = {}, {}, {}
    edge_set = set()
    for edge in edges:
        key = (edge["from"], edge["to"], edge["delayOwner"], edge["kind"])
        _require(
            key not in edge_set
            and edge["from"] in instructions
            and edge["to"] in instructions,
            "conditional stack word control edge differs",
        )
        edge_set.add(key)
        outgoing.setdefault(edge["from"], []).append(edge)
        incoming.setdefault(edge["to"], []).append(edge["from"])
        delay_owners.setdefault(edge["to"], set()).add(edge["delayOwner"])
    reachable, pending = set(), [start]
    while pending:
        pc = pending.pop()
        if pc in reachable:
            continue
        reachable.add(pc)
        owners = delay_owners.get(pc, {None})
        _require(len(owners) == 1, "conditional stack word ambiguous delay owner")
        owner = next(iter(owners))
        kind, _ = memorycontracts._memory_word(instructions[pc]["word"])
        allowed = set()
        if owner is not None:
            _require(
                owner in instructions
                and pc == owner + 4
                and kind not in {"branch", "return"},
                "conditional stack word control in delay slot",
            )
            word = instructions[owner]["word"]
            owner_kind, _ = memorycontracts._memory_word(word)
            _require(owner_kind in {"branch", "return"}, "stack word delay owner kind")
            if owner_kind == "branch":
                target = (
                    ((owner + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
                    if word >> 26 == 2
                    else (owner + 4 + memorycontracts._signed16(word) * 4) & 0xFFFFFFFF
                )
                allowed.add((target, None, "branch"))
                if word >> 26 != 2:
                    allowed.add((owner + 8, None, "fallthrough"))
        else:
            allowed.add(
                (pc + 4, pc, "delay-slot")
                if kind in {"branch", "return"}
                else (pc + 4, None, "fallthrough")
            )
        observed = {
            (edge["to"], edge["delayOwner"], edge["kind"])
            for edge in outgoing.get(pc, [])
        }
        _require(
            observed == allowed,
            "conditional stack word direct control closure differs",
        )
        pending.extend(edge[0] for edge in observed)
    _require(
        reachable == set(instructions), "conditional stack word incomplete control"
    )
    load = definitions.get(witness["definition"])
    _require(
        load is not None
        and load["pc"] == witness["pc"]
        and load["register"] == word_fields(instructions[witness["pc"]]["word"]).rt
        and load["width"] == 32
        and load["operation"] == "full-register-load"
        and load["delayed"] is True
        and load["origin"] == {"kind": "instruction"}
        and load["inputs"] == load["formals"] == []
        and load["entries"] == [29]
        and load["bad"] == ["unproved-entry-register-29"]
        and sum(item["pc"] == witness["pc"] for item in definitions.values()) == 1,
        "conditional stack word delayed definition differs",
    )

    def derives(reference):
        pending, visited = [reference], set()
        while pending:
            ref = pending.pop()
            if ref in visited:
                continue
            visited.add(ref)
            _require(
                ref in definitions and len(visited) <= 4096, "stack word causality"
            )
            pending.extend(definitions[ref]["inputs"])
        return witness["definition"] in visited

    by_writer = {}
    for definition in definitions.values():
        _require(
            definition["width"] == 32
            and definition["origin"] == {"kind": "instruction"}
            and definition["pc"] in instructions
            and definition["bad"]
            == (["unproved-entry-register-29"] if derives(definition["id"]) else []),
            "conditional stack word unrelated unproved definition",
        )
        key = (definition["pc"], definition["register"])
        _require(key not in by_writer, "conditional stack word duplicate writer")
        by_writer[key] = definition["id"]

    def input_definitions(pc, reg):
        if reg == 0:
            return set(), False
        pending = [(previous, False) for previous in incoming.get(pc, [])]
        visited, found = set(), set()
        entry = pc == start
        while pending:
            previous, available_load = pending.pop()
            if (previous, available_load) in visited:
                continue
            visited.add((previous, available_load))
            _require(len(visited) <= 8192, "stack word predecessor bound")
            kind, destinations = memorycontracts._memory_word(
                instructions[previous]["word"]
            )
            if reg in destinations and (kind != "load" or available_load):
                reference = by_writer.get((previous, reg))
                _require(reference is not None, "stack word missing physical writer")
                found.add(reference)
                continue
            entry |= previous == start
            pending.extend((earlier, True) for earlier in incoming.get(previous, []))
        return found, entry

    for instruction in rows:
        pc, word = instruction["pc"], instruction["word"]
        kind, _ = memorycontracts._memory_word(word)
        for reg in _stack_word_inputs(word):
            _, entry = input_definitions(pc, reg)
            _require(
                not entry
                or 4 <= reg <= 7
                or (reg == 31 and kind == "return")
                or (reg == 29 and pc == witness["pc"]),
                "stack word physical operand has unrelated entry origin",
            )

    for definition in definitions.values():
        if definition["id"] == witness["definition"]:
            continue
        sources = {
            ref
            for reg in _stack_word_inputs(instructions[definition["pc"]]["word"])
            for ref in input_definitions(definition["pc"], reg)[0]
        }
        physical_taint = any(derives(ref) for ref in sources)
        _require(
            physical_taint == derives(definition["id"])
            and (
                not physical_taint
                or definition["bad"] == ["unproved-entry-register-29"]
            ),
            "stack word physical writer load causality differs",
        )

    tainted_uses = set()
    for demand in summary["demands"]:
        reg, pc = demand["register"], demand["pc"]
        if 4 <= reg <= 7 or (reg == 31 and demand["role"] == "continuation"):
            continue
        _require(reg == 29 and pc in instructions, "stack word unrelated entry demand")
        if pc != witness["pc"]:
            refs = {
                ref
                for source in _stack_word_inputs(instructions[pc]["word"])
                for ref in input_definitions(pc, source)[0]
            }
            _require(
                any(definitions[ref]["bad"] and derives(ref) for ref in refs),
                "stack word entry demand lacks load causality",
            )
        tainted_uses.add(pc)
    entry_obligations = []
    for obligation in evidence["obligations"]:
        if obligation["domain"] == "architectural":
            _require(
                native["kind"] == "unknown"
                and obligation["pc"] == start
                and obligation["reason"] == "all-path-v0-definition-unresolved",
                "stack word unrelated architectural failure",
            )
        if obligation["domain"] == "entry":
            _require(
                obligation["reason"] == "unproved-control-or-effect-input"
                and obligation["pc"] in tainted_uses
                and memorycontracts._memory_word(
                    instructions[obligation["pc"]]["word"]
                )[0]
                in {"branch", "store"},
                "stack word entry obligation lacks physical causality",
            )
            entry_obligations.append(obligation)
    exit, normal = evidence["exits"][0], summary["exits"][0]
    _require(
        entry_obligations
        and returns == [exit["pc"]]
        and exit["delaySlot"] == exit["pc"] + 4
        and exit["missing"] is False
        and exit["definitions"]
        and not exit["bad"]
        and not exit["pendingDefinitions"]
        and all(
            definitions[ref]["register"] == 2 and not definitions[ref]["bad"]
            for ref in exit["definitions"]
        )
        and normal["pc"] == exit["pc"]
        and normal["delay"] == exit["delaySlot"]
        and _complete(normal["incomingLink"])
        and normal["sampledEntry"] == 31,
        "stack word clean V0 or incoming-link exit differs",
    )
    slot_kind, slot_destinations = memorycontracts._memory_word(
        instructions[exit["delaySlot"]]["word"]
    )
    _require(
        slot_kind != "load" or 2 not in slot_destinations,
        "stack word return has a pending V0 load",
    )
    if 2 in slot_destinations:
        reference = by_writer.get((exit["delaySlot"], 2))
        _require(reference is not None, "stack word return slot writer is missing")
        reaching, entry_v0 = {reference}, False
    else:
        reaching, entry_v0 = input_definitions(exit["delaySlot"], 2)
    _require(
        not entry_v0 and set(exit["definitions"]) == reaching,
        "stack word exit V0 definitions differ from physical return",
    )
    _require(
        row.get("isVariadic", False) is False
        and row.get("returnType") == {"llvmIr": "i32"}
        and row.get("returnStorage")
        == {"type": "register", "value": "v0", "registerNumber": 4},
        "stack word mechanical return carrier differs",
    )
    parameters = _rows(row.get("parameters", []), 5, "stack word carriers")
    _require(len(parameters) == 5, "stack word carrier count differs")
    names = []
    for index, parameter in enumerate(parameters):
        _require(type(parameter) is dict, "stack word carrier object")
        name = parameter.get("name")
        storage = (
            {"type": "register", "value": f"a{index}", "registerNumber": index + 6}
            if index < 4
            else {"type": "stack", "value": 16}
        )
        _require(
            type(name) is str
            and name == f"arg{index + 1}"
            and name not in names
            and parameter.get("type") == {"llvmIr": "i32"}
            and parameter.get("storage") == storage,
            "stack word mechanical carrier order or storage differs",
        )
        names.append(name)


def validate_body_candidate(row: dict) -> bool:
    """Authenticate a body proposal without recovering a callee declaration."""
    if "originalBodyCandidate" not in row:
        return False
    if row["originalBodyCandidate"] == "mips-ordinary-body/v1":
        _validate_ordinary_body_candidate(row)
        return True
    if is_stack_word_body_candidate(row):
        _validate_stack_word_body_candidate(row)
        return True
    native = row["returnDisposition"]
    evidence = native["evidence"]
    summary = row["originalCallSummary"]
    start = evidence["start"]
    _require(
        row["originalBodyCandidate"] == "mips-empty-body/v1"
        and native["kind"] == "unknown"
        and evidence["selection"] == "selected-root"
        and evidence["decodeComplete"] is evidence["analysisComplete"] is True
        and evidence["end"] == start + 8
        and not evidence["calls"]
        and not evidence["definitions"]
        and not row.get("parameters", [])
        and "returnType" not in row
        and "returnStorage" not in row
        and all(_complete(summary[key]) for key in ("entry", "control", "effects"))
        and not _complete(summary["result"])
        and not _complete(summary["declaration"])
        and not summary["accesses"]
        and not summary["parameterRegisters"],
        "native empty-body proposal scope or contract differs",
    )
    _require(
        evidence["instructions"]
        == [
            dict(
                pc=start + 4 * index,
                word=word,
                owner=row["name"],
                bytes=4,
                mapped=True,
                undef=False,
                fullWidthRegisters=[],
            )
            for index, word in enumerate((0x03E00008, 0))
        ]
        and evidence["edges"]
        == [{"from": start, "to": start + 4, "delayOwner": start, "kind": "delay-slot"}]
        and evidence["obligations"]
        == [
            dict(
                pc=start,
                domain="architectural",
                reason="all-path-v0-definition-unresolved",
            )
        ]
        and summary["demands"] == [dict(pc=start, register=31, role="continuation")],
        "native empty-body proposal instruction/control closure differs",
    )
    _require(
        len(evidence["exits"]) == len(summary["exits"]) == 1
        and evidence["exits"][0]["pc"] == start
        and evidence["exits"][0]["delaySlot"] == start + 4
        and not evidence["exits"][0]["missing"]
        and not any(
            evidence["exits"][0][key]
            for key in ("definitions", "formals", "pendingDefinitions")
        )
        and _complete(summary["exits"][0]["incomingLink"])
        and summary["exits"][0]["sampledEntry"] == 31
        and len(summary["transfers"]) == 34
        and all(
            transfer["register"] == index
            and transfer["kind"] == "preserved"
            and _complete(transfer["proof"])
            and not transfer["pending"]
            and not transfer["pendingDefinitions"]
            for index, transfer in enumerate(summary["transfers"])
        ),
        "native empty-body proposal preservation differs",
    )
    return True


def validate_projected_declaration(row: dict) -> None:
    """Bind an actual projected declaration to its validated original summary."""
    summary = row["originalCallSummary"]
    _require(
        _complete(summary["declaration"]), "native projected declaration is incomplete"
    )
    _require(row.get("isVariadic", False) is False, "native variadic declaration")
    _require(
        row.get("returnType") == {"llvmIr": "i32"}
        and row.get("returnStorage")
        == {"type": "register", "value": "v0", "registerNumber": 4}
        and type(row["returnStorage"]["registerNumber"]) is int,
        "native projected i32/V0 return",
    )
    lanes = []
    for parameter in _rows(row.get("parameters", []), 4, "native projected parameter"):
        _require(type(parameter) is dict, "native projected parameter object")
        datatype = _keys(parameter.get("type"), {"llvmIr"}, "native parameter type")
        storage = _keys(
            parameter.get("storage"),
            {"type", "value", "registerNumber"},
            "native parameter storage",
        )
        register = storage["registerNumber"]
        _require(
            datatype["llvmIr"] == "i32"
            and type(register) is int
            and 6 <= register <= 9
            and storage["type"] == "register"
            and storage["value"] == f"a{register - 6}",
            "native projected i32 argument lane",
        )
        lanes.append(register - 2)
    _require(
        lanes == summary["parameterRegisters"],
        "native projected parameter order or inventory differs",
    )


@dataclass(frozen=True)
class BodyProposalFacts:
    member_key: str
    entry_symbol: str
    instructions: tuple[tuple[int, int], ...]
    exits: tuple[tuple[int, int], ...]
    pending_entry_claim: str


def project_validated_body(unit) -> BodyProposalFacts | None:
    from harness.reconstruction.nativeoutput import CONFIG_LIMIT, parse_json

    config = parse_json(unit.configuration, CONFIG_LIMIT)
    rows = [
        row
        for row in config.get("functions", [])
        if row.get("name") == unit.entry_symbol
    ]
    _require(len(rows) == 1, "selected body proposal must be unique")
    row = rows[0]
    if row.get("originalBodyCandidate") != "mips-empty-body/v1":
        return None
    validate_body_candidate(row)
    summary, evidence = row["originalCallSummary"], row["returnDisposition"]["evidence"]
    _require(
        summary["function"] == unit.member.key
        and evidence["symbol"] == unit.entry_symbol
        and evidence["start"] == unit.member.start
        and evidence["end"] == unit.member.end
        and summary["pendingEntry"] == "requires-no-pending-gpr-loads",
        "selected native body member, extent or pending-entry claim differs",
    )
    return BodyProposalFacts(
        unit.member.key,
        unit.entry_symbol,
        tuple((item["pc"], item["word"]) for item in evidence["instructions"]),
        tuple((item["pc"], item["delaySlot"]) for item in evidence["exits"]),
        summary["pendingEntry"],
    )


@dataclass(frozen=True)
class ValidatedContractFacts:
    member_key: str
    entry_symbol: str
    return_type: str
    return_register: int
    parameter_registers: tuple[int, ...]
    instructions: tuple[tuple[int, int], ...]
    exits: tuple[tuple[int, int], ...]
    calls: tuple[tuple[int, int | None, str | None], ...]
    demands: tuple[tuple[int, int, str], ...]
    accesses: tuple[tuple[int, str, int], ...]
    pending_entry_claim: str


def project_validated_contract(unit) -> ValidatedContractFacts | None:
    from harness.reconstruction.nativeoutput import CONFIG_LIMIT, parse_json

    config = parse_json(unit.configuration, CONFIG_LIMIT)
    rows = [
        row
        for row in config.get("functions", [])
        if row.get("name") == unit.entry_symbol
    ]
    _require(len(rows) == 1, "selected projected contract must be unique")
    row = rows[0]
    if validate_body_candidate(row):
        return None
    validate_projected_declaration(row)
    summary, evidence = row["originalCallSummary"], row["returnDisposition"]["evidence"]
    _require(
        summary["function"] == unit.member.key,
        "selected native contract member differs",
    )
    return ValidatedContractFacts(
        unit.member.key,
        unit.entry_symbol,
        "i32",
        2,
        tuple(summary["parameterRegisters"]),
        tuple((item["pc"], item["word"]) for item in evidence["instructions"]),
        tuple((item["pc"], item["delaySlot"]) for item in evidence["exits"]),
        tuple(
            (item["pc"], item["target"], item["contract"]["callee"])
            for item in evidence["calls"]
        ),
        tuple(
            (item["pc"], item["register"], item["role"]) for item in summary["demands"]
        ),
        tuple(
            (item["pc"], item["kind"], item["width"]) for item in summary["accesses"]
        ),
        summary["pendingEntry"],
    )
