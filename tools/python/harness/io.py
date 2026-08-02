    gcc_variants_root: Path
    @property
    def gcc_archive_cache_dir(self) -> Path:
        """Digest-verified GCC archive cache root under private assets.

        Derived (not a dataclass field) so callers that construct RepoLayout
        positionally keep working. Canonical GCC and every catalog variant
        share this cache; unrelated toolchain downloads stay in
        ``toolchains/downloads/``.
        """
        return self.private_assets_dir / "toolchains" / "gcc"

        gcc_variants_root=toolchains / "gcc-variants",
