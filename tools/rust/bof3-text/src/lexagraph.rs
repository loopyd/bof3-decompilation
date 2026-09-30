//! Heuristic locator for candidate raw-text runs: **leads, not verified text**.
//!
//! The banked classes are located structurally (a TOC load argument). Raw text has
//! no such marker, so this scanner latches contiguous spans that decode through the
//! character and command tables unusually well, then scores what decoded text
//! actually looks like: which **scripts** are present and, for scripts the game is
//! evidenced to carry, which **language** the run resembles.
//!
//! Deterministic and bounded: fixed window bounds, no backtracking, unknown bytes
//! recorded rather than rewritten. Nothing here writes to an input.
//!
//! ## Language model, and its honest limit
//!
//! Scoring runs on the *token-stripped* literal text. Per-character script comes from
//! the Unicode `Script` property (`unicode-script`); the language guess for a run
//! comes from `whatlang` and is reported with its own confidence and reliability flag,
//! because trigram models are weak on short samples.
//!
//! Only **Latin** is evidenced in this game's tables today (US ASCII plus the EU
//! accented-Latin range around `0x97`-`0xAA`). Han/Kana are therefore reported as
//! *detected but unevidenced*: no kanji or two-byte table is pinned yet, and until it
//! is, Japanese text does not even decode — it arrives as `{byte(0xNN)}` tokens. The
//! letter test below is script-aware so that a future pinned table starts working
//! without further changes here.

use serde::{Deserialize, Serialize};
use unicode_script::{Script, UnicodeScript};

use crate::command;
use crate::models::{
    Entry, Error, HEADER_SIZE, MAGIC, Result, SECTOR, TOC_ENTRY_SIZE, TextClass, command_for,
    literal_char,
};

// Every threshold lives in `crate::filters`; these are re-exports so call sites keep one spelling.
use crate::filters::FLOORS;
pub use crate::filters::{GAP_TOLERANCE, MAX_RUN, MIN_RUN, PREVIEW};
use crate::models::{PostFilter, PostFilterInput};

/// How well a script is evidenced for this game.
#[derive(Clone, Copy, Debug, Eq, PartialEq, Serialize, Deserialize)]
pub enum ScriptSupport {
    /// Evidenced in the pinned tables: US ASCII and the EU accented-Latin range.
    Evidenced,
    /// Detected in the data but not evidenced for this game: a lead only. Han and Kana
    /// live here until a kanji/two-byte table is pinned from the Japanese build.
    Unevidenced,
}

/// One script's share of a latch's literal text.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct ScriptShare {
    /// Unicode script name, e.g. `Latin`, `Han`, `Hiragana`, `Katakana`.
    pub script: String,
    /// Characters of this script in the literal text.
    pub characters: usize,
    /// Share of the latch's script letters.
    pub share: f64,
    /// Whether this game's tables are evidenced to carry the script.
    pub support: ScriptSupport,
}

/// The language signal for a latch, from `whatlang` over the token-stripped text.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct LanguageGuess {
    /// ISO 639-3 code, e.g. `eng`, `deu`, `fra`.
    pub language: String,
    /// `whatlang`'s own confidence in [0, 1].
    pub confidence: f64,
    /// `false` when `whatlang` itself flags the sample as too short or ambiguous.
    /// Unreliable guesses are reported, never acted on.
    pub reliable: bool,
}

/// One latched candidate raw-text run.
#[derive(Clone, Debug, Serialize)]
pub struct Latch {
    /// Offset of the run within its subfile payload.
    pub offset: usize,
    /// Run length in bytes.
    pub length: usize,
    /// Always [`TextClass::Heuristic`]: a lead, not verified text.
    pub class: TextClass,
    /// Bytes that decoded through the tables.
    pub printable: usize,
    /// Bytes with no modelled meaning.
    pub unknown: usize,
    /// `printable / length`.
    pub ratio: f64,
    /// Script letters in the decoded literal text.
    pub alpha: usize,
    /// Distinct byte values in the run.
    pub distinct: usize,
    /// Scripts present in the literal text, most frequent first.
    pub scripts: Vec<ScriptShare>,
    /// Language guess, when `whatlang` produced one.
    pub language: Option<LanguageGuess>,
    /// The run's **literal** text — command tokens stripped — truncated to [`PREVIEW`] characters.
    /// A result is shown as the words it carries rather than as its markup.
    pub preview: String,
    /// The run's longest literal word, lowercased. Decided where the whole literal text exists, so
    /// the vocabulary rule can check it where results are reported.
    #[serde(default)]
    pub word: String,
    /// Non-whitespace characters in the run's literal text — the denominator of the readability
    /// share, recorded so a reader can check the rule's own measurement on any result.
    #[serde(default)]
    pub literal_characters: usize,
    /// Whether the run's literal text passed the **shape** checks (mostly letters, one word of three
    /// letters with two distinct letters), before corroboration.
    #[serde(default)]
    pub shaped: bool,
    /// Whether the run's longest word appears in the game's own vocabulary. This is recorded for
    /// **validation**: a corpus run whose word the game's text attests is a corroborated lead, and one
    /// whose word is not is a lead still waiting for evidence. It is not a filter, because a byte-noise
    /// run can carry a word by coincidence and a real string can carry a word the game never uses.
    #[serde(default)]
    pub attested: bool,
    /// Whether the run carries a word worth reporting. Judged when the run is formed, where its whole
    /// literal text is available; only a preview is kept here.
    #[serde(default)]
    pub readable: bool,
}

/// Drop `{...}` control tokens, leaving only the text that carries language signal.
///
/// Without this the token vocabulary (`byte`, `end`, `choice`, `item`, …) dominates
/// the statistics and biases every detector toward English.
pub fn literal_only(text: &str) -> String {
    let mut out = String::with_capacity(text.len());
    let mut depth = 0usize;
    for character in text.chars() {
        match character {
            '{' => depth += 1,
            '}' if depth > 0 => depth -= 1,
            _ if depth == 0 => out.push(character),
            _ => {}
        }
    }
    out
}

fn support_for(script: Script) -> ScriptSupport {
    match script {
        // The only script this game's pinned tables are evidenced to carry.
        Script::Latin => ScriptSupport::Evidenced,
        // Han/Kana become usable here the moment a kanji table is pinned; until then
        // a match is a lead, not a language finding.
        _ => ScriptSupport::Unevidenced,
    }
}

/// Script profile of a latch's literal text, plus a language guess.
pub fn script_profile(text: &str) -> (Vec<ScriptShare>, Option<LanguageGuess>) {
    let literal = literal_only(text);
    let mut counts: Vec<(Script, usize)> = Vec::new();
    let mut letters = 0usize;
    for character in literal.chars() {
        if !character.is_alphabetic() {
            continue;
        }
        let script = character.script();
        if matches!(script, Script::Common | Script::Inherited | Script::Unknown) {
            continue;
        }
        letters += 1;
        match counts.iter_mut().find(|(seen, _)| *seen == script) {
            Some((_, count)) => *count += 1,
            None => counts.push((script, 1)),
        }
    }
    counts.sort_by_key(|(_, count)| std::cmp::Reverse(*count));
    let shares = counts
        .iter()
        .map(|(script, count)| ScriptShare {
            script: script.full_name().to_string(),
            characters: *count,
            share: if letters == 0 {
                0.0
            } else {
                *count as f64 / letters as f64
            },
            support: support_for(*script),
        })
        .collect();
    let language = whatlang::detect(&literal).map(|info| LanguageGuess {
        language: info.lang().code().to_string(),
        confidence: info.confidence(),
        reliable: info.is_reliable(),
    });
    (shares, language)
}

/// A classified window: `(start, length)` inside one subfile, already owned by a parse or a
/// reviewed record. Heuristic latching treats it as a barrier.
pub type Window = (usize, usize);

/// What one **named** window removed, so the accounting is per window rather than only aggregate.
#[derive(Clone, Copy, Debug, Default, Deserialize, Serialize)]
pub struct WindowTally {
    /// Position of this window in the caller's window list for the subfile, so the record maps back
    /// to a specific registry entry rather than to an anonymous range.
    pub index: usize,
    pub start: usize,
    pub length: usize,
    pub latches_removed: usize,
    pub latches_split: usize,
}

/// What the window check did, so a removal is never silent.
#[derive(Clone, Debug, Default, Deserialize, Serialize)]
pub struct Tally {
    /// Windows that fell inside the payload and were applied.
    pub windows: usize,
    /// Bytes covered by those windows.
    pub bytes_skipped: usize,
    /// Latches the un-windowed scan would have emitted entirely inside a window.
    pub latches_removed: usize,
    /// Latches the un-windowed scan emitted that only partly overlapped a window.
    pub latches_split: usize,
    /// Per-window removal counts, for the windows that removed or split something.
    pub per_window: Vec<WindowTally>,
    /// Per-family removals from the payload inventory, so no exclusion is silent.
    #[serde(default)]
    pub payloads: Vec<crate::models::PayloadRemoval>,
    /// Candidates dropped because they were token soup or symbol runs rather than readable text.
    #[serde(default)]
    pub readable_dropped: usize,
    /// Candidates dropped because the words a reader would see are not corroborated by the game's own
    /// text or by the project's documentation.
    #[serde(default)]
    pub unattested_dropped: usize,
}

impl Tally {
    pub fn add(&mut self, other: &Tally) {
        self.windows += other.windows;
        self.bytes_skipped += other.bytes_skipped;
        self.latches_removed += other.latches_removed;
        self.latches_split += other.latches_split;
        self.per_window.extend_from_slice(&other.per_window);
        self.readable_dropped += other.readable_dropped;
        self.unattested_dropped += other.unattested_dropped;
        for removal in &other.payloads {
            match self
                .payloads
                .iter_mut()
                .find(|seen| seen.magic == removal.magic)
            {
                Some(seen) => {
                    seen.candidates_removed += removal.candidates_removed;
                    seen.bytes_skipped += removal.bytes_skipped;
                }
                None => self.payloads.push(removal.clone()),
            }
        }
    }
}

/// One identified per-window removal: which subfile, which window of that subfile, and what it did.
#[derive(Clone, Debug, Deserialize, Serialize)]
pub struct WindowDetail {
    pub archive: String,
    pub entry: usize,
    /// Index into that subfile's registry window list.
    pub window: usize,
    pub start: usize,
    pub length: usize,
    pub latches_removed: usize,
    pub latches_split: usize,
}

/// A recorded string boundary: `(start, length)` inside one coordinate space, sorted by start.
pub type Extent = (usize, usize);

/// Walk **backwards** from a match offset over recorded extents to the string containing it.
///
/// The extents are the canonical recorded boundaries for one subfile — a banked section's row
/// extents, a raw-text run's latches, an index's instances. The walk begins at the match and moves
/// **back** over the list until it reaches the extent whose range contains the match, which is the
/// beginning of the string the match sits in. It is deliberately not told which extent matched:
/// resolving the beginning is the walk's job, so a caller that later changes how extents are
/// recorded cannot silently keep reporting a stale start.
pub fn walk_back_to_containing(extents: &[Extent], match_offset: usize) -> Option<Extent> {
    // Walk **backwards** from the last recorded extent: the first one that begins at or before the
    // match is the only candidate, and if it does not contain the match, an earlier one cannot
    // either (the extents are sorted and do not overlap), so the match sits in a gap.
    for (start, length) in extents.iter().rev() {
        if *start > match_offset {
            continue;
        }
        return (*start <= match_offset && match_offset < start + length)
            .then_some((*start, *length));
    }
    None
}

/// Walk **backwards** from a match offset to the beginning of the string it sits in.
///
/// Raw-text strings and corpus instances are recorded as several windows; the caller passes exactly
/// one group's windows and they may not all abut in payload terms (a command byte can sit between
/// them), so this returns only the **beginning**: the walk finds the window containing the match and
/// steps back while each earlier window abuts the next, stopping at a gap. The caller pairs that
/// beginning with the group's own recorded end, so the reported string is the whole recorded string
/// and the beginning is resolved by traversal rather than assumed from the group's first window.
pub fn walk_back_to_chain_start(extents: &[Extent], match_offset: usize) -> Option<usize> {
    // Walk backwards for the window that contains the match...
    let index = extents
        .iter()
        .rposition(|entry| entry.0 <= match_offset && match_offset < entry.0 + entry.1)?;
    // ...then keep walking back over the abutting ones before it.
    let mut begin = index;
    while begin > 0 {
        let (previous_start, previous_length) = extents[begin - 1];
        if previous_start + previous_length != extents[begin].0 {
            break;
        }
        begin -= 1;
    }
    Some(extents[begin].0)
}

/// The per-window details of one subfile's tally, stamped with the subfile they belong to.
pub fn details(archive: &str, entry: usize, tally: &Tally) -> Vec<WindowDetail> {
    tally
        .per_window
        .iter()
        .map(|window| WindowDetail {
            archive: archive.to_string(),
            entry,
            window: window.index,
            start: window.start,
            length: window.length,
            latches_removed: window.latches_removed,
            latches_split: window.latches_split,
        })
        .collect()
}

/// Latch candidate raw-text runs in one payload.
///
/// The scanner windows a long run only so that its statistics stay bounded; a run is **one** latch
/// however many windows it spans, so windows that abut are merged and the merged span is measured
/// over its whole length. That is why a latch may be longer than `MAX_RUN`.
pub fn scan(bytes: &[u8], min_run: usize) -> Vec<Latch> {
    scan_with(bytes, min_run, MAX_RUN)
}

/// The pipeline's entry point: form each run's readability verdict from its **whole** literal text.
///
/// `None` means the rule is off (nothing is judged); `Some(vocabulary)` judges every run, with a
/// vocabulary whose own rule may be disabled (shape only). The verdict cannot be taken later from the
/// latch: only a 72-character preview is kept, and judging readability on a prefix let a corroborated
/// opening admit a long run whose remaining text was noise.
pub fn scan_with_readability(
    bytes: &[u8],
    min_run: usize,
    max_run: usize,
    readability: Option<&crate::filters::Vocabulary>,
) -> Vec<Latch> {
    scan_judged(bytes, min_run, max_run, readability)
}

/// Latch candidate runs with an explicit window cap.
///
/// `max_run` of 0 means no cap. Cap-closed windows **abut**: the byte that hit the cap begins the
/// next window, so no byte is dropped and none is counted twice, and a phrase spanning the edge is
/// recovered by joining the abutting windows when searching. A gap-closed window advances past the
/// non-text byte that ended it, which is exactly the byte that is not latched.
pub fn scan_with(bytes: &[u8], min_run: usize, max_run: usize) -> Vec<Latch> {
    scan_judged(bytes, min_run, max_run, None)
}

fn scan_judged(
    bytes: &[u8],
    min_run: usize,
    max_run: usize,
    readability: Option<&crate::filters::Vocabulary>,
) -> Vec<Latch> {
    let mut latches = Vec::new();
    // A run is one contiguous stretch of text however many windows it spans, so its readability is
    // judged **once, over the whole run**, before any of it is reported. Judging each window alone let
    // a corroborated opening survive while an uncorroborated tail was dropped, and a later search join
    // cannot recover text the filter has already discarded. The windows remain extent bookkeeping.
    let mut run: Vec<Latch> = Vec::new();
    let mut run_start = 0usize;
    let mut run_printable = 0usize;
    let mut run_unknown = 0usize;
    let mut run_seen = [false; 256];
    let mut start = 0usize;
    let mut printable = 0usize;
    let mut unknown = 0usize;
    let mut gap = 0usize;
    let mut seen = [false; 256];
    let mut index = 0usize;
    while index <= bytes.len() {
        let byte = bytes.get(index).copied();
        let textish =
            byte.is_some_and(|value| literal_char(value).is_some() || command_for(value).is_some());
        if let Some(value) = byte {
            if textish {
                printable += 1;
                gap = 0;
                seen[value as usize] = true;
            } else {
                unknown += 1;
                gap += 1;
            }
        }
        let gap_closed = byte.is_none() || gap > GAP_TOLERANCE;
        let cap_closed = max_run > 0 && index - start >= max_run;
        if gap_closed || cap_closed {
            let length = index - start;
            let distinct = seen.iter().filter(|value| **value).count();
            // `min_run` counts **decoded literal characters**, not bytes: a run of command tokens
            // with few letters does not clear the floor even when its bytes are all textish. The
            // floors themselves are `filters::FLOORS`, so the thresholds live in one place.
            let span = &bytes[start..index];
            let decoded = command::deserialize(span);
            let characters = literal_only(&decoded).chars().count();
            {
                let (scripts, language) = script_profile(&decoded);
                let alpha: usize = scripts.iter().map(|share| share.characters).sum();
                let ratio = printable as f64 / length as f64;
                let literal = literal_only(&decoded);
                let word = literal
                    .split(|character: char| !character.is_alphabetic())
                    .max_by_key(|token| token.chars().count())
                    .unwrap_or("")
                    .to_lowercase();
                let candidate = PostFilterInput {
                    encoded: span,
                    length,
                    printable,
                    unknown,
                    distinct,
                    characters,
                    alpha,
                    literal: &literal,
                    min_run,
                };
                if FLOORS.keeps(&candidate) {
                    // The verdict is stamped when the whole run is known; this window's own measurements
                    // are kept for the latch's extents.
                    run.push(Latch {
                        offset: start,
                        length,
                        class: TextClass::Heuristic,
                        printable,
                        unknown,
                        ratio,
                        alpha,
                        distinct,
                        scripts,
                        language,
                        preview: literal.chars().take(PREVIEW).collect(),
                        word,
                        literal_characters: literal
                            .chars()
                            .filter(|character| !character.is_whitespace())
                            .count(),
                        attested: false,
                        shaped: false,
                        readable: false,
                    });
                }
            }
            run_printable += printable;
            run_unknown += unknown;
            for (slot, seen_here) in run_seen.iter_mut().zip(seen.iter()) {
                *slot |= *seen_here;
            }
            if cap_closed && !gap_closed {
                // Cap-closed: the next window begins at this byte, so the windows abut and no byte
                // is dropped or double counted.
                start = index;
            } else {
                // The run ends here, so judge it whole and release its windows with that verdict.
                let length = index - run_start;
                let span = &bytes[run_start..index];
                let decoded = command::deserialize(span);
                let literal = literal_only(&decoded);
                let alpha: usize = script_profile(&decoded)
                    .0
                    .iter()
                    .map(|share| share.characters)
                    .sum();
                let candidate = PostFilterInput {
                    encoded: span,
                    length,
                    printable: run_printable,
                    unknown: run_unknown,
                    distinct: run_seen.iter().filter(|value| **value).count(),
                    characters: literal.chars().count(),
                    alpha,
                    literal: &literal,
                    min_run,
                };
                let verdict =
                    crate::filters::readability_verdict(&candidate, &literal, readability);
                for mut latch in run.drain(..) {
                    latch.attested = verdict.attested;
                    latch.shaped = verdict.shaped;
                    latch.readable = verdict.readable;
                    latches.push(latch);
                }
                start = index + 1;
                run_start = start;
                run_printable = 0;
                run_unknown = 0;
                run_seen = [false; 256];
            }
            printable = 0;
            unknown = 0;
            gap = 0;
            seen = [false; 256];
            index = start;
            continue;
        }
        index += 1;
    }
    latches
}

/// The TOC entries of an EMI archive, unfiltered by text class.
pub fn entries(data: &[u8]) -> Result<Vec<(Entry, &[u8])>> {
    if data.len() < HEADER_SIZE || &data[8..16] != MAGIC {
        return Err(Error::Invalid(
            "not an EMI archive (missing MATH_TBL magic)".into(),
        ));
    }
    let count = u32::from_le_bytes(data[0..4].try_into().unwrap()) as usize;
    if count == 0 || HEADER_SIZE + count * TOC_ENTRY_SIZE > data.len() {
        return Err(Error::Invalid(
            "EMI TOC entry count does not fit the file".into(),
        ));
    }
    let mut out = Vec::new();
    let mut offset = 0x800;
    for index in 0..count {
        let at = HEADER_SIZE + index * TOC_ENTRY_SIZE;
        let size = u32::from_le_bytes(data[at..at + 4].try_into().unwrap()) as usize;
        let ram_ptr = u32::from_le_bytes(data[at + 4..at + 8].try_into().unwrap());
        if offset + size > data.len() {
            break;
        }
        out.push((
            Entry {
                index,
                offset,
                size: size as u32,
                ram_ptr,
            },
            &data[offset..offset + size],
        ));
        offset += size.div_ceil(SECTOR) * SECTOR;
    }
    Ok(out)
}

/// Every subfile's latches, leaving each subfile's classified windows alone.
///
/// Each subfile keeps its own tally, so a per-window removal stays identified by subfile rather
/// than being merged into one anonymous count.
pub fn scan_archive_excluding(
    data: &[u8],
    min_run: usize,
    archive: &str,
    filters: &crate::filters::ScanFilters<'_>,
) -> Result<Vec<(Entry, Vec<Latch>, Tally)>> {
    let mut out = Vec::new();
    let mut preceding: Vec<&[u8]> = Vec::new();
    for (entry, payload) in entries(data)? {
        let input = crate::models::PreFilterInput {
            archive,
            entry: entry.index,
            payload,
            class: TextClass::from_load_argument(entry.ram_ptr),
            // The VAB body is identified by pairing with a header earlier in this container.
            preceding: &preceding,
        };
        // One place decides admissibility: the classified-window barrier and the payload inventory.
        let (latches, tally) = filters.scan_bytes(&input, min_run);
        preceding.push(payload);
        out.push((entry, latches, tally));
    }
    Ok(out)
}

#[cfg(test)]
mod walk_back_tests {
    use super::*;

    #[test]
    fn the_walk_finds_the_containing_extent_from_anywhere_inside_it() {
        let extents = [(100, 40), (200, 30), (500, 10)];
        assert_eq!(walk_back_to_containing(&extents, 100), Some((100, 40)));
        assert_eq!(walk_back_to_containing(&extents, 139), Some((100, 40)));
        assert_eq!(walk_back_to_containing(&extents, 215), Some((200, 30)));
        assert_eq!(walk_back_to_containing(&extents, 505), Some((500, 10)));
    }

    #[test]
    fn the_walk_stops_at_a_gap_and_at_the_edges() {
        let extents = [(100, 40), (200, 30)];
        assert_eq!(
            walk_back_to_containing(&extents, 99),
            None,
            "before every extent"
        );
        assert_eq!(walk_back_to_containing(&extents, 140), None, "in the gap");
        assert_eq!(
            walk_back_to_containing(&extents, 230),
            None,
            "after every extent"
        );
        assert_eq!(walk_back_to_containing(&[], 5), None);
    }

    #[test]
    fn the_chain_walk_reaches_the_first_window_of_an_abutting_chain() {
        let extents = [(300, 100), (400, 2048), (2448, 5)];
        assert_eq!(walk_back_to_chain_start(&extents, 400), Some(300));
        assert_eq!(walk_back_to_chain_start(&extents, 2450), Some(300));
        assert_eq!(walk_back_to_chain_start(&extents, 310), Some(300));
    }

    #[test]
    fn the_chain_walk_does_not_cross_a_gap() {
        let extents = [(100, 10), (500, 10)];
        assert_eq!(walk_back_to_chain_start(&extents, 505), Some(500));
        assert_eq!(walk_back_to_chain_start(&extents, 110), None, "in the gap");
        // a gap splits the group: the abutting pair before it is one string, the window after it
        // begins its own
        let split = [(0, 10), (10, 10), (100, 10)];
        assert_eq!(walk_back_to_chain_start(&split, 15), Some(0));
        assert_eq!(walk_back_to_chain_start(&split, 105), Some(100));
    }
}

#[cfg(test)]
mod scan_excluding_tests {
    #![allow(clippy::needless_range_loop)]

    use super::*;

    /// A test-local pre-filter that refuses exactly the given windows, so the barrier tests drive the
    /// real `PreFilter`/`scan_admitted` path without needing a registry file on disk.
    struct Refuse<'a>(&'a [(usize, usize)]);

    impl crate::models::PreFilter for Refuse<'_> {
        fn admit(
            &self,
            input: &crate::models::PreFilterInput<'_>,
        ) -> crate::models::AdmittedRanges {
            let mut admitted = crate::models::AdmittedRanges::default();
            let mut cursor = 0usize;
            for (index, (start, length)) in self.0.iter().copied().enumerate() {
                if length == 0 || start >= input.payload.len() {
                    continue;
                }
                if start > cursor {
                    admitted.ranges.push((cursor, start - cursor));
                }
                admitted.refusals.push(crate::models::Refusal {
                    start,
                    length,
                    window: Some(index),
                    family: None,
                    magic: None,
                    recorded_candidates: 0,
                    basis: String::new(),
                    reason: "test window".to_string(),
                });
                cursor = start + length;
            }
            if cursor < input.payload.len() {
                admitted.ranges.push((cursor, input.payload.len() - cursor));
            }
            admitted
        }
    }

    fn scan_with_windows(
        payload: &[u8],
        min_run: usize,
        windows: &[(usize, usize)],
    ) -> (Vec<Latch>, Tally) {
        let input = crate::models::PreFilterInput {
            archive: "test",
            entry: 0,
            payload,
            class: None,
            preceding: &[],
        };
        crate::filters::scan_admitted(
            &input,
            min_run,
            &Refuse(windows),
            windows,
            false,
            &crate::filters::Vocabulary::empty(),
        )
    }

    #[test]
    fn a_run_is_judged_whole_however_many_windows_it_spans() {
        // The opening is attested and the tail is not, and the tail sits beyond a scanner window cap:
        // judging each window alone would report the opening, which is the defect this guards.
        // 0xFF is this table's literal space; ASCII 0x20 is not a literal and would split the run.
        let mut bytes: Vec<u8> = vec![b' ', 0xFF];
        bytes.extend_from_slice(b"the village");
        bytes.push(0xFF);
        bytes.extend(std::iter::repeat_n(0xFFu8, 60));
        bytes.push(0xFF);
        bytes.extend_from_slice(b"xzzzqqq");
        let vocabulary = crate::filters::Vocabulary::from_words(&["the", "village"]);
        let readable = |latches: &[Latch]| latches.iter().filter(|latch| latch.readable).count();
        let one_window = scan_with_readability(&bytes, 4, 0, Some(&vocabulary));
        let many_windows = scan_with_readability(&bytes, 4, 64, Some(&vocabulary));
        assert!(
            !one_window.is_empty(),
            "the run should still be formed; only its verdict is under test"
        );
        assert_eq!(
            readable(&one_window),
            0,
            "a run whose tail nothing corroborates must not be readable"
        );
        assert_eq!(
            readable(&many_windows),
            0,
            "the window cap must not change the verdict: {} of {} window(s) stayed readable",
            readable(&many_windows),
            many_windows.len()
        );
        // with the rule off the run is reported, so the verdict — not the scanner — is what changed
        assert!(readable(&scan_with_readability(&bytes, 4, 0, None)) > 0);
    }

    fn text_run(bytes: usize) -> Vec<u8> {
        (0..bytes).map(|index| b'a' + (index % 26) as u8).collect()
    }

    #[test]
    fn a_window_is_a_barrier_and_no_byte_is_lost() {
        let payload = text_run(600);
        let window = (200usize, 100usize);
        let (latches, tally) = scan_with_windows(&payload, 4, &[window]);
        assert_eq!(tally.windows, 1);
        assert_eq!(tally.bytes_skipped, 100);
        assert!(tally.latches_split >= 1, "the barrier split the run");
        for latch in &latches {
            let end = latch.offset + latch.length;
            assert!(
                end <= window.0 || latch.offset >= window.0 + window.1,
                "a latch inside the window"
            );
        }
        let mut covered = vec![false; payload.len()];
        for latch in &latches {
            for byte in latch.offset..latch.offset + latch.length {
                covered[byte] = true;
            }
        }
        for byte in window.0..window.0 + window.1 {
            covered[byte] = true;
        }
        let uncovered = covered.iter().filter(|seen| !**seen).count();
        assert!(
            uncovered < payload.len() / 10,
            "most bytes stay covered: {uncovered} uncovered"
        );
    }

    #[test]
    fn a_wholly_refused_payload_is_never_parsed() {
        // When nothing may be latched the payload is not decoded at all, so no removal count can be
        // measured; the barrier reports its bytes instead of a candidate count it would have to
        // decode the payload to obtain.
        let payload = text_run(600);
        let (latches, tally) = scan_with_windows(&payload, 4, &[(0, 600)]);
        assert!(latches.is_empty());
        assert_eq!(tally.bytes_skipped, 600);
        assert_eq!(tally.latches_removed, 0);
    }

    #[test]
    fn no_windows_leaves_the_scan_unchanged() {
        let payload = text_run(600);
        let (plain, tally) = scan_with_windows(&payload, 4, &[]);
        assert_eq!(tally.windows, 0);
        assert_eq!(plain.len(), scan(&payload, 4).len());
    }

    #[test]
    fn overlapping_windows_merge() {
        let payload = text_run(600);
        let (_, tally) = scan_with_windows(&payload, 4, &[(100, 100), (150, 100)]);
        assert_eq!(
            tally.windows, 1,
            "overlapping windows merge into one barrier"
        );
        assert_eq!(tally.bytes_skipped, 150);
    }
}
