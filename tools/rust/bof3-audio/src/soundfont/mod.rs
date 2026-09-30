//! Authored SF2 banks with mono PCM samples. Game-to-SF2 approximation is separate.
//!
//! Uses the Creative/E-mu SoundFont 2.04 specification, sections 4–8, while
//! emitting the 16-bit 2.01 subset. This is not an arbitrary-SF2 preservation reader.

pub mod bank;
pub mod envelope;
pub mod gain;
pub mod packing;
pub mod reader;
pub mod silence;
pub(crate) mod tables;

use crate::Result;
use std::collections::BTreeSet;
use std::ops::Range;

#[derive(Clone, Debug)]
pub struct Sample {
    pub name: String,
    pub pcm: Vec<i16>,
    pub rate: u32,
    pub root_key: u8,
    pub correction_cents: i8,
    /// Sample-relative, exclusive end. Storage is independent of each zone's mode.
    pub loop_range: Option<Range<usize>>,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LoopMode {
    None,
    Continuous,
    UntilRelease,
}

#[derive(Clone, Debug)]
pub struct Envelope {
    /// SF2 timecents; -32768 is the special zero-time value.
    pub delay: i16,
    pub attack: i16,
    pub hold: i16,
    pub decay: i16,
    pub sustain_cb: u16,
    pub release: i16,
}

impl Default for Envelope {
    fn default() -> Self {
        Self {
            delay: -12000,
            attack: -12000,
            hold: -12000,
            decay: -12000,
            sustain_cb: 0,
            release: -12000,
        }
    }
}

#[derive(Clone, Debug)]
pub struct Zone {
    pub sample: usize,
    pub keys: (u8, u8),
    pub velocities: (u8, u8),
    pub root_key: Option<u8>,
    pub coarse_tune: i16,
    pub fine_tune: i16,
    pub scale_tuning: u16,
    /// Tenths of a percent, -500 left to 500 right.
    pub pan: i16,
    pub attenuation_cb: u16,
    /// Tenths of a percent, 0..1000; additive with consumer channel send.
    pub reverb_send: u16,
    pub envelope: Envelope,
    pub loop_mode: LoopMode,
}

impl Zone {
    pub fn new(sample: usize) -> Self {
        Self {
            sample,
            keys: (0, 127),
            velocities: (0, 127),
            root_key: None,
            coarse_tune: 0,
            fine_tune: 0,
            scale_tuning: 100,
            pan: 0,
            attenuation_cb: 0,
            reverb_send: 0,
            envelope: Envelope::default(),
            loop_mode: LoopMode::None,
        }
    }

    fn generators(&self) -> Result<Vec<(u16, u16)>> {
        let mode = match self.loop_mode {
            LoopMode::None => 0,
            LoopMode::Continuous => 1,
            LoopMode::UntilRelease => 3,
        };
        let e = &self.envelope;
        let mut values = vec![
            (43, u16::from_le_bytes([self.keys.0, self.keys.1])),
            (
                44,
                u16::from_le_bytes([self.velocities.0, self.velocities.1]),
            ),
            (17, self.pan as u16),
            (16, self.reverb_send),
            (33, e.delay as u16),
            (34, e.attack as u16),
            (35, e.hold as u16),
            (36, e.decay as u16),
            (37, e.sustain_cb),
            (38, e.release as u16),
            (48, self.attenuation_cb),
            (51, self.coarse_tune as u16),
            (52, self.fine_tune as u16),
            (54, mode),
            (56, self.scale_tuning),
        ];
        if let Some(key) = self.root_key {
            values.push((58, u16::from(key)));
        }
        // The sample link must be last; key/velocity ranges must be first.
        values.push((53, index(self.sample, "sample")?));
        Ok(values)
    }
}

#[derive(Clone, Debug)]
pub struct Instrument {
    pub name: String,
    /// Overlapping zones are intentional layers; never collapse them.
    pub zones: Vec<Zone>,
}

#[derive(Clone, Debug)]
pub struct Preset {
    pub name: String,
    pub bank: u16,
    pub program: u8,
    pub instruments: Vec<usize>,
}

#[derive(Clone, Debug)]
pub struct Bank {
    pub name: String,
    pub samples: Vec<Sample>,
    pub instruments: Vec<Instrument>,
    pub presets: Vec<Preset>,
}

#[derive(Debug)]
pub struct Encoded {
    pub bytes: Vec<u8>,
    /// Optional hardware-portability constraints, not silent sample rewrites.
    pub diagnostics: Vec<String>,
}

impl Bank {
    fn validate(&self) -> Result<Vec<String>> {
        name(&self.name, 255)?;
        names(
            self.samples.iter().map(|x| x.name.as_str()),
            "sample",
            "EOS",
        )?;
        names(
            self.instruments.iter().map(|x| x.name.as_str()),
            "instrument",
            "EOI",
        )?;
        names(
            self.presets.iter().map(|x| x.name.as_str()),
            "preset",
            "EOP",
        )?;
        let mut diagnostics = Vec::new();
        let mut total_points = 0usize;
        for (i, sample) in self.samples.iter().enumerate() {
            let prefix = format!("SF2 sample {i} ({})", sample.name);
            if sample.pcm.is_empty() || sample.rate == 0 || sample.rate > i32::MAX as u32 {
                return Err(format!("{prefix}: empty PCM or invalid sample rate").into());
            }
            if sample.root_key > 127 || !(-99..=99).contains(&sample.correction_cents) {
                return Err(format!("{prefix}: invalid root key or pitch correction").into());
            }
            total_points = total_points
                .checked_add(sample.pcm.len())
                .and_then(|n| n.checked_add(46))
                .ok_or("SF2 sample storage overflow")?;
            if total_points > i32::MAX as usize / 2 {
                return Err("SF2: PCM storage exceeds supported signed 32-bit chunk size".into());
            }
            if !(400..=50000).contains(&sample.rate) || sample.pcm.len() < 48 {
                diagnostics.push(format!(
                    "{prefix}: sample rate or length outside portable SF2 recommendations"
                ));
            }
            if let Some(r) = &sample.loop_range {
                if r.start >= r.end || r.end > sample.pcm.len() {
                    return Err(format!("{prefix}: loop must be nonempty and within PCM").into());
                }
                if r.start < 8 || r.end - r.start < 32 || sample.pcm.len() - r.end < 8 {
                    diagnostics.push(format!("{prefix}: loop lacks recommended 8-point margins or 32-point length; endpoints retained"));
                }
            }
        }
        let mut zone_count = 0usize;
        let mut gen_count = 0usize;
        for (i, instrument) in self.instruments.iter().enumerate() {
            if instrument.zones.is_empty() {
                return Err(format!("SF2 instrument {i}: no zones").into());
            }
            for (j, z) in instrument.zones.iter().enumerate() {
                let prefix = format!("SF2 instrument {i} zone {j}");
                let sample = self
                    .samples
                    .get(z.sample)
                    .ok_or_else(|| format!("{prefix}: sample {} does not exist", z.sample))?;
                for (label, r) in [("key", z.keys), ("velocity", z.velocities)] {
                    if r.0 > r.1 || r.1 > 127 {
                        return Err(format!("{prefix}: invalid {label} range").into());
                    }
                }
                if z.root_key.is_some_and(|k| k > 127)
                    || !(-120..=120).contains(&z.coarse_tune)
                    || !(-99..=99).contains(&z.fine_tune)
                    || z.scale_tuning > 1200
                    || !(-500..=500).contains(&z.pan)
                    || z.attenuation_cb > 1440
                    || z.reverb_send > 1000
                {
                    return Err(format!(
                        "{prefix}: tuning, pan, attenuation or reverb outside SF2 range"
                    )
                    .into());
                }
                if z.loop_mode != LoopMode::None && sample.loop_range.is_none() {
                    return Err(
                        format!("{prefix}: loop mode requires sample loop endpoints").into(),
                    );
                }
                for (label, value, max) in [
                    ("delay", z.envelope.delay, 5000),
                    ("attack", z.envelope.attack, 8000),
                    ("hold", z.envelope.hold, 5000),
                    ("decay", z.envelope.decay, 8000),
                    ("release", z.envelope.release, 8000),
                ] {
                    if value != i16::MIN && !(-12000..=max).contains(&value) {
                        return Err(format!("{prefix}: {label} timecents outside SF2 range").into());
                    }
                }
                if z.envelope.sustain_cb > 1440 {
                    return Err(format!("{prefix}: sustain attenuation outside SF2 range").into());
                }
                zone_count += 1;
                gen_count += z.generators()?.len();
                index(zone_count, "instrument zone count")?;
                index(gen_count, "instrument generator count")?;
            }
        }
        let mut identities = BTreeSet::new();
        let mut preset_zones = 0usize;
        for (i, preset) in self.presets.iter().enumerate() {
            if preset.bank > 128
                || preset.program > 127
                || !identities.insert((preset.bank, preset.program))
            {
                return Err(
                    format!("SF2 preset {i}: invalid or duplicate bank/program identity").into(),
                );
            }
            if preset.instruments.is_empty() {
                return Err(format!("SF2 preset {i}: no instrument zones").into());
            }
            for &instrument in &preset.instruments {
                if instrument >= self.instruments.len() {
                    return Err(
                        format!("SF2 preset {i}: instrument {instrument} does not exist").into(),
                    );
                }
                preset_zones += 1;
                index(preset_zones, "preset zone count")?;
            }
        }
        Ok(diagnostics)
    }

    pub fn encode(&self) -> Result<Encoded> {
        let diagnostics = self.validate()?;
        let mut info = b"INFO".to_vec();
        chunk(&mut info, b"ifil", &[2, 0, 1, 0])?;
        chunk(&mut info, b"isng", b"EMU8000\0")?;
        chunk(&mut info, b"INAM", &zstring(&self.name))?;
        chunk(&mut info, b"ISFT", &zstring("bof3-audio"))?;

        let mut pcm = Vec::new();
        let mut shdr = Vec::new();
        for s in &self.samples {
            let start = (pcm.len() / 2) as u32;
            let end = start + s.pcm.len() as u32;
            fixed_name(&mut shdr, &s.name);
            let (loop_start, loop_end) = s.loop_range.as_ref().map_or((start, start), |r| {
                (start + r.start as u32, start + r.end as u32)
            });
            for value in [start, end, loop_start, loop_end, s.rate] {
                shdr.extend(value.to_le_bytes());
            }
            shdr.extend([s.root_key, s.correction_cents as u8]);
            pair(&mut shdr, 0, 1); // unlinked mono
            for sample in &s.pcm {
                pcm.extend(sample.to_le_bytes());
            }
            pcm.resize(pcm.len() + 92, 0); // 46 zero guard points after every sample
        }
        fixed_name(&mut shdr, "EOS");
        shdr.resize(shdr.len() + 26, 0);
        let mut sdta = b"sdta".to_vec();
        chunk(&mut sdta, b"smpl", &pcm)?;

        let mut inst = Vec::new();
        let mut ibag = Vec::new();
        let mut igen = Vec::new();
        for instrument in &self.instruments {
            fixed_name(&mut inst, &instrument.name);
            inst.extend(index(ibag.len() / 4, "instrument bag")?.to_le_bytes());
            for zone in &instrument.zones {
                pair(&mut ibag, index(igen.len() / 4, "instrument generator")?, 0);
                for (op, amount) in zone.generators()? {
                    pair(&mut igen, op, amount);
                }
            }
        }
        fixed_name(&mut inst, "EOI");
        inst.extend(index(ibag.len() / 4, "terminal instrument bag")?.to_le_bytes());
        pair(
            &mut ibag,
            index(igen.len() / 4, "terminal instrument generator")?,
            0,
        );
        pair(&mut igen, 0, 0);

        let mut phdr = Vec::new();
        let mut pbag = Vec::new();
        let mut pgen = Vec::new();
        for p in &self.presets {
            fixed_name(&mut phdr, &p.name);
            pair(&mut phdr, u16::from(p.program), p.bank);
            phdr.extend(index(pbag.len() / 4, "preset bag")?.to_le_bytes());
            phdr.resize(phdr.len() + 12, 0);
            for &instrument in &p.instruments {
                pair(&mut pbag, index(pgen.len() / 4, "preset generator")?, 0);
                pair(&mut pgen, 41, index(instrument, "instrument")?);
            }
        }
        fixed_name(&mut phdr, "EOP");
        pair(&mut phdr, 0, 0);
        phdr.extend(index(pbag.len() / 4, "terminal preset bag")?.to_le_bytes());
        phdr.resize(phdr.len() + 12, 0);
        pair(
            &mut pbag,
            index(pgen.len() / 4, "terminal preset generator")?,
            0,
        );
        pair(&mut pgen, 0, 0);

        let mut pdta = b"pdta".to_vec();
        for (id, bytes) in [
            (b"phdr", &phdr),
            (b"pbag", &pbag),
            (b"pmod", &vec![0; 10]),
            (b"pgen", &pgen),
            (b"inst", &inst),
            (b"ibag", &ibag),
            (b"imod", &vec![0; 10]),
            (b"igen", &igen),
            (b"shdr", &shdr),
        ] {
            chunk(&mut pdta, id, bytes)?;
        }
        let mut body = b"sfbk".to_vec();
        for list in [info, sdta, pdta] {
            chunk(&mut body, b"LIST", &list)?;
        }
        let mut bytes = Vec::new();
        chunk(&mut bytes, b"RIFF", &body)?;
        Ok(Encoded { bytes, diagnostics })
    }
}

fn index(value: usize, label: &str) -> Result<u16> {
    u16::try_from(value).map_err(|_| format!("SF2: {label} exceeds 16-bit index capacity").into())
}

fn name(value: &str, limit: usize) -> Result<()> {
    if value.is_empty() || value.len() > limit || !value.bytes().all(|b| (32..=126).contains(&b)) {
        return Err(format!("SF2: name must contain 1..={limit} printable ASCII bytes").into());
    }
    Ok(())
}

fn names<'a>(values: impl Iterator<Item = &'a str>, kind: &str, terminal: &str) -> Result<()> {
    let mut seen = BTreeSet::new();
    for value in values {
        name(value, 19).map_err(|e| format!("SF2 {kind}: {e}"))?;
        if value == terminal || !seen.insert(value) {
            return Err(format!("SF2 {kind}: duplicate or reserved name {value:?}").into());
        }
        index(seen.len(), &format!("{kind} count"))?;
    }
    if seen.is_empty() {
        return Err(format!("SF2: no {kind} records").into());
    }
    Ok(())
}

fn fixed_name(output: &mut Vec<u8>, value: &str) {
    output.extend(value.as_bytes());
    output.resize(output.len() + 20 - value.len(), 0);
}

fn zstring(value: &str) -> Vec<u8> {
    let mut bytes = value.as_bytes().to_vec();
    bytes.push(0);
    if !bytes.len().is_multiple_of(2) {
        bytes.push(0);
    }
    bytes
}

fn pair(output: &mut Vec<u8>, a: u16, b: u16) {
    output.extend(a.to_le_bytes());
    output.extend(b.to_le_bytes());
}

fn chunk(output: &mut Vec<u8>, id: &[u8; 4], data: &[u8]) -> Result<()> {
    // RustySynth and several consumers use signed RIFF lengths internally.
    let len = i32::try_from(data.len()).map_err(|_| "SF2 chunk exceeds supported size")?;
    output.extend(id);
    output.extend(len.to_le_bytes());
    output.extend(data);
    if !data.len().is_multiple_of(2) {
        output.push(0);
    }
    Ok(())
}
