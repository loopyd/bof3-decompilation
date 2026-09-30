//! Bind VAB identities, decoded samples and explicit rendering contexts to SF2.
//! Gain/pan contexts are required inputs, never guessed from unverified fields.

use crate::{
    bank::Bank, bank::Program, bank::Tone, codec::adpcm, codec::adpcm::Termination,
    digest::sha256_hex, soundfont, soundfont::envelope::Fit, soundfont::gain,
    soundfont::Instrument, soundfont::LoopMode, soundfont::Preset, soundfont::Sample,
    soundfont::Zone, voice::tuning::KeyTuning, voice::tuning::Reference, Result,
};
use serde::Serialize;
use std::collections::{BTreeMap, BTreeSet};

#[derive(Clone, Debug, Serialize)]
pub struct Identity {
    pub source: String,
    pub header_entry: usize,
    pub body_entry: usize,
    pub game_bank_id: u32,
}

#[derive(Clone, Debug, Serialize)]
pub struct Gain {
    pub attenuation_cb: u16,
    pub pan: i16,
    /// Source/context supplied by the caller; not an automatic evidence approval.
    pub provenance: String,
    pub fit: Option<gain::Fit>,
}

impl Gain {
    /// Bake bank/program/tone controls at a stated ordinary-sequence reference.
    /// Channel controls and velocity remain at unity/center; their dynamic
    /// SoundFont behavior is an approximation, not covered by this static fit.
    pub fn from_bank(
        bank: &Bank,
        program: &Program,
        tone: &Tone,
        model: gain::Model,
    ) -> Result<Self> {
        let fit = gain::fit(gain_context(bank, program, tone), model)?;
        let parameters = fit.parameters.unwrap_or(gain::Parameters {
            attenuation_cb: 0,
            pan: 0,
        });
        Ok(Self {
            attenuation_cb: parameters.attenuation_cb,
            pan: parameters.pan,
            provenance: "Verified US ordinary note-on volume arithmetic; bank/program/tone fields at unity sequence/channel controls; see gain-fit report".into(),
            fit: Some(fit),
        })
    }

    pub fn is_silent(&self) -> bool {
        self.fit
            .as_ref()
            .is_some_and(|fit| fit.parameters.is_none())
    }
}

fn gain_context(bank: &Bank, program: &Program, tone: &Tone) -> crate::voice::gain::Context {
    crate::voice::gain::Context {
        bank_volume: bank.volume,
        program_volume: program.volume,
        tone_volume: tone.volume,
        velocity: 127,
        channel_volume: 127,
        sequence_volume: [127, 127],
        tone_pan: tone.pan,
        program_pan: program.pan,
        channel_pan: 64,
        mono: false,
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct ReverbApproximation {
    pub send_tenths_percent: u16,
    pub provenance: String,
}

#[derive(Clone, Debug, Serialize)]
pub struct ToneContext {
    pub program: u8,
    pub tone: usize,
    pub gain: Gain,
    pub envelope: Fit,
    pub tuning: Vec<KeyTuning>,
    pub pitch_provenance: String,
}

impl ToneContext {
    pub fn from_reference(
        reference: &mut Reference,
        program: u8,
        tone: &Tone,
        sample_rate: u32,
        gain: Gain,
        envelope: Fit,
    ) -> Result<Self> {
        let tuning = reference.tone(tone, sample_rate)?;
        Ok(Self {
            program,
            tone: tone.index,
            gain,
            envelope,
            tuning,
            pitch_provenance: format!(
                "{} SHA256={} entry={:#010x}; isolated original note-on routine, not full runtime",
                reference.profile().target,
                reference.profile().exe_sha256,
                reference.profile().pitch_entry
            ),
        })
    }
}

pub struct Options {
    pub sf2_bank: u16,
    /// Add an explicit SF2 percussion-bank alias for MIDI channel 10 playback.
    pub percussion_alias: bool,
    pub sample_rate: u32,
    pub allow_predictor_loop_approximation: bool,
    pub allow_stopped_pitch_approximation: bool,
    /// Required for tone mode 4; no send level is inferred from its enable bit.
    pub reverb: Option<ReverbApproximation>,
}

#[derive(Clone, Debug, Serialize)]
pub struct SampleBinding {
    pub sample_id: u16,
    pub body_offset: usize,
    pub encoded_bytes: usize,
    pub sha256: String,
    pub sf2_sample: Option<usize>,
    pub pcm_frames: usize,
    pub loop_range: Option<(usize, usize)>,
    pub predictor_loop_approximation: bool,
    pub trailing_bytes: usize,
}

#[derive(Clone, Debug, Serialize)]
pub struct ToneBinding {
    pub source_program: u8,
    pub source_tone: Tone,
    pub sample_resolution: crate::sample::reference::Resolution,
    pub sf2_instrument: usize,
    /// Original decoded PCM remains in the SF2 even for a silent playback zone.
    pub source_sf2_sample: usize,
    pub sf2_sample: usize,
    pub silent: bool,
    /// These keys use the shared silence sample only for their initial zero step.
    #[serde(skip_serializing_if = "Vec::is_empty")]
    pub stopped_keys: Vec<u8>,
    pub zone_count: usize,
    pub context: ToneContext,
}

#[derive(Clone, Debug, Serialize)]
pub struct ProgramBinding {
    pub source_program: u8,
    pub source_tone_block: usize,
    pub sf2_bank: u16,
    pub sf2_program: u8,
    pub sf2_preset: usize,
    pub percussion_preset: Option<usize>,
}

#[derive(Clone, Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub identity: Identity,
    pub header_sha256: String,
    pub body_sha256: String,
    pub source_metadata: Bank,
    pub samples: Vec<SampleBinding>,
    pub tones: Vec<ToneBinding>,
    pub programs: Vec<ProgramBinding>,
    #[serde(skip_serializing_if = "Vec::is_empty")]
    pub empty_programs: Vec<crate::soundfont::silence::Program>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub empty_program_instrument: Option<usize>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub empty_program_sample: Option<usize>,
    pub diagnostics: Vec<String>,
    pub reverb_approximation: Option<ReverbApproximation>,
    /// Synthetic playback-only sample; it has no invented game sample ID.
    pub silence_sample: Option<usize>,
    pub limitations: Vec<&'static str>,
}

pub struct Bound {
    pub soundfont: soundfont::Bank,
    pub bytes: Vec<u8>,
    pub report: Report,
}

pub fn bind(
    identity: Identity,
    header: &[u8],
    body: &[u8],
    contexts: &[ToneContext],
    options: &Options,
) -> Result<Bound> {
    if options
        .reverb
        .as_ref()
        .is_some_and(|r| r.send_tenths_percent > 1000 || r.provenance.trim().is_empty())
    {
        return Err("bank SF2: reverb approximation requires a 0..1000 send and provenance".into());
    }
    if identity.source.is_empty() || identity.header_entry == identity.body_entry {
        return Err("bank SF2: qualified source and distinct VH/VB entries required".into());
    }
    if options.sf2_bank > 128
        || (options.percussion_alias && options.sf2_bank != 0)
        || options.sample_rate == 0
        || options.sample_rate > i32::MAX as u32
    {
        return Err(
            "bank SF2: bank must be 0..128, percussion alias requires bank 0, and sample rate must be positive/representable"
                .into(),
        );
    }
    let metadata = Bank::parse(header)?;
    if metadata.programs.len() != usize::from(metadata.declared_programs)
        || metadata
            .programs
            .iter()
            .map(|p| p.tones.len())
            .sum::<usize>()
            != usize::from(metadata.declared_tones)
    {
        return Err("bank SF2: inconsistent VAB program/tone counts".into());
    }
    let mut by_tone = BTreeMap::new();
    for context in contexts {
        if by_tone
            .insert((context.program, context.tone), context)
            .is_some()
        {
            return Err(format!(
                "bank SF2: duplicate context for program {} tone {}",
                context.program, context.tone
            )
            .into());
        }
    }
    let mut report = Report { schema: "bof3.audio.bank-sf2/v1", identity,
        header_sha256: sha256_hex(header), body_sha256: sha256_hex(body), source_metadata: metadata.clone(),
        samples: Vec::new(), tones: Vec::new(), programs: Vec::new(), diagnostics: metadata.diagnostics.clone(),
        empty_programs: Vec::new(), empty_program_instrument: None, empty_program_sample: None,
        reverb_approximation: options.reverb.clone(),
        silence_sample: None,
        limitations: vec![
            "Gain/pan are explicit caller contexts; this binder does not verify their relationship to game bank/program/tone controls.",
            "ADSR fits approximate the selected model over recorded probes; gameplay overrides and measured PC/PSX equivalence remain open.",
            "Note-on pitch is calibrated per key; tone-specific bend, existing-voice bend and new-note bend behavior are not implemented here.",
            "SF2 voice allocation does not reproduce PSX priorities/stealing. Only tone modes 0/4 are mapped; nonzero vibrato/portamento fields are rejected.",
            "Program mode is retained but unread by the verified US ordinary note-on path; other runtime paths remain outside this mapping.",
            "Tone mode 4 enables reverb routing. Its SF2 send requires an explicit approximation; PSX reverb preset/depth are not recovered from this bit. Set MIDI CC91=0 for per-tone routing without an extra channel send.",
            "Full original EMI/VH/VB bytes still belong in preservation XML; this authored SoundFont cannot reconstruct archive layout alone.",
            "Silent tones use a labeled zero-PCM playback sample while retaining original PCM and source/playback sample indices. Unmuting requires relinking to source PCM; gain edits alone cannot restore a zero sample.",
        ] };
    let mut font = soundfont::Bank {
        name: format!("BOF3 bank {}", report.identity.game_bank_id),
        samples: Vec::new(),
        instruments: Vec::new(),
        presets: Vec::new(),
    };
    for sample in &metadata.samples {
        let end = sample
            .body_offset
            .checked_add(sample.encoded_bytes)
            .ok_or("bank SF2: sample range overflow")?;
        let encoded = body.get(sample.body_offset..end).ok_or_else(|| {
            format!(
                "bank SF2 sample {}: body allocation out of bounds",
                sample.sample_id
            )
        })?;
        let decoded = adpcm::decode_sample(encoded)?;
        if decoded.termination == Termination::BoundedWithoutEnd {
            return Err(format!(
                "bank SF2 sample {}: missing end flag requires runtime continuation context",
                sample.sample_id
            )
            .into());
        }
        let approximate = decoded.sample_loop.is_some_and(|r| !r.pcm_repeat_is_stable);
        if approximate && !options.allow_predictor_loop_approximation {
            return Err(format!("bank SF2 sample {}: fixed PCM loop changes continuing ADPCM history; explicit approximation policy required", sample.sample_id).into());
        }
        let loop_range = decoded
            .sample_loop
            .map(|r| (r.start_frame, r.end_frame_exclusive));
        let sf2_sample = if decoded.pcm.is_empty() {
            None
        } else {
            Some(font.samples.len())
        };
        report.samples.push(SampleBinding {
            sample_id: sample.sample_id,
            body_offset: sample.body_offset,
            encoded_bytes: sample.encoded_bytes,
            sha256: sha256_hex(encoded),
            sf2_sample,
            pcm_frames: decoded.pcm.len(),
            loop_range,
            predictor_loop_approximation: approximate,
            trailing_bytes: decoded.trailing_bytes,
        });
        if sf2_sample.is_some() {
            font.samples.push(Sample {
                name: format!("S{:03}", sample.sample_id),
                pcm: decoded.pcm,
                rate: options.sample_rate,
                root_key: 60,
                correction_cents: 0,
                loop_range: loop_range.map(|(start, end)| start..end),
            });
        }
    }
    for program in &metadata.programs {
        let mut instruments = Vec::new();
        for tone in &program.tones {
            let prefix = format!("bank SF2 program {} tone {}", program.program, tone.index);
            if tone.program_reference != i16::from(program.program)
                || tone.key_min > tone.key_max
                || tone.key_max > 127
            {
                return Err(format!("{prefix}: inconsistent program reference/key range").into());
            }
            if !matches!(tone.mode, 0 | 4)
                || tone.vibrato_width != 0
                || tone.vibrato_time != 0
                || tone.portamento_width != 0
                || tone.portamento_time != 0
            {
                return Err(format!(
                    "{prefix}: mode/vibrato/portamento requires a verified SF2 mapping"
                )
                .into());
            }
            let context = by_tone
                .remove(&(program.program, tone.index))
                .ok_or_else(|| {
                    format!("{prefix}: explicit gain, envelope and tuning context required")
                })?;
            validate_context(
                context,
                &metadata,
                program,
                tone,
                options.sample_rate,
                options.allow_stopped_pitch_approximation,
            )
            .map_err(|e| format!("{prefix}: {e}"))?;
            let reverb_send = if tone.mode == 4 {
                options.reverb.as_ref().ok_or_else(|| format!("{prefix}: reverb-enabled tone mode 4 requires an explicit SF2 reverb approximation"))?.send_tenths_percent
            } else {
                0
            };
            let sample_resolution = crate::sample::reference::resolve_us_pcm(
                tone.sample_reference,
                metadata.declared_samples,
            )
            .map_err(|error| format!("{prefix}: {error}"))?;
            let sample = report
                .samples
                .iter()
                .find(|s| s.sample_id == sample_resolution.sample_id)
                .ok_or_else(|| {
                    format!(
                        "{prefix}: invalid sample reference {}",
                        tone.sample_reference
                    )
                })?;
            let source_sample_index = sample
                .sf2_sample
                .ok_or_else(|| format!("{prefix}: referenced sample has an empty allocation"))?;
            let silent = context.gain.is_silent();
            let sample_index = if silent {
                crate::soundfont::envelope::snapshot::silence(
                    &mut font,
                    &mut report.silence_sample,
                    options.sample_rate,
                )
            } else {
                source_sample_index
            };
            let mut template = Zone::new(sample_index);
            template.keys = (tone.key_min, tone.key_max);
            template.pan = context.gain.pan;
            template.attenuation_cb = context.gain.attenuation_cb;
            template.reverb_send = reverb_send;
            template.envelope = context.envelope.parameters.envelope();
            template.loop_mode = if silent || sample.loop_range.is_some() {
                LoopMode::Continuous
            } else {
                LoopMode::None
            };
            let mut zones = Vec::new();
            let mut stopped_keys = Vec::new();
            for row in &context.tuning {
                let zone = if crate::soundfont::envelope::snapshot::is_known_stop(row) {
                    let first = font.samples[source_sample_index].pcm[0];
                    let silence = crate::soundfont::envelope::snapshot::silence(
                        &mut font,
                        &mut report.silence_sample,
                        options.sample_rate,
                    );
                    stopped_keys.push(row.key);
                    crate::soundfont::envelope::snapshot::zone(row, &template, first, silence)?
                } else {
                    row.zone(&template).map_err(|e| format!("{prefix}: {e}"))?
                };
                zones.push(zone);
            }
            if !stopped_keys.is_empty() {
                report.diagnostics.push(format!("{prefix}: stopped-pitch keys {stopped_keys:?} use silence at key-on. Later pitch changes can activate the PSX source while this SF2 approximation stays silent; original PCM is retained."));
            }
            let instrument = font.instruments.len();
            report.tones.push(ToneBinding {
                source_program: program.program,
                source_tone: tone.clone(),
                sample_resolution,
                sf2_instrument: instrument,
                source_sf2_sample: source_sample_index,
                sf2_sample: sample_index,
                silent,
                stopped_keys,
                zone_count: zones.len(),
                context: context.clone(),
            });
            font.instruments.push(Instrument {
                name: format!("P{:03}T{:02}", program.program, tone.index),
                zones,
            });
            instruments.push(instrument);
        }
        let preset = font.presets.len();
        font.presets.push(Preset {
            name: format!("Bank{} P{:03}", options.sf2_bank, program.program),
            bank: options.sf2_bank,
            program: program.program,
            instruments: instruments.clone(),
        });
        let percussion_preset = if options.percussion_alias {
            let index = font.presets.len();
            font.presets.push(Preset {
                name: format!("Drum{} P{:03}", options.sf2_bank, program.program),
                bank: options.sf2_bank + 128,
                program: program.program,
                instruments,
            });
            Some(index)
        } else {
            None
        };
        report.programs.push(ProgramBinding {
            source_program: program.program,
            source_tone_block: program.tone_block,
            sf2_bank: options.sf2_bank,
            sf2_program: program.program,
            sf2_preset: preset,
            percussion_preset,
        });
    }
    if !by_tone.is_empty() {
        return Err("bank SF2: rendering contexts name nonexistent program/tone identities".into());
    }
    let present = report
        .source_metadata
        .programs
        .iter()
        .map(|p| p.program)
        .collect();
    if let Some(addition) = crate::soundfont::silence::add(
        &mut font,
        &present,
        options.sample_rate,
        options.sf2_bank,
        options.percussion_alias,
    ) {
        report.empty_program_sample = Some(addition.sample);
        report.empty_program_instrument = Some(addition.instrument);
        report.empty_programs.extend(addition.programs);
        report.diagnostics.push("Zero-tone VAB programs use explicit silent presets; original note-on allocates no voice, while SF2 consumers may allocate a short silent voice. Populate a preset by assigning existing VAB tone instruments to both its melodic and percussion aliases.".into());
    }
    let encoded = font.encode()?;
    report.diagnostics.extend(encoded.diagnostics);
    Ok(Bound {
        soundfont: font,
        bytes: encoded.bytes,
        report,
    })
}

fn validate_context(
    context: &ToneContext,
    bank: &Bank,
    program: &Program,
    tone: &Tone,
    sample_rate: u32,
    allow_stopped: bool,
) -> Result<()> {
    if context.gain.provenance.trim().is_empty()
        || context.pitch_provenance.trim().is_empty()
        || context.gain.attenuation_cb > 1440
        || !(-500..=500).contains(&context.gain.pan)
    {
        return Err("invalid or unqualified gain/pan/pitch context".into());
    }
    if let Some(fit) = &context.gain.fit {
        let measured = gain::fit(fit.source, fit.model)?;
        if fit.registers != measured.registers || fit.parameters != measured.parameters {
            return Err("gain fit does not match its source context".into());
        }
        let parameters = fit.parameters.unwrap_or(gain::Parameters {
            attenuation_cb: 0,
            pan: 0,
        });
        if parameters.attenuation_cb != context.gain.attenuation_cb
            || parameters.pan != context.gain.pan
        {
            return Err("gain generators conflict with the retained fit".into());
        }
        if fit.parameters.is_none() && fit.source != gain_context(bank, program, tone) {
            return Err("silent playback requires the bank's maximum ordinary-sequence reference context, not a temporary velocity/controller mute".into());
        }
    }
    if context.envelope.registers.adsr1 != tone.adsr1
        || context.envelope.registers.adsr2 != tone.adsr2
    {
        return Err(
            "envelope fit does not match source ADSR; overrides require explicit runtime mapping"
                .into(),
        );
    }
    let mut keys = BTreeSet::new();
    for row in &context.tuning {
        if row.center != tone.center
            || row.shift != tone.shift
            || row.sf2_sample_rate != sample_rate
            || row.key < tone.key_min
            || row.key > tone.key_max
            || !keys.insert(row.key)
        {
            return Err("inconsistent or duplicate pitch context".into());
        }
        if row.sf2.is_none()
            && !(allow_stopped && crate::soundfont::envelope::snapshot::is_known_stop(row))
        {
            return Err(format!("key {}: {}", row.key, row.unsupported_message()).into());
        }
    }
    if keys.len() != usize::from(tone.key_max - tone.key_min) + 1 {
        return Err("pitch contexts must cover every key; silent key omission is forbidden".into());
    }
    Ok(())
}
