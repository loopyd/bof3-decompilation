# Mission protocol — bof3-reverse executor

You lift ONE function `TARGET@0xADDRESS` to an exact byte-match. Load `$bof3-re`
and follow `AGENTS.md`. Stay strictly within the mission's authority scope.

## Procedure

1. Read the mission brief (`bin/rev-query mission` JSON). Note the SDK callees,
   callers/callees, duplicate group, and risk flags.
2. Validate the load address: `runtime_address − load_address == payload_offset`
   (`t_addr` from the PS-X header at `0x18`). A green diff does not validate a
   wrong load address.
3. Regenerate evidence: `bin/splat TARGET`, `bin/m2ctx TARGET@0xADDRESS`,
   `bin/m2c TARGET@0xADDRESS -o out/candidate.c`.
4. Name SDK calls from the brief's `sdk_callees`/`sdk_unresolved` (official PsyQ
   names + header declarations); never lift SDK bodies. Recover real signatures
   from callees/callers — the m2c seed has only stub signatures.
5. Write `src/<target>/func_<ADDR>.c` (readable C89). Recover structs from
   consumers: collect accessed offsets → name `unk_XX` → pin with
   `ASSERT_OFFSET`/`ASSERT_SIZE` → promote to evidence-backed names.
6. Iterate `bin/asm-diff TARGET@0xADDRESS --detail normal`; converge the current
   side onto the original (start at `first=`). Follow the playbook lever order;
   document every artificial aid with a `MATCHING_AID` comment.
7. Accept only `bin/byte-match TARGET@0xADDRESS` exit 0.
8. If you cannot reach an exact match within reasonable effort, return
   `status: "escalated"` with notes — never force a match with banned assembly.

## Bans

- No handwritten `__asm__` except `barrier()`/`CLOBBER_*`/`WEAK_SYMBOL_AT`.
- No `register X asm("$N")` pins or `extern X asm("NAME")` renames.
- No `INCLUDE_ASM` unless the user explicitly approved it for this function.
- Do not commit, push, reset, clean, rm, or run setup.

## Return

JSON: `{"function", "status": "exact"|"partial"|"escalated", "match_percent",
"files_changed": [...], "matching_aids": [...], "notes"}`.
