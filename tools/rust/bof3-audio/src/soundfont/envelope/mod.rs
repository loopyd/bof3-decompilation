//! Measured SF2 volume-envelope approximation for an explicitly selected SPU model.
//! The target is RustySynth 1.3.6, including its 10 ms release clamp. This is not
//! a hardware oracle or a claim that a fixed SF2 sustain reproduces PSX sustain.

pub(crate) mod snapshot;

use crate::{
    machine::adsr::{Adsr, Model, Phase, Registers},
    soundfont::Envelope,
    Result,
};
use serde::Serialize;

const RATE: f64 = 44100.0;

#[derive(Clone, Debug, Serialize)]
pub struct Probe {
    pub held_frames: Vec<u32>,
    pub release_frames: u32,
    pub sample_stride: u32,
}

impl Default for Probe {
    fn default() -> Self {
        Self {
            held_frames: vec![11025, 44100, 88200],
            release_frames: 88200,
            sample_stride: 441,
        }
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct Parameters {
    pub attack_tc: i16,
    pub decay_tc: i16,
    pub sustain_cb: u16,
    pub release_tc: i16,
}

impl Parameters {
    pub fn envelope(&self) -> Envelope {
        Envelope {
            delay: i16::MIN,
            attack: self.attack_tc,
            hold: i16::MIN,
            decay: self.decay_tc,
            sustain_cb: self.sustain_cb,
            release: self.release_tc,
        }
    }
    fn values(&self) -> [i32; 4] {
        [
            i32::from(self.attack_tc),
            i32::from(self.decay_tc),
            i32::from(self.sustain_cb),
            i32::from(self.release_tc),
        ]
    }
    fn from_values(values: [i32; 4]) -> Self {
        Self {
            attack_tc: values[0] as i16,
            decay_tc: values[1] as i16,
            sustain_cb: values[2] as u16,
            release_tc: values[3] as i16,
        }
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct Error {
    pub weighted_rms: f64,
    pub maximum_at_probe: f64,
    pub points: usize,
}

#[derive(Clone, Debug, Serialize)]
pub struct Fit {
    pub target: &'static str,
    pub registers: Registers,
    pub model: Model,
    pub probe: Probe,
    pub parameters: Parameters,
    pub error: Error,
    pub seed_error: Error,
    pub attack_end_frame: Option<u32>,
    pub decay_end_frame: Option<u32>,
    pub sustain_min: Option<i16>,
    pub sustain_max: Option<i16>,
    pub releases_still_active: usize,
    pub limitations: Vec<&'static str>,
}

struct Point {
    held: u32,
    frame: u32,
    value: f64,
    weight: u32,
}

pub fn fit(registers: Registers, model: Model, probe: &Probe) -> Result<Fit> {
    if probe.held_frames.is_empty()
        || probe.held_frames.len() > 16
        || probe.sample_stride == 0
        || probe.release_frames == 0
        || probe.held_frames.iter().any(|&n| n == 0 || n > 44100 * 60)
        || probe.release_frames > 44100 * 60
        || probe.sample_stride > 44100
    {
        return Err("SF2 envelope fit: require 1..16 positive holds and a positive release, each at most 60 seconds; stride 1..44100".into());
    }
    let mut points = Vec::new();
    let mut attack_end = None;
    let mut decay_end = None;
    let mut sustain_min = None;
    let mut sustain_max = None;
    let mut release_end = None;
    let mut releases_active = 0;
    let mut max_held_level = 0;
    for &held in &probe.held_frames {
        let mut adsr = Adsr::new(registers, model);
        adsr.key_on();
        let mut last = 0;
        for frame in 1..=held + probe.release_frames {
            if frame == held + 1 {
                adsr.key_off();
            }
            let before = adsr.phase();
            let level = adsr.tick();
            let after = adsr.phase();
            if frame <= held {
                max_held_level = max_held_level.max(level);
                if before == Phase::Attack && after == Phase::Decay {
                    attack_end = Some(frame);
                }
                if before == Phase::Decay && after == Phase::Sustain {
                    decay_end = Some(frame);
                }
                if after == Phase::Sustain {
                    sustain_min = Some(sustain_min.map_or(level, |x: i16| x.min(level)));
                    sustain_max = Some(sustain_max.map_or(level, |x: i16| x.max(level)));
                }
            } else if before == Phase::Release && after == Phase::Off {
                release_end = Some(release_end.map_or(frame - held, |x: u32| x.max(frame - held)));
            }
            // Dense early samples retain fast attack steps. Weights approximate
            // integration time, so dense points do not dominate long holds.
            if frame <= 64
                || frame % probe.sample_stride == 0
                || before != after
                || frame == held
                || frame == held + 1
                || frame == held + probe.release_frames
            {
                points.push(Point {
                    held,
                    frame,
                    value: f64::from(level.max(0)) / 32767.0,
                    weight: frame - last,
                });
                last = frame;
            }
        }
        releases_active += usize::from(adsr.phase() != Phase::Off);
    }
    let max_hold = *probe.held_frames.iter().max().unwrap();
    let attack_seconds = attack_end.map_or_else(
        || f64::from(max_hold) / RATE / (f64::from(max_held_level) / 32767.0).max(1e-8),
        |frame| f64::from(frame) / RATE,
    );
    let sustain = f64::from(sustain_min.unwrap_or(registers.sustain_level()).max(1)) / 32767.0;
    // Match the seed's observed decay interval to the target exponential slope.
    let decay_seconds = f64::from(
        decay_end
            .unwrap_or(max_hold)
            .saturating_sub(attack_end.unwrap_or(0)),
    ) / RATE
        * 9.226
        / (-sustain.ln()).max(1e-6);
    let release_seconds =
        f64::from(release_end.unwrap_or(probe.release_frames)) / RATE * 9.226 / -0.001f64.ln();
    let seed = Parameters {
        attack_tc: timecents(attack_seconds),
        decay_tc: timecents(decay_seconds),
        sustain_cb: (-200.0 * sustain.log10()).round().clamp(0.0, 1440.0) as u16,
        release_tc: timecents(release_seconds),
    };
    let seed_error = measure(&seed, &points);
    let mut parameters = seed;
    let mut error = seed_error.clone();
    // Bounded coordinate search over representable integer generators. Report
    // its result, not a claim of a global optimum or a universal note-length fit.
    for step in [2400, 1200, 600, 300, 100, 25, 5, 1] {
        for _ in 0..3 {
            let before = error.weighted_rms;
            for axis in 0..4 {
                let values = parameters.values();
                let base = values[axis].max(if axis == 2 { 0 } else { -12000 });
                let jump = if axis == 2 { (step / 6).max(1) } else { step };
                let candidates = if axis == 2 {
                    vec![(base - jump).max(0), (base + jump).min(1440)]
                } else {
                    vec![
                        (base - jump).max(-12000),
                        (base + jump).min(8000),
                        i32::from(i16::MIN),
                    ]
                };
                for candidate in candidates {
                    let mut values = parameters.values();
                    values[axis] = candidate;
                    let proposed = Parameters::from_values(values);
                    let proposed_error = measure(&proposed, &points);
                    if proposed_error.weighted_rms < error.weighted_rms {
                        parameters = proposed;
                        error = proposed_error;
                    }
                }
            }
            if before == error.weighted_rms {
                break;
            }
        }
    }
    Ok(Fit { target: "rustysynth-1.3.6 volume envelope", registers, model, probe: probe.clone(),
        parameters, error, seed_error, attack_end_frame: attack_end, decay_end_frame: decay_end,
        sustain_min, sustain_max, releases_still_active: releases_active,
        limitations: vec![
            "Approximation of a selected SPU arithmetic model, not hardware-validated ADSR or gameplay register overrides.",
            "Fixed SF2 sustain cannot reproduce moving PSX sustain; error depends on held-note duration.",
            "Error is normalized amplitude at the recorded probe points, weighted by elapsed frames; it excludes sample/filter/mixer and between-point error.",
            "The target includes RustySynth's exponential cutoff and minimum 10 ms release; other SF2 consumers may differ.",
            "No sample-block, key-on latency or scheduler quantization is included; the bounded coordinate fit is not a global optimum.",
        ] })
}

fn timecents(seconds: f64) -> i16 {
    (1200.0 * seconds.max(1e-20).log2())
        .round()
        .clamp(-12000.0, 8000.0) as i16
}

struct Curve {
    attack: f64,
    decay: f64,
    sustain: f64,
    release: f64,
    zero: f64,
}
impl Curve {
    fn new(parameters: &Parameters) -> Self {
        // Match the selected consumer's f32 generator conversion before its f64
        // envelope arithmetic, including special zero-time values as consumed.
        let seconds = |tc: i16| f64::from(2f32.powf(f32::from(tc) * (1.0 / 1200.0)));
        Self {
            attack: seconds(parameters.attack_tc),
            decay: seconds(parameters.decay_tc),
            sustain: f64::from(10f32.powf(-f32::from(parameters.sustain_cb) / 200.0)),
            release: seconds(parameters.release_tc).max(f64::from(0.01f32)),
            zero: seconds(i16::MIN),
        }
    }
    fn held(&self, time: f64) -> f64 {
        if time < self.zero {
            0.0
        } else if time < self.zero + self.attack {
            (time - self.zero) / self.attack
        } else if time < 2.0 * self.zero + self.attack {
            1.0
        } else {
            audible(
                exponential(-9.226 * (time - 2.0 * self.zero - self.attack) / self.decay)
                    .max(self.sustain),
            )
        }
    }
    fn value(&self, held: u32, frame: u32) -> f64 {
        if frame <= held {
            self.held(f64::from(frame) / RATE)
        } else {
            audible(
                self.held(f64::from(held) / RATE)
                    * exponential(-9.226 * f64::from(frame - held) / RATE / self.release),
            )
        }
    }
}
fn exponential(value: f64) -> f64 {
    if value < f64::from(-6.907_755_4f32) {
        0.0
    } else {
        value.exp()
    }
}
fn audible(value: f64) -> f64 {
    if value > f64::from(1.0e-3f32) {
        value
    } else {
        0.0
    }
}
fn measure(parameters: &Parameters, points: &[Point]) -> Error {
    let curve = Curve::new(parameters);
    let mut square = 0.0;
    let mut weight = 0u64;
    let mut maximum = 0.0f64;
    for point in points {
        let difference = curve.value(point.held, point.frame) - point.value;
        square += difference * difference * f64::from(point.weight);
        weight += u64::from(point.weight);
        maximum = maximum.max(difference.abs());
    }
    Error {
        weighted_rms: (square / weight as f64).sqrt(),
        maximum_at_probe: maximum,
        points: points.len(),
    }
}

/// Evaluate the chosen consumer envelope without oscillator/filter/mixer effects.
pub fn target_level(parameters: &Parameters, held_frames: u32, frame: u32) -> f64 {
    Curve::new(parameters).value(held_frames, frame)
}
