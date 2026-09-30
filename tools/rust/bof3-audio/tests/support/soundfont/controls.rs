use bof3_audio::{soundfont::gain::Parameters, soundfont::reader::Font};

pub fn set(bytes: &mut [u8], instrument: usize, parameters: Parameters, all_zones: bool) {
    map(bytes, instrument, all_zones, |op, amount| match op {
        17 => parameters.pan as u16,
        48 => parameters.attenuation_cb,
        _ => amount,
    });
}

pub fn map(
    bytes: &mut [u8],
    instrument: usize,
    all_zones: bool,
    mut edit: impl FnMut(u16, u16) -> u16,
) {
    let font = Font::from_bytes(bytes.to_vec()).unwrap();
    let start = |id: &[u8; 4]| font.chunks.iter().find(|c| &c.id == id).unwrap().data.start;
    let inst = start(b"inst");
    let bags = start(b"ibag");
    let gens = start(b"igen");
    let word = |at: usize| usize::from(u16::from_le_bytes(bytes[at..at + 2].try_into().unwrap()));
    let first = word(inst + instrument * 22 + 20);
    let end = word(inst + (instrument + 1) * 22 + 20);
    let ranges: Vec<_> = (first..end)
        .map(|bag| word(bags + bag * 4)..word(bags + (bag + 1) * 4))
        .collect();
    for range in ranges
        .into_iter()
        .take(if all_zones { usize::MAX } else { 1 })
    {
        for index in range {
            let row = &mut bytes[gens + index * 4..gens + index * 4 + 4];
            let amount = edit(
                u16::from_le_bytes([row[0], row[1]]),
                u16::from_le_bytes([row[2], row[3]]),
            );
            row[2..].copy_from_slice(&amount.to_le_bytes());
        }
    }
}
