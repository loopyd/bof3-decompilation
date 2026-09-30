//! Initial voice volume for the verified US sequence note-on path.
//! This produces SPU register magnitudes, not SoundFont attenuation/pan or PCM.
use crate::Result;
use serde::Serialize;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub struct Context {
    pub bank_volume: u8,
    pub program_volume: u8,
    pub tone_volume: u8,
    pub velocity: u8,
    pub channel_volume: u8,
    pub sequence_volume: [u8; 2],
    pub tone_pan: u8,
    pub program_pan: u8,
    pub channel_pan: u8,
    pub mono: bool,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub struct Registers {
    pub left: u16,
    pub right: u16,
}

impl Context {
    /// The game truncates after each division and squares each final channel.
    /// Scope: ordinary sequence handles, seven-bit inputs, initial note-on.
    /// Special handle 0x21 and subsequent live volume updates are separate paths.
    pub fn registers(self) -> Result<Registers> {
        let values = [
            self.bank_volume,
            self.program_volume,
            self.tone_volume,
            self.velocity,
            self.channel_volume,
            self.sequence_volume[0],
            self.sequence_volume[1],
            self.tone_pan,
            self.program_pan,
            self.channel_pan,
        ];
        if values.iter().any(|&value| value > 127) {
            return Err(
                "voice gain: ordinary sequence context requires seven-bit volumes/pans".into(),
            );
        }
        let velocity = u32::from(self.velocity) * u32::from(self.channel_volume) / 127;
        let bank = velocity * (16383 * u32::from(self.bank_volume)) / 16129;
        let gain = bank * u32::from(self.program_volume) * u32::from(self.tone_volume) / 16129;
        let mut left = gain * u32::from(self.sequence_volume[0]) / 127;
        let mut right = gain * u32::from(self.sequence_volume[1]) / 127;
        for pan in [self.tone_pan, self.program_pan, self.channel_pan] {
            if pan < 64 {
                right = right * u32::from(pan) / 63;
            } else {
                left = left * (127 - u32::from(pan)) / 63;
            }
        }
        if self.mono {
            left = left.max(right);
            right = left;
        }
        Ok(Registers {
            left: (left * left / 16383) as u16,
            right: (right * right / 16383) as u16,
        })
    }
}
