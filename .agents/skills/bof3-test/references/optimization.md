# Test optimization protocol

1. Measure selected unit with `pytest -q tools/python/tests/<unit> --durations=20`.
   Identify slow fixtures/tests with numbers before editing.
2. Prefer shared/session fixtures, then memoized test-only build/setup helpers,
   then proven duplicate coverage removal. Preserve every assertion.
3. Delete test only with written duplication proof and explicit user approval.
   Historical `test_distinct_target_shared_sequence_cli` deletion retained proof at
   deletion site; it grants no new deletion permission.
4. Record before/after wall-clock. Run affected unit plus `just check`.
5. Run `just check-all` only when changed shared fixture/cross-unit infrastructure
   gained consumers beyond units already checked. Report actual gates and failures.

Review authorizes test infrastructure only, never production behavior/output changes,
dependencies, assertion weakening or unrequested regression tests.
