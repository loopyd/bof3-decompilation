//! Shared dialogue, bank and command models.

use std::fmt;

use serde::{Deserialize, Serialize};

/// EMI container geometry (see `docs/specs/formats/emi.md`).
pub const MAGIC: &[u8; 8] = b"MATH_TBL";
pub const HEADER_SIZE: usize = 0x10;
pub const TOC_ENTRY_SIZE: usize = 0x10;
pub const FIRST_PAYLOAD: usize = 0x800;
pub const SECTOR: usize = 0x800;
/// `{end}` control byte; also the padding byte used inside a rewritten extent.
pub const STRING_TERMINATOR: u8 = 0x00;
/// Document grammar version.
pub const DOCUMENT_VERSION: u32 = 2;

#[derive(Debug)]
pub enum Error {
    Io(std::io::Error),
    Invalid(String),
}

impl fmt::Display for Error {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Error::Io(error) => write!(formatter, "{error}"),
            Error::Invalid(message) => write!(formatter, "{message}"),
        }
    }
}

impl From<std::io::Error> for Error {
    fn from(error: std::io::Error) -> Self {
        Error::Io(error)
    }
}

pub type Result<T> = std::result::Result<T, Error>;

pub fn invalid<T>(message: impl Into<String>) -> Result<T> {
    Err(Error::Invalid(message.into()))
}

/// One TOC entry of an EMI archive.
#[derive(Clone, Copy, Debug)]
pub struct Entry {
    pub index: usize,
    pub offset: usize,
    pub size: u32,
    pub ram_ptr: u32,
}

/// One dialogue row: a slot inside a bank.
///
/// A row's bytes are the slot's whole extent, from its pointer up to the next
/// larger pointer in the table (or the end of the subfile). That extent contains
/// the visible string, its `{end}` terminator, and — for choice rows — the
/// option strings that follow it, which no pointer addresses directly.
#[derive(Clone, Debug)]
pub struct DialogueRow {
    /// One-based slot number within its bank; stable across extract and pack.
    pub number: usize,
    /// Bank-relative extent of this row. `None` marks an empty slot.
    pub span: Option<(usize, usize)>,
    /// The extent's bytes, decoded through the command/character table.
    pub encoded: Vec<u8>,
}

impl DialogueRow {
    pub fn is_empty(&self) -> bool {
        self.span.is_none()
    }
}

/// One pointer bank: an offset table plus row extents.
#[derive(Clone, Debug)]
pub struct Bank {
    /// Zero-based bank index within its section.
    pub index: usize,
    /// Bank-relative start of the bank inside its section.
    pub start: usize,
    /// Bank extent in bytes.
    pub size: usize,
    pub pointer_bytes: usize,
    /// The original offset table, re-emitted verbatim.
    pub offsets: Vec<u16>,
    pub rows: Vec<DialogueRow>,
    /// Every byte from the end of the offset table to the end of the bank.
    pub body: Vec<u8>,
}

impl Bank {
    pub fn used(&self) -> usize {
        self.rows.iter().filter(|row| !row.is_empty()).count()
    }
}

/// A parsed dialogue subfile: one or more pointer banks.
#[derive(Clone, Debug)]
pub struct Section {
    /// Bytes before the first bank. Empty for a single-bank subfile; an 8-byte
    /// header (`u32 8`, `u32 block1_start`) for a two-bank one.
    pub prefix: Vec<u8>,
    pub banks: Vec<Bank>,
    pub size: usize,
}

impl Section {
    pub fn rows(&self) -> usize {
        self.banks.iter().map(|bank| bank.rows.len()).sum()
    }

    pub fn used(&self) -> usize {
        self.banks.iter().map(Bank::used).sum()
    }
}

/// The generic command model: a control byte plus its operand bytes.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Command {
    code: u8,
    operands: Vec<u8>,
}

impl Command {
    /// Build a command from its control byte and operands.
    pub fn new(code: u8, operands: &[u8]) -> Self {
        Command {
            code,
            operands: operands.to_vec(),
        }
    }

    pub fn operands(&self) -> &[u8] {
        &self.operands
    }

    /// The wire bytes of this command: the control byte followed by its operands.
    pub fn bytes(&self) -> Vec<u8> {
        let mut output = Vec::with_capacity(1 + self.operands.len());
        output.push(self.code);
        output.extend_from_slice(&self.operands);
        output
    }
}

/// A defined command class. Each class names the control byte it owns and
/// inherits the generic [`Command`] model for its operands.
pub trait CommandClass {
    const CODE: u8;
    const NAME: &'static str;
    const OPERANDS: usize;
}

macro_rules! command_class {
    ($name:ident, $code:expr, $label:expr, $operands:expr) => {
        pub struct $name;
        impl CommandClass for $name {
            const CODE: u8 = $code;
            const NAME: &'static str = $label;
            const OPERANDS: usize = $operands;
        }
    };
}

// Only codes with a US occurrence are modelled; `0x08`, `0x09` and `0x20` are
// control bytes in other builds but occur zero times in US data, so they stay
// opaque (`{byte(0xNN)}`) rather than asserting an unverified operand width.
command_class!(End, 0x00, "end", 0);
command_class!(LineBreak, 0x01, "nl", 0);
command_class!(Clear, 0x02, "clear", 0);
command_class!(Player, 0x03, "player", 0);
command_class!(SpeakerName, 0x04, "name", 1);
command_class!(SetColor, 0x05, "color", 1);
command_class!(ColorReset, 0x06, "/color", 0);
command_class!(Item, 0x07, "item", 1);
command_class!(Sound, 0x0a, "sound", 1);
command_class!(Pause, 0x0b, "pause", 0);
command_class!(BoxPosition, 0x0c, "pos", 1);
command_class!(TextAnimation, 0x0d, "anim", 0);
command_class!(Effect, 0x0e, "effect", 1);
command_class!(Rumble, 0x0f, "rumble", 2);
command_class!(Fast, 0x10, "fast", 0);
command_class!(FastEnd, 0x11, "/fast", 0);
command_class!(Choice, 0x14, "choice", 3);
command_class!(Time, 0x16, "time", 1);

/// One row of the command registry: the serialized view of a command class.
#[derive(Clone, Copy, Debug, Serialize)]
pub struct CommandDef {
    pub code: u8,
    pub name: &'static str,
    pub operands: usize,
    /// When the first operand equals the given byte, this many extra operand
    /// bytes follow. Used by `0x0E`, whose width depends on its operand.
    pub extra_when: Option<(u8, usize)>,
}

const fn defined(
    code: u8,
    name: &'static str,
    operands: usize,
    extra_when: Option<(u8, usize)>,
) -> CommandDef {
    CommandDef {
        code,
        name,
        operands,
        extra_when,
    }
}

/// The verified command registry, in code order.
pub static COMMAND_CLASSES: &[CommandDef] = &[
    defined(End::CODE, End::NAME, End::OPERANDS, None),
    defined(LineBreak::CODE, LineBreak::NAME, LineBreak::OPERANDS, None),
    defined(Clear::CODE, Clear::NAME, Clear::OPERANDS, None),
    defined(Player::CODE, Player::NAME, Player::OPERANDS, None),
    defined(
        SpeakerName::CODE,
        SpeakerName::NAME,
        SpeakerName::OPERANDS,
        None,
    ),
    defined(SetColor::CODE, SetColor::NAME, SetColor::OPERANDS, None),
    defined(
        ColorReset::CODE,
        ColorReset::NAME,
        ColorReset::OPERANDS,
        None,
    ),
    defined(Item::CODE, Item::NAME, Item::OPERANDS, None),
    defined(Sound::CODE, Sound::NAME, Sound::OPERANDS, None),
    defined(Pause::CODE, Pause::NAME, Pause::OPERANDS, None),
    defined(
        BoxPosition::CODE,
        BoxPosition::NAME,
        BoxPosition::OPERANDS,
        None,
    ),
    defined(
        TextAnimation::CODE,
        TextAnimation::NAME,
        TextAnimation::OPERANDS,
        None,
    ),
    defined(
        Effect::CODE,
        Effect::NAME,
        Effect::OPERANDS,
        Some((0x0f, 1)),
    ),
    defined(Rumble::CODE, Rumble::NAME, Rumble::OPERANDS, None),
    defined(Fast::CODE, Fast::NAME, Fast::OPERANDS, None),
    defined(FastEnd::CODE, FastEnd::NAME, FastEnd::OPERANDS, None),
    defined(Choice::CODE, Choice::NAME, Choice::OPERANDS, None),
    defined(Time::CODE, Time::NAME, Time::OPERANDS, None),
];

pub fn command_for(code: u8) -> Option<&'static CommandDef> {
    COMMAND_CLASSES.iter().find(|class| class.code == code)
}

pub fn command_named(name: &str) -> Option<&'static CommandDef> {
    COMMAND_CLASSES.iter().find(|class| class.name == name)
}

impl CommandDef {
    /// Operand bytes this command occupies, given its first operand.
    pub fn width(&self, first: Option<u8>) -> usize {
        match (self.extra_when, first) {
            (Some((expected, extra)), Some(value)) if value == expected => self.operands + extra,
            _ => self.operands,
        }
    }
}

/// Literal byte/character pairs, excluding the alphanumeric range.
pub const LITERALS: &[(u8, char)] = &[
    (0xff, ' '),
    (0x3a, '('),
    (0x3b, ')'),
    (0x3c, ','),
    (0x3d, '-'),
    (0x3e, '.'),
    (0x3f, '/'),
    (0x40, '='),
    (0x5c, '?'),
    (0x5d, '!'),
    (0x8e, '\''),
    (0x8f, ':'),
    (0x90, '"'),
    (0x91, ';'),
    (0x93, '%'),
    (0x5b, '…'),
    (0x5e, '♥'),
    (0x5f, '♪'),
    (0x60, 'Ƶ'),
    (0x7b, '↑'),
    (0x7c, '↓'),
    (0x7d, '←'),
    (0x7e, '→'),
    (0x7f, '〜'),
    (0x80, '○'),
    (0x81, '△'),
    (0x82, '×'),
    (0x83, '□'),
    (0x84, '★'),
    (0x85, '►'),
    (0x86, '↖'),
    (0x87, '↘'),
    (0x88, '↗'),
    (0x89, '↙'),
    (0x8a, '©'),
    (0x8d, '&'),
    (0x92, '•'),
];

/// The character a literal byte maps to, if any.
pub fn literal_char(byte: u8) -> Option<char> {
    if byte.is_ascii_alphanumeric() {
        return Some(char::from(byte));
    }
    LITERALS
        .iter()
        .find(|(value, _)| *value == byte)
        .map(|(_, character)| *character)
}

/// The byte a literal character maps to, if any.
pub fn literal_byte(character: char) -> Option<u8> {
    if character.is_ascii_alphanumeric() {
        return Some(character as u8);
    }
    LITERALS
        .iter()
        .find(|(_, value)| *value == character)
        .map(|(byte, _)| *byte)
}

/// The kind of text a source holds. This is the single owner of class identity:
/// new classes are added here and nowhere else.
#[derive(Clone, Copy, Debug, Eq, PartialEq, Hash, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum TextClass {
    /// Single-bank pointer table: area dialogue.
    Dialogue,
    /// Two-bank system/battle message table.
    Battle,
    /// Heuristically latched raw text: a lead, **not** consumer-verified text.
    Heuristic,
}

impl TextClass {
    /// The class a text-bank load argument selects, if any.
    pub fn from_load_argument(load_argument: u32) -> Option<Self> {
        match load_argument {
            0x8001_0000 => Some(TextClass::Dialogue),
            0x8001_a000 => Some(TextClass::Battle),
            _ => None,
        }
    }

    /// The load address this class is banked at, when it is a banked class.
    pub fn load_address(self) -> Option<u32> {
        match self {
            TextClass::Dialogue => Some(0x8001_0000),
            TextClass::Battle => Some(0x8001_a000),
            TextClass::Heuristic => None,
        }
    }

    pub fn name(self) -> &'static str {
        match self {
            TextClass::Dialogue => "dialogue",
            TextClass::Battle => "battle",
            TextClass::Heuristic => "heuristic",
        }
    }

    /// Every text class. The CLI derives its accepted `--class` values, and therefore its
    /// help and its validation, from this list, so adding a variant cannot leave stale help
    /// text behind.
    pub const ALL: &'static [TextClass] =
        &[TextClass::Dialogue, TextClass::Battle, TextClass::Heuristic];

    /// Parse a class selector (CLI `--class`).
    pub fn parse(value: &str) -> Option<Self> {
        match value.trim().to_ascii_lowercase().as_str() {
            "dialogue" => Some(TextClass::Dialogue),
            "battle" => Some(TextClass::Battle),
            "heuristic" | "raw" => Some(TextClass::Heuristic),
            _ => None,
        }
    }
}

impl std::fmt::Display for TextClass {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(self.name())
    }
}

/// Where a text object came from: `PATH/ARCHIVE.EMI[#ENTRY]@0xADDRESS`.
#[derive(Clone, Debug, Eq, PartialEq, Serialize, Deserialize)]
pub struct Source {
    /// Repository-relative archive path.
    pub archive: String,
    /// TOC entry index; omitted when the archive holds a single text subfile.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub entry: Option<usize>,
    /// RAM address the subfile loads at (the TOC load argument).
    pub address: u32,
    /// The class this source belongs to.
    pub class: TextClass,
}

impl Source {
    pub fn new(
        archive: impl Into<String>,
        entry: Option<usize>,
        address: u32,
        class: TextClass,
    ) -> Self {
        Source {
            archive: archive.into(),
            entry,
            address,
            class,
        }
    }
}

impl std::fmt::Display for Source {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.archive)?;
        if let Some(entry) = self.entry {
            write!(f, "#{entry}")?;
        }
        write!(f, "@0x{:08X}", self.address)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn every_class_is_offered_to_the_cli() {
        let all = [TextClass::Dialogue, TextClass::Battle, TextClass::Heuristic];
        for class in all {
            // This match is exhaustive on purpose: adding a variant stops this test
            // compiling until the variant is listed here, and the assertions below then
            // require it to be in `ALL` and parseable. Together they make the CLI's
            // accepted set unable to drift from the enum unnoticed.
            match class {
                TextClass::Dialogue | TextClass::Battle | TextClass::Heuristic => {}
            }
            assert!(
                TextClass::ALL.contains(&class),
                "{} is missing from TextClass::ALL",
                class.name()
            );
            assert!(
                TextClass::parse(class.name()).is_some(),
                "{} is not accepted by TextClass::parse",
                class.name()
            );
        }
        assert_eq!(
            TextClass::ALL.len(),
            all.len(),
            "TextClass::ALL and the enum disagree"
        );
    }
}

// --- filter contracts --------------------------------------------------------------------------

/// What a pre-filter is told about one subfile before any candidate is latched from it.
///
/// `archive` and `class` are read by the payload filter that the inventory adds; the window barrier
/// needs only the payload and the registry key, so they are declared here rather than added later.
#[allow(dead_code)]
pub struct PreFilterInput<'a> {
    /// Corpus-relative archive name, which is the key the window registry is stored under.
    pub archive: &'a str,
    /// The subfile's index within its archive.
    pub entry: usize,
    /// The subfile's payload bytes.
    pub payload: &'a [u8],
    /// The subfile's class, when its load argument names one.
    pub class: Option<TextClass>,
    /// The payloads **before this one in the same container**, in order. The VAB body is identified by
    /// pairing with a header anywhere earlier in the container — the sequence data often sits between
    /// the two — so the filter needs the container's context, not just the immediate predecessor.
    pub preceding: &'a [&'a [u8]],
}

/// A byte range a pre-filter refuses to latch, and why.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Refusal {
    /// Start of the refused range within the payload.
    pub start: usize,
    /// Length of the refused range.
    pub length: usize,
    /// Which of the subfile's recorded windows refused it, when the refusal is window-owned; the
    /// registry list position, so the refusal can be named back to its owning record.
    pub window: Option<usize>,
    /// The inventory family that refused it, when the refusal came from the payload inventory.
    pub family: Option<String>,
    /// The signature that identified the payload, when the refusal came from the inventory.
    pub magic: Option<String>,
    /// Candidates this refusal removed, when the refusing family recorded a measurement. A payload is
    /// never parsed, so this is a recorded figure rather than something the current run observed.
    pub recorded_candidates: usize,
    /// How that candidate count was obtained.
    pub basis: String,
    /// Why the range is refused, in words a reader can check.
    pub reason: String,
}

/// What a pre-filter returns for one subfile: the ranges that may be latched, and what it refused.
#[derive(Clone, Debug, Default)]
pub struct AdmittedRanges {
    /// Ranges that may be latched, in order.
    pub ranges: Vec<(usize, usize)>,
    /// Ranges that may not, each with its reason.
    pub refusals: Vec<Refusal>,
}

/// Decides which byte ranges of a subfile may be latched at all.
///
/// A pre-filter runs before the scanner sees the bytes, so a range it refuses is never parsed or
/// latched. It answers with ranges rather than a yes/no because refusing part of a subfile is the
/// normal case: a classified window is a barrier inside an otherwise searchable payload.
pub trait PreFilter {
    fn admit(&self, input: &PreFilterInput<'_>) -> AdmittedRanges;
}

/// Measurements a post-filter judges one candidate run by.
///
/// `encoded`, `unknown` and `characters` are read by the readability filter; the scanner floors read
/// the rest, so the whole measurement set is declared here in one place.
#[allow(dead_code)]
#[derive(Clone, Copy, Debug)]
pub struct PostFilterInput<'a> {
    /// The candidate's encoded bytes.
    pub encoded: &'a [u8],
    /// Bytes in the candidate.
    pub length: usize,
    /// Bytes that decode through the character or command tables.
    pub printable: usize,
    /// Bytes that do not.
    pub unknown: usize,
    /// Distinct byte values seen.
    pub distinct: usize,
    /// Decoded literal characters, commands excluded.
    pub characters: usize,
    /// Script letters in the decoded run.
    pub alpha: usize,
    /// The run's decoded literal text, command tokens stripped. The readability rule needs words,
    /// which only the literal text carries.
    pub literal: &'a str,
    /// The caller's minimum run length. It is an input rather than a constant because a caller may
    /// ask for a shorter run than the default floor.
    pub min_run: usize,
}

/// Decides whether a produced candidate run is reported at all.
pub trait PostFilter {
    /// True when the candidate may be reported.
    fn keeps(&self, candidate: &PostFilterInput<'_>) -> bool;
}

/// What excluding one identified payload family removed, so an exclusion is never silent.
#[derive(Clone, Debug, Default, Deserialize, Serialize)]
pub struct PayloadRemoval {
    /// The inventory's family name, e.g. `audio` or `music`.
    pub family: String,
    /// The signature that identified it.
    pub magic: String,
    /// Candidates the exclusion dropped from the naive scan.
    pub candidates_removed: usize,
    /// Bytes the exclusion removed from searching.
    pub bytes_skipped: usize,
    /// How the candidate count was obtained. The tool never parses an excluded payload, so the count
    /// is a recorded measurement rather than something this run observed.
    #[serde(default)]
    pub basis: String,
}
