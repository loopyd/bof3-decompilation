//! Quantized SF2 gain/pan approximation of an explicit game note-on context.
use crate::{voice::gain::Context, voice::gain::Registers, Result};
use serde::Serialize;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub enum Model {
    /// Specification attenuation units with an explicit equal-power pan model;
    /// this does not claim every SF2 consumer implements the same pan curve.
    SpecificationScale,
    /// Published RustySynth 1.3.6 applies 40% of initial attenuation.
    RustySynth136,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub struct Parameters {
    pub attenuation_cb: u16,
    pub pan: i16,
}

#[derive(Clone, Debug, Serialize)]
pub struct Fit {
    pub source: Context,
    pub registers: Registers,
    pub model: Model,
    /// None requires an explicit zero-PCM representation, not finite attenuation.
    pub parameters: Option<Parameters>,
    /// Each SPU register / 16383 / sqrt(2): unity center maps to an
    /// unattenuated SF2 voice. This is a comparison normalization, not PSX PCM.
    pub target: [f64; 2],
    pub predicted: [f64; 2],
    pub max_absolute_error: f64,
    pub specification_scale_prediction: [f64; 2],
    pub rustysynth136_prediction: [f64; 2],
    pub limitations: Vec<&'static str>,
}

impl Model {
    fn attenuation_scale(self) -> f64 {
        match self {
            Self::SpecificationScale => 1.0,
            Self::RustySynth136 => 0.4,
        }
    }
}

/// Unit velocity, full 14-bit channel volume/expression, CC10=64/CC42=0,
/// full envelope, no filter attenuation, LFO, effects or master gain.
pub fn predict(parameters: Parameters, model: Model) -> [f64; 2] {
    let (amplitude, angle) = match model {
        Model::SpecificationScale => (
            10.0f64.powf(-f64::from(parameters.attenuation_cb) / 200.0),
            (f64::from(parameters.pan) + 500.0) * std::f64::consts::PI / 2000.0,
        ),
        Model::RustySynth136 => {
            // Retain published engine's f32 operation ordering.
            let attenuation = 0.4f32 * (0.1f32 * f32::from(parameters.attenuation_cb));
            let amplitude = 10.0f32.powf(0.05f32 * -attenuation);
            let channel_pan = (100.0f32 / 16383.0) * 8192.0 - 50.0;
            let instrument_pan = 0.1f32 * f32::from(parameters.pan);
            let angle = (std::f32::consts::PI / 200.0) * (channel_pan + instrument_pan + 50.0);
            let angle = angle.clamp(0.0, std::f32::consts::FRAC_PI_2);
            let gains = if angle <= 0.0 {
                [amplitude, 0.0]
            } else if angle >= std::f32::consts::FRAC_PI_2 {
                [0.0, amplitude]
            } else {
                [amplitude * angle.cos(), amplitude * angle.sin()]
            };
            // The steady-state mixer skips a channel below NON_AUDIBLE.
            // This comparison assumes master_volume=1 and a full envelope.
            return gains.map(|v| if v < 0.001 { 0.0 } else { f64::from(v) });
        }
    };
    if parameters.pan == -500 {
        return [amplitude, 0.0];
    }
    if parameters.pan == 500 {
        return [0.0, amplitude];
    }
    [amplitude * angle.cos(), amplitude * angle.sin()]
}

pub fn fit(source: Context, model: Model) -> Result<Fit> {
    let registers = source.registers()?;
    if registers.left == 0 && registers.right == 0 {
        return Ok(Fit {
            source, registers, model, parameters: None, target: [0.0; 2], predicted: [0.0; 2],
            max_absolute_error: 0.0, specification_scale_prediction: [0.0; 2], rustysynth136_prediction: [0.0; 2],
            limitations: vec![
                "Exact silence requires an explicit zero-PCM sample; no attenuation/pan generator pair is supplied.",
                "Preserve the original sample and tone identity separately. This context alone does not prove silence under other runtime controls.",
            ],
        });
    }
    let norm = std::f64::consts::FRAC_1_SQRT_2 / 16383.0;
    let target = [
        f64::from(registers.left) * norm,
        f64::from(registers.right) * norm,
    ];
    let ideal_pan = target[1].atan2(target[0]) * 2000.0 / std::f64::consts::PI - 500.0;
    let center = ideal_pan.round() as i16;
    let mut best = (
        f64::INFINITY,
        Parameters {
            attenuation_cb: 0,
            pan: 0,
        },
        [0.0; 2],
    );
    // Direction is monotone; inspect neighbors to include integer/f32 rounding.
    for pan in (center - 2).max(-500)..=(center + 2).min(500) {
        let direction = predict(
            Parameters {
                attenuation_cb: 0,
                pan,
            },
            model,
        );
        let amplitude = target[0] * direction[0] + target[1] * direction[1];
        let ideal_cb = -200.0 * amplitude.log10() / model.attenuation_scale();
        let center_cb = ideal_cb.round().clamp(0.0, 1440.0) as i32;
        for cb in (center_cb - 2).max(0)..=(center_cb + 2).min(1440) {
            let parameters = Parameters {
                attenuation_cb: cb as u16,
                pan,
            };
            let predicted = predict(parameters, model);
            let error = (predicted[0] - target[0]).powi(2) + (predicted[1] - target[1]).powi(2);
            if error < best.0 {
                best = (error, parameters, predicted);
            }
        }
    }
    Ok(Fit {
        source, registers, model, parameters: Some(best.1), target, predicted: best.2,
        max_absolute_error: (best.2[0] - target[0]).abs().max((best.2[1] - target[1]).abs()),
        specification_scale_prediction: predict(best.1, Model::SpecificationScale),
        rustysynth136_prediction: predict(best.1, Model::RustySynth136),
        limitations: vec![
            "Static note-on fit with explicit unity-center normalization; not PSX/PC PCM equivalence.",
            "Reference playback requires velocity 127, CC7/39 and CC11/43 at 127, CC10=64 and CC42=0; source controls are baked into this fit.",
            "Dynamic pan, volume and velocity curves differ; this fit does not establish equivalence away from its reference context.",
            "RustySynth 1.3.6 scales initial attenuation by 0.4; an engine-targeted fit changes loudness in consumers using specification-scale attenuation.",
            "Master volume, envelope, filter, effects and output scaling are outside this gain comparison.",
            "RustySynth prediction includes its steady-state 0.001 mixer cutoff at master volume 1 and full envelope; other master levels change the cutoff.",
        ],
    })
}
