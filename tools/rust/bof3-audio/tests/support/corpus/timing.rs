use bof3_audio::{
    interchange::midi::{Message, Midi},
    sequence::editing::{self, Options},
    sequence::midi as translation,
    sequence::SequenceSet,
};

pub fn check(bytes: &[u8], midis: &[Midi]) -> (usize, usize) {
    let mut edited = midis.to_vec();
    for midi in &mut edited {
        for track in &mut midi.tracks {
            for event in &mut track.events {
                event.tick *= 3;
            }
        }
    }
    let selections: Vec<_> = edited.iter().enumerate().collect();
    let rebuilt = editing::rebuild_entry(bytes, &selections, &Options::default()).unwrap();
    let parsed = SequenceSet::parse(&rebuilt.bytes).unwrap();
    for (sequence, desired) in parsed.sequences.iter().zip(&edited) {
        let mut actual = translation::translate(
            sequence,
            &rebuilt.bytes[sequence.data_offset..sequence.data_offset + sequence.data_bytes],
            &Options::default().limits,
        )
        .unwrap();
        translation::initialize_channels(&mut actual).unwrap();
        for (a, b) in desired.tracks.iter().zip(&actual.midi.tracks) {
            assert_eq!(a.events.len(), b.events.len());
            for (a, b) in a.events.iter().zip(&b.events) {
                assert_eq!(a.tick, b.tick);
                if !matches!(a.message, Message::Meta { kind: 6, .. }) {
                    assert_eq!(a.message, b.message);
                }
            }
        }
        let consumer =
            rustysynth::MidiFile::new(&mut std::io::Cursor::new(actual.midi.to_bytes().unwrap()))
                .unwrap();
        let before = rustysynth::MidiFile::new(&mut std::io::Cursor::new(
            midis[sequence.sequence_index].to_bytes().unwrap(),
        ))
        .unwrap();
        assert!((consumer.get_length() - 3.0 * before.get_length()).abs() < 1e-6);
    }
    let deltas = rebuilt.reports.iter().map(|r| r.timing_edits.len()).sum();
    (deltas, usize::from(rebuilt.bytes.len() != bytes.len()))
}
