//! Invert complete fitted SF2 envelopes through one source ADSR field family.
use crate::{
    bank::Bank,
    machine::adsr::Registers,
    soundfont::{bank as binding, envelope, envelope::Fit, reader::model::Zone, reader::Font},
    Result,
};
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub tones: Vec<ToneReport>,
    pub limitations: Vec<&'static str>,
}

#[derive(Debug, Serialize)]
pub struct ToneReport {
    pub program: u8,
    pub tone: usize,
    pub sf2_instrument: usize,
    pub original_registers: Registers,
    pub matching_candidates: usize,
    pub fit: Fit,
}

pub(crate) struct Packed {
    pub patches: Vec<(usize, [u8; 2])>,
    pub report: Report,
}

/// The enclosing packer validates topology and every other generator before
/// applying these patches. A fit matches quantized generators, not PSX PCM.
pub(crate) fn pack(
    binding: &binding::Report,
    header: &[u8],
    baseline: &Font,
    edited: &Font,
) -> Result<Packed> {
    let bank = Bank::parse(header)?;
    let mut tones = Vec::new();
    let mut patches = Vec::new();
    for tone in &binding.tones {
        let before = baseline
            .instruments
            .get(tone.sf2_instrument)
            .ok_or("SF2 envelope edit: missing baseline instrument")?;
        let after = edited
            .instruments
            .get(tone.sf2_instrument)
            .ok_or("SF2 envelope edit: missing edited instrument")?;
        if before.zones.len() != after.zones.len() {
            return Err("SF2 envelope edit: zone insertion/removal unsupported".into());
        }
        if before
            .zones
            .iter()
            .zip(&after.zones)
            .all(|(a, b)| parameters(a) == parameters(b))
        {
            continue;
        }
        let prefix = format!(
            "SF2 envelope edit: program {} tone {}",
            tone.source_program, tone.source_tone.index
        );
        if tone.silent || !tone.stopped_keys.is_empty() {
            return Err(
                format!("{prefix}: silent and stopped-pitch envelope edits unsupported").into(),
            );
        }
        let program = bank
            .programs
            .iter()
            .find(|p| p.program == tone.source_program)
            .ok_or("SF2 envelope edit: missing source program")?;
        let source = program
            .tones
            .get(tone.source_tone.index)
            .ok_or("SF2 envelope edit: missing source tone")?;
        let original = &tone.context.envelope;
        let registers = Registers {
            adsr1: source.adsr1,
            adsr2: source.adsr2,
        };
        if original.registers != registers {
            return Err(format!("{prefix}: envelope context differs from source registers").into());
        }
        let verified = envelope::fit(registers, original.model, &original.probe)?;
        let expected = fitted(&verified);
        if fitted(original) != expected || before.zones.iter().any(|z| parameters(z) != expected) {
            return Err(format!("{prefix}: baseline differs from regenerated envelope fit").into());
        }
        let requested = after
            .zones
            .first()
            .map(parameters)
            .ok_or("SF2 envelope edit: missing key zones")?;
        if after.zones.iter().any(|z| parameters(z) != requested) {
            return Err(format!("{prefix}: all key zones must request the same envelope").into());
        }
        let (fit, matching_candidates) =
            inverse(original, requested).map_err(|e| format!("{prefix}: {e}"))?;
        let offset = 0x820 + program.tone_block * 512 + source.index * 32;
        patches.extend([
            (offset + 16, fit.registers.adsr1.to_le_bytes()),
            (offset + 18, fit.registers.adsr2.to_le_bytes()),
        ]);
        tones.push(ToneReport {
            program: program.program,
            tone: source.index,
            sf2_instrument: tone.sf2_instrument,
            original_registers: registers,
            matching_candidates,
            fit,
        });
    }
    Ok(Packed { patches, report: Report {
        schema: "bof3.audio.sf2-envelope-pack/v1", tones,
        limitations: vec![
            "Search source fields in order: release shift/mode (64), attack mode/rate (256), decay shift (16), sustain level (16), sustain rate (128), sustain mode/direction (4). Each search preserves every other source bit, including unused ADSR2 bit13. All six SF2 volume-envelope generators must exactly match a complete fit at the recorded model and probe.",
            "All key zones of an audible, non-stopped tone must request the same envelope. Delay/hold changes, unmatched tuples, silent/stopped tones and arbitrary ADSR or topology reconstruction are unsupported.",
            "Within the first matching field family, equivalent candidates minimize raw field distance from the source, then the field value; matching_candidates counts that family only. Combined-field searches are unsupported. Fits retain their approximation error; exact SF2 generators do not establish hardware or PC/PSX PCM equivalence.",
        ],
    }})
}

pub(crate) fn verify(header: &[u8], report: &Report) -> Result<()> {
    let bank = Bank::parse(header)?;
    for item in &report.tones {
        let tone = bank
            .programs
            .iter()
            .find(|p| p.program == item.program)
            .and_then(|p| p.tones.get(item.tone))
            .ok_or("SF2 envelope edit: rebuilt tone missing")?;
        let registers = Registers {
            adsr1: tone.adsr1,
            adsr2: tone.adsr2,
        };
        let actual = envelope::fit(registers, item.fit.model, &item.fit.probe)?;
        if registers != item.fit.registers || fitted(&actual) != fitted(&item.fit) {
            return Err("SF2 envelope edit: rebuilt envelope failed forward verification".into());
        }
    }
    Ok(())
}

fn parameters(zone: &Zone) -> [u16; 6] {
    // SF2 defaults for delay, attack, hold, decay, sustain attenuation, release.
    let defaults = [
        (-12000i16) as u16,
        (-12000i16) as u16,
        (-12000i16) as u16,
        (-12000i16) as u16,
        0,
        (-12000i16) as u16,
    ];
    std::array::from_fn(|i| {
        zone.generators
            .iter()
            .find(|g| g.operator == 33 + i as u16)
            .map_or(defaults[i], |g| g.amount)
    })
}

fn fitted(fit: &Fit) -> [u16; 6] {
    let e = fit.parameters.envelope();
    [
        e.delay as u16,
        e.attack as u16,
        e.hold as u16,
        e.decay as u16,
        e.sustain_cb,
        e.release as u16,
    ]
}

fn inverse(original: &Fit, requested: [u16; 6]) -> Result<(Fit, usize)> {
    for (adsr1, mask, shift) in [
        (false, 0x003f, 0),
        (true, 0xff00, 8),
        (true, 0x00f0, 4),
        (true, 0x000f, 0),
        (false, 0x1fc0, 6),
        (false, 0xc000, 14),
    ] {
        let register = if adsr1 {
            original.registers.adsr1
        } else {
            original.registers.adsr2
        };
        let source = (register & mask) >> shift;
        let mut best = None;
        let mut rank = (u16::MAX, u16::MAX);
        let mut count = 0;
        for value in 0..=(mask >> shift) {
            let mut registers = original.registers;
            let target = if adsr1 {
                &mut registers.adsr1
            } else {
                &mut registers.adsr2
            };
            *target = (register & !mask) | (value << shift);
            let candidate = envelope::fit(registers, original.model, &original.probe)?;
            if fitted(&candidate) != requested {
                continue;
            }
            count += 1;
            let next = (value.abs_diff(source), value);
            if next < rank {
                rank = next;
                best = Some(candidate);
            }
        }
        if let Some(fit) = best {
            return Ok((fit, count));
        }
    }
    Err("complete envelope tuple has no exact source-tone single-field inverse".into())
}
