from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


PACKAGE_DIR = Path(__file__).resolve().parent
ROOT = PACKAGE_DIR.parents[1]
SCRIPTS_DIR = ROOT / "scripts"
THIRD_PARTY_DIR = ROOT / "third_party"
THIRD_PARTY_TOOLS_DIR = THIRD_PARTY_DIR / "tools"
THIRD_PARTY_REFERENCES_DIR = THIRD_PARTY_DIR / "references"
DEPS_DIR = ROOT / "deps"
DEPS_DOWNLOAD_DIR = DEPS_DIR / "downloads"
TMP_DIR = ROOT / "tmp"
BOF3_DIR = ROOT / "bof3"
BOF3_SOURCE_DIR = BOF3_DIR / "src"
BOF3_INCLUDE_DIR = BOF3_DIR / "include"
MODULES_SOURCE_DIR = BOF3_SOURCE_DIR / "modules"
TOOL_BUILD_DIR = ROOT / "build" / "third_party"

DEFAULT_MATCH_ROOT = TMP_DIR / "matching"
DEFAULT_GHIDRA_DECOMP_ROOT = TMP_DIR / "ghidra_decomp"
DEFAULT_GHIDRA_DECOMP_PROJECT_NAME = "bof3_decomp"
PROGRESS_PREFIX = "BOF3_PROGRESS "

PROFILE_CAPCOM97_BOF3 = "capcom97-bof3"
SUPPORTED_PSX_PROFILES = (PROFILE_CAPCOM97_BOF3,)
DEFAULT_PSX_PROFILE = PROFILE_CAPCOM97_BOF3

GCC272_PSX_ROOT = DEPS_DIR / "gcc-2.7.2-psx"
GCC272_PSX_GCC = GCC272_PSX_ROOT / "gcc"
GCC272_PSX_GXX = GCC272_PSX_ROOT / "g++"
OLD_GCC_TOOLCHAINS_ROOT = DEPS_DIR / "old_gcc_toolchains"
PSN00B_TOOLCHAIN_ROOT = DEPS_DIR / "psn00b_toolchain"
PSN00B_TOOLCHAIN_BIN = PSN00B_TOOLCHAIN_ROOT / "bin"
PSN00B_SDK_ROOT = DEPS_DIR / "psn00bsdk"
PSYQ_ORIGINAL_ROOT = DEPS_DIR / "psyq-original"
PSYQ_ORIGINAL_40_ROOT = PSYQ_ORIGINAL_ROOT / "4.0"
BOF3_DISK_BINARY = TOOL_BUILD_DIR / "bof3-disk" / "bof3-disk"
EMI_EX_BINARY = ROOT / "build" / "tools" / "emi-ex-v2" / "cli" / "emi-ex"
GHIDRA_SRC_DIR = THIRD_PARTY_TOOLS_DIR / "bof3-ghidra" / "src"
GHIDRA_MAIN_MODULE = "bof3_ghidra"
ASM_DIFFER_SCRIPT = THIRD_PARTY_TOOLS_DIR / "asm-differ" / "diff.py"
DECOMP_PERMUTER_DIR = THIRD_PARTY_TOOLS_DIR / "decomp-permuter"
DECOMP_PERMUTER_SCRIPT = DECOMP_PERMUTER_DIR / "permuter.py"
OBJDIFF_BINARY = TOOL_BUILD_DIR / "objdiff" / "release" / "objdiff-cli"
MIPSMATCH_BINARY = TOOL_BUILD_DIR / "mipsmatch" / "release" / "mipsmatch"
M2C_SCRIPT = THIRD_PARTY_TOOLS_DIR / "m2c" / "m2c.py"
MASPSX_CC_WRAPPER = SCRIPTS_DIR / "rebof3" / "toolchain" / "maspsx-cc"

PSX_PROFILE_ROOTS = {
    PROFILE_CAPCOM97_BOF3: PSYQ_ORIGINAL_40_ROOT,
}
PSX_PROFILE_ASPSX_VERSIONS = {
    PROFILE_CAPCOM97_BOF3: "2.56",
}
PSX_PROFILE_DISPLAY_NAMES = {
    PROFILE_CAPCOM97_BOF3: "Capcom 1997 BOF3 candidate (PsyQ 4.0 + maspsx 2.56)",
}
PSX_PROFILE_SDK_KINDS = {
    PROFILE_CAPCOM97_BOF3: "original",
}
PSX_PROFILE_CFLAGS = (
    "-O2",
    "-G0",
    "-funsigned-char",
    "-msoft-float",
    "-gcoff",
)


@dataclass(frozen=True, slots=True)
class RepoPaths:
    package_dir: Path
    root: Path
    scripts_dir: Path
    third_party_dir: Path
    third_party_tools_dir: Path
    third_party_references_dir: Path
    deps_dir: Path
    deps_download_dir: Path
    tmp_dir: Path
    bof3_dir: Path
    bof3_source_dir: Path
    bof3_include_dir: Path
    modules_source_dir: Path
    tool_build_dir: Path


@dataclass(frozen=True, slots=True)
class ToolchainPaths:
    gcc272_psx_root: Path
    gcc272_psx_gcc: Path
    gcc272_psx_gxx: Path
    old_gcc_toolchains_root: Path
    psn00b_toolchain_root: Path
    psn00b_toolchain_bin: Path
    psn00b_sdk_root: Path
    psyq_original_root: Path
    psyq_original_40_root: Path


@dataclass(frozen=True, slots=True)
class PsxProfile:
    key: str
    root: Path
    aspsx_version: str
    display_name: str
    sdk_kind: str


REPO_PATHS = RepoPaths(
    package_dir=PACKAGE_DIR,
    root=ROOT,
    scripts_dir=SCRIPTS_DIR,
    third_party_dir=THIRD_PARTY_DIR,
    third_party_tools_dir=THIRD_PARTY_TOOLS_DIR,
    third_party_references_dir=THIRD_PARTY_REFERENCES_DIR,
    deps_dir=DEPS_DIR,
    deps_download_dir=DEPS_DOWNLOAD_DIR,
    tmp_dir=TMP_DIR,
    bof3_dir=BOF3_DIR,
    bof3_source_dir=BOF3_SOURCE_DIR,
    bof3_include_dir=BOF3_INCLUDE_DIR,
    modules_source_dir=MODULES_SOURCE_DIR,
    tool_build_dir=TOOL_BUILD_DIR,
)

TOOLCHAIN_PATHS = ToolchainPaths(
    gcc272_psx_root=GCC272_PSX_ROOT,
    gcc272_psx_gcc=GCC272_PSX_GCC,
    gcc272_psx_gxx=GCC272_PSX_GXX,
    old_gcc_toolchains_root=OLD_GCC_TOOLCHAINS_ROOT,
    psn00b_toolchain_root=PSN00B_TOOLCHAIN_ROOT,
    psn00b_toolchain_bin=PSN00B_TOOLCHAIN_BIN,
    psn00b_sdk_root=PSN00B_SDK_ROOT,
    psyq_original_root=PSYQ_ORIGINAL_ROOT,
    psyq_original_40_root=PSYQ_ORIGINAL_40_ROOT,
)

PSX_PROFILES = {
    profile: PsxProfile(
        key=profile,
        root=PSX_PROFILE_ROOTS[profile],
        aspsx_version=PSX_PROFILE_ASPSX_VERSIONS[profile],
        display_name=PSX_PROFILE_DISPLAY_NAMES[profile],
        sdk_kind=PSX_PROFILE_SDK_KINDS[profile],
    )
    for profile in SUPPORTED_PSX_PROFILES
}


def normalize_psx_profile(profile: str | None) -> str:
    normalized = DEFAULT_PSX_PROFILE if profile is None else str(profile).strip()
    if normalized not in SUPPORTED_PSX_PROFILES:
        raise ValueError(
            f"unsupported PSX profile: {normalized} (expected one of {', '.join(SUPPORTED_PSX_PROFILES)})"
        )
    return normalized


def psyq_root_for_profile(profile: str | None) -> Path:
    normalized = normalize_psx_profile(profile)
    return PSX_PROFILE_ROOTS[normalized]


def aspsx_version_for_profile(profile: str | None) -> str:
    normalized = normalize_psx_profile(profile)
    return PSX_PROFILE_ASPSX_VERSIONS[normalized]


def sdk_kind_for_profile(profile: str | None) -> str:
    normalized = normalize_psx_profile(profile)
    return PSX_PROFILE_SDK_KINDS[normalized]
