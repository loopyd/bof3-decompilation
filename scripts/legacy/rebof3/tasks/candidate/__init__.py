from __future__ import annotations

"""Small reusable tasks for the candidate compile/diff pipeline."""

from .common import (
    DEFAULT_CANDIDATE_BUILD_ROOT,
    DEFAULT_CANDIDATE_WORKSPACE_ROOT,
    STUB_BUILD_CACHE_VARS,
    STUB_BUILD_GENERATOR,
    STUB_BUILD_TOOLCHAIN_FILE,
    build_candidate_workspace_payload,
    build_stub_configure_command,
    candidate_include_directive,
    candidate_stub_source_path,
    stable_function_name,
    wrap_candidate_source_text,
)
from .compile_workspace import CompileWorkspaceTask
from .configure_stub_build import ConfigureStubBuildTask
from .resolve_function import ResolveFunctionTask
from .run_decomp_bundle import RunDecompBundleTask
from .run_diff import RunDiffTask
from .run_permuter import RunPermuterTask
from .select_candidate_source import SelectCandidateSourceTask
from .write_candidate_stub import WriteCandidateStubTask
from .write_candidate_workspace import WriteCandidateWorkspaceTask

__all__ = [
    "CompileWorkspaceTask",
    "ConfigureStubBuildTask",
    "DEFAULT_CANDIDATE_BUILD_ROOT",
    "DEFAULT_CANDIDATE_WORKSPACE_ROOT",
    "ResolveFunctionTask",
    "RunDecompBundleTask",
    "RunDiffTask",
    "RunPermuterTask",
    "STUB_BUILD_CACHE_VARS",
    "STUB_BUILD_GENERATOR",
    "STUB_BUILD_TOOLCHAIN_FILE",
    "SelectCandidateSourceTask",
    "WriteCandidateStubTask",
    "WriteCandidateWorkspaceTask",
    "build_candidate_workspace_payload",
    "build_stub_configure_command",
    "candidate_include_directive",
    "candidate_stub_source_path",
    "stable_function_name",
    "wrap_candidate_source_text",
]
