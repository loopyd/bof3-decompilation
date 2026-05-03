# BOF3 Assembly Mirror

`bof3/asm/` mirrors `bof3/src/` for reviewed original assembly baselines and
fallback assembly. Generated compiler output, extracted bytes, objdump output,
normalized assembly, and diffs stay under `out/asm-diff/<function>/`.

When lifting a function, keep the authored C in `bof3/src/...` and put only the
human-reviewed original assembly baseline in the matching `bof3/asm/...` path
when that baseline is useful for review.
