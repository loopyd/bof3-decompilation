//! Command-transparent reading streams for text search.
//!
//! Search must find what a player reads, and must never match raw command bytes. A row's
//! encoded bytes are decoded into two parallel **readings**, each character remembering the
//! byte it came from:
//!
//! - `spaced` renders every control command, and every byte with no modelled meaning, as a
//!   single separator — so `it'll{nl}be a good crop`, which the game breaks across a line
//!   break, matches the phrase a player reads.
//! - `joined` drops them instead, so a word a command splits with no displayed break still
//!   matches. It can therefore synthesise a join across a command; every hit names the
//!   reading that produced it so that is visible rather than hidden.
//!
//! Neither reading ever contains a token name or an operand byte, so `--grep end` cannot
//! match a `{end}` terminator and `--grep 0x81` cannot match an operand.

use crate::models::{TextClass, command_for, literal_char};

/// One searchable rendering of an encoded row extent, with a map back to raw bytes.
#[derive(Clone, Debug, Default)]
pub struct Reading {
    /// Literal text; control and unmodelled bytes become separators; whitespace collapsed.
    pub text: String,
    /// For each char of `text`, the byte it came from, or `None` for a separator standing
    /// in for command or unmodelled bytes.
    pub marks: Vec<Option<usize>>,
}

impl Reading {
    fn push(&mut self, character: char, offset: Option<usize>) {
        self.text.push(character);
        self.marks.push(offset);
    }

    /// A separator: never doubled, but **not** dropped at either end.
    ///
    /// Dropping a leading or trailing separator is what deleted literal spaces at a window
    /// boundary: the game's space is the literal byte `0xFF`, so trimming a window's reading
    /// silently removed the spaces a neighbouring window's text depended on. A break is part of
    /// the reading, wherever it falls.
    fn separator(&mut self) {
        if self.text.ends_with(' ') {
            return;
        }
        self.push(' ', None);
    }

    /// A literal character. Whitespace collapses exactly like a separator, so a needle
    /// typed with single spaces matches text stored with runs of them.
    fn literal(&mut self, character: char, offset: usize) {
        if character.is_whitespace() {
            self.separator();
            return;
        }
        self.push(character, Some(offset));
    }

    /// The matched text as read.
    fn slice(&self, start: usize, end: usize) -> String {
        self.text.chars().skip(start).take(end - start).collect()
    }
}

/// A located match.
#[derive(Clone, Debug)]
pub struct Hit {
    /// Which reading matched: `spaced` or `joined`.
    pub reading: &'static str,
    /// The matched text as read.
    pub text: String,
    /// Byte range within the encoded row extent, when the match has literal bytes.
    pub byte_range: Option<(usize, usize)>,
}

/// Normalize a needle the same way the readings collapse whitespace, so a needle typed with
/// tabs or repeated spaces still matches.
pub fn normalize(needle: &str) -> String {
    let mut out = String::with_capacity(needle.len());
    for character in needle.chars() {
        if character.is_whitespace() {
            if !out.is_empty() && !out.ends_with(' ') {
                out.push(' ');
            }
        } else {
            out.push(character);
        }
    }
    out.trim_end().to_string()
}

/// Build both readings of one encoded row extent.
pub fn readings(encoded: &[u8]) -> (Reading, Reading) {
    let mut spaced = Reading::default();
    let mut joined = Reading::default();
    let mut index = 0usize;
    while index < encoded.len() {
        let byte = encoded[index];
        if let Some(character) = literal_char(byte) {
            spaced.literal(character, index);
            joined.literal(character, index);
            index += 1;
            continue;
        }
        let Some(class) = command_for(byte) else {
            // No modelled meaning: raw bytes, so transparent in both readings.
            spaced.separator();
            index += 1;
            continue;
        };
        let first = encoded.get(index + 1).copied();
        let width = class.width(first);
        if width > encoded.len() - index - 1 {
            // Operands cut off by the extent. The command's operands are *here*, in the bytes
            // that remain, so admitting any of them as text would make raw command bytes
            // matchable: an extent ending `[0x0F, 0x41]` would expose `A`, which is an operand
            // of `{rumble}`. We cannot tell which trailing byte was the operand and which was
            // text, so the rest of the extent is opaque — fail closed rather than guess. Text
            // before the cut-off command still reads normally, and a phrase may still span the
            // boundary because the command is a separator.
            spaced.separator();
            index = encoded.len();
            continue;
        }
        spaced.separator();
        index += 1 + width;
    }
    (spaced, joined)
}

/// First occurrence of `needle` in `haystack`, as a char range. Used by the single-string
/// matcher that pins plain substring behaviour in the tests; the corpus matcher is
/// [`find_in_reading_pair`].
#[cfg(test)]
fn locate(haystack: &str, needle: &str, ignore_case: bool) -> Option<(usize, usize)> {
    let hay: Vec<char> = haystack.chars().collect();
    let pattern: Vec<char> = needle.chars().collect();
    if pattern.is_empty() || pattern.len() > hay.len() {
        return None;
    }
    let same = |left: char, right: char| {
        if ignore_case {
            left.eq_ignore_ascii_case(&right)
        } else {
            left == right
        }
    };
    (0..=hay.len() - pattern.len())
        .find(|start| {
            pattern
                .iter()
                .enumerate()
                .all(|(offset, want)| same(hay[start + offset], *want))
        })
        .map(|start| (start, start + pattern.len()))
}

/// Whether `next` continues the scanner run that `previous` ended.
///
/// Only a run split by the scanner's size cap continues: the two instances must be `heuristic`
/// and their extents must **abut**. Independent text rows are separate rows and are never joined,
/// so a needle can never span two dialogue rows.
pub fn continues_run(previous: (TextClass, usize, usize), next: (TextClass, usize)) -> bool {
    let (previous_class, previous_offset, previous_length) = previous;
    let (next_class, next_offset) = next;
    previous_class == TextClass::Heuristic
        && next_class == TextClass::Heuristic
        && next_offset == previous_offset + previous_length
}

/// Join readings into one searchable string, with the byte offset of every character.
///
/// The single implementation of the join: [`join_instance_readings`] delegates here, so the text
/// the index is matched against and the text a hit is resolved from cannot diverge. Whitespace
/// collapses across the join and trailing space is trimmed, exactly as a per-instance reading does.
pub fn joined_readings(parts: &[(usize, usize, Reading)]) -> (String, Vec<Option<usize>>) {
    let mut text = String::new();
    let mut marks: Vec<Option<usize>> = Vec::new();
    let mut previous_end: Option<usize> = None;
    for (start, length, reading) in parts {
        if let Some(end) = previous_end {
            if *start > end && !text.ends_with(' ') {
                text.push(' ');
                marks.push(None);
            }
        }
        for (index, character) in reading.text.chars().enumerate() {
            if character == ' ' && text.ends_with(' ') {
                continue;
            }
            // A reading's marks are relative to its own extent, so they are rebased onto this
            // instance's start; a separator has no byte of its own and takes the instance start.
            let byte = reading
                .marks
                .get(index)
                .copied()
                .flatten()
                .map(|offset| *start + offset);
            text.push(character);
            marks.push(byte);
        }
        previous_end = Some(start + length);
    }
    while text.ends_with(' ') {
        text.pop();
        marks.pop();
    }
    (text, marks)
}

/// The byte range covering a match at char range `[start, end)` of a joined reading.
///
/// Separator characters have no byte of their own, so the range is tightened to the literal bytes
/// actually matched rather than reporting an instance start as if it were the found byte.
pub fn joined_byte_range(
    marks: &[Option<usize>],
    start: usize,
    end: usize,
) -> Option<(usize, usize)> {
    let window = marks.get(start..end)?;
    let first = window.iter().flatten().next().copied()?;
    let last = window.iter().flatten().last().copied()?;
    Some((first, last + 1))
}

/// Which characters of a spaced reading came from a command rather than from a literal byte.
///
/// The index stores a spaced reading and, when it differs, a joined one with the commands dropped.
/// Aligning the two recovers which spaces are command-derived, without needing opcode semantics: a
/// space the joined reading does not have at that point was produced by a command.
fn breaks_from_pair(spaced: &str, joined: &str) -> Vec<bool> {
    let left: Vec<char> = spaced.chars().collect();
    let right: Vec<char> = joined.chars().collect();
    let mut breaks = vec![false; left.len()];
    let mut next = 0usize;
    for (index, character) in left.iter().enumerate() {
        if next < right.len() && *character == right[next] {
            next += 1;
        } else if *character == ' ' {
            breaks[index] = true;
        }
    }
    breaks
}

/// Match `needle` against a reading in which a command-derived break may **either** be skipped
/// **or** stand for a single space the needle contains.
///
/// A command's displayed width is not reconstructed by this tool, so a break is neither assumed
/// zero-width nor assumed to be a space, and the matcher accepts either. That is what lets one
/// phrase order answer two different command mixes: `Saved{nl}the` answers both `Savedthe` and
/// `Saved the`, and `heroes{/color}, eh?` answers the phrase a player reads (`heroes, eh?`) without
/// inventing a space before the comma. Literal bytes still have to match exactly, so token names
/// and operand bytes remain unmatchable.
pub fn find_in_reading_pair(
    spaced: &str,
    joined: &str,
    needle: &str,
    ignore_case: bool,
) -> Option<(usize, usize)> {
    let needle = normalize(needle);
    if needle.is_empty() {
        return None;
    }
    let text: Vec<char> = spaced.chars().collect();
    let pattern: Vec<char> = needle.chars().collect();
    let breaks = breaks_from_pair(spaced, joined);
    for start in 0..text.len() {
        let mut index = start;
        let mut position = 0usize;
        let mut choices: Vec<(usize, usize)> = Vec::new();
        loop {
            if position == pattern.len() {
                return Some((start, index));
            }
            if index == text.len() {
                match choices.pop() {
                    Some((next_index, next_position)) => {
                        index = next_index;
                        position = next_position;
                        continue;
                    }
                    None => break,
                }
            }
            if breaks[index] {
                if pattern[position].is_whitespace() {
                    // prefer reading the break as the space the needle expects, remembering that it
                    // could also have been skipped
                    choices.push((index + 1, position));
                    index += 1;
                    position += 1;
                } else {
                    index += 1;
                }
                continue;
            }
            let same = if ignore_case {
                text[index].eq_ignore_ascii_case(&pattern[position])
            } else {
                text[index] == pattern[position]
            };
            if same {
                index += 1;
                position += 1;
                continue;
            }
            match choices.pop() {
                Some((next_index, next_position)) => {
                    index = next_index;
                    position = next_position;
                }
                None => break,
            }
        }
    }
    None
}

/// The joined text alone, for matching the readings stored in the index.
pub fn join_instance_readings(parts: &[(usize, usize, &str)]) -> String {
    let readings: Vec<(usize, usize, Reading)> = parts
        .iter()
        .map(|(start, length, text)| {
            (
                *start,
                *length,
                Reading {
                    text: (*text).to_string(),
                    marks: vec![None; text.chars().count()],
                },
            )
        })
        .collect();
    joined_readings(&readings).0
}

/// Match `needle` across a group of abutting instances of one subfile.
///
/// The group is given as **payload-relative** `(offset, length)` spans. Each span is read with the
/// same per-instance readings and joined by the one join implementation, so a latch query, a banked
/// query and corpus `search` all answer a phrase the same way — including a phrase that straddles a
/// window the scanner had to split. Returns the payload-relative byte range of the literal bytes
/// matched.
pub fn find_in_group(
    payload: &[u8],
    spans: &[(usize, usize)],
    needle: &str,
    ignore_case: bool,
) -> Option<(usize, usize)> {
    let mut spaced_parts: Vec<(usize, usize, Reading)> = Vec::new();
    let mut bare_texts: Vec<(usize, usize, String)> = Vec::new();
    for (offset, length) in spans {
        let end = offset.checked_add(*length)?;
        let bytes = payload.get(*offset..end)?;
        let (spaced, joined) = readings(bytes);
        let bare = if joined.text != spaced.text {
            joined.text
        } else {
            spaced.text.clone()
        };
        spaced_parts.push((*offset, *length, spaced));
        bare_texts.push((*offset, *length, bare));
    }
    let (spaced_text, marks) = joined_readings(&spaced_parts);
    let refs: Vec<(usize, usize, &str)> = bare_texts
        .iter()
        .map(|(offset, length, text)| (*offset, *length, text.as_str()))
        .collect();
    let bare_text = join_instance_readings(&refs);
    let (start, end) = find_in_reading_pair(&spaced_text, &bare_text, needle, ignore_case)?;
    joined_byte_range(&marks, start, end)
}

/// Locate `needle` in an already-normalized single reading string.
///
/// The corpus path uses [`find_in_reading_pair`], which also knows where the command breaks are;
/// this single-string form is what the unit tests use to pin the plain substring behaviour.
#[cfg(test)]
///
/// The corpus index stores each instance's reading but not its per-character byte marks (that
/// would dwarf the artifact), so a match is found here and the true byte offset is then
/// re-derived from the single owning archive with [`find`].
pub fn find_in_reading(reading: &str, needle: &str, ignore_case: bool) -> Option<(usize, usize)> {
    let needle = normalize(needle);
    if needle.is_empty() {
        return None;
    }
    locate(reading, &needle, ignore_case)
}

/// The canonical match for one encoded instance: command-aware, with its true byte range.
///
/// **Every public matching path routes through here** — `query` on banked rows, the raw-text
/// (`heuristic`) query, and corpus `search` — so the modes cannot disagree about what a piece of
/// text says. A command-derived break may be skipped or stand for a single space; literal bytes
/// must match exactly, and the returned byte range covers only the literal bytes matched.
pub fn find_in_instance(encoded: &[u8], needle: &str, ignore_case: bool) -> Option<Hit> {
    let (spaced, bare) = readings(encoded);
    let (start, end) = find_in_reading_pair(&spaced.text, &bare.text, needle, ignore_case)?;
    let window = spaced.marks.get(start..end)?;
    let first = window.iter().flatten().next().copied();
    let last = window.iter().flatten().last().copied();
    Some(Hit {
        reading: "command-aware",
        text: spaced.slice(start, end),
        byte_range: first.zip(last).map(|(first, last)| (first, last + 1)),
    })
}

/// Find a command-transparent match of `needle` in an encoded row extent.
pub fn find(encoded: &[u8], needle: &str, ignore_case: bool) -> Option<Hit> {
    find_in_instance(encoded, needle, ignore_case)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn spaced(encoded: &[u8]) -> String {
        readings(encoded).0.text
    }

    // A command that displays as a break must read as a break, so the phrase a player
    // actually reads is findable. This is the gap-audit case G1.
    #[test]
    fn a_command_reads_as_a_break_so_a_phrase_matches() {
        // The game's apostrophe is byte 0x8E, not ASCII 0x27: ASCII 0x27 has no literal in
        // the verified table, so a `"it'll"` fixture written with it decodes as `it ll`.
        let mut encoded = b"it".to_vec();
        encoded.push(0x8e);
        encoded.extend_from_slice(b"ll");
        encoded.push(0x01); // {nl}
        encoded.extend_from_slice(b"be a good crop");
        assert_eq!(spaced(&encoded), "it'll be a good crop");
        let hit = find(&encoded, "it'll be a good", false).expect("phrase across a command");
        assert_eq!(hit.reading, "command-aware");
        assert_eq!(hit.text, "it'll be a good");
    }

    // A command splitting a word with no displayed break is caught by the joined reading.
    #[test]
    fn a_command_inside_a_word_still_matches() {
        let encoded = [b'w', b'o', b'r', 0x05, 0x01, b'd']; // wor{color(0x01)}d
        let hit = find(&encoded, "word", false).expect("word split by a command");
        assert_eq!(hit.reading, "command-aware");
    }

    // Gap-audit case G2: raw command bytes must never be searchable.
    #[test]
    fn token_names_and_operands_are_not_searchable() {
        let encoded = [0x0c, 0x81, b'h', b'i']; // {pos(0x81)}hi
        assert!(
            find(&encoded, "pos", false).is_none(),
            "token name must not match"
        );
        assert!(
            find(&encoded, "0x81", false).is_none(),
            "operand spelling must not match"
        );
        assert!(
            find(&encoded, "81", false).is_none(),
            "bare operand must not match"
        );
        assert!(
            find(&encoded, "hi", false).is_some(),
            "literal text still matches"
        );
    }

    #[test]
    fn an_operand_holding_a_text_byte_is_not_searchable() {
        let encoded = [0x07, 0x41]; // {item(0x41)}, where 0x41 is 'A'
        assert!(find(&encoded, "A", false).is_none());
    }

    // Not present in the US corpus (0 of 840 `{byte(0xNN)}` tokens stand for a byte that
    // has a modelled command), so this path is covered here instead of on real data.
    #[test]
    fn a_command_cut_off_by_the_extent_is_transparent() {
        let encoded = [b'x', 0x0a]; // {sound} needs an operand and the extent ends
        // a trailing break is data and is retained, not trimmed away
        assert_eq!(spaced(&encoded), "x ");
        assert!(find(&encoded, "sound", false).is_none());
        assert!(find(&encoded, "x", false).is_some());
    }

    // The auditor's case: `{rumble}` takes two operands, so an extent ending `[0x0F, 0x41]` has a
    // surviving operand byte, and that byte must not become searchable text.
    #[test]
    fn a_surviving_operand_of_a_truncated_command_is_not_searchable() {
        let encoded = [b'x', 0x0f, 0x41];
        // a trailing break is data and is retained, not trimmed away
        assert_eq!(spaced(&encoded), "x ");
        assert!(
            find(&encoded, "A", false).is_none(),
            "the surviving operand byte must not match"
        );
        assert!(
            find(&encoded, "x", false).is_some(),
            "earlier text still matches"
        );
    }

    #[test]
    fn every_trailing_byte_of_a_truncated_command_is_opaque() {
        // `{rumble}` needs two operands and the extent ends after the first: `[0x0F, 0x41, 0x42]`
        // has only one operand present, so the command is cut off and both trailing bytes are
        // opaque. Neither may match, alone or together.
        let encoded = [b'y', 0x0f, 0x41, 0x42];
        assert_eq!(spaced(&encoded), "y ");
        for needle in ["A", "B", "AB"] {
            assert!(
                find(&encoded, needle, false).is_none(),
                "{needle} must not match"
            );
        }
        assert!(find(&encoded, "y", false).is_some());
    }

    // The genuine boundary case: a phrase that straddles the edge between two scanner windows.
    // Small windows are driven here so the edge is deterministic.
    //
    // Spaces in this table are byte 0xFF, not ASCII 0x20: ASCII 0x20 has no literal, so a fixture
    // written with it decodes as unmodelled bytes and never reaches the latch thresholds.
    #[test]
    fn windows_abut_without_skipping_or_double_counting_a_byte() {
        let text: Vec<u8> =
            b"alpha\xffbravo\xffcharlie\xffdelta\xffecho\xfffoxtrot\xffgolf".to_vec();
        let windows = crate::lexagraph::scan_with(&text, 4, 32);
        assert!(
            windows.len() >= 2,
            "the cap should have split this: {:?}",
            windows
                .iter()
                .map(|w| (w.offset, w.length))
                .collect::<Vec<_>>()
        );
        for pair in windows.windows(2) {
            assert_eq!(
                pair[1].offset,
                pair[0].offset + pair[0].length,
                "consecutive cap-closed windows must abut: no byte dropped, none counted twice"
            );
        }
    }

    #[test]
    // The loop iterates a byte range and indexes a coverage array: the index is the value under test.
    #[allow(clippy::needless_range_loop)]
    fn no_byte_is_dropped_at_the_window_cap() {
        // A single long textish run, longer than several windows: every byte must appear in some
        // window, which is what the old `start = index + 1` at the cap broke.
        let text: Vec<u8> = (0..5000u32).map(|i| b'a' + (i % 26) as u8).collect();
        let windows = crate::lexagraph::scan_with(&text, 4, 512);
        assert!(
            windows.len() >= 5,
            "expected several windows, got {}",
            windows.len()
        );
        let mut covered = vec![false; text.len()];
        for window in &windows {
            for byte in window.offset..window.offset + window.length {
                covered[byte] = true;
            }
        }
        let uncovered: Vec<usize> = covered
            .iter()
            .enumerate()
            .filter(|(_, seen)| !**seen)
            .map(|(index, _)| index)
            .collect();
        assert!(
            uncovered.is_empty(),
            "every byte must be latched by some window; missing {}",
            uncovered.len()
        );
        for pair in windows.windows(2) {
            assert_eq!(
                pair[1].offset,
                pair[0].offset + pair[0].length,
                "cap-closed windows must abut"
            );
        }
    }

    #[test]
    fn a_phrase_spanning_a_window_edge_is_found_only_once_the_readings_are_joined() {
        let text: Vec<u8> =
            b"alpha\xffbravo\xffcharlie\xffdelta\xffecho\xfffoxtrot\xffgolf".to_vec();
        let windows = crate::lexagraph::scan_with(&text, 4, 32);
        assert!(windows.len() >= 2);
        let edge = windows[0].offset + windows[0].length;
        // the phrase, read the way the matcher reads it, chosen to straddle the edge
        let phrase = crate::command::deserialize(&text[edge - 6..edge + 6]);
        // neither window contains it on its own...
        for window in &windows {
            let span = &text[window.offset..window.offset + window.length];
            assert!(
                find(span, &phrase, false).is_none(),
                "the phrase must genuinely straddle the edge: {phrase:?}"
            );
        }
        // ...but the joined reading does
        let window_texts: Vec<(usize, usize, String)> = windows
            .iter()
            .map(|w| {
                (
                    w.offset,
                    w.length,
                    crate::command::deserialize(&text[w.offset..w.offset + w.length]),
                )
            })
            .collect();
        let refs: Vec<(usize, usize, &str)> = window_texts
            .iter()
            .map(|(start, length, reading)| (*start, *length, reading.as_str()))
            .collect();
        let joined = join_instance_readings(&refs);
        assert!(
            find_in_reading(&joined, &phrase, false).is_some(),
            "the joined reading must contain the boundary-spanning phrase {phrase:?}: {joined:?}"
        );
        // and the match must map back to bytes that genuinely straddle the edge
        let marked: Vec<(usize, usize, Reading)> = windows
            .iter()
            .map(|window| {
                (
                    window.offset,
                    window.length,
                    crate::search::readings(&text[window.offset..window.offset + window.length]).0,
                )
            })
            .collect();
        let (joined, marks) = joined_readings(&marked);
        let (start, end) = find_in_reading(&joined, &phrase, false).expect("joined match");
        let (first, last) = joined_byte_range(&marks, start, end).expect("byte range");
        assert!(
            first < edge && last > edge,
            "the match must span the edge: {first:#x}..{last:#x} against edge {edge:#x}"
        );
    }

    // And the operand rule still holds at an extent boundary: the text before a cut-off command
    // reads, a phrase matches up to the boundary, and nothing from the command is matchable.
    #[test]
    fn a_phrase_reads_up_to_a_cut_off_command_without_exposing_it() {
        let mut encoded = b"hello wor".to_vec();
        encoded.extend_from_slice(&[0x0f, 0x41]);
        assert_eq!(spaced(&encoded), "hello wor ");
        assert!(find(&encoded, "hello wor", false).is_some());
        assert!(find(&encoded, "A", false).is_none());
    }

    #[test]
    fn an_unmodelled_byte_is_transparent() {
        let encoded = [b'a', 0x21, b'b'];
        assert!(
            find(&encoded, "ab", false).is_some(),
            "joined drops the raw byte"
        );
        assert!(
            find(&encoded, "a b", false).is_some(),
            "spaced reads it as a break"
        );
    }

    #[test]
    fn whitespace_collapses_on_both_sides() {
        let mut encoded = b"a ".to_vec();
        encoded.push(0x01);
        encoded.extend_from_slice(b"b");
        assert!(find(&encoded, "a b", false).is_some());
        assert!(
            find(&encoded, "a   b", false).is_some(),
            "needle collapses too"
        );
        assert!(
            find(&encoded, " a b ", false).is_some(),
            "needle is trimmed"
        );
    }

    #[test]
    fn hits_report_the_raw_byte_range() {
        let encoded = [b'a', b'b', 0x01, b'c', b'd'];
        let hit = find(&encoded, "b c", false).expect("match spanning a break");
        assert_eq!(spaced(&encoded), "ab cd");
        assert_eq!(hit.byte_range, Some((1, 4)));
    }

    #[test]
    fn matching_can_ignore_ascii_case() {
        let encoded = b"Spring in McNeil".to_vec();
        assert!(find(&encoded, "SPRING", false).is_none());
        assert!(find(&encoded, "SPRING", true).is_some());
    }

    #[test]
    fn command_bytes_never_appear_in_a_reading() {
        let encoded = [0x05, 0x01, b'h', b'i', 0x00];
        let (spaced, joined) = readings(&encoded);
        for reading in [spaced, joined] {
            // the boundary breaks are retained, so compare the content, not the padding
            assert_eq!(reading.text.trim(), "hi");
            assert!(!reading.text.contains("color"));
            assert!(!reading.text.contains("0x01"));
        }
    }

    // The auditor's regression: grouping must not merge independent rows, and a needle spanning
    // two rows must not match.
    #[test]
    fn only_an_abutting_heuristic_run_continues() {
        use crate::models::TextClass;
        // a run the scanner had to split at its window cap
        assert!(continues_run(
            (TextClass::Heuristic, 100, 8192),
            (TextClass::Heuristic, 8292)
        ));
        // a gap means the next instance begins a new run
        assert!(!continues_run(
            (TextClass::Heuristic, 100, 8192),
            (TextClass::Heuristic, 9000)
        ));
        // two dialogue rows never continue one another, even when their extents abut
        assert!(!continues_run(
            (TextClass::Dialogue, 100, 50),
            (TextClass::Dialogue, 150)
        ));
        // nor does a banked row continue into a heuristic instance
        assert!(!continues_run(
            (TextClass::Dialogue, 100, 50),
            (TextClass::Heuristic, 150)
        ));
    }

    // The auditor's second boundary case: the game's space is the literal byte 0xFF, so a window
    // ending in spaces must keep one, and a following literal must not be glued onto the word.
    // The auditor's mixed-command case, from WORLD00/AREA000.EMI row 50: a zero-width command
    // (`{/color}`) before a comma and a line break (`{nl}`) between two words. Neither global
    // reading answers it, so the matcher must accept either role for a break.
    #[test]
    fn a_phrase_survives_a_mixed_command_boundary() {
        let mut encoded = b"heroes".to_vec();
        encoded.push(0x06); // {/color}: displayed zero-width
        encoded.push(0x3c); // ',' — the game's comma byte, not ASCII 0x2C
        encoded.push(0xff); // the literal space byte
        encoded.extend_from_slice(b"eh");
        encoded.push(0x5c); // '?' — the game's question-mark byte, not ASCII 0x3F
        encoded.push(0xff);
        encoded.extend_from_slice(b"Saved");
        encoded.push(0x01); // {nl}: a displayed break
        encoded.extend_from_slice(b"the");
        encoded.push(0xff);
        encoded.extend_from_slice(b"village");
        let (spaced, bare) = readings(&encoded);
        assert_eq!(spaced.text, "heroes , eh? Saved the village");
        assert_eq!(bare.text, "heroes, eh? Savedthe village");
        // the phrase a player reads is found...
        assert!(
            find_in_reading_pair(
                &spaced.text,
                &bare.text,
                "heroes, eh? Saved the village",
                false
            )
            .is_some(),
            "spaced={:?} bare={:?}",
            spaced.text,
            bare.text
        );
        // ...while the single-reading forms alone cannot express it
        assert!(find_in_reading(&spaced.text, "heroes, eh? Saved the village", false).is_none());
        assert!(find_in_reading(&bare.text, "heroes, eh? Saved the village", false).is_none());
        // and a needle may still rely on a break reading as a space, or read straight through it
        assert!(
            find_in_reading_pair(&spaced.text, &bare.text, "Saved the village", false).is_some()
        );
        assert!(find_in_reading_pair(&spaced.text, &bare.text, "Savedthe", false).is_some());
    }

    // The same case through the canonical per-instance entry point that every public mode uses
    // (`query`, the raw-text query and corpus search), so a mode cannot regress on its own.
    #[test]
    fn the_canonical_instance_matcher_finds_the_mixed_command_phrase() {
        let mut encoded = b"heroes".to_vec();
        encoded.push(0x06); // {/color}
        encoded.push(0x3c); // ','
        encoded.push(0xff); // space
        encoded.extend_from_slice(b"eh");
        encoded.push(0x5c); // '?'
        encoded.push(0xff);
        encoded.extend_from_slice(b"Saved");
        encoded.push(0x01); // {nl}
        encoded.extend_from_slice(b"the");
        encoded.push(0xff);
        encoded.extend_from_slice(b"village");
        let hit = find_in_instance(&encoded, "heroes, eh? Saved the village", false)
            .expect("the phrase a player reads");
        assert_eq!(hit.reading, "command-aware");
        let (first, last) = hit.byte_range.expect("byte range");
        assert_eq!(
            last - first,
            30,
            "the range covers the literal bytes matched"
        );
        assert_eq!(&encoded[first..first + 8], b"heroes\x06<");
        // a break may equally be read as zero-width, so the fused spelling matches too
        assert!(
            find_in_instance(&encoded, "heroes, eh? Savedthe village", false).is_some(),
            "a break read as zero-width is also a valid reading"
        );
        // while a token name is still not text at all
        assert!(find_in_instance(&encoded, "color", false).is_none());
    }

    #[test]
    fn a_literal_space_survives_a_window_boundary() {
        let mut encoded = b"KLK5".to_vec();
        encoded.extend(std::iter::repeat_n(0xff, 64));
        encoded.push(0x92); // the bullet that follows the run of spaces
        let (whole, _) = readings(&encoded);
        assert!(whole.text.starts_with("KLK5 "), "{:?}", whole.text);
        assert!(
            find_in_reading(&whole.text, "KLK5 •", false).is_some(),
            "the literal space must be searchable: {:?}",
            whole.text
        );
        assert!(
            find_in_reading(&whole.text, "KLK5•", false).is_none(),
            "the deleted-space spelling must not match: {:?}",
            whole.text
        );
        // and the same across a window boundary, joining what the scanner would have split
        let parts: Vec<(usize, usize, Reading)> = vec![
            (0, 4, readings(b"KLK5").0),
            (4, encoded.len() - 4, readings(&encoded[4..]).0),
        ];
        let (joined, _) = joined_readings(&parts);
        assert!(
            find_in_reading(&joined, "KLK5 •", false).is_some(),
            "a space at the window boundary must survive the join: {joined:?}"
        );
        assert!(
            find_in_reading(&joined, "KLK5•", false).is_none(),
            "the joined reading must not fuse the word and the bullet: {joined:?}"
        );
    }

    #[test]
    fn a_leading_break_is_not_dropped_either() {
        // a command at the very start of a reading is a break, not nothing
        let (spaced, _) = readings(&[0x01, b'a', b'b']);
        assert!(spaced.text.starts_with(' '), "{:?}", spaced.text);
    }

    #[test]
    fn an_empty_needle_never_matches() {
        assert!(find(b"text", "", false).is_none());
        assert!(find(b"text", "   ", false).is_none());
    }
}
