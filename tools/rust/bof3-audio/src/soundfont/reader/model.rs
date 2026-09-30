//! Parsed SF2 hydra records shared by the reader and table parser.

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Generator {
    pub operator: u16,
    pub amount: u16,
}
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Modulator {
    pub source: u16,
    pub destination: u16,
    pub amount: i16,
    pub amount_source: u16,
    pub transform: u16,
}
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Zone {
    pub generators: Vec<Generator>,
    pub modulators: Vec<Modulator>,
    pub target: Option<usize>,
}
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Preset {
    pub name: [u8; 20],
    pub program: u16,
    pub bank: u16,
    pub library: u32,
    pub genre: u32,
    pub morphology: u32,
    pub zones: Vec<Zone>,
}
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Instrument {
    pub name: [u8; 20],
    pub zones: Vec<Zone>,
}
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Sample {
    pub name: [u8; 20],
    pub start: u32,
    pub end: u32,
    pub loop_start: u32,
    pub loop_end: u32,
    pub rate: u32,
    pub root_key: u8,
    pub correction_cents: i8,
    pub link: u16,
    pub kind: u16,
}
