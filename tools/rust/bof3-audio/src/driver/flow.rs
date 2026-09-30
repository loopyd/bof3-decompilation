#[derive(Clone, Copy, Debug)]
pub(super) enum Flow {
    Next,
    Branch {
        target: u32,
        taken: bool,
        untaken: bool,
        link: bool,
    },
    Jump {
        target: u32,
        link: bool,
    },
    Indirect {
        register: u8,
        link: bool,
    },
    Trap(&'static str),
    Unknown,
}
pub(super) fn decode(pc: u32, word: u32) -> Flow {
    let rs = ((word >> 21) & 31) as u8;
    let rt = ((word >> 16) & 31) as u8;
    let target = pc
        .wrapping_add(4)
        .wrapping_add(((word as i16 as i32) << 2) as u32);
    match word >> 26 {
        0 => match word & 63 {
            8 | 9 => Flow::Indirect {
                register: rs,
                link: word & 63 == 9 && (word >> 11) & 31 != 0,
            },
            12 => Flow::Trap("syscall exception handler and return"),
            13 => Flow::Trap("break exception handler and return"),
            0 | 2..=4 | 6 | 7 | 16..=19 | 24..=27 | 32..=39 | 42 | 43 => Flow::Next,
            _ => Flow::Unknown,
        },
        1 if matches!(rt, 0 | 1 | 16 | 17) => Flow::Branch {
            target,
            taken: rs != 0 || rt & 1 != 0,
            untaken: rs != 0 || rt & 1 == 0,
            link: rt >= 16,
        },
        2 | 3 => Flow::Jump {
            target: (pc.wrapping_add(4) & 0xf0000000) | ((word & 0x03ffffff) << 2),
            link: word >> 26 == 3,
        },
        4 => Flow::Branch {
            target,
            taken: true,
            untaken: rs != rt,
            link: false,
        },
        5 => Flow::Branch {
            target,
            taken: rs != rt,
            untaken: true,
            link: false,
        },
        6 if rt == 0 => Flow::Branch {
            target,
            taken: true,
            untaken: rs != 0,
            link: false,
        },
        7 if rt == 0 => Flow::Branch {
            target,
            taken: rs != 0,
            untaken: true,
            link: false,
        },
        8..=15 | 0x20..=0x26 | 0x28..=0x2b | 0x2e => Flow::Next,
        0x10 if ((rs == 0 || rs == 4) && word & 0x7ff == 0) || word == 0x42000010 => Flow::Next,
        // COP2/GTE is a dependency needing separate verification, not normal code.
        _ => Flow::Unknown,
    }
}
