//! Allocate the existing MP3 frame capacity across ordinary long-block granules.

use crate::{interchange::wave::Wave, Result};
use oxideav_mp3::{
    alias::{alias_ca, alias_cs},
    analysis::{analyze_granule, AnalysisState},
    encoder::write_side_info,
    frame::{parse_header, MpegVersion},
    huffman::{decode_huffman, partition_split},
    inner_loop::{exact_bit_count_band_aligned, subdivide_bands},
    main_data::{assemble_main_data, GranuleChannelData},
    mdct::{forward_overlap, mdct, window_long_family_analysis, MdctState},
    quantize::quantize,
    requantize::requantize,
    scalefactors::{decode_scalefactors, MainDataReader, Reservoir, ScaleFactors},
    side_info::{BlockType, GranuleChannel},
};

#[derive(Clone)]
struct GranuleChoice {
    gc: GranuleChannel,
    data: GranuleChannelData,
    bits: usize,
    error: f64,
}
struct Analysis {
    analysis: Vec<AnalysisState>,
    overlap: Vec<Vec<MdctState>>,
}
impl Analysis {
    fn new(channels: usize) -> Self {
        Self {
            analysis: (0..channels).map(|_| AnalysisState::new()).collect(),
            overlap: (0..channels)
                .map(|_| (0..32).map(|_| MdctState::new()).collect())
                .collect(),
        }
    }
    fn spectrum(
        &mut self,
        wave: &Wave,
        granule: usize,
        ch: usize,
        gc: &GranuleChannel,
    ) -> [f32; 576] {
        let pcm = std::array::from_fn(|i| {
            f32::from(
                *wave
                    .pcm
                    .get((granule * 576 + i) * wave.channels as usize + ch)
                    .unwrap_or(&0),
            ) / 32768.
        });
        let mut bands = analyze_granule(&pcm, &mut self.analysis[ch]);
        for sb in (1..32).step_by(2) {
            for t in (1..18).step_by(2) {
                bands[sb][t] = -bands[sb][t];
            }
        }
        let short = gc.window_switching_flag && gc.block_type == BlockType::Short;
        let mut xr = [0.; 576];
        for sb in 0..32 {
            let current = std::array::from_fn(|i| f64::from(bands[sb][i]));
            if short && (!gc.mixed_block_flag || sb >= 2) {
                let bins = oxideav_mp3::short_block::forward_short_mdct_subband(
                    &current,
                    &mut self.overlap[ch][sb],
                );
                xr[sb * 18..sb * 18 + 18].copy_from_slice(&bins);
            } else {
                let lapped = forward_overlap(&current, &mut self.overlap[ch][sb]);
                let bt = if short {
                    BlockType::Long
                } else {
                    gc.block_type
                };
                let bins = mdct(&window_long_family_analysis(&lapped, bt), 36);
                for k in 0..18 {
                    xr[sb * 18 + k] = (bins[k] / 9.) as f32;
                }
            }
        }
        if short {
            oxideav_mp3::short_block::forward_reorder(&xr, gc, wave.sample_rate, MpegVersion::Mpeg1)
        } else {
            let cs = alias_cs();
            let ca = alias_ca();
            for sb in 1..32 {
                for i in 0..8 {
                    let lo = sb * 18 - 1 - i;
                    let hi = sb * 18 + i;
                    let (a, b) = (xr[lo], xr[hi]);
                    xr[lo] = a * cs[i] + b * ca[i];
                    xr[hi] = b * cs[i] - a * ca[i];
                }
            }
            xr
        }
    }
}
fn distortion(
    xr: &[f32; 576],
    q: &[i32; 576],
    gc: &GranuleChannel,
    sf: &ScaleFactors,
    rate: u32,
) -> f64 {
    let back = requantize(q, gc, sf, rate, MpegVersion::Mpeg1);
    xr.iter()
        .zip(back)
        .map(|(&x, y)| (f64::from(x) - f64::from(y)).powi(2))
        .sum()
}
fn choices(
    xr: &[f32; 576],
    wire: [i32; 576],
    gc: GranuleChannel,
    sf: &ScaleFactors,
    rate: u32,
    part2: usize,
) -> Vec<GranuleChoice> {
    let split = partition_split(&wire);
    let baseline_error = distortion(xr, &wire, &gc, sf, rate);
    let mut rows = vec![GranuleChoice {
        gc,
        data: GranuleChannelData {
            is: wire,
            big_pairs: split.big_pairs,
            count1_quads: split.count1_quads,
        },
        bits: usize::from(gc.part2_3_length),
        error: baseline_error,
    }];
    if gc.window_switching_flag || baseline_error == 0. {
        return rows;
    }
    // Keep the encoder's scalefactors and block choices; search only finer gains.
    for gain in (0..gc.global_gain).rev() {
        let mut trial = gc;
        trial.global_gain = gain;
        let q = quantize(xr, &trial, sf, rate, MpegVersion::Mpeg1);
        if q.iter().any(|x| x.unsigned_abs() > 8191) {
            continue;
        }
        let error = distortion(xr, &q, &trial, sf, rate);
        if error >= baseline_error {
            continue;
        }
        let Some(count) = exact_bit_count_band_aligned(&q, &trial, rate, MpegVersion::Mpeg1) else {
            continue;
        };
        if count.bits + part2 > 4095 {
            continue;
        }
        let regions = subdivide_bands(rate, MpegVersion::Mpeg1, count.split.big_pairs);
        trial.big_values = count.split.big_pairs as u16;
        trial.table_select = count.table_select;
        trial.region0_count = regions.region0_count;
        trial.region1_count = regions.region1_count;
        trial.count1table_select = count.count1table_b;
        rows.push(GranuleChoice {
            gc: trial,
            data: GranuleChannelData {
                is: q,
                big_pairs: count.split.big_pairs,
                count1_quads: count.split.count1_quads,
            },
            bits: count.bits + part2,
            error,
        });
    }
    rows.sort_by(|a, b| {
        a.bits
            .cmp(&b.bits)
            .then(a.error.total_cmp(&b.error))
            .then(b.gc.global_gain.cmp(&a.gc.global_gain))
    });
    let mut best = f64::INFINITY;
    rows.retain(|row| {
        if row.error < best {
            best = row.error;
            true
        } else {
            false
        }
    });
    rows
}
pub(super) fn refine(raw: &[u8], wave: &Wave) -> Result<Vec<u8>> {
    let profile = super::stream::inspect(raw)?;
    if profile.sample_rate != wave.sample_rate || profile.channels != wave.channels {
        return Err("MP3 allocation input format differs from PCM".into());
    }
    let nch = wave.channels as usize;
    let mut front = Analysis::new(nch);
    let mut reservoir = Reservoir::new();
    let (mut offset, mut frame_index) = (0, 0);
    let mut output = Vec::with_capacity(raw.len());
    while offset < raw.len() {
        let header = parse_header(&raw[offset..])?;
        let len = header
            .frame_len()
            .ok_or("MP3 allocation requires fixed-length frames")?;
        let end = offset
            .checked_add(len)
            .ok_or("MP3 allocation frame offset overflow")?;
        let frame = raw
            .get(offset..end)
            .ok_or("MP3 allocation frame is truncated")?;

        let mut side = oxideav_mp3::side_info::parse_side_info(&header, &frame[4..])?;
        let slot = &frame[4 + side.byte_len()..];
        let run = reservoir.assemble(side.main_data_begin as usize, slot)?;
        let factors = decode_scalefactors(&header, &side, &run)?;
        let mut cursor = 0;
        let mut options = Vec::new();
        for gr in 0..2 {
            for ch in 0..nch {
                let gc = side.granules[gr][ch];
                let part2 = factors.part2_bits[gr][ch] as usize;
                let xr = front.spectrum(wave, frame_index * 2 + gr, ch, &gc);
                let mut reader = MainDataReader::new(&run);
                let mut skip = cursor + part2;
                while skip > 0 {
                    let n = skip.min(32);
                    reader.read(n as u32);
                    skip -= n;
                }
                let wire = decode_huffman(
                    &mut reader,
                    &gc,
                    gc.part2_3_length as u32 - part2 as u32,
                    wave.sample_rate,
                    MpegVersion::Mpeg1,
                )?;
                if quantize(
                    &xr,
                    &gc,
                    &factors.granules[gr][ch],
                    wave.sample_rate,
                    MpegVersion::Mpeg1,
                ) != wire
                {
                    return Err(format!("MP3 allocation could not reproduce encoder coefficients at frame {frame_index}, granule {gr}, channel {ch}").into());
                }
                options.push(choices(
                    &xr,
                    wire,
                    gc,
                    &factors.granules[gr][ch],
                    wave.sample_rate,
                    part2,
                ));
                cursor += gc.part2_3_length as usize;
            }
        }
        // Pareto choices are ordered by bits. Minimize summed spectral error
        // across every granule/channel without borrowing from another frame.
        let capacity = slot.len() * 8;
        let mut costs = vec![f64::INFINITY; capacity + 1];
        costs[0] = 0.;
        let mut selections = vec![[0usize; 4]; capacity + 1];
        for (cell, rows) in options.iter().enumerate() {
            let mut next = vec![f64::INFINITY; capacity + 1];
            let mut selected = vec![[0usize; 4]; capacity + 1];
            for used in 0..=capacity {
                if !costs[used].is_finite() {
                    continue;
                }
                for (i, row) in rows.iter().enumerate() {
                    let new = used + row.bits;
                    if new > capacity {
                        break;
                    }
                    let error = costs[used] + row.error;
                    if error < next[new] {
                        next[new] = error;
                        selected[new] = selections[used];
                        selected[new][cell] = i;
                    }
                }
            }
            costs = next;
            selections = selected;
        }
        let used = (0..=capacity)
            .filter(|&i| costs[i].is_finite())
            .min_by(|&a, &b| costs[a].total_cmp(&costs[b]))
            .ok_or("MP3 allocation has no representable frame budget")?;
        let mut data: [[GranuleChannelData; 2]; 2] = std::array::from_fn(|_| {
            std::array::from_fn(|_| GranuleChannelData {
                is: [0; 576],
                big_pairs: 0,
                count1_quads: 0,
            })
        });
        for (gr, granule) in data.iter_mut().enumerate() {
            for (ch, channel) in granule.iter_mut().take(nch).enumerate() {
                let cell = gr * nch + ch;
                let choice = &options[cell][selections[used][cell]];

                side.granules[gr][ch] = choice.gc;
                *channel = choice.data.clone();
            }
        }
        let assembled = assemble_main_data(&header, &mut side, &factors, &data)?;
        if assembled.total_bits != used
            || assembled.bytes.len() > slot.len()
            || assembled
                .part2_3_lengths
                .iter()
                .flatten()
                .any(|&x| x > 4095)
        {
            return Err("MP3 allocation exceeded its checked frame or granule budget".into());
        }
        side.main_data_begin = 0;
        let mut new = frame[..4].to_vec();
        new.extend(write_side_info(&side));
        new.extend(assembled.bytes);
        new.resize(len, 0);
        output.extend(new);
        frame_index += 1;
        offset = end;
    }
    if output.len() != raw.len() {
        return Err("MP3 allocation changed the stream byte length".into());
    }
    super::stream::inspect(&output)?;
    Ok(output)
}
