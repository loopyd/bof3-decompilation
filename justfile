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

# Scoped repository gate: shared gates only, no pytest units. Fast; used per
# lift/batch. Harness development tests run with `just check-all` or one unit
# with `just check-unit <unit>`.
check: venv
    # Ordered so that a gate which is already failing cannot mask the ones after it: the
    # pre-existing naming debt in `source symbols check` used to abort this recipe before
    # `validate_sources` was ever reached, and would have hidden the text gates too.
    @PYTHONPATH={{ pythonpath }} {{ python }} -m ruff check tools/python
    @{{ root }}/bin/harness source docs
    # Text tool gates, measured over the 880-archive US corpus and kept identical to the performance
    # table in out/text-format/validation-summary.md:
    #   document drift 0.1 s; ruff 0.05 s; Rust unit tests 5.5 s warm; the crate test suite
    #   ≈205–215 s warm (its corpus-backed integration targets dominate)
    #   the locked release rebuild ~15 s cold and instantaneous when fresh
    #   the classified-window registry 1.66 s (a 14,690,707-byte (14.01 MiB) artifact)
    #   the index build 10.8 s warm and 1.0 s when fresh (an 11,264,459-byte (10.74 MiB) artifact)
    #   the corpus round trip 0.48 s; index integrity 2.94 s
    #   the segment map 7.58 s (a 1,495,430-byte (1.43 MiB) artifact)
    #   the public-output and documentation tests ≈2.7 s (32 tests); the whole recipe ≈235–241 s warm
    # The operator accepted a slower gate in exchange for these running; none may be dropped or
    # weakened to regain speed.
    @cargo test --locked --release --manifest-path tools/rust/bof3-text/Cargo.toml --target-dir build/tools/rust/bof3-text
    # The crate is binary-only: `cargo test` builds only its test executable, so the production
    # executable every CLI gate below uses must be rebuilt explicitly or it can be stale.
    @cargo build --locked --release --manifest-path tools/rust/bof3-text/Cargo.toml --target-dir build/tools/rust/bof3-text
    # The classified-window registry every latching gate below honours; built first so the gates
    # exercise the real one (~1.7 s).
    @{{ root }}/bin/harness text windows
    @{{ root }}/bin/harness text build-index
    @{{ root }}/bin/harness text build-index --check
    @{{ root }}/bin/harness text verify
    @{{ root }}/bin/harness text search --verify-entries
    # Public-output regression tests for the text tool: they assert hit offsets and lengths against
    # original corpus bytes, which the internal gates cannot show, and they fail with an actionable
    # message when the binary or the corpus is missing rather than skipping silently.
    @PYTHONPATH={{ pythonpath }} {{ python }} -m pytest -q -p no:cacheprovider tools/python/tests/text
    @{{ root }}/bin/harness source symbols check
    @PYTHONPATH={{ pythonpath }} {{ python }} -m harness.commands.validate_sources

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
