from harness.toolchain.helpers import (
    find_matching_files,
    paths_under,
    require_path_under,
    unique_paths,
)


def test_unique_paths_expands_and_keeps_first_occurrence(tmp_path: Path) -> None:
    path = tmp_path / "archive.zip"

    assert unique_paths([path, path, Path(str(path))]) == [path]


def test_paths_under_filters_and_require_path_under_rejects(tmp_path: Path) -> None:
    inputs = tmp_path / "inputs"
    allowed = inputs / "source.zip"
    outside = tmp_path / "outside.zip"

    assert paths_under([allowed, outside, allowed], inputs) == [allowed]
    assert require_path_under(allowed, inputs, label="archive") == allowed
    with pytest.raises(ValueError, match="repo's inputs"):
        require_path_under(outside, inputs, label="archive")


def test_find_matching_files_returns_sorted_matches(tmp_path: Path) -> None:
    nested = tmp_path / "nested"
    nested.mkdir()
    (tmp_path / "z.zip").touch()
    (nested / "a.zip").touch()
    (nested / "skip.txt").touch()

    matches = find_matching_files(tmp_path, lambda path: path.suffix == ".zip")

    assert matches == [nested / "a.zip", tmp_path / "z.zip"]
