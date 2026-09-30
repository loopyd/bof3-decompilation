# BOF3 test units - canonical map

Every `tools/python/tests/<unit>/test_*.py` belongs to one unit. Resolve listed
files here; unmapped files need explicit inventory repair, not guessed ownership.
Counts describe this inventory, not pytest case totals.

Contents: [analysis](#analysis-9---harnessanalysis),
[application](#application-7---harnesstypesmacros-application--common-transaction),
[build](#build-8---harnessbuild), [commands](#commands-12---harnesscommands),
[decomp](#decomp-2---harnessdecomp), [domain](#domain-10---harnessdomain),
[emi](#emi-2---harnessemi), [macros](#macros-5---harnessmacros),
[naming](#naming-12---harnessnaming), [perf](#perf-3---cross-cutting-performance-budgets),
[skills](#skills-3---skillagent-contracts--workflow-moderation),
[text](#text-1---text-harness-integration),
[toolchain](#toolchain-9---harnesstoolchain--mediaaudio),
[types](#types-2---harnesstypes), [fixtures](#shared-fixtures-keep-at-toolspythontests).

## analysis (9) - harness.analysis

Run: `just check-unit analysis`

- `analysis/test_analyzer.py`
- `analysis/test_carve.py`
- `analysis/test_flag_search.py`
- `analysis/test_frozen_queue_accounting.py`
- `analysis/test_index_command.py`
- `analysis/test_manifest_claims.py`
- `analysis/test_rev_query.py`
- `analysis/test_reverse_index.py`
- `analysis/test_target_manifest_consumers.py`

## application (7) - harness.types/macros application + common transaction

Run: `just check-unit application`

- `application/test_application_history.py`
- `application/test_application_revalidation.py`
- `application/test_application_revalidation_cli.py`
- `application/test_application_review.py`
- `application/test_private_application_transition.py`
- `application/test_shared_application_pre.py`
- `application/test_transaction_files.py`

## build (8) - harness.build

Run: `just check-unit build`

- `build/test_asm_link.py`
- `build/test_bin_cc_pipeline.py`
- `build/test_build.py`
- `build/test_compiler_config.py`
- `build/test_clone.py`
- `build/test_lift.py`
- `build/test_permute.py`
- `build/test_splat.py`

## commands (12) - harness.commands

Run: `just check-unit commands`

- `commands/test_context_output.py`
- `commands/test_doctor.py`
- `commands/test_harness_dry.py`
- `commands/test_managed_process.py`
- `commands/test_patch.py`
- `commands/test_plans.py`
- `commands/test_receipts.py`
- `commands/test_runtime.py`
- `commands/test_scratchpad.py`
- `commands/test_setup.py`
- `commands/test_tool_command.py`
- `commands/test_wrapper_bootstrap.py`

## decomp (2) - harness.decomp

Run: `just check-unit decomp`

- `decomp/test_decomp_coverage.py`
- `decomp/test_decomp_status.py`

## domain (10) - harness.domain

Run: `just check-unit domain`

- `domain/test_barrier_header.py`
- `domain/test_canonical_symbols.py`
- `domain/test_domain_sources.py`
- `domain/test_function_ids.py`
- `domain/test_memory_api_headers.py`
- `domain/test_psyq.py`
- `domain/test_psyq_headers.py`
- `domain/test_repository_metadata_preflight.py`
- `domain/test_sdk_symbols.py`
- `domain/test_target_manifest_lookup.py`

## emi (2) - harness.emi

Run: `just check-unit emi`

- `emi/test_emi_catalog.py`
- `emi/test_emi_operations.py`

## macros (5) - harness.macros

Run: `just check-unit macros`

- `macros/test_macro_accounting.py`
- `macros/test_macro_facts.py`
- `macros/test_macro_index.py`
- `macros/test_macro_opportunities.py`
- `macros/test_macro_transactions.py`

## naming (12) - harness.naming

Run: `just check-unit naming`

- `naming/test_naming_audit_bulk_validate.py`
- `naming/test_naming_audit_check.py`
- `naming/test_naming_audit_cli_conclude.py`
- `naming/test_naming_audit_preflight.py`
- `naming/test_naming_conclusion.py`
- `naming/test_naming_debt.py`
- `naming/test_naming_evidence_facts.py`
- `naming/test_naming_evidence_root.py`
- `naming/test_naming_evidence_run.py`
- `naming/test_naming_source_identifiers.py`
- `naming/test_naming_table_consumer.py`
- `naming/test_naming_terminal_review.py`

## perf (3) - cross-cutting performance budgets

Run: `just check-unit perf`

- `perf/test_p1_perf_budgets.py`
- `perf/test_p2_performance.py`
- `perf/test_toolchain_perf_budgets.py`

## skills (3) - skill/agent contracts + workflow moderation

Run: `just check-unit skills`

- `skills/test_bof3_cleanup_agent.py`
- `skills/test_github_moderation_workflow.py`
- `skills/test_skill_status_manifest_loading.py`

## text (1) - text harness integration

Run: `just check-unit text`

- `text/test_harness_integration.py`

## toolchain (9) - harness.toolchain + media/audio

Run: `just check-unit toolchain`

- `toolchain/test_audio_setup.py`
- `toolchain/test_audio_surface.py`
- `toolchain/test_disc_toolchain.py`
- `toolchain/test_gcc_variants.py`
- `toolchain/test_patches.py`
- `toolchain/test_python_cli_toolchains.py`
- `toolchain/test_sdl.py`
- `toolchain/test_toolchain_helpers.py`
- `toolchain/test_toolchain_registry.py`

## types (2) - harness.types

Run: `just check-unit types`

- `types/test_type_index.py`
- `types/test_type_transactions.py`

## Shared fixtures (keep at tools/python/tests)

`conftest.py`, `naming_synthetic_fixture.py`, `subprocess_fixture.py`,
`transaction_cli_fixture.py`, `transaction_native_fixture.py`, `fixtures/`

Total: 85 test files across 14 units. Inventory from
`tools/python/tests/*/test_*.py`; count files, not collected cases.
