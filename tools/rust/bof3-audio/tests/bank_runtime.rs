use bof3_audio::{
    catalog::loader, digest::sha256_hex, machine::adsr, machine::boot, machine::bus::Bus,
    machine::bus::Width, machine::executable::Executable, machine::execution::Execution,
    machine::firmware::Image, machine::spu_clock, machine::spu_reverb, machine::spu_sample,
    machine::spu_voice_ports::DisableModel,
};
use emi_ex_v2::image::ArchiveImage;

fn copy(execution: &mut Execution, address: u32, data: &[u8]) {
    for (i, &value) in data.iter().enumerate() {
        execution
            .bus
            .write(address + i as u32, Width::Byte, u32::from(value))
            .unwrap();
    }
}

#[test]
#[ignore = "requires BOF3_AUDIO_BIOS, BOF3_AUDIO_EXE and BOF3_AUDIO_CORPUS"]
fn original_bank_transfer_preserves_body_padding_and_uses_bios_interrupts() {
    let exe =
        Executable::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_EXE").unwrap()).unwrap())
            .unwrap();
    let rom =
        Image::from_bytes(std::fs::read(std::env::var_os("BOF3_AUDIO_BIOS").unwrap()).unwrap())
            .unwrap();
    let root = std::path::PathBuf::from(std::env::var_os("BOF3_AUDIO_CORPUS").unwrap());
    let archive =
        ArchiveImage::from_bytes(std::fs::read(root.join("BIN/BGM/BGM000.EMI")).unwrap()).unwrap();
    assert_eq!(
        sha256_hex(archive.bytes()),
        "e1caf8633ce6be70524bd288b5af1c4b6042e4ca5cb3b02600affd68b8013a91"
    );
    let layout = loader::initialize_layout(&exe, 0).unwrap();
    let slot = &layout.slots[0];
    let machine = boot::load_us(rom, &exe, 3_000_000).unwrap();
    let mut execution = Execution::new(
        machine.cpu,
        machine.bus,
        spu_clock::Model::EmulatorReference,
    );
    execution
        .bus
        .configure_spu_voices(
            spu_sample::Model::EmulatorReference,
            adsr::Model::EmulatorReference,
        )
        .unwrap();
    execution
        .bus
        .configure_spu_disable(DisableModel::EmulatorReference)
        .unwrap();
    execution
        .bus
        .configure_spu_reverb(spu_reverb::Model::EmulatorReference)
        .unwrap();
    execution.call(0x8015cd00, [0; 4], 1_000_000).unwrap();
    execution
        .call(loader::LAYOUT_INITIALIZE, [0; 4], 1000)
        .unwrap();
    let before = execution.bus.spu_transfer().ram().to_vec();
    assert_eq!(archive.entries()[0].file_type, 6);
    assert_eq!(archive.entries()[1].file_type, 10);
    assert_eq!(archive.entries()[2].file_type, 7);
    copy(
        &mut execution,
        slot.header_address,
        archive.entry(0).unwrap(),
    );
    copy(
        &mut execution,
        slot.sequence_address,
        archive.entry(1).unwrap(),
    );
    let id = u32::from(slot.vab_id);
    assert_eq!(
        execution
            .call(
                0x80173c50,
                [slot.header_address, id, slot.spu_base, 0],
                100_000
            )
            .unwrap()
            .result,
        id
    );
    let body = archive.entry(2).unwrap();
    let offset = archive.entries()[2].offset as usize;
    let size = body.len().div_ceil(64) * 64;
    let source = &archive.bytes()[offset..offset + size];
    assert_eq!((body.len(), size), (233504, 233536));
    assert!(source[body.len()..].iter().all(|&byte| byte == 0x5f));
    let mut interrupts = 0;
    for (index, chunk) in body.chunks(2048).enumerate() {
        let start = index * 2048;
        copy(
            &mut execution,
            0x80010000,
            &source[start..start + chunk.len().div_ceil(64) * 64],
        );
        let transfer = execution
            .call(0x80174354, [0x80010000, chunk.len() as u32, id, 0], 100_000)
            .unwrap();
        assert_eq!(
            transfer.result,
            if start + chunk.len() == body.len() {
                0
            } else {
                0xffff_fffe
            }
        );
        let wait = execution.call(0x80174598, [1, 0, 0, 0], 100_000).unwrap();
        assert_eq!(wait.result, 1);
        interrupts += transfer.interrupts + wait.interrupts;
    }
    assert!(interrupts > 0);
    let start = slot.spu_base as usize;
    let after = execution.bus.spu_transfer().ram();
    assert_eq!(&after[start..start + size], source);
    assert_eq!(&after[..start], &before[..start]);
    assert_eq!(&after[start + size..], &before[start + size..]);
    assert_eq!(
        execution
            .call(0x8016b38c, [slot.sequence_address, id, 4, 0], 100_000)
            .unwrap()
            .result,
        0
    );

    // SDK mode 1 resolves to NTSC VBlank mode 5. Start installs its original
    // global scheduler pointer through the SDK callback and real BIOS kernel.
    let base = execution.bus.read(0x80190308, Width::Word).unwrap();
    let records_before = execution.bus.ram().bytes()[0x148a50..0x148fb0].to_vec();
    let cursor_before = execution.bus.read(base + 4, Width::Word).unwrap();
    execution.call(0x8016b9cc, [0, 0, 1, 2], 100_000).unwrap();
    execution.call(0x8015ce70, [0; 4], 100_000).unwrap();
    assert_eq!(execution.bus.read(0x80184440, Width::Word).unwrap(), 5);
    assert_eq!(
        execution.bus.read(0x80184448, Width::Word).unwrap(),
        0x8016c548
    );
    assert_eq!(execution.bus.read(0x80184450, Width::Byte).unwrap(), 1);
    assert_ne!(execution.bus.interrupts().mask() & 1, 0);
    let mut key_on_seen = false;
    for _ in 0..120 {
        execution.bus.set_vblank(true);
        let tick = execution.call(0x801753dc, [0; 4], 100_000).unwrap();
        assert_eq!(tick.interrupts, 1);
        assert_eq!(tick.syscalls, 0);
        assert_eq!(execution.bus.interrupts().status() & 1, 0);
        execution.bus.set_vblank(false);
        key_on_seen |= execution.bus.read(0x1f801d88, Width::Half).unwrap() != 0
            || execution.bus.read(0x1f801d8a, Width::Half).unwrap() != 0;
    }
    assert!(key_on_seen);
    assert_ne!(
        execution.bus.read(base + 4, Width::Word).unwrap(),
        cursor_before
    );
    assert_eq!(execution.bus.read(base + 0x90, Width::Word).unwrap(), 1);
    // Only sequence zero was selected: other records stay byte-identical.
    assert_eq!(
        &execution.bus.ram().bytes()[0x148afc..0x148fb0],
        &records_before[0xac..]
    );

    // Stop the first sequence through the original SDK, settle its stop flag,
    // then select sequence one without reopening or merging the SEP streams.
    execution.call(0x8016d534, [0, 0, 0, 0], 100_000).unwrap();
    execution.bus.set_vblank(true);
    execution.call(0x801753dc, [0; 4], 100_000).unwrap();
    execution.bus.set_vblank(false);
    let stopped = execution.bus.ram().bytes()[0x148a50..0x148fb0].to_vec();
    let second = base + 0xac;
    let cursor = execution.bus.read(second + 4, Width::Word).unwrap();
    execution.call(0x8016b9cc, [0, 1, 1, 2], 100_000).unwrap();
    for _ in 0..120 {
        execution.bus.set_vblank(true);
        assert_eq!(
            execution
                .call(0x801753dc, [0; 4], 100_000)
                .unwrap()
                .interrupts,
            1
        );
        execution.bus.set_vblank(false);
    }
    assert_ne!(execution.bus.read(second + 4, Width::Word).unwrap(), cursor);
    assert_eq!(
        &execution.bus.ram().bytes()[0x148a50..0x148afc],
        &stopped[..0xac]
    );
    assert_eq!(
        &execution.bus.ram().bytes()[0x148ba8..0x148fb0],
        &stopped[0x158..]
    );

    // Clocked playback: every guest instruction advances transfer, timer,
    // output and VBlank clocks. Host leaf calls do not dispatch audio events.
    use bof3_audio::machine::output_clock;
    execution
        .enable_output(output_clock::Model::PcsxReduxNtsc)
        .unwrap();
    assert!(execution
        .enable_output(output_clock::Model::PcsxReduxNtsc)
        .is_err());
    let mut frames = 0;
    let mut peak = 0;
    let mut irqs = 0;
    while execution.output().unwrap().vblanks() < 120
        && execution.output().unwrap().ticks() < 3 * 33_868_800
    {
        irqs += execution
            .call(0x801753dc, [0; 4], 100_000)
            .unwrap()
            .interrupts;
        let block = execution.take_audio_frames().unwrap();
        frames += block.len();
        peak = peak.max(
            block
                .iter()
                .flatten()
                .map(|x| x.unsigned_abs())
                .max()
                .unwrap_or(0),
        );
    }
    assert_eq!(execution.output().unwrap().vblanks(), 120);
    // The IRQ edge can coincide with the caller's return instruction.
    if execution.bus.interrupts().pending() {
        irqs += execution
            .call(0x801753dc, [0; 4], 100_000)
            .unwrap()
            .interrupts;
        let block = execution.take_audio_frames().unwrap();
        frames += block.len();
    }
    assert_eq!(irqs, 120);
    assert!(peak > 0);
    assert!((87_000..89_000).contains(&frames));
    let clock = execution.output().unwrap();
    assert_eq!(frames as u64, clock.frames());
    assert_eq!(clock.frames(), clock.ticks() / 768);
}
