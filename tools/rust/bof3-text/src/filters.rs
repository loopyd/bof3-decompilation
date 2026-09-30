//! Every filter and every threshold in one place.
//!
//! A **pre-filter** decides which byte ranges of a subfile may be latched at all — the classified
//! window barrier is the one this tool ships, and a payload inventory adds to it. A **post-filter**
//! decides whether a produced candidate run is worth reporting — the scanner's quality floors live
//! here, and the readability rule joins them. Nothing outside this module may hold filter logic or a
//! threshold constant; the scanner and the CLI obtain filtering only through these types.

use std::fs;
use std::path::Path;

use serde::{Deserialize, Serialize};

use crate::lexagraph::{Latch, Tally, Window, WindowTally, scan, scan_with_readability};
use crate::models::{
    AdmittedRanges, Error, PayloadRemoval, PostFilter, PostFilterInput, PreFilter, PreFilterInput,
    Refusal, Result,
};
use crate::windows::WindowIndex;

/// Fewest decoded characters that can latch a run.
pub const MIN_RUN: usize = 12;
/// Unprintable bytes tolerated inside one run before it is closed.
pub const GAP_TOLERANCE: usize = 4;
/// Hard cap on a single scanned window.
pub const MAX_RUN: usize = 8192;
/// Minimum fraction of a candidate's bytes that must decode before it is reported.
pub const MIN_RATIO: f64 = 0.90;
/// Fewest script letters a candidate must contain. Runs of filler (repeated `0x00`, `0x3F`) decode
/// as tokens yet carry no words, so a letter floor removes most false positives.
pub const MIN_ALPHA: usize = 8;
/// Fewest distinct byte values a candidate must contain, to reject constant filler runs.
pub const MIN_DISTINCT: usize = 6;
/// Decoded characters kept as a latch preview.
pub const PREVIEW: usize = 72;
/// Fewest letters in a single word for a result to be worth reporting.
pub const MIN_WORD_LETTERS: usize = 3;
/// Least share of a run's literal characters (command tokens stripped, whitespace excluded) that must
/// be letters. This is the request's own bar: a result may not be "just full of command tokens or
/// symbols", so its literal text has to be mostly letters.
///
/// The denominator is the literal text with command tokens removed, not the encoded byte length: a
/// run that is mostly `{end}`/`{byte(0xNN)}` markup has few literal characters to begin with, and
/// measuring against bytes would let markup-heavy runs pass on a technicality. A real sentence is
/// roughly three-quarters letters, so this floor is what separates prose from markup.
pub const MIN_LETTER_SHARE: f64 = 0.50;
/// Fewest distinct letters in the run's longest word. `qqqq` and `cddddddddddddd` are alphabetic runs,
/// not words, and this is what rejects them.
///
/// There is deliberately **no vowel requirement**: labels and abbreviations this game writes
/// (`KLK5`, `HP`, `STR`) carry no vowel, and the request's bar is "single human readable words are
/// fine", not "only dictionary words". Whether the longest word is *corroborated* is recorded per
/// entry as [`crate::lexagraph::Latch::attested`] from the game's own vocabulary, which a reader can
/// weigh without the filter deciding it for them.
pub const MIN_WORD_DISTINCT: usize = 2;

/// The scanner's quality floors: the post-filter every candidate must clear to be reported.
pub struct Floors;

impl PostFilter for Floors {
    fn keeps(&self, candidate: &PostFilterInput<'_>) -> bool {
        // Admission is a filtering decision, so it lives here rather than in the scanner: the
        // caller's minimum run length arrives as part of the candidate.
        if candidate.length == 0 || candidate.characters < candidate.min_run {
            return false;
        }
        let ratio = candidate.printable as f64 / candidate.length as f64;
        ratio >= MIN_RATIO && candidate.distinct >= MIN_DISTINCT && candidate.alpha >= MIN_ALPHA
    }
}

/// The floors as a value the scanner can hold.
pub const FLOORS: Floors = Floors;

/// The readability floor: a result must contain a word, not only decode as text.
///
/// It judges a produced candidate by its own measurements (its preview, its letter count and its
/// length), so it can run where candidates are reported rather than while they are formed.
pub struct Readable;

impl PostFilter for Readable {
    /// True when the run's literal text contains a word of at least [`MIN_WORD_LETTERS`] letters and
    /// its letters are at least [`MIN_LETTER_SHARE`] of its encoded bytes.
    ///
    /// The candidate carries the whole run's literal text with command tokens stripped, so both the
    /// word and the share are measured over the run rather than a preview.
    fn keeps(&self, candidate: &PostFilterInput<'_>) -> bool {
        let literal = candidate.literal;
        if candidate.length == 0 || literal.is_empty() {
            return false;
        }
        let characters = literal
            .chars()
            .filter(|character| !character.is_whitespace())
            .count();
        if characters == 0 {
            return false;
        }
        let letters = literal
            .chars()
            .filter(|character| character.is_alphabetic())
            .count();
        if (letters as f64 / characters as f64) < MIN_LETTER_SHARE {
            return false;
        }
        // The longest word must be a word: enough letters, and more than one distinct letter.
        let longest = literal
            .split(|character: char| !character.is_alphabetic())
            .map(|token| {
                let distinct = token
                    .chars()
                    .flat_map(|character| character.to_lowercase())
                    .collect::<std::collections::BTreeSet<_>>()
                    .len();
                (token.chars().count(), distinct, true)
            })
            .max_by_key(|(length, _, _)| *length);
        match longest {
            Some((length, distinct, vowel)) => {
                length >= MIN_WORD_LETTERS && distinct >= MIN_WORD_DISTINCT && vowel
            }
            None => false,
        }
    }
}

/// What the filters decide about one candidate run.
#[derive(Clone, Copy, Debug, Default)]
pub struct Verdict {
    /// The run may be reported: it passed the shape checks and, when the rule is on, its words are
    /// corroborated.
    pub readable: bool,
    /// The run passed the shape checks, before corroboration.
    pub shaped: bool,
    /// The run's words are all the game's own. False when the rule is off, so nothing is claimed.
    pub attested: bool,
}

/// The **whole** readability policy for one run: shape and corroboration composed here, so no caller
/// decides it. `readability` is `None` when the rule is off, and a vocabulary whose own rule may be
/// disabled (shape only).
pub fn readability_verdict(
    candidate: &PostFilterInput<'_>,
    literal: &str,
    readability: Option<&Vocabulary>,
) -> Verdict {
    let shaped = READABLE.keeps(candidate);
    let (corroborated, attested) = match readability {
        None => (true, false),
        Some(vocabulary) => {
            let corroborates = vocabulary.corroborates(literal);
            if vocabulary.is_disabled() {
                (true, false)
            } else {
                (corroborates, corroborates)
            }
        }
    };
    Verdict {
        readable: shaped && corroborated,
        shaped,
        attested,
    }
}

/// The readability floor as a value the scanner can hold.
pub const READABLE: Readable = Readable;

/// The schema this build understands of the vocabulary.
pub const VOCABULARY_SCHEMA: &str = "bof3.text-vocabulary/v1";

#[derive(Debug, Deserialize)]
struct VocabularyFile {
    schema: String,
    words: Vec<String>,
}

/// The words the game's own verified text and the project's documentation attest.
///
/// A word has to be in here to be reported: an alphabetic run is not a word just because it is
/// alphabetic, and this is the check that separates the two.
#[derive(Clone, Debug, Default)]
pub struct Vocabulary {
    words: std::collections::BTreeSet<String>,
    /// Set when the caller explicitly turned the vocabulary rule off. A disabled vocabulary must
    /// **not** veto every word: it means "do not check", not "attest nothing".
    disabled: bool,
}

impl Vocabulary {
    /// No vocabulary at all — only for a caller comparing against the unfiltered scanner.
    pub fn empty() -> Self {
        Self {
            disabled: true,
            ..Self::default()
        }
    }

    pub fn len(&self) -> usize {
        self.words.len()
    }

    /// A vocabulary from words given directly — for tests, so a calibration can use the real rule
    /// without a file on disk.
    #[cfg(test)]
    pub fn from_words(words: &[&str]) -> Self {
        Self {
            words: words.iter().map(|word| word.to_lowercase()).collect(),
            disabled: false,
        }
    }

    /// Read the vocabulary. A missing or unreadable file is an error: a readability rule without a
    /// vocabulary would report alphabetic noise as text.
    pub fn load(path: &Path) -> Result<Self> {
        let text = fs::read_to_string(path)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        let file: VocabularyFile = serde_json::from_str(&text)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        if file.schema != VOCABULARY_SCHEMA {
            return Err(Error::Invalid(format!(
                "{}: unsupported vocabulary schema {}; expected {VOCABULARY_SCHEMA}",
                path.display(),
                file.schema
            )));
        }
        if file.words.is_empty() {
            return Err(Error::Invalid(format!(
                "{}: the vocabulary is empty",
                path.display()
            )));
        }
        Ok(Self {
            disabled: false,
            words: file
                .words
                .into_iter()
                .map(|word| word.to_lowercase())
                .collect(),
        })
    }

    /// Was the vocabulary rule switched off? A disabled vocabulary attests nothing: marking a run as
    /// corroborated because the check was skipped would be a false claim.
    pub fn is_disabled(&self) -> bool {
        self.disabled
    }

    /// Do the words in this run's literal text corroborate it?
    ///
    /// **Every** alphabetic token of three or more letters in it must be a word the game itself writes.
    ///
    /// Three rules were measured over the corpus (`readability.md`): every token attested reports
    /// nothing; two-or-more-and-a-majority reports 8 runs and a 72-character-preview majority reports 76,
    /// both dominated by byte noise (`z;;;; ;drrddz;`, `,,ZZ,,0,ZZZ`, `←zzzz○↓→→xxxxxyy`). No token
    /// statistic separates a label list inside binary from the binary itself, so the rule reports only a
    /// run it can fully corroborate — and on this corpus that is none, because the game's text arrives
    /// through the parse (46,057 rows), not through scanner candidates. The measurement that chose this bar is in
    /// `readability.md`: requiring **every** token attested removed even genuine label lists (a single
    /// odd token like `CTTTT` failed an otherwise readable run), while a majority test judged on a
    /// 72-character preview admitted noise whose later text was never examined. Judging the whole
    /// literal at this bar is what keeps `Worker CTTTT c Xc H … Operator …` and rejects
    /// `gee△X△…` and `qqqqqqqqqqqt`.
    pub fn corroborates(&self, literal: &str) -> bool {
        let tokens: Vec<&str> = literal
            .split(|character: char| !character.is_alphabetic())
            .filter(|token| token.len() >= MIN_WORD_LETTERS)
            .collect();
        if tokens.is_empty() {
            return false;
        }
        tokens
            .iter()
            .all(|token| self.words.contains(&token.to_lowercase()))
    }
}

/// Merge overlapping or adjacent windows, sorted, ignoring empty and out-of-range ones.
pub fn merge_windows(bytes: usize, windows: &[Window]) -> Vec<Window> {
    let mut kept: Vec<Window> = windows
        .iter()
        .filter(|(start, length)| *length > 0 && *start < bytes)
        .map(|(start, length)| (*start, (*length).min(bytes - start)))
        .collect();
    kept.sort_unstable();
    let mut merged: Vec<Window> = Vec::with_capacity(kept.len());
    for (start, length) in kept {
        if let Some(last) = merged.last_mut() {
            if start <= last.0 + last.1 {
                let end = (last.0 + last.1).max(start + length);
                last.1 = end - last.0;
                continue;
            }
        }
        merged.push((start, length));
    }
    merged
}

/// A classified window is a barrier: the pre-filter that refuses ranges an existing owner accounts for.
pub struct WindowBarrier<'a> {
    pub archive: &'a str,
    pub windows: &'a WindowIndex,
}

impl PreFilter for WindowBarrier<'_> {
    fn admit(&self, input: &PreFilterInput<'_>) -> AdmittedRanges {
        let windows = self.windows.get(self.archive, input.entry);
        let merged = merge_windows(input.payload.len(), windows);
        let mut admitted = AdmittedRanges::default();
        // The refusals carry the registry list position of each window supplied, so a refusal can be
        // named back to its owning record even where the merged barrier is one range.
        for (index, (start, length)) in windows.iter().copied().enumerate() {
            if length == 0 || start >= input.payload.len() {
                continue;
            }
            let length = length.min(input.payload.len() - start);
            admitted.refusals.push(Refusal {
                start,
                length,
                window: Some(index),
                family: None,
                magic: None,
                recorded_candidates: 0,
                basis: String::new(),
                reason: "classified window: an existing owner accounts for this range".to_string(),
            });
        }
        let mut cursor = 0usize;
        for (start, length) in &merged {
            if *start > cursor {
                admitted.ranges.push((cursor, start - cursor));
            }
            cursor = start + length;
        }
        if cursor < input.payload.len() {
            admitted.ranges.push((cursor, input.payload.len() - cursor));
        }
        admitted
    }
}

/// The schema this build understands of the payload inventory.
pub const INVENTORY_SCHEMA: &str = "bof3.payload-inventory/v1";

/// A structural check an inventory entry may name. The checks live here, so an inventory cannot
/// invent one, and the same code confirms a payload in the tool and in the probe's verdict.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum PayloadCheck {
    VabHeaderSize,
    SeqEventChain,
}

impl PayloadCheck {
    pub fn from_kind(kind: &str) -> Option<Self> {
        match kind {
            "vab_header_size" => Some(Self::VabHeaderSize),
            "seq_event_chain" => Some(Self::SeqEventChain),
            _ => None,
        }
    }

    /// Does this payload really have that structure? A signature that fails is **not** a payload.
    pub fn holds(self, payload: &[u8]) -> bool {
        match self {
            Self::VabHeaderSize => {
                if payload.len() < 0x20 || &payload[..4] != b"pBAV" {
                    return false;
                }
                let programs = u16::from_le_bytes([payload[0x12], payload[0x13]]) as usize;
                0x20 + 128 * 16 + programs * 16 * 32 + 512 == payload.len()
            }
            Self::SeqEventChain => {
                if payload.len() < 19 || &payload[..4] != b"pQES" {
                    return false;
                }
                let mut cursor = 19
                    + u32::from_be_bytes([payload[15], payload[16], payload[17], payload[18]])
                        as usize;
                let mut tracks = 1usize;
                while cursor < payload.len() {
                    if tracks >= 4 || cursor + 13 > payload.len() {
                        return false;
                    }
                    let event_len = u32::from_be_bytes([
                        payload[cursor + 9],
                        payload[cursor + 10],
                        payload[cursor + 11],
                        payload[cursor + 12],
                    ]) as usize;
                    cursor += 13 + event_len;
                    tracks += 1;
                }
                cursor == payload.len()
            }
        }
    }
}

/// Does `body` pair with `header` as that header's VAB sample body?
///
/// The pairing is the record's own identity — the doc says the body is the sibling whose size equals
/// the header's VAG-size-table sum multiplied by 8 — so it is a check, not a guess.
pub fn paired_vab_body(header: &[u8], body: &[u8]) -> bool {
    if header.len() < 512 || &header[..4] != b"pBAV" {
        return false;
    }
    let table = header.len() - 512;
    let total: usize = (0..256)
        .map(|index| {
            u16::from_le_bytes([header[table + 2 * index], header[table + 2 * index + 1]]) as usize
        })
        .sum::<usize>()
        * 8;
    total == body.len() && !body.is_empty()
}

/// One identified family, as the inventory states it.
#[derive(Clone, Debug, Deserialize, Serialize)]
pub struct PayloadFamily {
    /// The signature, when the family has one. A body identified by pairing has none.
    #[serde(default)]
    pub magic: String,
    pub family: String,
    pub format: String,
    #[serde(default)]
    pub records: Vec<String>,
    pub check: PayloadCheckSpec,
    /// What the exclusion removed, measured once against the corpus and recorded here: the tool never
    /// parses an excluded payload, so a runtime count would either be zero (a lie) or require the very
    /// parse the short-circuit exists to avoid.
    #[serde(default)]
    pub measured: Option<PayloadMeasurements>,
}

/// The measured effect of one exclusion, recorded in the inventory.
#[derive(Clone, Debug, Default, Deserialize, Serialize)]
pub struct PayloadMeasurements {
    /// Candidates the exclusion removes on its own, with the shape rule switched off.
    #[serde(default)]
    pub removed: usize,
    /// Subfiles in which it removes at least one candidate.
    #[serde(default)]
    pub removed_subfiles: usize,
    /// Candidates left for it to remove once the shape rule has run.
    #[serde(default)]
    pub removed_marginal: usize,
    /// How those numbers were obtained.
    #[serde(default)]
    pub basis: String,
}

/// The structural check an entry names.
#[derive(Clone, Debug, Deserialize, Serialize)]
pub struct PayloadCheckSpec {
    pub kind: String,
    #[serde(default)]
    pub detail: String,
}

#[derive(Debug, Deserialize)]
struct Inventory {
    schema: String,
    exclusions: Vec<PayloadFamily>,
    #[serde(default)]
    identified_without_signature: Vec<PayloadFamily>,
}

/// The payload inventory the pre-filter reads: families known not to be text.
#[derive(Clone, Debug, Default)]
pub struct KnownPayload {
    families: Vec<PayloadFamily>,
    /// The family the inventory names for a body identified by pairing, when it names one.
    body_family: Option<PayloadFamily>,
}

impl KnownPayload {
    /// No payload knowledge at all — used when the caller explicitly opts out.
    pub fn empty() -> Self {
        Self::default()
    }

    /// Families loaded.
    pub fn len(&self) -> usize {
        self.families.len()
    }

    /// Read the inventory. A missing or unreadable file is an error, never "nothing is excluded".
    pub fn load(path: &Path) -> Result<Self> {
        let bytes = fs::read(path)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        let text = String::from_utf8(bytes)
            .map_err(|_| Error::Invalid(format!("{}: not valid UTF-8", path.display())))?;
        let inventory: Inventory = serde_json::from_str(&text)
            .map_err(|error| Error::Invalid(format!("{}: {error}", path.display())))?;
        if inventory.schema != INVENTORY_SCHEMA {
            return Err(Error::Invalid(format!(
                "{}: unsupported payload inventory schema {}; expected {INVENTORY_SCHEMA}",
                path.display(),
                inventory.schema
            )));
        }
        for family in &inventory.exclusions {
            if family.records.is_empty() {
                return Err(Error::Invalid(format!(
                    "{}: {} is excluded without a named record",
                    path.display(),
                    family.magic
                )));
            }
            if PayloadCheck::from_kind(&family.check.kind).is_none() {
                return Err(Error::Invalid(format!(
                    "{}: {} names an unimplemented structural check {}",
                    path.display(),
                    family.magic,
                    family.check.kind
                )));
            }
        }
        let body_family = inventory
            .identified_without_signature
            .into_iter()
            .find(|family| family.check.kind == "vab_body_pairing");
        Ok(Self {
            families: inventory.exclusions,
            body_family,
        })
    }

    /// The recorded removal counts per family, for reporting once per run rather than once per
    /// payload: an excluded payload is never parsed, so the count is a recorded measurement.
    pub fn totals(&self) -> Vec<(String, String, usize, usize, String)> {
        self.families
            .iter()
            .chain(self.body_family.as_ref())
            .filter_map(|family| {
                let measured = family.measured.as_ref()?;
                Some((
                    family.family.clone(),
                    if family.magic.is_empty() {
                        "pBAV+ct7".to_string()
                    } else {
                        family.magic.clone()
                    },
                    measured.removed,
                    measured.removed_marginal,
                    measured.basis.clone(),
                ))
            })
            .collect()
    }

    /// The family that identifies this payload, when one does **and** its check holds.
    pub fn identify<'a>(&'a self, payload: &[u8]) -> Option<&'a PayloadFamily> {
        self.families.iter().find(|family| {
            payload.starts_with(family.magic.as_bytes())
                && PayloadCheck::from_kind(&family.check.kind)
                    .is_some_and(|check| check.holds(payload))
        })
    }
}

impl KnownPayload {
    /// The family that identifies this payload by **pairing** with the subfile before it, when the
    /// inventory names one for a paired body and the pairing holds.
    pub fn identify_paired<'a>(
        &'a self,
        preceding: &[&[u8]],
        payload: &[u8],
    ) -> Option<&'a PayloadFamily> {
        let pairs = preceding
            .iter()
            .any(|header| header.starts_with(b"pBAV") && paired_vab_body(header, payload));
        if pairs {
            return self.body_family.as_ref();
        }
        None
    }
}

impl PreFilter for KnownPayload {
    fn admit(&self, input: &PreFilterInput<'_>) -> AdmittedRanges {
        let identified = self
            .identify(input.payload)
            .or_else(|| self.identify_paired(input.preceding, input.payload));
        match identified {
            Some(family) => AdmittedRanges {
                ranges: Vec::new(),
                refusals: vec![Refusal {
                    start: 0,
                    length: input.payload.len(),
                    window: None,
                    family: Some(family.family.clone()),
                    magic: Some(family.magic.clone()),
                    recorded_candidates: 0,
                    basis:
                        "see payload_totals: the family's recorded removal count is reported once"
                            .to_string(),
                    reason: format!(
                        "identified {} payload ({}), named by {}",
                        family.family,
                        family.format,
                        family.records.first().cloned().unwrap_or_default()
                    ),
                }],
            },
            None => AdmittedRanges {
                ranges: vec![(0, input.payload.len())],
                refusals: Vec::new(),
            },
        }
    }
}

/// Filters applied in order: a range must be admitted by **every** filter, and a refused range can
/// never be re-admitted by a later one.
pub struct PreFilters<'a> {
    pub filters: Vec<&'a dyn PreFilter>,
}

fn intersect(left: &[(usize, usize)], right: &[(usize, usize)]) -> Vec<(usize, usize)> {
    let mut out = Vec::new();
    for (left_start, left_length) in left {
        for (right_start, right_length) in right {
            let start = (*left_start).max(*right_start);
            let end = (left_start + left_length).min(right_start + right_length);
            if start < end {
                out.push((start, end - start));
            }
        }
    }
    out.sort_unstable();
    out
}

impl PreFilter for PreFilters<'_> {
    fn admit(&self, input: &PreFilterInput<'_>) -> AdmittedRanges {
        let mut admitted: Option<Vec<Window>> = None;
        let mut refusals = Vec::new();
        for filter in &self.filters {
            let verdict = filter.admit(input);
            refusals.extend(verdict.refusals);
            admitted = Some(match admitted {
                None => verdict.ranges,
                Some(current) => intersect(&current, &verdict.ranges),
            });
        }
        AdmittedRanges {
            ranges: admitted.unwrap_or_else(|| vec![(0, input.payload.len())]),
            refusals,
        }
    }
}

/// Whether the readability rule runs, as the filters understand it.
///
/// The caller supplies a flag; the **meaning** of that flag — that switching the rule off means "do not
/// judge", never "report nothing" — lives here, so no caller has to know it.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Readability {
    /// The rule runs: a run must be readable text to be reported.
    On,
    /// The rule is switched off.
    Off,
}

impl Readability {
    /// From a caller's opt-out flag.
    pub fn of_opt_out(opt_out: bool) -> Self {
        if opt_out { Self::Off } else { Self::On }
    }

    pub fn is_on(self) -> bool {
        self == Self::On
    }
}

/// The one place an opt-out becomes a filter.
///
/// A disabled filter is the **empty** filter, never a missing one, and the composition itself is
/// decided here rather than by each caller.
pub fn compose<'a>(
    windows: &'a WindowIndex,
    payloads: &'a KnownPayload,
    vocabulary: &'a Vocabulary,
    readability: Readability,
) -> ScanFilters<'a> {
    ScanFilters {
        windows,
        payloads,
        readable: readability.is_on(),
        vocabulary,
    }
}

/// Load the classified-window registry, or the empty one when the caller opted out. A missing registry
/// fails closed: an unavailable ownership source must not silently become "no barriers".
pub fn load_windows(path: &Path, opt_out: bool) -> Result<WindowIndex> {
    if opt_out {
        return Ok(WindowIndex::none());
    }
    if !path.is_file() {
        return Err(Error::Invalid(format!(
            "classified-window registry {} is missing; build it with `bin/harness text windows`, or pass --no-windows to latch without the check",
            path.display()
        )));
    }
    WindowIndex::load(path)
}

/// Load the payload inventory, or the empty one when the caller opted out. A missing inventory is an
/// error on the searching path rather than "nothing is excluded".
pub fn load_payloads(path: &Path, opt_out: bool) -> Result<KnownPayload> {
    if opt_out {
        return Ok(KnownPayload::empty());
    }
    if !path.is_file() {
        return Err(Error::Invalid(format!(
            "payload inventory {} is missing; run `bin/harness text prepare` to prepare the default corpus artifacts, or provide --payloads PATH",
            path.display()
        )));
    }
    KnownPayload::load(path)
}

/// Load the vocabulary, or the empty one when the caller opted out. A missing vocabulary would make the
/// rule attest nothing, so it fails closed instead.
pub fn load_vocabulary(path: &Path, opt_out: bool) -> Result<Vocabulary> {
    if opt_out {
        return Ok(Vocabulary::empty());
    }
    if !path.is_file() {
        return Err(Error::Invalid(format!(
            "vocabulary {} is missing; run `bin/harness text prepare` to derive it from the extracted corpus, or provide --vocabulary PATH",
            path.display()
        )));
    }
    Vocabulary::load(path)
}

/// The one place admissibility is decided: the classified-window barrier and the payload inventory.
pub struct ScanFilters<'a> {
    pub windows: &'a WindowIndex,
    pub payloads: &'a KnownPayload,
    /// Whether the readability floor applies; false only for a caller comparing against the
    /// un-filtered scanner.
    pub readable: bool,
    /// The words a reported result may use.
    pub vocabulary: &'a Vocabulary,
}

impl ScanFilters<'_> {
    /// Bind a retained index to the exact selection policy used to build it.
    /// Bump the policy version when filter or reading semantics change.
    pub fn fingerprint(&self) -> Result<String> {
        let bytes = serde_json::to_vec(&(
            "bof3.text-filters/v1",
            self.windows.source_hash(),
            &self.payloads.families,
            &self.payloads.body_family,
            self.readable,
            self.vocabulary.disabled,
            &self.vocabulary.words,
        ))
        .map_err(|error| Error::Invalid(error.to_string()))?;
        let mut digest = crate::textindex::Digest::new();
        digest.update(&bytes);
        Ok(digest.hex())
    }

    /// The readability choice this composition carries, so a caller never re-derives it from a flag.
    pub fn readability(&self) -> Readability {
        if self.readable {
            Readability::On
        } else {
            Readability::Off
        }
    }

    /// Does the **payload inventory** identify this blob?
    ///
    /// True means non-text by a named record, so a caller must not parse it at all — the short-circuit's
    /// whole point. This is deliberately narrower than "the filters admit nothing": a classified
    /// *window* can cover a subfile entirely while the subfile is still the game's text, and parsing it
    /// is how this tool recognises that. The decision lives here so no consumer re-derives it.
    pub fn identifies_payload(&self, input: &PreFilterInput<'_>) -> bool {
        !self.payloads.admit(input).refusals.is_empty()
    }

    /// Latch one subfile through the composed filters.
    pub fn scan_bytes(&self, input: &PreFilterInput<'_>, min_run: usize) -> (Vec<Latch>, Tally) {
        let barrier = WindowBarrier {
            archive: input.archive,
            windows: self.windows,
        };
        let chain = PreFilters {
            filters: vec![&barrier, self.payloads],
        };
        scan_admitted(
            input,
            min_run,
            &chain,
            self.windows.get(input.archive, input.entry),
            self.readable,
            self.vocabulary,
        )
    }
}

/// Drop the candidates that decode but carry no word, counting them so the loss is visible.
fn apply_readability(latches: &mut Vec<Latch>, tally: &mut Tally, enabled: bool) {
    if !enabled {
        return;
    }
    let before = latches.len();
    // Of the loss, how much failed on corroboration rather than on shape: a **subset** of the total,
    // not a second loss, and the evidence says so.
    tally.unattested_dropped += latches
        .iter()
        .filter(|latch| latch.shaped && !latch.attested)
        .count();
    // The verdict was formed with the whole literal text in hand by `readability_verdict`, so all that
    // is left is to report the loss and remove what it refused: `readable_dropped` is the whole loss and
    // `unattested_dropped` the part of it that failed corroboration.
    latches.retain(|latch| latch.readable);
    tally.readable_dropped += before - latches.len();
}

/// Latch the ranges a pre-filter admits, tallying what was refused and what that removed.
///
/// A refused range is a barrier: a candidate that would straddle it closes at its start and a fresh
/// one begins at its end, so unclassified bytes on either side stay latchable and none is dropped.
pub fn scan_admitted(
    input: &PreFilterInput<'_>,
    min_run: usize,
    filter: &dyn PreFilter,
    windows: &[Window],
    readable: bool,
    vocabulary: &Vocabulary,
) -> (Vec<Latch>, Tally) {
    let readability = if readable { Some(vocabulary) } else { None };
    let admitted = filter.admit(input);
    if admitted.refusals.is_empty() {
        let mut latches = scan_with_readability(input.payload, min_run, MAX_RUN, readability);
        let mut tally = Tally::default();
        apply_readability(&mut latches, &mut tally, readable);
        return (latches, tally);
    }
    // A payload nothing may be latched from is never decoded: an identified payload is skipped
    // before it is parsed at all. Its removal count is therefore not measured per run — the bytes are
    // reported, and the candidate comparison is available by running with `--no-payloads`.
    let fully_refused = admitted.ranges.iter().all(|(_, length)| *length == 0);
    if fully_refused {
        let merged = merge_windows(input.payload.len(), windows);
        let mut tally = Tally {
            windows: merged.len(),
            bytes_skipped: merged.iter().map(|(_, length)| *length).sum(),
            ..Tally::default()
        };
        for refusal in admitted
            .refusals
            .iter()
            .filter(|refusal| refusal.family.is_some())
        {
            let family = refusal.family.clone().unwrap_or_default();
            let magic = refusal.magic.clone().unwrap_or_default();
            match tally.payloads.iter_mut().find(|seen| seen.magic == magic) {
                Some(seen) => seen.bytes_skipped += refusal.length,
                None => tally.payloads.push(PayloadRemoval {
                    family,
                    magic,
                    // Recorded by the inventory, because the payload is never parsed here.
                    candidates_removed: refusal.recorded_candidates,
                    bytes_skipped: refusal.length,
                    basis: refusal.basis.clone(),
                }),
            }
        }
        return (Vec::new(), tally);
    }
    let naive = scan(input.payload, min_run);
    let mut latches = Vec::new();
    for (start, length) in &admitted.ranges {
        for mut latch in scan_with_readability(
            &input.payload[*start..*start + *length],
            min_run,
            MAX_RUN,
            readability,
        ) {
            latch.offset += start;
            latches.push(latch);
        }
    }
    // The totals are measured over the merged barriers, exactly as the slice form does, so an
    // overlapping or duplicated window set cannot inflate the bytes skipped or the removal counts;
    // the refusals are kept for naming each barrier back to its owning record.
    let merged = merge_windows(input.payload.len(), windows);
    let mut tally = Tally {
        windows: merged.len(),
        bytes_skipped: merged.iter().map(|(_, length)| *length).sum(),
        ..Tally::default()
    };
    for latch in &naive {
        let end = latch.offset + latch.length;
        let overlaps = merged
            .iter()
            .any(|(start, length)| latch.offset < start + length && *start < end);
        if !overlaps {
            continue;
        }
        let inside = merged
            .iter()
            .any(|(start, length)| latch.offset >= *start && end <= start + length);
        if inside {
            tally.latches_removed += 1;
        } else {
            tally.latches_split += 1;
        }
    }
    for (index, (start, length)) in windows.iter().copied().enumerate() {
        if length == 0 || start >= input.payload.len() {
            continue;
        }
        let length = length.min(input.payload.len() - start);
        let end = start + length;
        let mut removed = 0usize;
        let mut split = 0usize;
        for latch in &naive {
            let latch_end = latch.offset + latch.length;
            if latch.offset >= end || start >= latch_end {
                continue;
            }
            if latch.offset >= start && latch_end <= end {
                removed += 1;
            } else {
                split += 1;
            }
        }
        if removed + split > 0 {
            tally.per_window.push(WindowTally {
                index,
                start,
                length,
                latches_removed: removed,
                latches_split: split,
            });
        }
    }
    // The readability floor is the reporting rule, applied on every path so no candidate reaches a
    // consumer without it.
    apply_readability(&mut latches, &mut tally, readable);
    // Per-family accounting, so an exclusion is reported and not silent. A candidate is attributed
    // to the family whose refusal contains it; families here are distinct payloads, so a candidate
    // is never counted for two of them.
    for refusal in admitted
        .refusals
        .iter()
        .filter(|refusal| refusal.family.is_some())
    {
        let family = refusal.family.clone().unwrap_or_default();
        let magic = refusal.magic.clone().unwrap_or_default();
        let end = refusal.start + refusal.length;
        let removed = naive
            .iter()
            .filter(|latch| latch.offset >= refusal.start && latch.offset + latch.length <= end)
            .count();
        match tally.payloads.iter_mut().find(|seen| seen.magic == magic) {
            Some(seen) => {
                seen.candidates_removed += removed;
                seen.bytes_skipped += refusal.length;
            }
            None => tally.payloads.push(PayloadRemoval {
                family,
                magic,
                candidates_removed: removed,
                bytes_skipped: refusal.length,
                basis: "counted in this run".to_string(),
            }),
        }
    }
    latches.sort_by_key(|latch| latch.offset);
    (latches, tally)
}
