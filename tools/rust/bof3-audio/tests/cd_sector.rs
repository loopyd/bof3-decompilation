use bof3_audio::machine::{
    bus::{Bus, Ram, Width},
    cd_data::{Fifo, Model},
    cd_dma::Channel,
    interconnect::Interconnect,
};

fn payload(size: usize) -> Vec<u8> {
    (0..size).map(|i| (i * 37 + i / 256) as u8).collect()
}
fn bus() -> Interconnect {
    let mut bus = Interconnect::from_ram(Ram::default());
    bus.configure_cd_host([128, 0, 128, 0]).unwrap();
    bus.configure_cd_data(Model::EmulatorReference).unwrap();
    bus.write(0x1f801018, Width::Word, 0x20943).unwrap();
    bus.write(0x1f801020, Width::Word, 0x1323).unwrap();
    bus
}
fn start(bus: &mut Interconnect, address: u32, words: u32, control: u32) {
    bus.write(0x1f8010b0, Width::Word, address).unwrap();
    bus.write(0x1f8010b4, Width::Word, 0xabcd0000 | words)
        .unwrap();
    bus.write(0x1f8010b8, Width::Word, control).unwrap();
}
fn request(bus: &mut Interconnect) {
    bus.write(0x1f801800, Width::Byte, 0).unwrap();
    bus.write(0x1f801803, Width::Byte, 0x80).unwrap();
}

#[test]
fn block_requests_partial_reads_rewind_and_exhaustion_follow_selected_model() {
    for size in [2048, 2340] {
        let bytes = payload(size);
        let mut fifo = Fifo::new(Model::EmulatorReference);
        assert!(fifo.request(true).is_err());
        assert!(fifo.present(&bytes[..size - 1]).is_err());
        fifo.present(&bytes).unwrap();
        assert!(!fifo.ready());
        assert!(fifo.read(1).is_err());
        assert!(fifo.present(&bytes).is_err());
        fifo.request(true).unwrap();
        assert_eq!(fifo.read(1).unwrap(), u32::from(bytes[0]));
        fifo.request(true).unwrap(); // Repeated assertion keeps the cursor.
        assert_eq!(
            fifo.read(2).unwrap(),
            u32::from(u16::from_le_bytes([bytes[1], bytes[2]]))
        );
        fifo.request(false).unwrap();
        assert_eq!(fifo.remaining(), size);
        fifo.request(true).unwrap();
        for word in bytes.as_chunks::<4>().0 {
            assert_eq!(fifo.read(4).unwrap(), u32::from_le_bytes(*word));
        }
        assert!(!fifo.ready());
        assert_eq!(fifo.remaining(), 0);
        assert!(fifo.read(1).is_err());
        assert!(fifo.request(true).is_err());
        fifo.present(&bytes).unwrap();
    }
}

#[test]
fn data_mmio_shares_byte_halfword_cursor_and_rejects_atomic_overread() {
    let mut bus = bus();
    let bytes = payload(2048);
    bus.present_cd_data(&bytes).unwrap();
    request(&mut bus);
    assert_eq!(bus.read(0xbf801800, Width::Byte).unwrap() & 0x40, 0x40);
    assert_eq!(
        bus.read(0x9f801802, Width::Half).unwrap(),
        u32::from(u16::from_le_bytes([bytes[0], bytes[1]]))
    );
    for &byte in &bytes[2..2047] {
        assert_eq!(bus.read(0x1f801802, Width::Byte).unwrap(), u32::from(byte));
    }
    assert!(bus.read(0x1f801802, Width::Half).is_err());
    assert_eq!(
        bus.read(0x1f801802, Width::Byte).unwrap(),
        u32::from(bytes[2047])
    );
    assert_eq!(bus.read(0x1f801800, Width::Byte).unwrap() & 0x40, 0);
    assert!(bus.read(0x1f801802, Width::Word).is_err());
}

#[test]
fn burst_count_zero_address_wrap_and_visible_registers_match_manual_mode() {
    let mut dma = Channel::default();
    dma.write(0, 3).unwrap();
    dma.write(4, 0xabcd0000).unwrap();
    dma.write(8, 0x11000002).unwrap();
    for i in 0..65536u32 {
        assert_eq!(
            dma.next_word(false),
            Some(0u32.wrapping_sub(i * 4) & 0x00ffffff)
        );
        assert_eq!(dma.finish_word().unwrap(), i == 65535);
    }
    assert_eq!(dma.read(0).unwrap(), 3);
    assert_eq!(dma.read(4).unwrap(), 0xabcd0000);
    assert_eq!(dma.read(8).unwrap(), 2);
    assert!(!dma.active());
    assert!(dma.finish_word().is_err());
}

#[test]
fn dma_priority_gating_budgets_ram_mirrors_and_irq_acknowledgement() {
    let mut bus = bus();
    let bytes = payload(2048);
    bus.present_cd_data(&bytes).unwrap();
    request(&mut bus);
    let enabled = (1 << 23) | (1 << 19);
    bus.write(0x1f8010f4, Width::Word, enabled).unwrap();
    start(&mut bus, 0x210003, 512, 0x11000000);
    assert_eq!(bus.service_cd(512).unwrap(), 0);
    bus.write(0x1f8010f0, Width::Word, 1 << 15).unwrap();
    assert_eq!(bus.service_cd(0).unwrap(), 0);
    assert_eq!(bus.read(0x1f8010b8, Width::Word).unwrap(), 0x11000000);
    assert_eq!(bus.service_cd(17).unwrap(), 17);
    assert_eq!(bus.read(0x1f8010b8, Width::Word).unwrap(), 0x01000000);
    assert_eq!(bus.service_cd(1000).unwrap(), 495);
    assert_eq!(&bus.ram().bytes()[0x10000..0x10800], &bytes);
    assert_eq!(bus.read(0x1f8010b0, Width::Word).unwrap(), 0x210003);
    assert_eq!(bus.read(0x1f8010b4, Width::Word).unwrap(), 0xabcd0200);
    assert_eq!(bus.dma().interrupt(), enabled | (1 << 27) | (1 << 31));
    assert_eq!(bus.interrupts().status() & 8, 8);
    bus.write(0x1f8010f4, Width::Word, enabled | (1 << 27))
        .unwrap();
    assert_eq!(bus.interrupts().status() & 8, 8);
    bus.write(0x1f801070, Width::Word, !8).unwrap();
    assert_eq!(bus.interrupts().status() & 8, 0);
}

#[test]
fn unsupported_transfers_and_bus_errors_preserve_pending_data() {
    let mut dma = Channel::default();
    for control in [0x11000001, 0x11400100, 0x01000200, 0x31000000, 0x51000000] {
        assert!(dma.write(8, control).is_err());
        assert!(!dma.active());
    }
    dma.write(4, 2).unwrap();
    dma.write(8, 0x01000000).unwrap();
    assert_eq!(dma.next_word(false), None);
    assert!(dma.write(0, 4).is_err());
    assert!(dma.write(4, 4).is_err());
    assert!(dma.write(8, 0x11000000).is_err());
    assert_eq!(dma.next_word(true), Some(0));
    assert!(!dma.finish_word().unwrap());
    assert_eq!(dma.next_word(false), Some(4));
    dma.write(8, 0).unwrap();
    assert!(!dma.active());
    let mut bus = bus();
    let bytes = payload(2048);
    bus.present_cd_data(&bytes).unwrap();
    request(&mut bus);
    bus.write(0x1f8010f0, Width::Word, 1 << 15).unwrap();
    start(&mut bus, 0x800000, 1, 0x11000000);
    assert!(bus
        .service_cd(1)
        .unwrap_err()
        .to_string()
        .contains("RAM mirror"));
    assert_eq!(bus.dma().interrupt() & 0x80008000, 0x80008000);
    assert_eq!(
        bus.read(0x1f801802, Width::Byte).unwrap(),
        u32::from(bytes[0])
    );
}

#[test]
fn dma_overread_does_not_report_completion_or_discard_prior_progress() {
    let mut bus = bus();
    let bytes = payload(2048);
    bus.present_cd_data(&bytes).unwrap();
    request(&mut bus);
    bus.write(0x1f8010f0, Width::Word, 1 << 15).unwrap();
    start(&mut bus, 0x10000, 513, 0x11000000);
    assert!(bus
        .service_cd(513)
        .unwrap_err()
        .to_string()
        .contains("overread"));
    assert_eq!(&bus.ram().bytes()[0x10000..0x10800], &bytes);
    assert_eq!(bus.read(0x1f8010b8, Width::Word).unwrap(), 0x01000000);
    assert_eq!(bus.dma().interrupt(), 0);
    assert!(bus.write(0x1f801018, Width::Word, 0).is_err());
    assert!(bus.write(0x1f801020, Width::Half, 0x1323).is_err());
}
