use super::{call, fixture, prepare, BASE, DATA, RETURN};
use bof3_audio::{
    machine::bus::{Bus, Width},
    sequence::editing::{self, Options},
    sequence::events::Kind,
    sequence::midi as translation,
    sequence::timeline,
    sequence::Sequence,
};

#[test]
#[ignore = "requires BOF3_AUDIO_EXE; retimed streams through original delta reader and loop dispatcher"]
fn retimed_delta_and_loop_streams_match_original_us_execution() {
    let mut ram = fixture();
    let mut cases = 0;
    for count in [0, 1, 2, 3, 127] {
        for factor in [1, 100, 10000] {
            let source = [
                3, 0xb0, 99, 20, 0, 98, count, 0, 0xc1, 8, 5, 0xb2, 99, 30, 7, 0xff, 0x2f, 0xee,
            ];
            let sequence = Sequence {
                sequence_index: 0,
                sequence_id: 17,
                resolution: 96,
                tempo_us: 500000,
                time_numerator: 4,
                time_denominator_power: 2,
                data_offset: 19,
                data_bytes: source.len(),
            };
            let options = Options::default();
            let mut baseline = translation::translate(&sequence, &source, &options.limits).unwrap();
            translation::initialize_channels(&mut baseline).unwrap();
            let mut edited = baseline.midi.clone();
            for t in &mut edited.tracks {
                for e in &mut t.events {
                    e.tick *= factor;
                }
            }
            let rebuilt = editing::rebuild(&sequence, &source, &edited, &options).unwrap();
            let trace = timeline::trace(&rebuilt.bytes, &options.limits).unwrap();
            for offset in 0..0xac {
                ram.write(BASE + offset, Width::Byte, 0).unwrap();
            }
            let state = prepare(&mut ram, 0, 0, &rebuilt.bytes);
            let initial = call(&mut ram, 0x8016_aad4, &[0, 0], RETURN).register(2);
            assert_eq!(initial, 30 * factor as u32);
            let mut tick = u64::from(initial / 10);
            for (index, step) in trace.steps.iter().enumerate() {
                let m = &baseline.report.mappings[index];
                assert_eq!(tick, edited.tracks[m.track].events[m.event].tick);
                assert_eq!(
                    ram.read(state + 4, Width::Word).unwrap(),
                    DATA + step.source_cursor as u32
                );
                if step.kind == Kind::End {
                    break;
                }
                call(&mut ram, 0x8016_d0e0, &[0, 0], RETURN);
                assert_eq!(
                    ram.read(state + 4, Width::Word).unwrap(),
                    DATA + step.next_cursor as u32
                );
                let delay = ram.read(state + 0x88, Width::Word).unwrap();
                assert_eq!(delay, step.next_delay * 10);
                tick += u64::from(delay / 10);
            }
            cases += 1;
        }
    }
    assert_eq!(cases, 15);
    eprintln!("Retimed original-US execution: {cases} streams match requested ticks and relocated cursors through the original delta reader and controller dispatcher; no scheduler/audio fidelity claim.");
}
