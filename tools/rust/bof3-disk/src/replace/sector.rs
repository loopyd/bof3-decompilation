// Mode 2 Form 1 EDC/ECC adapted from PCSX-Redux's iec-60908b/edcecc.c.
//
// MIT License
//
// Copyright (c) 2022 Nicolas "Pixel" Noble
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

use crate::Error;

pub(super) fn validate(bytes: &[u8]) -> Result<(), Error> {
    if bytes.len() != 2352
        || bytes[..12] != [0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0]
        || bytes[15] != 2
        || bytes[16..20] != bytes[20..24]
        || bytes[18] & 0x20 != 0
    {
        return Err(Error::InvalidImage(
            "replacement requires raw Mode 2 Form 1 sectors with matching subheaders",
        ));
    }
    let mut expected = bytes.to_vec();
    repair(&mut expected);
    if expected != bytes {
        return Err(Error::InvalidImage(
            "original replacement sector has invalid EDC/ECC",
        ));
    }
    Ok(())
}

pub(super) fn repair(bytes: &mut [u8]) {
    let mut edc = 0_u32;
    for byte in &bytes[16..2072] {
        edc ^= u32::from(*byte);
        for _ in 0..8 {
            edc = (edc >> 1) ^ if edc & 1 != 0 { 0xd8018001 } else { 0 };
        }
    }
    bytes[2072..2076].copy_from_slice(&edc.to_le_bytes());
    let location: [u8; 4] = bytes[12..16].try_into().unwrap();
    bytes[12..16].fill(0);
    let mut mul2 = [0_u8; 256];
    let mut div3 = [0_u8; 256];
    for value in 0..256 {
        let doubled = ((value << 1) ^ if value & 0x80 != 0 { 0x11d } else { 0 }) as u8;
        mul2[value] = doubled;
        div3[usize::from(doubled ^ value as u8)] = value as u8;
    }
    for column in 0..86 {
        let (low, high) = parity(
            (0..24).map(|row| bytes[12 + 86 * row + column]),
            &mul2,
            &div3,
        );
        bytes[2076 + column] = low;
        bytes[2162 + column] = high;
    }
    for column in 0..52 {
        let (low, high) = parity(
            (0..43)
                .map(|row| bytes[12 + ((44 * row + 43 * (column / 2)) % 1118) * 2 + (column & 1)]),
            &mul2,
            &div3,
        );
        bytes[2248 + column] = low;
        bytes[2300 + column] = high;
    }
    bytes[12..16].copy_from_slice(&location);
}

fn parity(bytes: impl Iterator<Item = u8>, mul2: &[u8; 256], div3: &[u8; 256]) -> (u8, u8) {
    let mut low = 0_u8;
    let mut high = 0_u8;
    for byte in bytes {
        low = mul2[usize::from(low ^ byte)];
        high ^= byte;
    }
    low = div3[usize::from(mul2[usize::from(low)] ^ high)];
    (low, high ^ low)
}
