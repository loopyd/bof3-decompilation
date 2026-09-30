//! Shared ADPCM unit search and decoded loss measurement; not a media format.

use crate::codec::adpcm::{History, BLOCK_FRAMES};
use serde::Serialize;

#[derive(Clone, Debug, Serialize)]
pub struct Loss {
    /// Mono frames; multichannel encoders report one loss record per channel.
    pub frames: usize,
    pub peak_absolute_error: u32,
    pub rms_error: f64,
    pub mean_error: f64,
    /// Undefined for silent input or exact reconstruction.
    pub signal_to_noise_db: Option<f64>,
}

pub(crate) struct Unit {
    pub parameter: u8,
    pub codes: [i8; BLOCK_FRAMES],
    pub pcm: [i16; BLOCK_FRAMES],
    pub history: History,
    error: u64,
}

pub(crate) fn search(
    target: &[i16; BLOCK_FRAMES],
    history: History,
    bits: u8,
    predictors: u8,
    prediction: impl Fn(History, usize) -> i32,
) -> Unit {
    debug_assert!(matches!(bits, 4 | 8) && (1..=5).contains(&predictors));
    let mut best: Option<Unit> = None;
    for filter in 0..predictors {
        // Shifts above eight for 8-bit input add no reconstructed integer values:
        // their residual range is a subset of shift eight, which already steps by one.
        for shift in 0..=16 - bits {
            let mut unit = Unit {
                parameter: filter << 4 | shift,
                codes: [0; BLOCK_FRAMES],
                pcm: [0; BLOCK_FRAMES],
                history,
                error: 0,
            };
            let step = 1i32 << (16 - bits - shift);
            let limit = 1i32 << (bits - 1);
            for (index, &value) in target.iter().enumerate() {
                let predicted = prediction(unit.history, usize::from(filter));
                let floor = (i32::from(value) - predicted).div_euclid(step);
                let low = floor.clamp(-limit, limit - 1);
                let high = (floor + 1).clamp(-limit, limit - 1);
                let reconstruct = |q: i32| (predicted + q * step).clamp(-32768, 32767) as i16;
                let error = |sample: i16| i64::from(value) - i64::from(sample);
                let mut q = low;
                let mut sample = reconstruct(low);
                if error(reconstruct(high)).abs() < error(sample).abs() {
                    q = high;
                    sample = reconstruct(high);
                }
                unit.error += error(sample).unsigned_abs().pow(2);
                unit.codes[index] = q as i8;
                unit.pcm[index] = sample;
                unit.history.previous_previous = unit.history.previous;
                unit.history.previous = sample;
            }
            if best.as_ref().is_none_or(|b| unit.error < b.error) {
                best = Some(unit);
            }
        }
    }
    best.expect("at least one predictor/shift candidate")
}

pub(crate) fn measure<T: Copy + Into<i32>>(input: &[T], decoded: &[T]) -> Loss {
    debug_assert_eq!(input.len(), decoded.len());
    let mut peak = 0;
    let (mut squared_error, mut sum_error, mut signal) = (0f64, 0f64, 0f64);
    for (&source, &actual) in input.iter().zip(decoded) {
        let error = i64::from(actual.into()) - i64::from(source.into());
        peak = peak.max(error.unsigned_abs() as u32);
        squared_error += (error as f64).powi(2);
        sum_error += error as f64;
        signal += f64::from(source.into()).powi(2);
    }
    let divisor = input.len().max(1) as f64;
    Loss {
        frames: input.len(),
        peak_absolute_error: peak,
        rms_error: (squared_error / divisor).sqrt(),
        mean_error: sum_error / divisor,
        signal_to_noise_db: (signal > 0.0 && squared_error > 0.0)
            .then(|| 10.0 * (signal / squared_error).log10()),
    }
}
