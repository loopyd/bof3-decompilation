//! XA ADPCM sector encoding within an existing stream layout and coding format.

use crate::{
    codec::edc::{self, Status},
    codec::quantization::{self, Loss},
    xa::{self, Arithmetic, Decoder, Format, Histories, Stream},
    Result,
};
use serde::Serialize;

#[derive(Clone, Debug, Serialize)]
pub struct Report {
    pub schema: &'static str,
    pub stream: Stream,
    pub format: Format,
    pub arithmetic: Arithmetic,
    pub frames: usize,
    pub reused_original: bool,
    pub initial_histories: Histories,
    pub final_histories: Histories,
    pub channel_loss: Vec<Loss>,
    pub predictor_units: [usize; 4],
    pub original_edc: Status,
    pub output_edc: Status,
    pub limitations: Vec<&'static str>,
}

#[derive(Debug)]
pub struct Encoded {
    pub bytes: Vec<u8>,
    pub report: Report,
}

#[derive(Clone, Debug)]
pub struct Encoder {
    decoder: Decoder,
}

impl Encoder {
    pub fn new(stream: Stream, arithmetic: Arithmetic, histories: Histories) -> Result<Self> {
        Ok(Self {
            decoder: Decoder::new(stream, arithmetic, histories)?,
        })
    }

    pub fn histories(&self) -> Histories {
        self.decoder.histories()
    }

    pub fn reset(&mut self, histories: Histories) {
        self.decoder.reset(histories);
    }

    /// One sector's exact PCM capacity, interleaved for stereo. The template's
    /// subheaders, unused parameters and 20-byte spare area remain unchanged.
    /// All errors leave encoder history and caller-owned template data intact.
    pub fn encode_sector(&mut self, template: &[u8], pcm: &[i16]) -> Result<Encoded> {
        let format = self.decoder.format();
        let channels = usize::from(format.channels());
        let frames = format.frames_per_sector();
        if pcm.len() != frames * channels {
            return Err(format!("XA encode: needs exactly {frames} frames / {} interleaved samples for this sector, received {}; padding, truncation and coding changes are not implicit", frames * channels, pcm.len()).into());
        }
        let original_edc = edc::check_form2(template)?;
        let mut original_decoder = self.decoder.clone();
        let original_pcm = original_decoder.decode_sector(template)?;
        let reused_original = original_pcm == pcm;
        let mut bytes = template.to_vec();
        let initial_histories = self.histories();
        let mut histories = initial_histories;
        let bits = format.bits_per_sample();
        let units = 32 / usize::from(bits);
        let arithmetic = self.decoder.arithmetic();
        let mut expected = vec![0; pcm.len()];
        let mut predictor_units = [0; 4];
        for group_index in 0..18 {
            let group = &mut bytes[8 + group_index * 128..8 + (group_index + 1) * 128];
            if reused_original {
                for unit in 0..units {
                    predictor_units[usize::from(group[4 + unit] >> 4)] += 1;
                }
                continue;
            }
            group[16..].fill(0);
            for unit in 0..units {
                let mut target = [0; 28];
                let frame_at = |frame| {
                    group_index * units * 28
                        + (unit / channels * 28 + frame) * channels
                        + unit % channels
                };
                for (frame, value) in target.iter_mut().enumerate() {
                    *value = pcm[frame_at(frame)];
                }
                let history = if channels == 2 && unit % 2 == 1 {
                    &mut histories.right
                } else {
                    &mut histories.left
                };
                let chosen = quantization::search(&target, *history, bits, 4, |h, filter| {
                    xa::prediction(h, filter, arithmetic)
                });
                group[4 + unit] = chosen.parameter;
                predictor_units[usize::from(chosen.parameter >> 4)] += 1;
                for frame in 0..28 {
                    let code = chosen.codes[frame] as u8;
                    if bits == 4 {
                        group[16 + frame * 4 + unit / 2] |= (code & 15) << ((unit % 2) * 4);
                    } else {
                        group[16 + frame * 4 + unit] = code;
                    }
                    expected[frame_at(frame)] = chosen.pcm[frame];
                }
                *history = chosen.history;
            }
            group.copy_within(4..8, 0);
            if bits == 4 {
                group.copy_within(8..12, 12);
            }
        }
        if reused_original {
            expected = original_pcm;
            histories = original_decoder.histories();
        }
        let output_edc = if reused_original {
            original_edc
        } else {
            edc::update_form2(&mut bytes, original_edc)?
        };
        let mut check = self.decoder.clone();
        let decoded = check.decode_sector(&bytes)?;
        if decoded != expected || check.histories() != histories {
            return Err("XA encode: decoded output differs from candidate reconstruction".into());
        }
        let channel_loss = (0..channels)
            .map(|channel| {
                let source: Vec<_> = pcm[channel..].iter().step_by(channels).copied().collect();
                let actual: Vec<_> = decoded[channel..]
                    .iter()
                    .step_by(channels)
                    .copied()
                    .collect();
                quantization::measure(&source, &actual)
            })
            .collect();
        let report = Report {
            schema: "bof3.audio.xa-encoding/v1",
            stream: self.decoder.stream(), format, arithmetic, frames, reused_original,
            initial_histories, final_histories: histories, channel_loss,
            predictor_units, original_edc, output_edc,
            limitations: vec![
                "One existing sector's exact capacity; no implicit padding, truncation, resampling, channel conversion or cue relocation.",
                "Loss is measured at encoded rate with the selected predictor arithmetic; CD resampling, emphasis, mixing and hardware fidelity are not established.",
                "Per-unit greedy quantization searches four predictors; shifts 0..12 for 4-bit and 0..8 for 8-bit. It is not a global optimum.",
                "Caller must supply correct initial history and follow stream order; EOF does not reset history automatically.",
                "Changed history can affect following unedited sectors; partial stream publication requires an explicit boundary policy.",
            ],
        };
        self.decoder = check;
        Ok(Encoded { bytes, report })
    }
}
