# Arguments and locals

Use `bin/harness naming list TARGET --kind identifiers --json` for current
function-qualified declarations and use counts. These are lexical leads, not role
proof. Unsupported declarators require direct inspection and an explicit blocker
where coverage cannot be established.

Prove an argument or local's role from its definition, all uses and original
instructions. Check shadowing, macro expansion, labels, member names and declaration
scope before replacing a token. Rename one evidenced identity consistently; preserve
types, declaration order, body structure, ABI and all other tokens.

Follow [direct editing](direct-editing.md) and shared PRE/recovery rules. Check the
complete owning translation unit and every affected consumer. Keep partial bodies
and metadata unchanged for spelling-only work. Report unresolved roles without
inventing names or treating use counts as semantic evidence.
