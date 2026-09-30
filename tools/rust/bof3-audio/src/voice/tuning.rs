//! SF2 note-on tuning measured by executing the identified game's pitch routine.
//! This isolates one original routine, not a full sound-runtime boot or PCM oracle.

use crate::{
    bank::Tone,
    digest::sha256_hex,
    machine::{
        bus::{Bus, BusError, Ram, Width},
        cpu::Cpu,
        executable::Executable,
        profile::Profile,
    },
    soundfont::Zone,
    Result,
};
use serde::Serialize;
use std::collections::BTreeMap;

const TONE_CONTEXT: u32 = 0x8001_0000;
const RETURN: u32 = 0x8000_1000;

pub struct Reference {
    profile: Profile,
    ram: Ram,
    load_address: u32,
    text_size: usize,
    registers: BTreeMap<(u8, u8, u8), (u16, Lookup)>,
}

/// A witnessed halfword load, not an arithmetic prediction of the address.
#[derive(Clone, Copy, Debug, Serialize)]
pub struct Lookup {
    pub runtime_address: u32,
    pub source_file_offset: u32,
    pub value: u16,
    pub region: LookupRegion,
    /// Adjacent EXE data can change during initialization and scheduling.
    pub execution_state: &'static str,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum LookupRegion {
    NoteOnPitchTable,
    EarlierSpuPitchTable,
    AdjacentData,
}

impl LookupRegion {
    pub(crate) fn is_pitch_table(self) -> bool {
        matches!(self, Self::NoteOnPitchTable | Self::EarlierSpuPitchTable)
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct KeyTuning {
    pub key: u8,
    pub center: u8,
    pub shift: u8,
    /// Original routine's unmodified, low-16-bit return value.
    pub pitch_register: u16,
    /// Unmodulated SPU step; PMON and noise are not represented by this mapping.
    pub effective_step: u16,
    pub sf2_sample_rate: u32,
    /// Set only when measured through Reference; out-of-table reads are not
    /// accepted as stable initialized-game state merely because EXE RAM is readable.
    pub pitch_table_index: Option<i32>,
    pub pitch_lookup: Option<Lookup>,
    pub sf2: Option<Parameters>,
    pub unsupported: Option<&'static str>,
}

#[derive(Clone, Debug, Serialize)]
pub struct Parameters {
    pub root_key: u8,
    pub coarse_tune: i16,
    pub fine_tune: i16,
    pub target_pcm_rate: f64,
    pub sf2_pcm_rate: f64,
    /// SF2 minus unmodulated SPU source-rate pitch, in cents.
    pub error_cents: f64,
}

impl Reference {
    pub fn from_executable(exe: &Executable) -> Result<Self> {
        let profile = Profile::identify(exe)?;
        let mut ram = Ram::from_executable(exe);
        for address in [profile.pitch_table, profile.spu_pitch_table] {
            let size = profile.pitch_table_entries * 2;
            let offset = crate::machine::executable::ram_offset(address, size)?;
            if sha256_hex(&ram.bytes()[offset..offset + size])
                != crate::machine::profile::US_PITCH_SHA256
            {
                return Err("tuning: verified pitch-table identity changed".into());
            }
        }
        // Select physical tone block 0 / tone 0 in the original pitch routine's
        // globals. All other memory, including code/table bytes, is from the EXE.
        ram.write(0x8018_e7df, Width::Byte, 0)?;
        ram.write(0x8018_e7e4, Width::Byte, 0)?;
        ram.write(0x8018_e25c, Width::Word, TONE_CONTEXT)?;
        Ok(Self {
            profile,
            ram,
            load_address: exe.header().load_address,
            text_size: exe.header().text_size,
            registers: BTreeMap::new(),
        })
    }

    pub fn profile(&self) -> &Profile {
        &self.profile
    }

    /// Initial note-on uses fine argument zero. Channel bend is a separate path.
    pub fn pitch_register(&mut self, key: u8, center: u8, shift: u8) -> Result<u16> {
        Ok(self.measure(key, center, shift)?.0)
    }

    fn measure(&mut self, key: u8, center: u8, shift: u8) -> Result<(u16, Lookup)> {
        if key > 127 {
            return Err("tuning: note-on key must be seven-bit".into());
        }
        let identity = (key, center, shift);
        if let Some(&value) = self.registers.get(&identity) {
            return Ok(value);
        }
        let value = self.execute(key, center, shift)?;
        self.registers.insert(identity, value);
        Ok(value)
    }

    fn execute(&mut self, key: u8, center: u8, shift: u8) -> Result<(u16, Lookup)> {
        if key > 127 {
            return Err("tuning: note-on key must be seven-bit".into());
        }
        self.ram
            .write(TONE_CONTEXT + 4, Width::Byte, u32::from(center))?;
        self.ram
            .write(TONE_CONTEXT + 5, Width::Byte, u32::from(shift))?;
        let mut cpu = Cpu::new(self.profile.pitch_entry);
        cpu.set_register(4, u32::from(key));
        cpu.set_register(5, 0);
        cpu.set_register(31, RETURN);
        let mut observed = Observed {
            ram: &mut self.ram,
            reads: 0,
            lookup: None,
        };
        cpu.run_until(&mut observed, RETURN, 100).map_err(|e| {
            format!("tuning key={key} center={center} shift={shift}: original pitch execution failed: {e}")
        })?;
        let value = u16::try_from(cpu.register(2))
            .map_err(|_| "tuning: original pitch return exceeds verified 16-bit contract")?;
        if observed.reads != 1 {
            return Err("tuning: original routine must perform exactly one halfword lookup".into());
        }
        let (address, table_value) = observed.lookup.ok_or("tuning: missing lookup witness")?;
        let offset = address
            .checked_sub(self.load_address)
            .filter(|offset| (*offset as usize) + 2 <= self.text_size)
            .ok_or("tuning: lookup is outside supplied executable data")?;
        let table_bytes = self.profile.pitch_table_entries as u32 * 2;
        let region = if (self.profile.pitch_table..self.profile.pitch_table + table_bytes)
            .contains(&address)
        {
            LookupRegion::NoteOnPitchTable
        } else if (self.profile.spu_pitch_table..self.profile.spu_pitch_table + table_bytes)
            .contains(&address)
        {
            LookupRegion::EarlierSpuPitchTable
        } else {
            LookupRegion::AdjacentData
        };
        let lookup = Lookup {
            runtime_address: address,
            source_file_offset: offset + 0x800,
            value: table_value,
            region,
            execution_state: "isolated_executable",
        };
        Ok((value, lookup))
    }

    pub fn tone(&mut self, tone: &Tone, sample_rate: u32) -> Result<Vec<KeyTuning>> {
        if tone.key_min > tone.key_max || tone.key_max > 127 {
            return Err(format!("tuning tone {}: invalid key range", tone.index).into());
        }
        (tone.key_min..=tone.key_max)
            .map(|key| self.key(key, tone.center, tone.shift, sample_rate))
            .collect()
    }

    pub fn key(&mut self, key: u8, center: u8, shift: u8, sample_rate: u32) -> Result<KeyTuning> {
        let (register, lookup) = self.measure(key, center, shift)?;
        self.map_key(key, center, shift, sample_rate, register, lookup)
    }

    /// Search probes must not grow the retained export cache with rejected edits.
    pub(crate) fn probe_key(
        &mut self,
        key: u8,
        center: u8,
        shift: u8,
        sample_rate: u32,
    ) -> Result<KeyTuning> {
        let (register, lookup) = self.execute(key, center, shift)?;
        self.map_key(key, center, shift, sample_rate, register, lookup)
    }

    fn map_key(
        &self,
        key: u8,
        center: u8,
        shift: u8,
        sample_rate: u32,
        register: u16,
        lookup: Lookup,
    ) -> Result<KeyTuning> {
        let mut row = KeyTuning::from_register(key, center, shift, register, sample_rate)?;
        let index =
            ((i64::from(lookup.runtime_address) - i64::from(self.profile.pitch_table)) / 2) as i32;
        row.pitch_table_index = Some(index);
        row.pitch_lookup = Some(lookup);
        if !lookup.region.is_pitch_table() {
            row.sf2 = None;
            row.unsupported = Some("pitch routine reads outside verified pitch table; initialized game data context is required");
        }
        Ok(row)
    }
}

impl KeyTuning {
    /// Convert an explicitly supplied register context; callers must establish
    /// its provenance. SF2 sample-header pitch correction must be zero.
    pub fn from_register(
        key: u8,
        center: u8,
        shift: u8,
        pitch_register: u16,
        sample_rate: u32,
    ) -> Result<Self> {
        if key > 127 || sample_rate == 0 || sample_rate > i32::MAX as u32 {
            return Err("tuning: invalid MIDI key or SF2 sample rate".into());
        }
        let effective_step = pitch_register.min(0x4000);
        let mut row = Self {
            key,
            center,
            shift,
            pitch_register,
            effective_step,
            sf2_sample_rate: sample_rate,
            pitch_table_index: None,
            pitch_lookup: None,
            sf2: None,
            unsupported: None,
        };
        if effective_step == 0 {
            row.unsupported = Some("zero SPU step cannot be represented as a positive SF2 sample rate; no silent key omission is authorized");
            return Ok(row);
        }
        let preferred_root = center.min(127);
        let target_pcm_rate = f64::from(effective_step) * 44100.0 / 4096.0;
        let target_cents = 1200.0 * (target_pcm_rate / f64::from(sample_rate)).log2();
        let fit = |root_key: u8| {
            let key_cents = (f64::from(key) - f64::from(root_key)) * 100.0;
            let tune = (target_cents - key_cents).round();
            let coarse = (tune / 100.0).trunc();
            let fine = tune - coarse * 100.0;
            if !(-120.0..=120.0).contains(&coarse) || !(-99.0..=99.0).contains(&fine) {
                return None;
            }
            let sf2_pcm_rate = f64::from(sample_rate) * 2.0f64.powf((key_cents + tune) / 1200.0);
            Some(Parameters {
                root_key,
                coarse_tune: coarse as i16,
                fine_tune: fine as i16,
                target_pcm_rate,
                sf2_pcm_rate,
                error_cents: 1200.0 * (sf2_pcm_rate / target_pcm_rate).log2(),
            })
        };
        // Root/coarse/fine jointly encode pitch. Keep the source center when it
        // fits; otherwise use the nearest legal root without changing combined cents.
        row.sf2 = fit(preferred_root).or_else(|| {
            (0..=127)
                .filter_map(fit)
                .min_by_key(|p| p.root_key.abs_diff(preferred_root))
        });
        if row.sf2.is_none() {
            row.unsupported =
                Some("required SF2 coarse/fine tuning exceeds supported generator range");
        }
        Ok(row)
    }

    pub(crate) fn unsupported_message(&self) -> String {
        let witness = self.pitch_lookup.map_or_else(String::new, |lookup| {
            format!(
                "; lookup 0x{:08X} (EXE offset 0x{:X}, {:?}) = 0x{:04X} in {} state",
                lookup.runtime_address,
                lookup.source_file_offset,
                lookup.region,
                lookup.value,
                lookup.execution_state
            )
        });
        format!(
            "{}{}",
            self.unsupported.unwrap_or("missing SF2 mapping"),
            witness
        )
    }

    /// Make a single-key layer without altering sample, loop, volume or envelope
    /// choices. Merging adjacent equivalent zones is a separate layout operation.
    pub fn zone(&self, template: &Zone) -> Result<Zone> {
        if self.key < template.keys.0 || self.key > template.keys.1 {
            return Err("tuning: key outside the supplied tone's zone range".into());
        }
        let parameters = self
            .sf2
            .as_ref()
            .ok_or_else(|| format!("tuning key {}: {}", self.key, self.unsupported_message()))?;
        let mut zone = template.clone();
        zone.keys = (self.key, self.key);
        zone.root_key = Some(parameters.root_key);
        zone.coarse_tune = parameters.coarse_tune;
        zone.fine_tune = parameters.fine_tune;
        zone.scale_tuning = 100;
        Ok(zone)
    }
}

struct Observed<'a> {
    ram: &'a mut Ram,
    reads: usize,
    lookup: Option<(u32, u16)>,
}

impl Bus for Observed<'_> {
    fn read(&mut self, address: u32, width: Width) -> std::result::Result<u32, BusError> {
        let value = self.ram.read(address, width)?;
        if width == Width::Half {
            self.reads += 1;
            self.lookup = Some((address, value as u16));
        }
        Ok(value)
    }

    fn write(
        &mut self,
        address: u32,
        width: Width,
        value: u32,
    ) -> std::result::Result<(), BusError> {
        self.ram.write(address, width, value)
    }

    fn write_masked(
        &mut self,
        address: u32,
        value: u32,
        lanes: u8,
    ) -> std::result::Result<(), BusError> {
        self.ram.write_masked(address, value, lanes)
    }
}
