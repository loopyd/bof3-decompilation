pub fn synthetic_archive(unsupported: bool) -> Vec<u8> {
    let mut vh = vec![0; 0x820 + 2 * 512 + 512];
    vh[..4].copy_from_slice(b"pBAV");
    vh[4..8].copy_from_slice(&7u32.to_le_bytes());
    vh[8..12].copy_from_slice(&41u32.to_le_bytes());
    for (at, value) in [(0x12, 2u16), (0x14, 3), (0x16, 1)] {
        vh[at..at + 2].copy_from_slice(&value.to_le_bytes());
    }
    vh[0x18] = 127;
    vh[0x19] = 64;
    for (block, program) in [0usize, 5].into_iter().enumerate() {
        let p = 32 + program * 16;
        vh[p] = 1;
        vh[p + 1] = 127;
        vh[p + 4] = 64;
        let t = 0x820 + block * 512;
        vh[t + 2] = 127;
        vh[t + 3] = 64;
        vh[t + 4] = 60;
        vh[t + 6] = 60;
        vh[t + 7] = 61;
        vh[t + 16..t + 18].copy_from_slice(&0x000fu16.to_le_bytes());
        vh[t + 18..t + 20].copy_from_slice(&0x1fc0u16.to_le_bytes());
        vh[t + 20..t + 22].copy_from_slice(&(program as i16).to_le_bytes());
        vh[t + 22..t + 24].copy_from_slice(&1u16.to_le_bytes());
    }
    vh[32] = 2;
    vh.copy_within(0x820..0x840, 0x840);
    vh[0x823] = 32;
    vh[0x843] = 96;
    vh[0xc22..0xc24].copy_from_slice(&4u16.to_le_bytes());
    let mut body = vec![0; 32];
    body[0] = 0x04;
    body[2..16].fill(0x77);
    body[16] = 0x04;
    body[17] = 1;
    body[18..].fill(0x88);
    let first = vec![
        0, 0xc0, 0, 0, 0x90, 60, 100, 3, 0xe0, 17, 65, 0, 0xb0, 7, 90, 0, 0xb0, 10, 30, 4, 0xff,
        0x51, 6, 0x1a, 0x80, 5, 0x90, 60, 0, 0, 0xff, 0x2f, 0,
    ];
    let second = if unsupported {
        vec![0, 0xb9, 64, 127, 0, 0xff, 0x2f, 0]
    } else {
        vec![
            0, 0xb9, 99, 20, 0, 98, 127, 0, 0xc9, 5, 0, 0x99, 61, 90, 12, 0x99, 61, 0, 0, 0xb9, 99,
            30, 7, 0xff, 0x2f, 0,
        ]
    };
    let mut sep = b"pQES\0\0".to_vec();
    for (id, data) in [(17u16, first), (91, second)] {
        sep.extend(id.to_be_bytes());
        sep.extend(96u16.to_be_bytes());
        sep.extend([7, 0xa1, 0x20, 4, 2]);
        sep.extend((data.len() as u32).to_be_bytes());
        sep.extend(data);
    }
    sep.extend([0; 19]);
    let mut emi = vec![0; 2048];
    emi[..4].copy_from_slice(&4u32.to_le_bytes());
    emi[8..16].copy_from_slice(b"MATH_TBL");
    for (index, (kind, data)) in [(6u16, vh), (7, body), (9, sep), (0x55, b"opaque".to_vec())]
        .into_iter()
        .enumerate()
    {
        let at = 16 + index * 16;
        emi[at..at + 4].copy_from_slice(&(data.len() as u32).to_le_bytes());
        emi[at + 4..at + 8].copy_from_slice(&2u32.to_le_bytes());
        emi[at + 12..at + 14].copy_from_slice(&kind.to_le_bytes());
        emi.extend(data);
        emi.resize(emi.len().div_ceil(2048) * 2048, 0xa5);
    }
    emi.extend(b"archive trailer");
    emi
}
