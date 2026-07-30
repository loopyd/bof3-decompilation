from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
from unittest.mock import patch

from harness.build import configure


def test_configure_reuses_complete_ninja_tree(tmp_path: Path) -> None:
    cache = tmp_path / "build/cmake/CMakeCache.txt"
    cache.parent.mkdir(parents=True)
    cache.write_text(f"CMAKE_HOME_DIRECTORY:INTERNAL={tmp_path.resolve()}\n")
    (cache.parent / "build.ninja").touch()

    with patch("harness.build.subprocess.run") as run:
        assert configure(tmp_path) == cache.parent

    run.assert_not_called()


def test_configure_recovers_corrupt_cache(tmp_path: Path) -> None:
    """A cache missing its source root is discarded before CMake runs."""
    (tmp_path / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(test NONE)\n"
    )
    build_tree = tmp_path / "build/cmake"
    build_tree.mkdir(parents=True)
    (build_tree / "CMakeCache.txt").write_text("corrupt\n")
    (build_tree / "build.ninja").touch()

    assert configure(tmp_path) == build_tree
    assert (
        f"CMAKE_HOME_DIRECTORY:INTERNAL={tmp_path.resolve()}"
        in (build_tree / "CMakeCache.txt").read_text()
    )


def test_configure_recovers_foreign_root_tree(tmp_path: Path) -> None:
    """CMake can configure after a complete foreign tree is discarded."""
    source_a = tmp_path / "source-a"
    source_b = tmp_path / "source-b"
    for source in (source_a, source_b):
        source.mkdir()
        (source / "CMakeLists.txt").write_text(
            "cmake_minimum_required(VERSION 3.20)\nproject(test NONE)\n"
        )

    # Produce a real configured tree for source A, then place it where source B
    # would normally configure. This reproduces a copied build/ directory.
    subprocess.run(
        ["cmake", "-S", str(source_a), "-B", str(source_a / "build/cmake")],
        check=True,
        text=True,
        capture_output=True,
    )
    shutil.copytree(source_a / "build/cmake", source_b / "build/cmake")

    build_tree = configure(source_b)

    assert build_tree == source_b / "build/cmake"
    assert (build_tree / "CMakeCache.txt").read_text().find(
        f"CMAKE_HOME_DIRECTORY:INTERNAL={source_b.resolve()}"
    ) >= 0


def test_configure_recovers_incomplete_makefile_tree(tmp_path: Path) -> None:
    """A partial tree cannot retain its old generator cache."""
    (tmp_path / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(test NONE)\n"
    )
    build_tree = tmp_path / "build/cmake"
    subprocess.run(
        ["cmake", "-S", str(tmp_path), "-B", str(build_tree), "-G", "Unix Makefiles"],
        check=True,
        text=True,
        capture_output=True,
    )
    (build_tree / "Makefile").unlink()

    assert configure(tmp_path) == build_tree
    assert (build_tree / "build.ninja").is_file() or (build_tree / "Makefile").is_file()


def test_batch_build_passes_multiple_cmake_targets(
    tmp_path: Path,
) -> None:
    """Phase 2.3.2: batch_build passes every target through CMake."""
    import subprocess
    from unittest.mock import patch

    from harness.build import batch_build

    # Prepare a cached build tree so configure() is a no-op
    cache = tmp_path / "build/cmake/CMakeCache.txt"
    cache.parent.mkdir(parents=True)
    cache.touch()
    (cache.parent / "build.ninja").touch()

    with patch(
        "harness.build.subprocess.run",
        return_value=subprocess.CompletedProcess([], 0, "", ""),
    ) as run:
        batch_build(tmp_path, ["lift_aaa", "lift_bbb", "lift_ccc"])

    args = run.call_args[0][0] if run.call_args else []
    assert "cmake" in args
    assert "--build" in args
    assert "--target" in args
    target_idx = args.index("--target")
    assert args[target_idx + 1] == "lift_aaa", (
        f"first target should be lift_aaa, got {args[target_idx + 1]}"
    )
    assert args[target_idx + 2 :] == ["lift_bbb", "lift_ccc"]
    assert "--" not in args


def test_batch_build_raises_on_empty_targets(
    tmp_path: Path,
) -> None:
    from harness.build import batch_build
    import pytest

    with pytest.raises(ValueError, match="at least one target"):
        batch_build(tmp_path, [])
