"""Prove complete static effects of one reviewed caller or predicate body.

The instruction-transfer mechanism -- the interned expression graph, delayed
MIPS-I loads, the frame-cell model, static word/edge classification and the
bounded acyclic walk -- lives in :mod:`harness.naming.staticeffects`.  This owner
adds the two cross-target policies that run on top of it:

* :class:`_CallerBody` proves one complete selected-call caller: every word,
  syntactic edge, delayed load, ordered memory event and direct call argument is
  accounted for, the four argument registers are sampled after the ``jal`` delay
  slot and before the callee clobbers, and the selected ``v0`` is bound to the
  ``beq``/``bne`` polarity and continuations that actually consume it.
* :class:`_PredicateBody` proves one complete leaf predicate: every normal
  completion leaves ``0`` or ``1``, derived from the returned expression rather
  than from a positional template.

Both policies are acyclic: a backward transfer, an indirect transfer, a transfer
into a delay slot, an unsupported encoding, an escaped or unexplained word, and
every exhausted resource bound are refused with the exact unsupported
obligation.

Frame policy is the accepted caller premise.  A body's own entry-relative
``$sp`` save is a protected cell; an overwriting ``$sp`` store of the same frame
drops it.  A data-pointer store is not proved disjoint: the receipt names the
premise and the exact cells each such access relies on.  Concrete overlaps with
the captured instruction bytes, or with the selected callee entry, are refused
outright.
"""

from __future__ import annotations

import json
import struct

from harness.naming.effects import _overlaps_instructions
from harness.naming.staticeffects import (
    REGISTER_NAMES,
    STACK_POINTER,
    WIDTHS,
    MachineState,
    StaticBody,
    hex_address,
    node_role,
    register_name,
    render_address,
    save_cell,
    value_cone,
)

PROTECTED_FRAME_PREMISE = (
    "Data-pointer accesses used by this analysis do not overlap the identified "
    "protected caller save cells; normal-return callees preserve those cells and "
    "SP under the stated ABI."
)

SHARED_PREMISES = (
    "Normal mapped memory completion for every observed access.",
    "Instruction bytes, address mappings and the reviewed layout stay unchanged "
    "while this body runs.",
    "Each direct call returns normally under O32: callee-saved registers, SP and "
    "the caller save cells this body writes are preserved.",
    PROTECTED_FRAME_PREMISE,
)

SCOPE_CALLER = (
    "Complete static caller body under the declared premises. Every original word, "
    "control edge, delayed load, memory event and direct call argument is accounted "
    "for, but no callee effect, called return value, object extent, storage "
    "ownership or gameplay meaning is proved."
)

SCOPE_PREDICATE = (
    "Complete static predicate body under the declared premises. Every original "
    "word, control edge, delayed load, memory event and guard is accounted for and "
    "every normal completion returns 0 or 1, but no object extent, owner identity, "
    "gameplay meaning or actual execution is proved."
)

_CALLER_SUPPORTED = frozenset(
    {
        "nop",
        "sll",
        "sra",
        "addiu",
        "addu",
        "subu",
        "andi",
        "ori",
        "nor",
        "sltiu",
        "lui",
        "lw",
        "lh",
        "lhu",
        "lbu",
        "sw",
        "sb",
        "sh",
        "beq",
        "bne",
        "bgez",
        "j",
        "jal",
        "jr",
    }
)
# A predicate is a leaf: the selected predicate context additionally needs the
# logical shift, the unsigned comparison and the constant complement.
_PREDICATE_SUPPORTED = frozenset(
    (_CALLER_SUPPORTED - {"j", "jal"}) | {"srl", "sltu", "xori"}
)
_ZERO_VALUES = frozenset({0, 1})
_REGISTER_INDEX = {name: index for index, name in enumerate(REGISTER_NAMES)}


def _is_prefix(before: list[str], after: list[str]) -> bool:
    """Decide whether one recorded decision sequence runs before another."""

    return after[: len(before)] == before


def _unique(entries: list[dict]) -> list[dict]:
    """Drop repeats that one shared site contributes through several paths."""

    kept: list[dict] = []
    seen: set[str] = set()
    for entry in entries:
        key = json.dumps(entry, sort_keys=True)
        if key in seen:
            continue
        seen.add(key)
        kept.append(entry)
    return kept


class _CompleteBody(StaticBody):
    """One frozen acyclic body and the frame proof its caller premise relies on."""

    max_words = 256
    max_calls = 16
    max_edges = 512
    max_nodes = 4096
    max_walk_steps = 4096
    max_pending = 64
    max_completed = 64
    selected: int | None = None

    def __init__(self, start: int, words: list[int]) -> None:
        super().__init__(start, words)
        self.protected_saves: list[dict] = []
        self.premise_accesses: list[dict] = []
        self.outgoing_slots: list[dict] = []
        self.value_memo: dict[int, frozenset[int] | None] = {}

    # -- policy -----------------------------------------------------------

    def sltiu_immediate(self, fields) -> int:
        return fields.signed_immediate

    def memory_address(self, node: int, offset: int) -> dict:
        """Normalize proved constants and SP offsets before applying alias premises."""
        memo: dict[int, tuple[str, int] | None] = {}

        def resolve(number: int) -> tuple[str, int] | None:
            if number in memo:
                return memo[number]
            item = self.graph.get(number)
            kind = item["kind"]
            result = None
            if kind == "constant":
                result = ("constant", item["value"])
            elif kind in {"entry_register", "entry_offset"}:
                if item["register"] == STACK_POINTER:
                    result = ("sp", item.get("offset", 0))
            elif kind == "indexed_base":
                base = resolve(item["index"])
                if base is not None:
                    result = (base[0], base[1] + item["constant"])
            elif kind == "expression":
                left, right = resolve(item["left"]), resolve(item["right"])
                op = item["operation"]
                if left is not None and right is not None:
                    a, b = left[1], right[1]
                    if left[0] == right[0] == "constant":
                        operations = {
                            "add": lambda: a + b,
                            "sub_unsigned": lambda: a - b,
                            "and": lambda: a & b,
                            "or": lambda: a | b,
                            "xor": lambda: a ^ b,
                            "nor": lambda: ~(a | b),
                            "shift_left": lambda: a << (b & 31),
                            "shift_right_logical": lambda: a >> (b & 31),
                            "shift_right_arithmetic": lambda: (
                                (a - 0x100000000 if a & 0x80000000 else a) >> (b & 31)
                            ),
                            "less_than_unsigned": lambda: int(a < b),
                        }
                        if op in operations:
                            result = ("constant", operations[op]() & 0xFFFFFFFF)
                    elif (
                        op
                        in {
                            "shift_left",
                            "shift_right_logical",
                            "shift_right_arithmetic",
                        }
                        and right[0] == "constant"
                        and b & 31 == 0
                    ):
                        result = left
                    elif op == "add" and {left[0], right[0]} == {"constant", "sp"}:
                        result = ("sp", a + b)
                    elif (
                        op == "sub_unsigned"
                        and left[0] == "sp"
                        and right[0] == "constant"
                    ):
                        result = ("sp", a - b)
            memo[number] = result
            return result

        value = resolve(node)
        if value is not None:
            address = (value[1] + offset) & 0xFFFFFFFF
            if value[0] == "constant":
                return {"kind": "absolute", "address": address}
            signed = address - 0x100000000 if address & 0x80000000 else address
            return {
                "kind": "entry_relative",
                "register": STACK_POINTER,
                "offset": signed,
            }

        def known_address_cone(number: int) -> bool:
            item = self.graph.get(number)
            if item["kind"] == "constant":
                return True
            if item["kind"] in {"entry_register", "entry_offset"}:
                return item["register"] == STACK_POINTER
            if item["kind"] == "indexed_base":
                return known_address_cone(item["index"])
            if item["kind"] == "expression":
                return known_address_cone(item["left"]) and known_address_cone(
                    item["right"]
                )
            return False

        if known_address_cone(node):
            raise self.refuse(
                "cannot normalize a constant or own-SP-derived memory address"
            )
        return super().memory_address(node, offset)

    def forward_edge(self, index: int, target: int) -> None:
        """Refuse the backward transfer a complete acyclic body cannot follow."""

        if target <= self.address(index):
            raise self.refuse_at(
                f"transfer at {hex_address(self.address(index))} continues backward "
                f"to {hex_address(target)}"
            )

    def check_store(self, index: int, address: dict, opcode: str) -> None:
        """Refuse a store that overwrites captured or selected code."""

        width = WIDTHS[opcode]
        if address["kind"] != "absolute":
            return
        absolute = address["address"]
        end = self.start + len(self.words) * 4
        if _overlaps_instructions(absolute, width, self.start, end):
            raise self.refuse_at(
                f"store at {hex_address(self.address(index))} overlaps the captured "
                "instruction bytes"
            )
        if self.selected is not None and _overlaps_instructions(
            absolute, width, self.selected, self.selected + 4
        ):
            raise self.refuse_at(
                f"store at {hex_address(self.address(index))} overwrites the selected "
                "callee entry"
            )

    def store_cells(
        self, state: MachineState, index: int, address: dict, opcode: str
    ) -> tuple:
        """Record ``$sp`` saves and name the premise every other store relies on."""

        site = hex_address(self.address(index))
        width = WIDTHS[opcode]
        if address["kind"] == "entry_relative" and address["register"] == STACK_POINTER:
            if address["offset"] < 0:
                # A caller-owned write into the protected area replaces the cell it
                # covers, so no saved value survives an overlapping own write.
                cells = save_cell(
                    state.cells,
                    STACK_POINTER,
                    address["offset"],
                    width,
                    state.registers[self.fields[index].rt],
                    site,
                )
                self.protected_saves.append(
                    {
                        "site": site,
                        "offset": address["offset"],
                        "width_bytes": width,
                        "node": state.registers[self.fields[index].rt],
                        "replaced": [
                            {
                                "offset": cell.offset,
                                "width_bytes": cell.width,
                                "store_site": cell.site,
                            }
                            for cell in state.cells
                            if cell not in cells
                        ],
                    }
                )
                return cells
            self.outgoing_slots.append(
                {"site": site, "offset": address["offset"], "width_bytes": width}
            )
            return state.cells
        self.premise_accesses.append(
            {
                "site": site,
                "opcode": opcode,
                "address": render_address(address),
                "protected_cells": [
                    {
                        "offset": cell.offset,
                        "width_bytes": cell.width,
                        "store_site": cell.site,
                    }
                    for cell in state.cells
                ],
                "basis": PROTECTED_FRAME_PREMISE,
            }
        )
        return state.cells

    def call_effect(
        self,
        index: int,
        address: int,
        callee: int,
        state: MachineState,
        guards: tuple[str, ...],
        path: int,
    ) -> None:
        """Observe one call after its slot settled and before the callee clobbers."""

    # -- execution --------------------------------------------------------

    def run(self) -> dict:
        self.describe()
        self.walk(0, self.entry_state(), ())
        self.finished_covered()
        if not self.returns:
            raise self.refuse("has no proved terminal return")
        saved = self.protected_saves
        self.protected_saves = _unique(saved)
        self.premise_accesses = _unique(self.premise_accesses)
        self.outgoing_slots = _unique(self.outgoing_slots)
        return self._report()

    # -- shared projections ----------------------------------------------

    def _coverage(self) -> dict:
        return {
            "instructions": len(self.words),
            "executed": len(self.covered),
            "edges": sum(len(record["successors"]) for record in self.records),
            "calls": len(self.call_sites),
            "paths": len(self.returns),
        }

    def _resources(self) -> dict:
        return {
            "words": len(self.words),
            "call_sites": len(self.call_sites),
            "control_edges": sum(len(record["successors"]) for record in self.records),
            "completed_paths": len(self.returns),
            "memory_events": len(self.events),
            "expression_nodes": len(self.graph.nodes),
            "walk_steps": self.walk_steps,
            "limits": {
                "words": self.max_words,
                "call_sites": self.max_calls,
                "control_edges": self.max_edges,
                "completed_alternatives": self.max_completed,
                "expression_nodes": self.max_nodes,
                "walk_steps": self.max_walk_steps,
            },
        }

    def _protected_frame(self) -> dict:
        """Report the save cells the premise covers and who relies on it."""

        return {
            "premise": PROTECTED_FRAME_PREMISE,
            "saves": [
                {
                    "site": event["site"],
                    "base": register_name(STACK_POINTER),
                    "offset": event["offset"],
                    "width_bytes": event["width_bytes"],
                    "node": event["node"],
                }
                for event in self.protected_saves
            ],
            "data_pointer_accesses": self.premise_accesses,
            "outgoing_argument_slots": self.outgoing_slots,
            "stack_pointer_writes": (
                "an overlapping $sp write of the same frame replaces the protected "
                "cell, so the saved value is not forwarded across it"
            ),
            "scope": (
                "the premise covers only the save cells listed above; no other memory "
                "is assumed nonaliasing and no symbolic store is called disjoint"
            ),
        }

    def _returns(self) -> list[dict]:
        return [
            {**entry, "value_role": node_role(self.graph, entry["value"])}
            for entry in sorted(
                self.returns, key=lambda item: (item["site"], item["guards"])
            )
        ]

    def _report(self) -> dict:
        raise NotImplementedError


class _CallerBody(_CompleteBody):
    """One frozen caller body and every effect its instructions prove."""

    label = "caller body"
    subject = "caller"
    supported = _CALLER_SUPPORTED

    def __init__(self, start: int, words: list[int], selected: int) -> None:
        super().__init__(start, words)
        self.selected = selected
        self.observations: list[dict] = []
        self.executions = 0

    def call_effect(
        self,
        index: int,
        address: int,
        callee: int,
        state: MachineState,
        guards: tuple[str, ...],
        path: int,
    ) -> None:
        """Sample the four argument registers after the slot, before clobber."""

        self.executions += 1
        self.observations.append(
            {
                "site": hex_address(address),
                "callee": hex_address(callee),
                "guards": list(guards),
                "execution": self.executions,
                "arguments": {
                    f"a{register - 4}": state.registers[register]
                    for register in range(4, 8)
                },
            }
        )

    # -- call projections -------------------------------------------------

    def _calls_between(
        self,
        after: dict | None,
        until: list[str],
        site: str,
    ) -> list[str]:
        """Return the call sites one path executed after ``after``, before ``until``."""

        floor = 0 if after is None else after["execution"]
        found = [
            observation
            for observation in self.observations
            if observation["site"] != site
            and observation["execution"] > floor
            and _is_prefix(
                [] if after is None else after["guards"], observation["guards"]
            )
            and _is_prefix(observation["guards"], until)
        ]
        found.sort(key=lambda observation: observation["execution"])
        return [observation["site"] for observation in found]

    def _prior_calls(self, observation: dict) -> list[str]:
        """Return the calls one path executed before ``observation``."""

        found = [
            other
            for other in self.observations
            if other["execution"] < observation["execution"]
            and _is_prefix(other["guards"], observation["guards"])
        ]
        found.sort(key=lambda other: other["execution"])
        return [other["site"] for other in found]

    def _calls(self) -> list[dict]:
        rendered = []
        for site in sorted({observation["site"] for observation in self.observations}):
            record = self.call_sites[site]
            paths = [
                {
                    "guards": observation["guards"],
                    "arguments": observation["arguments"],
                    "prior_calls": self._prior_calls(observation),
                }
                for observation in self.observations
                if observation["site"] == site
            ]
            agreement = (
                "identical_across_paths"
                if len(
                    {json.dumps(path["arguments"], sort_keys=True) for path in paths}
                )
                == 1
                else "path_dependent"
            )
            rendered.append(
                {
                    "site": site,
                    "callee": record["callee"],
                    "delay_slot": record["delay_slot"],
                    "return_address": record["return_address"],
                    "architectural_ra": record["architectural_ra"],
                    "outputs": record["outputs"],
                    "argument_agreement": agreement,
                    "paths": paths,
                }
            )
        return rendered

    def _selected_site(self) -> str:
        sites = sorted(
            {
                observation["site"]
                for observation in self.observations
                if observation["callee"] == hex_address(self.selected)
            }
        )
        if len(sites) != 1:
            raise self.refuse(
                "does not prove exactly one direct call of the selected callee "
                f"{hex_address(self.selected)}"
            )
        return sites[0]

    def _result_register(self, guard: dict, registers: dict) -> str | None:
        for name, node in registers.items():
            if guard["left"] == node or guard["right"] == node:
                return name
        return None

    def _result_guard(self, guard: dict, register: str) -> dict:
        """Describe the polarity one branch gives the selected call result."""

        selected_outputs = self.call_sites[self._selected_site()]["outputs"]
        node = selected_outputs[register]
        other = guard["right"] if guard["left"] == node else guard["left"]
        other_item = self.graph.get(other)
        against_zero = other_item["kind"] == "constant" and other_item["value"] == 0
        if against_zero:
            polarity = {
                ("beq", "taken"): "result_is_zero",
                ("beq", "not_taken"): "result_is_not_zero",
                ("bne", "taken"): "result_is_not_zero",
                ("bne", "not_taken"): "result_is_zero",
            }.get((guard["opcode"], guard["outcome"]), "result_relation")
        else:
            polarity = "result_compared_with_operand"
        taken = guard["outcome"] == "taken"
        return {
            "site": guard["site"],
            "opcode": guard["opcode"],
            "outcome": guard["outcome"],
            "register": register,
            "polarity": polarity,
            "compares_with_zero": against_zero,
            "left": guard["left"],
            "right": guard["right"],
            "continuation": "taken_target" if taken else "not_taken_fall_through",
            "continuation_address": (
                guard["target"] if taken else hex_address(int(guard["site"], 16) + 8)
            ),
        }

    def _result(self) -> dict:
        site = self._selected_site()
        record = self.call_sites[site]
        registers = record["outputs"]
        paths: list[dict] = []
        guards: list[dict] = []
        returns: dict[tuple[str, str], list[str]] = {}
        for observation in self.observations:
            if observation["site"] != site:
                continue
            for entry in self.returns:
                if not _is_prefix(observation["guards"], entry["guards"]):
                    continue
                later = entry["guards"][len(observation["guards"]) :]
                consumed = []
                for name in later:
                    guard = self.guards[name]
                    register = self._result_register(guard, registers)
                    if register is None:
                        continue
                    described = self._result_guard(guard, register)
                    consumed.append(described)
                    guards.append(described)
                    returns.setdefault(
                        (described["site"], described["outcome"]), []
                    ).append(entry["site"])
                paths.append(
                    {
                        "arguments": observation["arguments"],
                        "guards": later,
                        "consumed_result_guards": [item["site"] for item in consumed],
                        "later_calls": self._calls_between(
                            observation, entry["guards"], site
                        ),
                        "return_site": entry["site"],
                        "return_value": entry["value"],
                        "return_value_role": node_role(self.graph, entry["value"]),
                        "stack_pointer": entry["stack_pointer"],
                        "return_address": entry["return_address"],
                    }
                )
        reported = []
        for guard in _unique(guards):
            reported.append(
                {
                    **guard,
                    "returns": sorted(set(returns[(guard["site"], guard["outcome"])])),
                }
            )
        return {
            "kind": "uninterpreted_call_registers",
            "register": "v0",
            "registers": registers,
            "guards": reported,
            "paths": paths,
            "scope": (
                "local register flow only; the callee return value is not "
                "interpreted and later calls invalidate any forwarded claim"
            ),
        }

    def _transition(self) -> dict:
        """Describe how every completed path returns to its caller."""

        return {
            "kind": "conditional_return_to_caller",
            "stack_pointer": "restored_entry_sp",
            "return_address": "entry_ra",
            "returns": [
                {
                    "site": entry["site"],
                    "delay_slot": entry["delay_slot"],
                    "guards": entry["guards"],
                }
                for entry in self._returns()
            ],
            "assumptions": [
                "every observed memory operation completes normally",
                "every direct call returns normally and preserves sp, the O32 "
                "callee-saved registers and this body's protected save cells",
            ],
        }

    def _report(self) -> dict:
        site = self._selected_site()
        record = self.call_sites[site]
        body = {
            "kind": "complete_static_caller_body",
            "start": hex_address(self.start),
            "words": len(self.words),
            "selected": {
                "address": hex_address(self.selected),
                "callsite": site,
                "callee": record["callee"],
                "delay_slot": record["delay_slot"],
                "return_address": record["return_address"],
                "argument_registers": ["a0", "a1", "a2", "a3"],
            },
            "calls": self._calls(),
            "result": self._result(),
            "returns": self._returns(),
            "transition": self._transition(),
            "protected_frame": self._protected_frame(),
            "instructions": self.records,
            "memory": [self.render_event(event) for event in self.events],
            "guards": self.guards,
            "coverage": self._coverage(),
            "resources": self._resources(),
            "premises": list(SHARED_PREMISES),
            "scope": SCOPE_CALLER,
        }
        body["values"] = self.values_table(body)
        return body


class _PredicateBody(_CompleteBody):
    """One frozen leaf predicate and the value its completions return."""

    label = "predicate body"
    subject = "predicate"
    supported = _PREDICATE_SUPPORTED

    def _bounded_values(self, node: int) -> frozenset[int] | None:
        """Return every value a node can take when that set is small, else ``None``."""

        if node in self.value_memo:
            return self.value_memo[node]
        item = self.graph.get(node)
        kind = item["kind"]
        values: frozenset[int] | None = None
        if kind == "constant":
            values = frozenset({item["value"]})
        elif kind == "expression":
            left = self._bounded_values(item["left"])
            right = self._bounded_values(item["right"])
            operation = item["operation"]
            if operation in ("less_than_unsigned", "less_than_signed"):
                values = _ZERO_VALUES
            elif (
                operation in ("and", "or", "xor")
                and left is not None
                and right is not None
                and len(left) * len(right) <= 64
            ):
                combine = {
                    "and": lambda a, b: a & b,
                    "or": lambda a, b: a | b,
                    "xor": lambda a, b: a ^ b,
                }[operation]
                values = frozenset(
                    combine(a, b) & 0xFFFFFFFF for a in left for b in right
                )
        self.value_memo[node] = values
        return values

    def _result_roots(self) -> list[int]:
        roots = [entry["value"] for entry in self.returns]
        for guard in self.guards.values():
            roots.extend((guard["left"], guard["right"]))
        return roots

    def _computed_nodes(self) -> list[int]:
        nodes: set[int] = set()
        for root in self._result_roots():
            nodes |= value_cone(self.graph, root)
        return sorted(nodes)

    def _arithmetic(self) -> dict:
        operations = []
        for node in self._computed_nodes():
            item = self.graph.get(node)
            if item["kind"] != "expression":
                continue
            entry = {
                "site": self.value_sites.get(node),
                "operation": item["operation"],
                "left": item["left"],
                "right": item["right"],
            }
            right = self.graph.get(item["right"])
            if right["kind"] == "constant":
                entry["immediate"] = right["value"]
            operations.append(entry)
        return {
            "width_bits": 32,
            "operations": operations,
            "wrap": "unsigned differences wrap modulo 2^32",
            "comparisons": [
                {
                    "site": self.value_sites.get(node),
                    "operation": self.graph.get(node)["operation"],
                }
                for node in self._computed_nodes()
                if self.graph.get(node)["kind"] == "expression"
                and self.graph.get(node)["operation"].startswith("less_than")
            ],
            "scope": (
                "only the operations reached by a returned value or a guard; no "
                "wider arithmetic, object extent or gameplay meaning is described"
            ),
        }

    def _inputs(self, body: dict) -> list[dict]:
        """Report every entry register whose entry value or address the body uses."""

        nodes: dict[str, int | None] = {}
        uses: dict[str, set[str]] = {}
        for node, item in body["values"].items():
            if item["kind"] == "entry_register":
                nodes[item["register"]] = int(node)
                uses.setdefault(item["register"], set()).add("operand")
        for event in body["memory"]:
            address = event["address"]
            if address["kind"] == "entry_relative":
                nodes.setdefault(address["register"], None)
                uses.setdefault(address["register"], set()).add("address_base")
        first_read: dict[str, str] = {}
        for record in self.records:
            for name in record["reads"]:
                first_read.setdefault(name, record["address"])
        return [
            {
                "register": name,
                "node": nodes[name],
                "used_as": sorted(entry),
                "first_read_site": first_read.get(name),
            }
            for name, entry in sorted(uses.items())
        ]

    def _result(self) -> dict:
        paths = []
        for entry in self._returns():
            values = self._bounded_values(entry["value"])
            if values is None or not values <= _ZERO_VALUES:
                raise self.refuse(
                    f"returns a value that is not proved to be 0 or 1 at "
                    f"{entry['site']}"
                )
            paths.append(
                {
                    "guards": entry["guards"],
                    "value": entry["value"],
                    "role": entry["value_role"],
                    "values": sorted(values),
                }
            )
        return {
            "register": "v0",
            "range": {"min": 0, "max": 1},
            "paths": paths,
            "basis": (
                "each normal completion returns a constant 0 or 1, an unsigned "
                "comparison, or the constant complement of one"
            ),
            "scope": (
                "the returned value only; which input makes it 1 is described by "
                "the emitted guards and comparisons, not by gameplay meaning"
            ),
        }

    def _report(self) -> dict:
        body = {
            "kind": "complete_static_predicate_body",
            "start": hex_address(self.start),
            "words": len(self.words),
            "result": self._result(),
            "arithmetic": self._arithmetic(),
            "returns": self._returns(),
            "protected_frame": self._protected_frame(),
            "instructions": self.records,
            "memory": [self.render_event(event) for event in self.events],
            "guards": self.guards,
            "coverage": self._coverage(),
            "resources": self._resources(),
            "premises": list(SHARED_PREMISES),
            "scope": SCOPE_PREDICATE,
        }
        body["values"] = self.values_table(body)
        body["inputs"] = self._inputs(body)
        return body


def decode_caller_body(code: bytes, start: int, selected: int) -> dict:
    """Explain every word of one reviewed selected-call caller, or refuse it."""

    if not code or len(code) % 4:
        raise ValueError("cross-target caller is not a finite aligned body")
    if start % 4:
        raise ValueError("cross-target caller does not start on a word boundary")
    if selected % 4:
        raise ValueError("cross-target selected callee is not word aligned")
    words = list(struct.unpack_from(f"<{len(code) // 4}I", code, 0))
    return _CallerBody(start, words, selected).run()


def decode_predicate_body(code: bytes, start: int) -> dict:
    """Explain every word of one reviewed leaf predicate, or refuse it."""

    if not code or len(code) % 4:
        raise ValueError("cross-target predicate is not a finite aligned body")
    if start % 4:
        raise ValueError("cross-target predicate does not start on a word boundary")
    words = list(struct.unpack_from(f"<{len(code) // 4}I", code, 0))
    return _PredicateBody(start, words).run()
