    cache = build_tree / "CMakeCache.txt"
    if cache.is_file():
        complete = (build_tree / "build.ninja").is_file() or (
            build_tree / "Makefile"
        ).is_file()
        for line in cache.read_text().splitlines():
            if line.startswith("CMAKE_HOME_DIRECTORY:"):
                cached_home = line.split("=", 1)[1].strip()
                if complete and cached_home == str(root.resolve()):
                    return build_tree
                break
        # CMake cannot overwrite either a foreign cache or an incomplete cache
        # from another generator, so start this disposable tree afresh.
        shutil.rmtree(build_tree)
    if shutil.which("ninja"):


def batch_build(root: Path, targets: list[str]) -> subprocess.CompletedProcess[str]:
    """Build multiple CMake targets in one CMake build invocation."""
    if not targets:
        raise ValueError("batch_build requires at least one target")
        ["cmake", "--build", str(build_tree), "--target", *targets],
