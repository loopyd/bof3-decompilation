"""Prove complete static effects of one reviewed byte-lookup consumer body.

This owner keeps the byte-lookup question only.  The instruction-transfer
mechanism it runs -- the interned expression graph, delayed MIPS-I loads, the
frame-cell model, static word/edge classification and the bounded acyclic walk
-- lives in :mod:`harness.naming.staticeffects`, which this body extends with
lookup policy:

* a natural loop is proved structurally and replaced by one symbolic iteration
  instead of being unrolled: every continuing path must execute the step, mask
  and comparison, a carried register is either proved as a relation on the
  induction value or havocked, and loop-written exit values are havocked unless
  they cannot depend on the iteration;
* only a body that reads the selected table object with one settled unsigned
  byte index qualifies, and every other selected-object or captured-code write
  is refused outright rather than placed under a caller premise;
* ``$sp`` frame cells are forwarded only for the body's own entry-relative
  saves, and a store through an unproved or differently based pointer clears
  them, so no memory cell is assumed mutually nonaliasing;
* the ``values`` table is derived from the emitted receipt itself, so every
  reference a section makes is present.

Acyclic control flow is enumerated as bounded paths: every reviewed body here
has at most three sentinel guards outside its loops, and the alternative bound
refuses anything wider.  Unsupported encodings, transfers, addresses, loop
shapes and resource limits raise ``ValueError`` with the exact unsupported
obligation, which the byte-lookup capture turns into a named consumer gap.
"""

from __future__ import annotations

import struct

from harness.naming.staticeffects import (
    CALL_CLOBBERED,
    STACK_POINTER,
    WIDTHS,
    ZERO,
    MachineState,
    Outcome,
    StaticBody,
    branch_target,
    cell_register_value,
    hex_address,
    jump_target,
    node_role,
    read_registers,
    register_name,
    render_address,
    save_cell,
    value_cone,
    written_register,
)

SHARED_PREMISES = (
    "Normal mapped memory completion for every observed access.",
    "Instruction bytes, address mappings and the reviewed layout stay unchanged "
    "while this body runs.",
    "Each direct call returns normally under O32: callee-saved registers, SP and "
    "the caller save cells this body writes are preserved.",
    "Wherever a proven address is treated as distinct from other memory, stack, "
    "code and data nonaliasing is a stated premise, not a proved fact.",
)

SCOPE = (
    "Complete static consumer body under the declared premises. Every original "
    "word, control edge, delayed load, memory event and direct call is accounted "
    "for, but no callee effect, called return value, object extent, safe table "
    "index, storage ownership, initializer identity or gameplay meaning is proved."
)

_LIMITS = {"loops": 2}
_UNSIGNED_BYTE = "lbu"
_SUPPORTED = frozenset(
    {
        "nop",
        "sll",
        "addiu",
        "sra",
        "addu",
        "subu",
        "sltu",
        "andi",
        "xori",
        "sltiu",
        "lui",
        "lw",
        "lbu",
        "lh",
        "lhu",
        "sb",
        "sw",
        "beq",
        "bne",
        "j",
        "jal",
        "jr",
    }
)


def _targets_selected(address: dict, selected: int) -> bool:
    """Prove that one decoded address reaches the selected object."""

    if address["kind"] == "absolute":
        return address["address"] == selected
    if address["kind"] == "indexed":
        return address["fixed_base"] == selected
    return False


class _Body(StaticBody):
    """One frozen consumer body and every effect its instructions prove."""

    label = "consumer body"
    subject = "consumer"
    supported = _SUPPORTED
    max_words = 256
    max_edges = 512
    max_calls = 11
    max_nodes = 4096
    max_walk_steps = 4096
    max_pending = 64

    def __init__(self, start: int, words: list[int], selected: int) -> None:
        super().__init__(start, words)
        self.selected = selected
        self.loops: list[dict] = []
        self.loop_ranges: list[tuple[int, int]] = []
        self.loop_headers: dict[int, dict] = {}
        self.loop_exits: dict[int, tuple[MachineState, MachineState]] = {}

    def loop_state(
        self, index: int, state: MachineState, guards: tuple[str, ...]
    ) -> MachineState:
        """Replace one proved natural loop by a single symbolic iteration."""

        return self._summarize_loop(index, state, guards)

    def store_cells(
        self, state: MachineState, index: int, address: dict, opcode: str
    ) -> tuple:
        """Record this body's own ``$sp`` saves and clear them on aliasing stores."""

        fields = self.fields[index]
        if (
            address["kind"] == "entry_relative"
            and address["register"] == STACK_POINTER
            and address["offset"] < 0
        ):
            return save_cell(
                state.cells,
                address["register"],
                address["offset"],
                WIDTHS[opcode],
                state.registers[fields.rt],
                hex_address(self.address(index)),
            )
        if address["kind"] in ("entry_relative", "symbolic"):
            # A store through an unproved or differently based pointer may alias
            # the entry frame, so no save cell from before it survives.
            return ()
        return state.cells

    def _find_loops(self) -> None:
        found: list[tuple[int, int]] = []
        for index, fields in enumerate(self.fields):
            target = branch_target(fields, self.address(index))
            if target is None or target > self.address(index):
                continue
            found.append((self.index_of(target), index))
        if len(found) > _LIMITS["loops"]:
            raise ValueError(
                f"consumer body exceeds {_LIMITS['loops']} nonnested natural loops"
            )
        for position, (header, latch) in enumerate(sorted(found)):
            if position and header <= self.loop_ranges[-1][1]:
                raise ValueError("consumer body has nested or overlapping loops")
            self.loop_ranges.append((header, latch + 1))
            self.loop_headers[header] = {"header": header, "latch": latch}
            self.summarized_loops[header] = latch + 2

    def _definition_before(self, register: int, index: int, low: int) -> int:
        """Return the reaching definition index of one register inside a loop."""

        for candidate in range(index - 1, low - 1, -1):
            fields = self.fields[candidate]
            if fields.opcode == 0x03 and register in CALL_CLOBBERED:
                raise ValueError(
                    f"consumer loop reads {register_name(register)} at "
                    f"{hex_address(self.address(index))} after a call that may clobber it"
                )
            if written_register(fields) == register:
                return candidate
        raise ValueError(
            f"consumer loop reads {register_name(register)} at "
            f"{hex_address(self.address(index))} without a defining write inside the loop"
        )

    def _prove_loop(
        self, header: int, entry_state: MachineState, guards: tuple[str, ...]
    ) -> dict:
        latch = self.loop_headers[header]["latch"]
        slot = latch + 1
        key = hex_address(self.address(header))
        for index, fields in enumerate(self.fields):
            if header <= index <= slot:
                continue
            target = branch_target(fields, self.address(index))
            if target is None and fields.opcode == 0x02:
                target = jump_target(fields, self.address(index))
            if target is not None and header <= self.index_of(target) <= slot:
                raise ValueError(
                    f"consumer loop at {key} has a second control entry at "
                    f"{hex_address(self.address(index))}"
                )
        for index in range(header, latch + 1):
            fields = self.fields[index]
            target = branch_target(fields, self.address(index))
            if target is None:
                continue
            target_index = self.index_of(target)
            if not header <= target_index <= slot:
                raise ValueError(
                    f"consumer loop at {key} has an exit branch at "
                    f"{hex_address(self.address(index))} instead of the latch"
                )
        latch_fields = self.fields[latch]
        if latch_fields.opcode != 0x05 or latch_fields.rt != ZERO:
            raise ValueError(
                f"consumer loop latch at {hex_address(self.address(latch))} is not a bnez "
                "on the proved induction comparison"
            )
        condition = latch_fields.rs
        compare = self._definition_before(condition, latch, header)
        compare_fields = self.fields[compare]
        if compare_fields.opcode != 0x0B and (
            compare_fields.opcode != 0 or compare_fields.funct != 0x2B
        ):
            raise ValueError(
                f"consumer loop latch at {key} does not test an unsigned comparison"
            )
        masked = compare_fields.rs
        mask = self._definition_before(masked, compare, header)
        mask_fields = self.fields[mask]
        if (
            mask_fields.opcode != 0x0C
            or mask_fields.rt != masked
            or mask_fields.immediate not in (0xFF, 0xFFFF)
        ):
            raise ValueError(
                f"consumer loop at {key} does not mask the induction register before "
                "its latch comparison"
            )
        induction = mask_fields.rs
        step = self._definition_before(induction, mask, header)
        step_fields = self.fields[step]
        if (
            step_fields.opcode != 0x09
            or step_fields.rs != induction
            or step_fields.rt != induction
            or step_fields.signed_immediate != 1
        ):
            raise ValueError(
                f"consumer loop at {key} has no proved unit induction step"
            )
        writers = [
            index
            for index in range(header, slot + 1)
            if written_register(self.fields[index]) == induction
        ]
        if writers != [step]:
            raise ValueError(
                f"consumer loop at {key} writes {register_name(induction)} outside its single "
                "unit step"
            )
        initialize = self._definition_before(induction, header, 0)
        initial_fields = self.fields[initialize]
        zero_initialized = (
            initial_fields.opcode == 0
            and initial_fields.funct == 0x21
            and initial_fields.rs == ZERO
            and initial_fields.rt == ZERO
            and initial_fields.rd == induction
        ) or (
            initial_fields.opcode == 0x09
            and initial_fields.rs == ZERO
            and initial_fields.rt == induction
            and initial_fields.signed_immediate == 0
        )
        if not zero_initialized:
            raise ValueError(
                f"consumer loop at {key} does not zero-initialize {register_name(induction)}"
            )
        if entry_state.registers[induction] != self.graph.constant(0):
            raise ValueError(
                f"consumer loop at {key} is entered with a nonzero {register_name(induction)}"
            )
        if compare_fields.opcode == 0x0B:
            bound = self.graph.constant(compare_fields.immediate)
            bound_kind = "immediate"
            iterations = compare_fields.immediate
            if not 1 <= iterations <= 255:
                raise ValueError(
                    f"consumer loop at {key} has an unusable immediate iteration bound"
                )
        else:
            bound = entry_state.registers[compare_fields.rt]
            node = self.graph.get(bound)
            if node["kind"] != "memory_read" or node["opcode"] != _UNSIGNED_BYTE:
                raise ValueError(
                    f"consumer loop at {key} compares against a value that is not a "
                    "cached unsigned byte count"
                )
            if not self._proves_nonzero(guards, bound):
                raise ValueError(
                    f"consumer loop at {key} has no guard proving its cached byte "
                    "count is nonzero at the entry"
                )
            bound_kind = "cached_unsigned_byte"
            iterations = 255
        return {
            "key": key,
            "header": header,
            "latch": latch,
            "slot": slot,
            "condition": condition,
            "compare": compare,
            "masked": masked,
            "mask": mask,
            "mask_immediate": mask_fields.immediate,
            "induction": induction,
            "step": step,
            "initialize": initialize,
            "bound": bound,
            "bound_kind": bound_kind,
            "iterations": iterations,
        }

    def _proves_nonzero(self, guards: tuple[str, ...], value: int) -> bool:
        """Decide whether one entered guard proves a value is not zero."""

        zero = self.graph.constant(0)
        for name in guards:
            guard = self.guards.get(name)
            if guard is None:
                continue
            if guard["left"] == value:
                other = guard["right"]
            elif guard["right"] == value:
                other = guard["left"]
            else:
                continue
            if other != zero:
                continue
            if guard["opcode"] == "beq" and guard["outcome"] == "not_taken":
                return True
            if guard["opcode"] == "bne" and guard["outcome"] == "taken":
                return True
        return False

    def _loop_effects(self, header: int, slot: int) -> tuple[set[int], set[int]]:
        """Return the registers a loop iteration writes and reads, slots included."""

        written: set[int] = set()
        read: set[int] = set()
        for index in range(header, slot + 1):
            fields = self.fields[index]
            read |= set(read_registers(fields))
            destination = written_register(fields)
            if destination is not None:
                written.add(destination)
            if fields.opcode == 0x03:
                written |= set(CALL_CLOBBERED)
        return written, read

    def _checkpoint(self) -> tuple:
        return (
            len(self.events),
            dict(self.guards),
            [dict(record) for record in self.records],
            set(self.covered),
            dict(self.value_sites),
            list(self.returns),
            dict(self.call_sites),
        )

    def _restore(self, checkpoint: tuple) -> None:
        events, guards, records, covered, value_sites, returns, call_sites = checkpoint
        del self.events[events:]
        self.guards = guards
        self.records = records
        self.covered = covered
        self.value_sites = value_sites
        self.returns = returns
        self.call_sites = call_sites

    def _probe_relations(
        self, proof: dict, entry_state: MachineState, carried: list[int]
    ) -> dict[int, int]:
        """Prove what each carried register holds when an iteration starts.

        A register whose value at the backedge is a function of the induction value
        alone carries that relation into the next iteration.  Replacing the stepped
        value by the induction value gives the entry relation, which must also hold
        for the first iteration.
        """

        if not carried:
            return {}
        key = proof["key"]
        induction_node = self.graph.induction(key, proof["induction"], proof["bound"])
        step_node = self.graph.add_immediate(induction_node, 1)
        registers = list(entry_state.registers)
        for register in carried:
            registers[register] = self.graph.carried(key, register)
        registers[proof["induction"]] = induction_node
        probe = MachineState(tuple(registers), None, entry_state.cells)
        checkpoint = self._checkpoint()
        try:
            outcomes = self.walk(
                proof["header"],
                probe,
                (),
                cut={
                    proof["header"]: "iteration_continues",
                    proof["latch"] + 2: "loop_exit",
                },
                loop=key,
                owner=proof["header"],
            )
        except ValueError:
            return {}
        finally:
            self._restore(checkpoint)
        continues = [
            outcome for outcome in outcomes if outcome.label == "iteration_continues"
        ]
        if not continues:
            return {}
        relations: dict[int, int] = {}
        for register in carried:
            values = {outcome.state.registers[register] for outcome in continues}
            if len(values) != 1:
                continue
            value = values.pop()
            if not self._depends_only_on(induction_node, value):
                continue
            relation = self._substitute(value, {step_node: induction_node})
            at_zero = self._substitute(
                relation, {induction_node: self.graph.constant(0)}
            )
            if at_zero != entry_state.registers[register]:
                continue
            relations[register] = relation
        return relations

    def _depends_only_on(self, allowed: int, value: int) -> bool:
        """Decide whether every leaf of a value is one node or a constant."""

        stack = [value]
        seen: set[int] = set()
        while stack:
            node = stack.pop()
            if node in seen:
                continue
            seen.add(node)
            if node == allowed:
                continue
            item = self.graph.get(node)
            kind = item["kind"]
            if kind == "constant":
                continue
            if kind == "expression":
                stack.extend((item["left"], item["right"]))
                continue
            if kind == "indexed_base":
                stack.append(item["index"])
                continue
            return False
        return True

    def _substitute(self, value: int, replacements: dict[int, int]) -> int:
        """Rebuild one expression with nodes replaced, interning the result."""

        memo: dict[int, int] = {}

        def rebuild(node: int) -> int:
            if node in replacements:
                return replacements[node]
            known = memo.get(node)
            if known is not None:
                return known
            item = self.graph.get(node)
            kind = item["kind"]
            if kind == "expression":
                result = self.graph.operation(
                    item["operation"], rebuild(item["left"]), rebuild(item["right"])
                )
            elif kind == "indexed_base":
                result = self.graph.base(item["constant"], rebuild(item["index"]))
            else:
                result = node
            memo[node] = result
            return result

        return rebuild(value)

    def _iteration_independent(self, key: str, value: int) -> bool:
        """Decide whether a value cannot depend on the arbitrary iteration."""

        stack = [value]
        seen: set[int] = set()
        while stack:
            node = stack.pop()
            if node in seen:
                continue
            seen.add(node)
            item = self.graph.get(node)
            kind = item["kind"]
            if kind == "constant":
                continue
            if kind == "unknown":
                return False
            if kind in ("induction", "loop_carried") and item["loop"] == key:
                return False
            if kind in ("call_result", "call_clobber") and item["loop"] == key:
                return False
            if kind == "memory_read":
                if item["loop"] == key:
                    return False
                address = item["address"]
                for field in ("index", "source"):
                    if isinstance(address.get(field), int):
                        stack.append(address[field])
                continue
            if kind == "expression":
                stack.extend((item["left"], item["right"]))
            elif kind == "indexed_base":
                stack.append(item["index"])
        return True

    def _summarize_loop(
        self, header: int, entry_state: MachineState, guards: tuple[str, ...] = ()
    ) -> MachineState:
        cached = self.loop_exits.get(header)
        if cached is not None:
            if cached[0] != entry_state:
                raise ValueError(
                    f"consumer loop at {hex_address(self.address(header))} is entered from "
                    "more than one state"
                )
            return cached[1]
        proof = self._prove_loop(header, entry_state, guards)
        latch, induction, key = proof["latch"], proof["induction"], proof["key"]
        if entry_state.pending is not None:
            raise ValueError(
                f"consumer loop at {key} is entered with an unresolved delayed load"
            )
        written, read = self._loop_effects(header, proof["slot"])
        carried = sorted((written & read) - {induction})
        relations = self._probe_relations(proof, entry_state, carried)
        induction_node = self.graph.induction(key, induction, proof["bound"])
        registers = list(entry_state.registers)
        for register in carried:
            registers[register] = relations.get(
                register, self.graph.carried(key, register)
            )
        registers[induction] = induction_node
        iteration_state = MachineState(tuple(registers), None, entry_state.cells)
        outcomes = self.walk(
            header,
            iteration_state,
            (),
            cut={header: "iteration_continues", latch + 2: "loop_exit"},
            loop=key,
            owner=header,
        )
        exits = [outcome for outcome in outcomes if outcome.label == "loop_exit"]
        continues = [
            outcome for outcome in outcomes if outcome.label == "iteration_continues"
        ]
        if not exits:
            raise ValueError(f"consumer loop at {key} has no proved exit")
        if not continues:
            raise ValueError(f"consumer loop at {key} has no executed backedge")
        step_node = self.graph.add_immediate(induction_node, 1)
        mask_node = self.graph.operation(
            "and", step_node, self.graph.constant(proof["mask_immediate"])
        )
        compare_node = self.graph.operation(
            "less_than_unsigned", mask_node, proof["bound"]
        )
        for outcome in continues:
            sampled = outcome.sampled or outcome.state
            if sampled.registers[induction] != step_node:
                raise ValueError(
                    f"consumer loop at {key} reaches its backedge at "
                    f"{hex_address(self.address(outcome.index if outcome.index < len(self.words) else latch))} "
                    "without executing the unit step"
                )
            if (
                proof["masked"] != proof["condition"]
                and sampled.registers[proof["masked"]] != mask_node
            ):
                raise ValueError(
                    f"consumer loop at {key} reaches its backedge without masking the "
                    "stepped induction"
                )
            if sampled.registers[proof["condition"]] != compare_node:
                raise ValueError(
                    f"consumer loop at {key} reaches its backedge without comparing the "
                    "masked stepped induction"
                )
        cells = {
            (event["address"]["register"], event["address"]["offset"])
            for event in self.events
            if event.get("loop") == key
            and event["kind"] == "write"
            and event["address"]["kind"] == "entry_relative"
        }
        exited = self._merge_loop_state(
            exits, key, induction, proof["bound"], written, cells
        )
        self.loops.append(
            self._loop_record(proof, carried, relations, exits, continues)
        )
        self.loop_exits[header] = (entry_state, exited)
        return exited

    def _merge_loop_state(
        self,
        exits: list[Outcome],
        key: str,
        induction: int,
        bound: int,
        written: set[int],
        cells: set[tuple[int, int]],
    ) -> MachineState:
        """Merge the exits, havocking every value that depends on the iteration."""

        merged = list(exits[0].state.registers)
        for register in range(len(merged)):
            if register == induction or register not in written:
                continue
            value = exits[0].state.registers[register]
            agrees = all(
                outcome.state.registers[register] == value for outcome in exits[1:]
            )
            if agrees and self._iteration_independent(key, value):
                merged[register] = value
            else:
                merged[register] = self.graph.unknown(
                    f"written by an iteration of the loop at {key}"
                )
        for outcome in exits:
            if outcome.state.pending is not None:
                raise ValueError(
                    f"consumer loop at {key} leaves an unresolved delayed load"
                )
        merged[induction] = bound
        common = [
            cell
            for cell in exits[0].state.cells
            if (cell.base, cell.offset) not in cells
            and self._iteration_independent(key, cell.node)
            and all(
                cell_register_value(outcome.state, cell.base, cell.offset, cell.width)
                == (cell.node, cell.site)
                for outcome in exits[1:]
            )
        ]
        return MachineState(tuple(merged), None, tuple(common))

    def _loop_record(
        self,
        proof: dict,
        carried: list[int],
        relations: dict[int, int],
        exits: list,
        continues: list,
    ) -> dict:
        key = proof["key"]
        record = {
            "header": key,
            "latch": hex_address(self.address(proof["latch"])),
            "delay_slot": hex_address(self.address(proof["slot"])),
            "exit": hex_address(self.address(proof["latch"] + 2)),
            "entry": hex_address(self.address(proof["header"] - 1)),
            "words": proof["latch"] - proof["header"] + 2,
            "backedge": {
                "site": hex_address(self.address(proof["latch"])),
                "opcode": "bnez",
                "condition": register_name(proof["condition"]),
                "target": key,
                "delay_slot": hex_address(self.address(proof["slot"])),
            },
            "induction": {
                "register": register_name(proof["induction"]),
                "initialized": hex_address(self.address(proof["initialize"])),
                "entry_value": 0,
                "step": hex_address(self.address(proof["step"])),
                "step_immediate": 1,
                "mask": {
                    "site": hex_address(self.address(proof["mask"])),
                    "immediate": proof["mask_immediate"],
                },
                "compare": {
                    "site": hex_address(self.address(proof["compare"])),
                    "operation": "less_than_unsigned",
                },
                "exit_value": proof["bound"],
                "positions": {"min": 0, "max": proof["iterations"] - 1},
            },
            "bound": {
                "kind": proof["bound_kind"],
                "node": proof["bound"],
                "maximum_iterations": proof["iterations"],
            },
            "carried": [
                {
                    "register": register_name(register),
                    "relation": relations.get(register),
                }
                for register in carried
            ],
            "havoc": [
                register_name(register)
                for register in carried
                if register not in relations
            ],
            "iteration": {
                "exits": len(exits),
                "continues": len(continues),
                "memory_events": sum(
                    1 for event in self.events if event.get("loop") == key
                ),
            },
            "premise": (
                "One arbitrary iteration with an induction value in "
                f"0..{proof['iterations'] - 1} stands for every iteration; values "
                "written by an earlier iteration are not assumed equal."
            ),
        }
        recurrences = self._recurrences(proof)
        if recurrences:
            record["recurrences"] = recurrences
        return record

    def _recurrences(self, proof: dict) -> list[dict]:
        """Report a loop-carried memory cell the proved iteration updates."""

        reads = [
            event
            for event in self.events
            if event.get("loop") == proof["key"] and event["kind"] == "read"
        ]
        writes = [
            event
            for event in self.events
            if event.get("loop") == proof["key"] and event["kind"] == "write"
        ]
        found: list[dict] = []
        for write in writes:
            if write["address"]["kind"] != "absolute":
                continue
            cell = write["address"]["address"]
            for read in reads:
                if (
                    read["address"]["kind"] != "absolute"
                    or read["address"]["address"] != cell
                ):
                    continue
                if read["node"] not in value_cone(self.graph, write["value"]):
                    continue
                stored = self.graph.get(write["value"])
                found.append(
                    {
                        "cell": hex_address(cell),
                        "read_site": read["site"],
                        "write_site": write["site"],
                        "opcode": read["opcode"],
                        "stored_width": write["width"],
                        "stored_operation": stored.get("operation"),
                        "initializer": self._initializer(cell),
                        "recurrence": (
                            "the bytes one iteration stores are the bytes the next "
                            "iteration reads from the same address"
                        ),
                        "premise": (
                            "this body's own store is the only writer of that cell "
                            "and no asynchronous writer changes it"
                        ),
                    }
                )
        return found

    def _initializer(self, address: int) -> dict | None:
        for event in self.events:
            if (
                event["kind"] == "write"
                and event["address"]["kind"] == "absolute"
                and event["address"]["address"] == address
                and event.get("loop") is None
            ):
                node = self.graph.get(event["value"])
                return {
                    "site": event["site"],
                    "kind": node["kind"],
                    "constant": node.get("value"),
                }
        return None

    def check_store(self, index: int, address: dict, opcode: str) -> None:
        if _targets_selected(address, self.selected):
            raise ValueError(
                f"consumer writes the selected object at {hex_address(self.address(index))}"
            )
        if address["kind"] != "absolute":
            return
        width = WIDTHS[opcode]
        absolute = address["address"]
        if absolute % width:
            raise ValueError(
                f"consumer store at {hex_address(self.address(index))} is not {width}-byte "
                "aligned"
            )
        end = self.start + len(self.words) * 4
        if absolute < end and absolute + width > self.start:
            raise ValueError(
                f"consumer store at {hex_address(self.address(index))} overlaps the captured "
                "instruction bytes"
            )

    def run(self) -> dict:
        self.describe()
        self._find_loops()
        self.walk(0, self.entry_state(), ())
        self.finished_covered()
        if not self.returns:
            raise ValueError("consumer body has no proved terminal return")
        return self._report()

    def _report(self) -> dict:
        selected_reads = [
            event
            for event in self.events
            if event["kind"] == "read"
            and _targets_selected(event["address"], self.selected)
        ]
        if not selected_reads:
            raise ValueError(
                "consumer body does not prove a read of the selected object"
            )
        table_reads = [self._table_read(event) for event in selected_reads]
        body = {
            "kind": "complete_static_consumer_body",
            "words": len(self.words),
            "table_reads": table_reads,
            "sums": self._sums(selected_reads),
            "masks": self._masks(),
            "comparisons": self._comparisons(),
            "sentinels": self._sentinels(table_reads),
            "loops": self.loops,
            "calls": [self.call_sites[site] for site in sorted(self.call_sites)],
            "returns": self._returns(),
            "instructions": self.records,
            "memory": [self.render_event(event) for event in self.events],
            "guards": self.guards,
            "coverage": {
                "instructions": len(self.words),
                "executed": len(self.covered),
                "edges": sum(len(record["successors"]) for record in self.records),
                "calls": len(self.call_sites),
                "loops": len(self.loops),
                "returns": len(self.returns),
            },
            "premises": list(SHARED_PREMISES),
            "scope": SCOPE,
        }
        # The value table is derived from the emitted projections themselves, so a
        # reference added to a section can never escape the receipt.
        body["values"] = self.values_table(body)
        return body

    def _table_read(self, event: dict) -> dict:
        if event["address"]["kind"] == "absolute":
            index = {"kind": "constant_element"}
            fixed = event["address"]["address"]
        else:
            node = self.graph.get(event["address"]["index"])
            if node["kind"] != "memory_read" or node["opcode"] != _UNSIGNED_BYTE:
                raise ValueError(
                    f"selected table read at {event['site']} does not use one settled "
                    "unsigned byte index"
                )
            index = {
                "kind": "loaded_byte",
                "node": event["address"]["index"],
                "site": node["site"],
                "opcode": node["opcode"],
                "range": {"min": 0, "max": 255},
                "loop": node["loop"],
            }
            fixed = event["address"]["fixed_base"]
        return {
            "site": event["site"],
            "opcode": event["opcode"],
            "width": event["width"],
            "fixed_base": hex_address(fixed),
            "index": index,
            "guards": event["guards"],
            "loop": event["loop"],
        }

    def _sums(self, selected_reads: list[dict]) -> list[dict]:
        selected = {event["node"] for event in selected_reads}
        sums: list[dict] = []
        for index, record in enumerate(self.records):
            for node in record.get("values") or ():
                self._sum(sums, index, node, selected)
        return sums

    def _sum(self, sums: list[dict], index: int, node: int, selected: set) -> None:
        """Append one recorded add that consumes a selected-object read."""

        item = self.graph.get(node)
        if item["kind"] != "expression" or item["operation"] != "add":
            return
        inside = value_cone(self.graph, node) & selected
        if not inside:
            return
        sums.append(
            {
                "site": hex_address(self.address(index)),
                "left": item["left"],
                "right": item["right"],
                "selected_reads": sorted(
                    self.graph.get(member)["site"] for member in inside
                ),
            }
        )

    def _masks(self) -> list[dict]:
        masks: list[dict] = []
        for index, record in enumerate(self.records):
            for node in record.get("values") or ():
                item = self.graph.get(node)
                if item["kind"] != "expression" or item["operation"] != "and":
                    continue
                right = self.graph.get(item["right"])
                if right["kind"] != "constant":
                    continue
                masks.append(
                    {
                        "site": hex_address(self.address(index)),
                        "immediate": right["value"],
                        "source": item["left"],
                    }
                )
        return masks

    def _comparisons(self) -> list[dict]:
        comparisons: list[dict] = []
        for index, record in enumerate(self.records):
            for node in record.get("values") or ():
                item = self.graph.get(node)
                if item["kind"] != "expression":
                    continue
                if item["operation"] != "less_than_unsigned":
                    continue
                comparisons.append(
                    {
                        "site": hex_address(self.address(index)),
                        "left": node_role(self.graph, item["left"]),
                        "right": node_role(self.graph, item["right"]),
                    }
                )
        return comparisons

    def _role(self, node: int) -> dict:
        item = self.graph.get(node)
        role: dict = {"node": node}
        if item["kind"] == "memory_read":
            role["role"] = {
                "lhu": "unsigned_halfword_read",
                "lh": "signed_halfword_read",
                "lbu": "unsigned_byte_read",
                "lw": "word_read",
            }[item["opcode"]]
            role["site"] = item["site"]
            role["opcode"] = item["opcode"]
            return role
        if item["kind"] == "expression" and item["operation"] == "and":
            right = self.graph.get(item["right"])
            if right["kind"] == "constant":
                cone = value_cone(self.graph, item["left"])
                reads = [
                    self.graph.get(member)["site"]
                    for member in cone
                    if self.graph.get(member)["kind"] == "memory_read"
                ]
                if reads:
                    role["role"] = (
                        "low_byte_of_accumulated_reads"
                        if right["value"] == 0xFF
                        else "masked_accumulation"
                    )
                    role["mask"] = right["value"]
                    role["sources"] = sorted(reads)
                    return role
        role["role"] = item["kind"]
        if item["kind"] == "constant":
            role["constant"] = item["value"]
        elif item["kind"] in ("call_result", "call_clobber"):
            role["site"] = item["site"]
            role["callee"] = item["callee"]
        elif item["kind"] == "unknown":
            role["reason"] = item["reason"]
        return role

    def _inversion(self, node: int) -> dict | None:
        item = self.graph.get(node)
        if item["kind"] != "expression" or item["operation"] != "xor":
            return None
        right = self.graph.get(item["right"])
        if right["kind"] != "constant" or right["value"] != 1:
            return None
        return {
            "operand": item["left"],
            "immediate": 1,
            "effect": "returns the complementary comparison result",
        }

    def _returns(self) -> list[dict]:
        rendered = []
        for entry in sorted(
            self.returns, key=lambda item: (item["site"], item["guards"])
        ):
            rendered.append(
                {
                    **entry,
                    "value_role": node_role(self.graph, entry["value"]),
                    "inversion": self._inversion(entry["value"]),
                }
            )
        return rendered

    def _sentinels(self, table_reads: list[dict]) -> list[dict]:
        """Pair every constant-guarded branch with what each outcome reads or skips."""

        latches = {loop["latch"] for loop in self.loops}
        sentinels = []
        for site in sorted({guard["site"] for guard in self.guards.values()}):
            if site in latches:
                continue
            decisions = {
                guard["outcome"]: key
                for key, guard in self.guards.items()
                if guard["site"] == site
            }
            first = self.guards[next(iter(decisions.values()))]
            constants = [
                self.graph.get(node)["value"]
                for node in (first["left"], first["right"])
                if self.graph.get(node)["kind"] == "constant"
            ]
            if len(constants) != 1:
                continue
            index = (int(site, 16) - self.start) // 4
            target = self.index_of(int(first["target"], 16))
            entry = {
                "site": site,
                "opcode": first["opcode"],
                "sentinel": constants[0],
                "taken": [
                    read["site"]
                    for read in table_reads
                    if "taken" in decisions and decisions["taken"] in read["guards"]
                ],
                "not_taken": [
                    read["site"]
                    for read in table_reads
                    if "not_taken" in decisions
                    and decisions["not_taken"] in read["guards"]
                ],
                "delay_slot": self._slot_effect(first["delay_slot"]),
            }
            skipped = [
                hex_address(self.address(header))
                for header in sorted(self.loop_headers)
                if index < header < target
            ]
            if skipped:
                entry["skips_loops"] = skipped
            sentinels.append(entry)
        return sentinels

    def _slot_effect(self, site: str) -> dict:
        for event in self.events:
            if event["site"] == site:
                return {
                    "site": site,
                    "kind": event["kind"],
                    "opcode": event["opcode"],
                    "address": render_address(event["address"]),
                }
        return {"site": site, "kind": "integer"}


def decode_consumer_body(code: bytes, start: int, selected: int) -> dict:
    """Explain every word of one reviewed consumer body, or refuse it."""

    if not code or len(code) % 4:
        raise ValueError("byte lookup consumer is not a finite aligned body")
    if start % 4:
        raise ValueError("byte lookup consumer does not start on a word boundary")
    words = list(struct.unpack_from(f"<{len(code) // 4}I", code, 0))
    return _Body(start, words, selected).run()
