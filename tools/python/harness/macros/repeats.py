"""Suffix-array intervals for repeated normalized instruction sequences."""

from __future__ import annotations

from collections.abc import Iterator

from harness.common.deadlines import check_deadline


def repeated_intervals(
    words: list[int], minimum: int
) -> Iterator[tuple[int, int, list[int]]]:
    """Yield maximal common-prefix intervals, never compare across unique sentinels."""
    check_deadline()
    length = len(words)
    if not length:
        return
    alphabet = {word: rank for rank, word in enumerate(sorted(set(words)))}
    ranks = [alphabet[word] for word in words]
    suffixes = list(range(length))
    span = 1
    while span < length:
        check_deadline()

        def key(position: int) -> tuple[int, int]:
            return ranks[position], ranks[
                position + span
            ] if position + span < length else -1

        suffixes.sort(key=key)
        updated = [0] * length
        rank = 0
        for index, (previous, current) in enumerate(zip(suffixes, suffixes[1:])):
            if index % 1024 == 0:
                check_deadline()
            rank += key(previous) != key(current)
            updated[current] = rank
        ranks = updated
        if rank == length - 1:
            break
        span *= 2
    inverse = [0] * length
    for rank, position in enumerate(suffixes):
        if rank % 1024 == 0:
            check_deadline()
        inverse[position] = rank
    common = [0] * length
    matched = 0
    for position in range(length):
        if position % 1024 == 0:
            check_deadline()
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
            if matched % 1024 == 0:
                check_deadline()
            matched += 1
        common[rank] = matched
        matched = max(0, matched - 1)
    stack = [(0, 0)]
    for rank in range(1, length + 1):
        if rank % 1024 == 0:
            check_deadline()
        height = common[rank] if rank < length else 0
        start = rank - 1
        while stack[-1][0] > height:
            size, start = stack.pop()
            if size >= minimum and rank - start >= 4:
                check_deadline()
                yield size, max(height, stack[-1][0]), suffixes[start:rank]
        if stack[-1][0] < height:
            stack.append((height, start))
    check_deadline()
