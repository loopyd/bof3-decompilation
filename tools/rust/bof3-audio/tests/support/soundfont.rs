use bof3_audio::{
    bank::Bank, machine::adsr::Model, machine::adsr::Registers, soundfont::bank::Gain,
    soundfont::bank::Identity, soundfont::bank::Options, soundfont::bank::ToneContext,
    soundfont::envelope, soundfont::envelope::Probe, voice::tuning::KeyTuning,
};

pub fn fixture() -> (Vec<u8>, Vec<u8>) {
    let mut vh = vec![0; 0x820 + 2 * 512 + 512];
    vh[..4].copy_from_slice(b"pBAV");
    vh[4..8].copy_from_slice(&7u32.to_le_bytes());
    vh[8..12].copy_from_slice(&99u32.to_le_bytes());
    for (offset, count) in [(0x12, 2u16), (0x14, 3), (0x16, 2)] {
        vh[offset..offset + 2].copy_from_slice(&count.to_le_bytes());
    }
    vh[0x18] = 127;
    vh[0x19] = 64;
    for (program, block, count) in [(0, 0, 2), (5, 1, 1)] {
        let p = 0x20 + program * 16;
        vh[p] = count;
        vh[p + 1] = 127;
        vh[p + 4] = 64;
        for tone in 0..usize::from(count) {
            let t = 0x820 + block * 512 + tone * 32;
            vh[t + 2] = 127;
            vh[t + 3] = 64;
            vh[t + 4] = 60;
            vh[t + 6] = 60;
            vh[t + 7] = 61;
            vh[t + 16..t + 18].copy_from_slice(&0x000fu16.to_le_bytes());
            vh[t + 18..t + 20].copy_from_slice(&0x1fc0u16.to_le_bytes());
            vh[t + 20..t + 22].copy_from_slice(&(program as u16).to_le_bytes());
            vh[t + 22..t + 24].copy_from_slice(&2u16.to_le_bytes());
        }
    }
    // Empty sample 1 remains an identity slot; sample 2 owns four ADPCM blocks.
    let table = 0x820 + 2 * 512;
    vh[table + 4..table + 6].copy_from_slice(&8u16.to_le_bytes());
    let mut vb = vec![0; 64];
    for block in 0..4 {
        vb[block * 16 + 2..block * 16 + 16].fill(if block % 2 == 0 { 0x77 } else { 0x88 });
    }
    vb[1] = 4;
    vb[49] = 3;
    (vh, vb)
}
pub fn identity() -> Identity {
    Identity {
        source: "synthetic/TEST.EMI".into(),
        header_entry: 3,
        body_entry: 4,
        game_bank_id: 7,
    }
}
pub fn options() -> Options {
    Options {
        sf2_bank: 0,
        percussion_alias: true,
        sample_rate: 44100,
        reverb: None,
        allow_predictor_loop_approximation: false,
        allow_stopped_pitch_approximation: false,
    }
}
pub fn contexts(vh: &[u8]) -> Vec<ToneContext> {
    let fit = envelope::fit(
        Registers {
            adsr1: 0x000f,
            adsr2: 0x1fc0,
        },
        Model::Published,
        &Probe {
            held_frames: vec![4410],
            release_frames: 4410,
            sample_stride: 441,
        },
    )
    .unwrap();
    Bank::parse(vh)
        .unwrap()
        .programs
        .into_iter()
        .flat_map(|p| {
            p.tones.into_iter().map({
                let fit = fit.clone();
                move |tone| ToneContext {
                    program: p.program,
                    tone: tone.index,
                    gain: Gain {
                        fit: None,
                        attenuation_cb: if p.program == 5 { 100 } else { 0 },
                        pan: if p.program == 5 {
                            0
                        } else if tone.index == 0 {
                            -500
                        } else {
                            500
                        },
                        provenance: "synthetic gain context; no game-control equivalence claim"
                            .into(),
                    },
                    envelope: fit.clone(),
                    tuning: (60..=61)
                        .map(|key| {
                            KeyTuning::from_register(
                                key,
                                60,
                                0,
                                if key == 60 { 4096 } else { 4352 },
                                44100,
                            )
                            .unwrap()
                        })
                        .collect(),
                    pitch_provenance: "synthetic register context".into(),
                }
            })
        })
        .collect()
}
