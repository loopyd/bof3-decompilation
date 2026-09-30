use bof3_audio::{
    machine::{bank::Stage, executable::Executable, firmware::Image, music, output_clock},
    Result,
};
use emi_ex_v2::image::ArchiveImage;
use std::{collections::BTreeMap, fs, path::PathBuf};
fn inputs() -> (Executable, Vec<u8>, ArchiveImage) {
    (
        Executable::from_bytes(fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap(),
        fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap(),
        ArchiveImage::from_bytes(
            fs::read(
                PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap())
                    .join("BIN/BGM/BGM004.EMI"),
            )
            .unwrap(),
        )
        .unwrap(),
    )
}
#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn preparation_observer_preserves_machine_state_and_playback() {
    let (exe, bios, archive) = inputs();
    let mut plain = music::prepare(
        &exe,
        Image::from_bytes(bios.clone()).unwrap(),
        &archive,
        None,
        0,
    )
    .unwrap();
    let mut counts = BTreeMap::<Stage, u64>::new();
    let mut observed = music::prepare_observed(
        &exe,
        Image::from_bytes(bios).unwrap(),
        &archive,
        None,
        0,
        &mut |stage, _| {
            *counts.entry(stage).or_default() += 1;
            Ok(())
        },
    )
    .unwrap();
    assert_eq!(counts.len(), 6);
    assert!(counts.values().all(|&n| n > 0));
    assert_eq!(
        counts.values().sum::<u64>(),
        observed
            .calls
            .iter()
            .map(|c| c.instructions + c.syscalls)
            .sum::<u64>()
    );
    assert_eq!(
        serde_json::to_value(&plain.calls).unwrap(),
        serde_json::to_value(&observed.calls).unwrap()
    );
    assert_eq!(plain.execution.cpu.pc(), observed.execution.cpu.pc());
    assert_eq!(
        plain.execution.cpu.instructions(),
        observed.execution.cpu.instructions()
    );
    for r in 0..32 {
        assert_eq!(
            plain.execution.cpu.register(r),
            observed.execution.cpu.register(r)
        );
    }
    assert_eq!(
        plain.execution.bus.ram().bytes(),
        observed.execution.bus.ram().bytes()
    );
    assert_eq!(
        plain.execution.bus.spu_transfer().ram(),
        observed.execution.bus.spu_transfer().ram()
    );
    let playback = |p: &mut music::Prepared| -> Result<Vec<[i16; 2]>> {
        let e = &mut p.execution;
        e.enable_output(output_clock::Model::PcsxReduxNtsc)?;
        e.call(0x8016b9cc, [p.handle, 0, 1, 1], 100000)?;
        let mut pcm = Vec::new();
        while e.output().unwrap().frames() < 8192 {
            e.call(0x801753dc, [0; 4], 100000)?;
            pcm.extend(e.take_audio_frames()?);
        }
        Ok(pcm)
    };
    assert_eq!(
        playback(&mut plain).unwrap(),
        playback(&mut observed).unwrap()
    );
}
#[test]
#[ignore = "requires BOF3_AUDIO_EXE, BOF3_AUDIO_BIOS and BOF3_AUDIO_CORPUS"]
fn preparation_observer_failure_aborts_the_guest_call() {
    let (exe, bios, archive) = inputs();
    let mut called = 0;
    let result = music::prepare_observed(
        &exe,
        Image::from_bytes(bios).unwrap(),
        &archive,
        None,
        0,
        &mut |stage, e| {
            called += 1;
            assert_eq!(stage, Stage::Initialization);
            assert_eq!(e.cpu.pc(), 0x8015cd00);
            Err("deliberate preparation observation failure".into())
        },
    );
    assert_eq!(called, 1);
    assert!(result
        .err()
        .unwrap()
        .to_string()
        .contains("deliberate preparation observation failure"));
}
