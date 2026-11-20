root := justfile_directory()
python_env := env_var_or_default("VIRTUAL_ENV", root + "/.venv")
python := python_env + "/bin/python"
pythonpath := root + "/tools/python"

default: help

help:
    @just --list

[private]
venv:
    @if [ -n "${VIRTUAL_ENV:-}" ]; then test -x "{{ python }}"; \
    elif [ ! -x "{{ python }}" ]; then UV_CACHE_DIR="{{ root }}/.uv-cache" uv sync --extra dev --frozen; fi

# Initialize retained dependencies, user-authorized media, and local toolchains.
setup *args: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.setup {{ args }}

doctor: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.doctor

# Restore reviewed executable payloads and EMI images without running setup.
binaries: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.binaries

build:
    @{{ root }}/bin/harness build all
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.compile_commands

# Run shared gates per lift or batch. Harness units use `check-all` or `check-unit`.
check: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m ruff check tools/python
    # Build the text CLI and refresh its registry and index before documentation and text tests.
    @cargo build --locked --release --manifest-path tools/rust/bof3-text/Cargo.toml --target-dir build/tools/rust/bof3-text
    @{{ root }}/bin/harness text windows
    @{{ root }}/bin/harness text build-index
    @{{ root }}/bin/harness source docs
    @just check-textures
    @PSX_BOF3_TEXTURES={{ root }}/build/tools/rust/bof3-textures/release/bof3-textures cargo test --locked --release --manifest-path tools/rust/bof3-text/Cargo.toml --target-dir build/tools/rust/bof3-text
    @{{ root }}/bin/harness text build-index --check
    @{{ root }}/bin/harness text verify
    @{{ root }}/bin/harness text search --verify-entries
    # Check public hit ranges against original corpus bytes.
    @PYTHONPATH={{ pythonpath }} {{ python }} -m pytest -q -p no:cacheprovider tools/python/tests/text
    @{{ root }}/bin/harness source symbols check
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.validate_sources

# Check the texture component with its installed pinned toolchain.
check-textures: venv
    @rustup run 1.98.1 cargo fmt --manifest-path tools/rust/bof3-textures/Cargo.toml -- --check
    @rustup run 1.98.1 cargo clippy --offline --locked --release --all-targets --manifest-path tools/rust/bof3-textures/Cargo.toml --target-dir build/tools/rust/bof3-textures -- -D warnings
    @{{ root }}/bin/harness textures categories --json

# Full gate: every pytest unit plus the shared scoped gate.
check-all: check
    @PYTHONDONTWRITEBYTECODE=1 PYTHONPATH={{ pythonpath }} {{ python }} -m pytest -q -p no:cacheprovider tools/python/tests

# Unit-scoped gate: run one module's tests plus the shared gates.
# Usage: just check-unit naming   (or any path/pattern under tools/python/tests)
check-unit unit:
    @PYTHONDONTWRITEBYTECODE=1 PYTHONPATH={{ pythonpath }} {{ python }} -m pytest -q -p no:cacheprovider tools/python/tests/{{ unit }}
    @just check

format: venv
    @PYTHONPATH={{ pythonpath }} {{ python }} -m ruff format tools/python
    @find src include -type f \( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 -r clang-format -i

index: venv
    @{{ root }}/bin/harness analysis index

clean:
    @{{ root }}/bin/harness build clean
