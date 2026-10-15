//! Explicit PCM16 delivery-rate conversion. This is not the PSX CD resampler.

use crate::Result;
use serde::Serialize;
use std::f64::consts::PI;

const RATES: [u32; 5] = [18900, 32000, 37800, 44100, 48000];
const HALF_WIDTH: f64 = 32.0;
const BANDWIDTH: f64 = 0.94;

#[derive(Debug, Serialize)]
pub struct Report {
    pub input_rate: u32,
    pub output_rate: u32,
    pub channels: u16,
    pub input_frames: usize,
    pub output_frames: usize,
    pub bypass: bool,
    pub cutoff_hz: f64,
    pub clipped_samples: usize,
}

#[derive(Debug)]
pub struct Converted {
    pub pcm: Vec<i16>,
    pub report: Report,
}

/// Convert complete, nonempty mono/stereo PCM at the declared rates.
///
/// Output frame n is evaluated at input time n * input_rate / output_rate.
/// Length is ceil(input_frames * output_rate / input_rate). A symmetric
/// Blackman-windowed sinc uses zero extension outside the input, with no added
/// delay or tail. Each phase has unity DC gain before boundary extension.
/// PCM16 rounding uses nearest, ties away from zero; saturation is counted.
/// Container metadata and loop coordinates are deliberately outside this API.
pub fn convert(pcm: &[i16], channels: u16, input_rate: u32, output_rate: u32) -> Result<Converted> {
    if !matches!(channels, 1 | 2)
        || pcm.is_empty()
        || !pcm.len().is_multiple_of(usize::from(channels))
    {
        return Err("rate conversion requires nonempty, complete mono/stereo PCM16 frames".into());
    }
    if !RATES.contains(&input_rate) || !RATES.contains(&output_rate) {
        return Err("rate conversion supports 18900, 32000, 37800, 44100 and 48000 Hz".into());
    }
    let channels_count = usize::from(channels);
    let input_frames = pcm.len() / channels_count;
    let bypass = input_rate == output_rate;
    let output_frames = usize::try_from(
        (input_frames as u128 * u128::from(output_rate)).div_ceil(u128::from(input_rate)),
    )?;
    let mut report = Report {
        input_rate,
        output_rate,
        channels,
        input_frames,
        output_frames,
        bypass,
        cutoff_hz: f64::from(input_rate.min(output_rate)) * 0.5,
        clipped_samples: 0,
    };
    if bypass {
        return Ok(Converted {
            pcm: pcm.to_vec(),
            report,
        });
    }
    report.cutoff_hz *= BANDWIDTH;
    let cutoff =
        BANDWIDTH * f64::from(output_rate).min(f64::from(input_rate)) / f64::from(input_rate);
    let radius = (HALF_WIDTH / cutoff).ceil() as i64;
    let divisor = gcd(input_rate, output_rate);
    let step = u64::from(input_rate / divisor);
    let phase_count = u64::from(output_rate / divisor);
    let filters: Vec<_> = (0..phase_count)
        .map(|phase| coefficients(phase as f64 / phase_count as f64, cutoff, radius))
        .collect();
    let samples = output_frames
        .checked_mul(channels_count)
        .ok_or("converted PCM size overflow")?;
    let mut output = Vec::new();
    output.try_reserve_exact(samples)?;
    for frame in 0..output_frames {
        let position = frame as u128 * u128::from(step);
        let center = i128::try_from(position / u128::from(phase_count))?;
        let filter = &filters[(position % u128::from(phase_count)) as usize];
        for channel in 0..channels_count {
            let mut value = 0.0;
            for (offset, &weight) in (-radius..=radius).zip(filter) {
                let index = center + i128::from(offset);
                if index >= 0 && index < input_frames as i128 {
                    value += f64::from(pcm[index as usize * channels_count + channel]) * weight;
                }
            }
            let rounded = value.round();
            report.clipped_samples +=
                usize::from(rounded < f64::from(i16::MIN) || rounded > f64::from(i16::MAX));
            output.push(rounded.clamp(f64::from(i16::MIN), f64::from(i16::MAX)) as i16);
        }
    }
    Ok(Converted {
        pcm: output,
        report,
    })
}

fn coefficients(phase: f64, cutoff: f64, radius: i64) -> Vec<f64> {
    let mut weights: Vec<_> = (-radius..=radius)
        .map(|offset| {
            let distance = phase - offset as f64;
            let x = distance * cutoff;
            if x.abs() >= HALF_WIDTH {
                return 0.0;
            }
            let sinc = if x == 0.0 {
                1.0
            } else {
                (PI * x).sin() / (PI * x)
            };
            let window =
                0.42 + 0.5 * (PI * x / HALF_WIDTH).cos() + 0.08 * (2.0 * PI * x / HALF_WIDTH).cos();
            cutoff * sinc * window
        })
        .collect();
    let sum: f64 = weights.iter().sum();
    for weight in &mut weights {
        *weight /= sum;
    }
    weights
}

fn gcd(mut a: u32, mut b: u32) -> u32 {
    while b != 0 {
        (a, b) = (b, a % b);
    }
    a
}
