"""Suffix-array intervals for repeated normalized instruction sequences."""

from __future__ import annotations

from collections.abc import Iterator


def repeated_intervals(
    words: list[int], minimum: int
) -> Iterator[tuple[int, int, list[int]]]:
    """Yield maximal common-prefix intervals, never compare across unique sentinels."""
    length = len(words)
    if not length:
        return
    alphabet = {word: rank for rank, word in enumerate(sorted(set(words)))}
    ranks = [alphabet[word] for word in words]
    suffixes = list(range(length))
    span = 1
    while span < length:

        def key(position: int) -> tuple[int, int]:
            return ranks[position], ranks[
                position + span
            ] if position + span < length else -1

        suffixes.sort(key=key)
        updated = [0] * length
        rank = 0
        for previous, current in zip(suffixes, suffixes[1:]):
            rank += key(previous) != key(current)
            updated[current] = rank
        ranks = updated
        if rank == length - 1:
            break
        span *= 2
    inverse = [0] * length
    for rank, position in enumerate(suffixes):
        inverse[position] = rank
    common = [0] * length
    matched = 0
    for position in range(length):
        rank = inverse[position]
        if rank == 0:
            matched = 0
            continue
        previous = suffixes[rank - 1]
        while (
            position + matched < length
            and previous + matched < length
            and words[position + matched] == words[previous + matched]
        ):
            matched += 1
        common[rank] = matched
        matched = max(0, matched - 1)
    stack = [(0, 0)]
    for rank in range(1, length + 1):
        height = common[rank] if rank < length else 0
        start = rank - 1
        while stack[-1][0] > height:
            size, start = stack.pop()
            if size >= minimum and rank - start >= 4:
                yield size, max(height, stack[-1][0]), suffixes[start:rank]
        if stack[-1][0] < height:
            stack.append((height, start))
