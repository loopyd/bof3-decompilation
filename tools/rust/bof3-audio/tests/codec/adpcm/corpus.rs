use bof3_audio::{
    archive::MediaImage, catalog::loader, catalog::model::AssetData, catalog::model::Catalog,
    codec::adpcm::decode_sample, codec::adpcm::SampleLoop, codec::adpcm::Termination,
    digest::sha256_hex, machine::executable::Executable,
};
use std::{collections::HashMap, path::PathBuf};

struct Summary {
    hash: String,
    frames: usize,
    termination: Termination,
    consumed: usize,
    trailing: usize,
    sample_loop: Option<SampleLoop>,
}

#[test]
#[ignore = "requires BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS original inputs"]
fn every_original_vab_sample_matches_independent_pcm_and_loop_reference() {
    let root = PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let mut catalog = Catalog::read(Some(&root), &[]).unwrap();
    loader::resolve(&mut catalog, &exe).unwrap();
    let mut cache = HashMap::<Vec<u8>, Summary>::new();
    let mut records = String::new();
    let (mut count, mut empty, mut mute, mut repeated, mut frames, mut trailing, mut unstable) =
        (0, 0, 0, 0, 0, 0, 0);
    for source in &catalog.sources {
        if source.container != "emi" {
            continue;
        }
        let MediaImage::Emi(image) = MediaImage::read(&root.join(&source.source)).unwrap() else {
            unreachable!()
        };
        for bank in catalog
            .assets
            .iter()
            .filter(|asset| asset.source == source.source)
        {
            let AssetData::Bank {
                metadata,
                content: Some(content),
                ..
            } = &bank.data
            else {
                continue;
            };
            let body = source
                .entries
                .iter()
                .find(|entry| entry.id == content.body_entry)
                .unwrap();
            let vb = image.entry(body.entry).unwrap();
            for sample in &metadata.samples {
                let raw = &vb[sample.body_offset..sample.body_offset + sample.encoded_bytes];
                if !cache.contains_key(raw) {
                    let decoded = decode_sample(raw).unwrap_or_else(|error| {
                        panic!("{}/sample={}: {error}", bank.id, sample.sample_id)
                    });
                    let bytes: Vec<_> = decoded
                        .pcm
                        .iter()
                        .flat_map(|sample| sample.to_le_bytes())
                        .collect();
                    // 44100 is an explicit export reference rate, not a VAB
                    // sample's intrinsic pitch. Preserve the loop approximation
                    // report separately; smpl cannot express predictor history.
                    let mut wave =
                        bof3_audio::interchange::wave::Wave::new(1, 44100, decoded.pcm.clone())
                            .unwrap();
                    if let Some(range) = decoded.sample_loop {
                        let mut sampler =
                            bof3_audio::interchange::wave::Sampler::new(44100, 60).unwrap();
                        sampler.loops.push(
                            bof3_audio::interchange::wave::SampleLoop::forward(
                                u32::try_from(range.start_frame).unwrap(),
                                u32::try_from(range.end_frame_exclusive).unwrap(),
                            )
                            .unwrap(),
                        );
                        wave.sampler = Some(sampler);
                    }
                    let encoded_wave = wave.to_bytes().unwrap();
                    let reloaded =
                        bof3_audio::interchange::wave::Wave::from_bytes(&encoded_wave).unwrap();
                    assert_eq!(reloaded.pcm, decoded.pcm);
                    assert_eq!(reloaded.sampler, wave.sampler);
                    assert_eq!(reloaded.to_bytes().unwrap(), encoded_wave);
                    cache.insert(
                        raw.to_vec(),
                        Summary {
                            hash: sha256_hex(&bytes),
                            frames: decoded.pcm.len(),
                            termination: decoded.termination,
                            consumed: decoded.decoded_bytes,
                            trailing: decoded.trailing_bytes,
                            sample_loop: decoded.sample_loop,
                        },
                    );
                }
                let decoded = &cache[raw];
                count += 1;
                frames += decoded.frames;
                trailing += decoded.trailing;
                let termination = match decoded.termination {
                    Termination::Empty => {
                        empty += 1;
                        "empty"
                    }
                    Termination::EndMute => {
                        mute += 1;
                        "end_mute"
                    }
                    Termination::EndRepeat => {
                        repeated += 1;
                        "end_repeat"
                    }
                    Termination::BoundedWithoutEnd => {
                        panic!("{}/sample={} has no end marker", bank.id, sample.sample_id)
                    }
                };
                let loop_fields = if let Some(range) = decoded.sample_loop {
                    if !range.pcm_repeat_is_stable {
                        unstable += 1
                    }
                    format!(
                        "{}|{}|{}",
                        range.start_frame, range.end_frame_exclusive, range.pcm_repeat_is_stable
                    )
                } else {
                    "-|-|-".into()
                };
                records.push_str(&format!(
                    "{}/sample={}|{}|{}|{}|{}|{}|{}\n",
                    bank.id,
                    sample.sample_id,
                    decoded.hash,
                    decoded.frames,
                    termination,
                    decoded.consumed,
                    decoded.trailing,
                    loop_fields
                ));
            }
        }
    }
    assert_eq!((count, empty, mute, repeated), (8385, 55, 7392, 938));
    assert_eq!((frames, trailing, unstable), (138296704, 130816, 790));
    assert_eq!(cache.len(), 1180);
    // Independent Python raw-TOC/sample-table traversal, floor arithmetic and
    // little-endian PCM hashing. Includes identity, frames, termination, tails
    // and loop/stability metadata for every sample, in source/entry/sample order.
    assert_eq!(
        sha256_hex(records.as_bytes()),
        "cf7f115ccb8e468b1e321657f4715ab4ffd5573cc0bbcd37a911b1f644522eae"
    );
}
