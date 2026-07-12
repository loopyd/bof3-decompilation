# BOF3 Assembly Mirror

`asm/` mirrors `src/` for reviewed original assembly baselines and
fallback assembly. Generated compiler output, extracted bytes, objdump output,
normalized assembly, and diffs stay under `out/matching/<function>/`.

When lifting a function, keep the authored C in `src/...` and put only the
human-reviewed original assembly baseline in the matching `asm/...` path
when that baseline is useful for review.
