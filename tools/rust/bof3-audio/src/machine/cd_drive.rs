//! Functional CD drive transactions with caller-supplied mechanical boundaries.
//! No BIOS defaults, authentication, media clock or response latency is inferred.
use super::{cd_host::Command, cd_position::CdPosition};
use crate::{codec::edc, Result};

/// Explicit program and pregap boundaries; no image metadata is inferred.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum TrackKind {
    Data,
    Audio,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Track {
    pub kind: TrackKind,
    pub start: CdPosition,
    pub pregap_start: CdPosition,
    pub end: CdPosition,
}

/// Consecutive tracks from one and exclusive lead-out, in absolute positions.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Toc {
    tracks: Vec<Track>,
    lead_out: CdPosition,
}

impl Toc {
    pub fn new(tracks: Vec<Track>, lead_out: CdPosition) -> Result<Self> {
        if !(1..=99).contains(&tracks.len())
            || tracks.iter().any(|track| {
                track.pregap_start.lba() < 0
                    || track.pregap_start.lba() > track.start.lba()
                    || track.start.lba() >= track.end.lba()
                    || track.end.lba() > lead_out.lba()
            })
            || tracks.windows(2).any(|pair| {
                pair[0].start.lba() >= pair[1].pregap_start.lba()
                    || pair[0].end.lba() > pair[1].start.lba()
            })
        {
            return Err(
                "CD TOC: require 1..99 ordered programs, valid pregaps and exclusive ends at or before lead-out"
                    .into(),
            );
        }
        Ok(Self { tracks, lead_out })
    }

    fn physical_track(&self, lba: i32) -> usize {
        self.tracks
            .partition_point(|track| track.start.lba() <= lba)
            .saturating_sub(1)
    }

    fn subq_track(&self, lba: i32) -> Option<usize> {
        if lba >= self.lead_out.lba() {
            return None;
        }
        let mut index = self.physical_track(lba);
        if self
            .tracks
            .get(index + 1)
            .is_some_and(|track| lba >= track.pregap_start.lba())
        {
            index += 1;
        }
        Some(index)
    }

    fn position(&self, lba: i32) -> Result<Option<[u8; 8]>> {
        let Some(index) = self.subq_track(lba) else {
            return Ok(None);
        };
        let start = self.tracks[index].start.lba();
        let relative = CdPosition::from_lba((lba - start).abs() - 150)?.bcd();
        let absolute = CdPosition::from_lba(lba)?.bcd();
        Ok(Some([
            bcd(index + 1),
            u8::from(lba >= start),
            relative[0],
            relative[1],
            relative[2],
            absolute[0],
            absolute[1],
            absolute[2],
        ]))
    }
}

/// A supplied playback boundary. Feed audio through the existing CDDA queue;
/// response publication and pending-feed completion remain caller-owned.
#[derive(Debug, PartialEq, Eq)]
pub struct PlayDelivery {
    pub audio: Option<Vec<u8>>,
    pub response: Option<Response>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Response {
    pub interrupt: u8,
    pub bytes: Vec<u8>,
}

#[derive(Debug)]
pub struct AppliedCommand {
    pub response: Response,
    pub xa_predictors_reset: bool,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Activity {
    Paused,
    Resetting,
    PreparingSeek {
        target: i32,
        physical: bool,
        status: u8,
    },
    Seeking {
        target: i32,
        physical: bool,
    },
    PreparingRead {
        target: i32,
    },
    SeekingRead {
        target: i32,
    },
    Reading,
    Playing,
    PreparingPlay {
        target: i32,
    },
    SeekingPlay {
        target: i32,
    },
    Pausing,
}

#[derive(Debug, PartialEq, Eq)]
pub enum Delivery {
    /// TOC-proven media error before a backing read; caller publishes INT5.
    Error(Response),
    /// Caller publishes the block and INT1 at its selected service boundary.
    Data {
        bytes: Vec<u8>,
        response: Response,
    },
    /// Extracted 2336-byte sector for the existing XA decoder; no data IRQ.
    Xa(Vec<u8>),
    Filtered,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Drive {
    activity: Activity,
    mode: u8,
    double_speed: bool,
    speed_changed: bool,
    filter: [u8; 2],
    next_lba: i32,
    seek_lba: i32,
    setloc_pending: bool,
    last_header: Option<[u8; 8]>,
    header_valid: bool,
    muted: bool,
    xa_ended: bool,
    toc: Option<Toc>,
    peak_right: Option<bool>,
    play_track: u8,
    play_start_cycle: u32,
    motor_on: bool,
    report: bool,
    auto_pause: bool,
}

impl Drive {
    /// Explicit already-authenticated, closed, spinning, unmuted disc context.
    /// This constructor is not a substitute for Init, boot or authentication.
    pub fn ready(position: CdPosition, mode: u8, filter: [u8; 2]) -> Result<Self> {
        check_mode(mode)?;
        if position.lba() < 0 {
            return Err("CD drive: lead-in media is not implemented".into());
        }
        Ok(Self {
            activity: Activity::Paused,
            mode,
            double_speed: mode & 0x80 != 0,
            speed_changed: false,
            filter,
            next_lba: position.lba(),
            seek_lba: position.lba(),
            setloc_pending: false,
            last_header: None,
            header_valid: false,
            muted: false,
            xa_ended: false,
            toc: None,
            peak_right: None,
            play_track: 0,
            play_start_cycle: 0,
            motor_on: true,
            report: mode & 4 != 0,
            auto_pause: mode & 2 != 0,
        })
    }

    /// Bind this disc's immutable metadata once, while paused. Data-only callers
    /// may omit it; GetTN/GetTD then fail without inventing a track or lead-out.
    pub fn configure_toc(&mut self, toc: Toc) -> Result<()> {
        if self.activity != Activity::Paused || self.toc.is_some() {
            return Err("CD drive: TOC requires a paused drive without configured metadata".into());
        }
        self.toc = Some(toc);
        Ok(())
    }

    /// Supply the native device's retained peak-channel phase once. Data-only
    /// callers need no playback context. This does not reset XA or queued audio.
    pub fn configure_playback(&mut self, peak_right: bool) -> Result<()> {
        if self.activity != Activity::Paused || self.toc.is_none() || self.peak_right.is_some() {
            return Err(
                "CD playback: require paused drive, explicit TOC and no prior playback context"
                    .into(),
            );
        }
        self.peak_right = Some(peak_right);
        Ok(())
    }

    pub fn activity(&self) -> Activity {
        self.activity
    }
    pub fn next_lba(&self) -> i32 {
        self.next_lba
    }
    pub fn muted(&self) -> bool {
        self.muted
    }
    pub(crate) fn mode(&self) -> u8 {
        self.mode
    }
    pub(crate) fn double_speed(&self) -> bool {
        self.double_speed
    }
    /// Native explicit-seek duration, before consuming the speed-settle latch.
    pub(crate) fn seek_delay_cycles(&self) -> Result<u64> {
        let Activity::PreparingSeek {
            target, physical, ..
        } = self.activity
        else {
            return Err("CD drive: seek delay requires a preparing seek".into());
        };
        Ok(self.seek_delay_micros(target, physical, 167_000)? * 33_868_800 / 1_000_000)
    }
    fn seek_delay_micros(&self, target: i32, physical: bool, base: u64) -> Result<u64> {
        self.toc
            .as_ref()
            .ok_or("CD seek timing requires explicit TOC metadata")?;
        Ok(match self.media_error_code(target, physical) {
            Some(0x10) => 650_000,
            Some(4) => 4_000_000,
            _ => (i64::from(target) - i64::from(self.next_lba)).unsigned_abs() * 3 + base,
        } + if self.speed_changed { 650_000 } else { 0 })
    }
    pub(crate) fn read_seek_delay_cycles(&self) -> Result<u64> {
        let (target, physical) = match self.activity {
            Activity::PreparingRead { target } => (target, false),
            Activity::PreparingPlay { target } => (target, true),
            _ => return Err("CD drive: read seek delay requires preparing read or Play".into()),
        };
        let micros = self.seek_delay_micros(target, physical, 1_000)? + self.read_period_micros();
        Ok(micros * 33_868_800 / 1_000_000)
    }
    fn read_period_micros(&self) -> u64 {
        if self.double_speed {
            6_666
        } else {
            13_333
        }
    }
    pub(crate) fn read_period_cycles(&self) -> u64 {
        self.read_period_micros() * 33_868_800 / 1_000_000
    }
    pub(crate) fn read_media_valid(&self) -> bool {
        self.media_error_code(self.next_lba, false).is_none()
    }
    pub(crate) fn adpcm_busy(&self) -> bool {
        self.mode & 0x40 != 0 && self.status() & 0xe0 == 0x20 && !self.xa_ended
    }
    pub fn status(&self) -> u8 {
        (u8::from(self.motor_on) * 2)
            | match self.activity {
                Activity::Reading => 0x20,
                Activity::PreparingSeek { status, .. } => status,
                Activity::Seeking { .. }
                | Activity::SeekingRead { .. }
                | Activity::SeekingPlay { .. } => 0x40,
                Activity::Playing => 0x80,
                Activity::Paused
                | Activity::Resetting
                | Activity::PreparingRead { .. }
                | Activity::PreparingPlay { .. }
                | Activity::Pausing => 0,
            }
    }
    fn response(&self, interrupt: u8) -> Response {
        Response {
            interrupt,
            bytes: vec![self.status()],
        }
    }
    fn parameter_error(&self, code: u8) -> Response {
        Response {
            interrupt: 5,
            bytes: vec![self.status() | 1, code],
        }
    }

    fn media_error_code(&self, lba: i32, physical_seek: bool) -> Option<u8> {
        let toc = self.toc.as_ref()?;
        match toc.subq_track(lba) {
            None => Some(0x10),
            Some(index) if !physical_seek && toc.tracks[index].kind == TrackKind::Audio => Some(4),
            _ => None,
        }
    }

    fn media_error(&self, code: u8) -> Response {
        Response {
            interrupt: 5,
            bytes: vec![self.status() | 4, code],
        }
    }

    /// Execute one accepted command. The caller owns host acceptance/IRQ timing.
    /// Unsupported operations return Err without changing drive state; known bad
    /// parameters produce the documented INT5 response instead of success.
    /// cycle is the explicit 33,868,800 Hz command boundary, used by immediate Play.
    pub fn command(&mut self, command: &Command, cycle: u32) -> Result<Response> {
        let count = match command.opcode {
            0x01 | 0x06 | 0x09 | 0x0a | 0x0b | 0x0c | 0x0f | 0x10 | 0x11 | 0x13 | 0x15 | 0x16
            | 0x1b => 0,
            0x03 => command.parameters.len(),
            0x02 => 3,
            0x0d => 2,
            0x0e | 0x14 => 1,
            value => {
                return Err(format!("CD drive: command {value:#04x} is not implemented").into())
            }
        };
        if command.parameters.len() > 16 || command.parameters.len() != count {
            return Ok(self.parameter_error(0x20));
        }
        // Read cancels an unfinished SeekP command and replaces its continuation.
        let read_takes_over_seekp = matches!(command.opcode, 0x06 | 0x1b)
            && matches!(
                self.activity,
                Activity::PreparingSeek { physical: true, .. }
                    | Activity::Seeking { physical: true, .. }
            );
        if matches!(
            self.activity,
            Activity::PreparingSeek { .. }
                | Activity::Seeking { .. }
                | Activity::PreparingRead { .. }
                | Activity::SeekingRead { .. }
                | Activity::PreparingPlay { .. }
                | Activity::SeekingPlay { .. }
                | Activity::Pausing
                | Activity::Resetting
        ) && !matches!(command.opcode, 0x01 | 0x0b | 0x0c | 0x0f | 0x10 | 0x11)
            && !read_takes_over_seekp
        {
            return Err("CD drive: command during mechanical transition is not implemented".into());
        }
        if command.opcode == 0x0a {
            self.motor_on = true;
        }
        let response = self.response(3);
        match command.opcode {
            0x01 => {}
            0x02 => {
                let Ok(position) = CdPosition::from_bcd(command.parameters[..].try_into()?) else {
                    return Ok(self.parameter_error(0x10));
                };
                if position.lba() < 0 {
                    return Err("CD drive: seeking into lead-in is not implemented".into());
                }
                self.seek_lba = position.lba();
                self.setloc_pending = true;
            }
            0x03 => {
                let toc = self
                    .toc
                    .as_ref()
                    .ok_or("CD Play requires explicit TOC metadata")?;
                if self.peak_right.is_none() || !self.motor_on {
                    return Err("CD Play requires explicit peak phase and a spinning motor".into());
                }
                let requested = command.parameters.first().copied().unwrap_or(0);
                let mut track = if requested >> 4 <= 9 && requested & 15 <= 9 {
                    usize::from(requested >> 4) * 10 + usize::from(requested & 15)
                } else {
                    100
                };
                if track > toc.tracks.len() {
                    track = toc
                        .position(self.next_lba)?
                        .map_or(0, |q| usize::from(q[0] >> 4) * 10 + usize::from(q[0] & 15));
                }
                if track != 0 {
                    self.seek_lba = toc.tracks[track - 1].start.lba();
                    self.setloc_pending = true;
                }
                let target = self.setloc_pending.then_some(self.seek_lba);
                self.setloc_pending = false;
                if let Some(target) = target {
                    self.activity = Activity::PreparingPlay { target };
                } else {
                    self.start_playing(cycle);
                }
            }
            0x06 | 0x1b | 0x15 | 0x16 => {
                self.setloc_pending = false;
                let target = self.seek_lba;
                self.activity = if matches!(command.opcode, 0x06 | 0x1b) {
                    Activity::PreparingRead { target }
                } else {
                    Activity::PreparingSeek {
                        target,
                        physical: command.opcode == 0x16,
                        status: self.status() & 0xe0,
                    }
                };
                // Read acknowledges Idle even when it interrupts an active read.
                if matches!(command.opcode, 0x06 | 0x1b) {
                    self.xa_ended = false;
                    return Ok(self.response(3));
                }
            }
            0x09 => {
                self.activity = Activity::Pausing;
                self.header_valid = false;
            }
            0x0a => {
                // CdlInit resets current/seek positions to 00:02:00 and clears
                // pending Setloc while preserving mode, mute and filter.
                // A retained transfer header must not redirect the next read.
                // ACK retains pre-transition status; completion is caller-supplied.
                // Opcode 0x1c is the separate CdlReset.
                self.activity = Activity::Resetting;
                self.double_speed = false;
                self.speed_changed = false;
                self.next_lba = 0;
                self.seek_lba = 0;
                self.setloc_pending = false;
                self.header_valid = self.last_header.is_some();
                self.auto_pause = false;
                self.report = false;
            }
            0x0d => {
                self.filter.copy_from_slice(&command.parameters);
            }
            0x0b => self.muted = true,
            0x0c => self.muted = false,
            0x0e => {
                check_mode(command.parameters[0])?;
                if self.mode & 0x40 == 0 && command.parameters[0] & 0x40 != 0 {
                    self.xa_ended = false;
                }
                self.mode = command.parameters[0];
                let double_speed = self.mode & 0x80 != 0;
                self.speed_changed |= self.double_speed != double_speed;
                self.double_speed = double_speed;
                self.auto_pause = self.mode & 2 != 0;
                self.report = self.mode & 4 != 0;
            }
            0x0f => {
                return Ok(Response {
                    interrupt: 3,
                    bytes: vec![self.status(), self.mode, 0, self.filter[0], self.filter[1]],
                })
            }
            0x10 => {
                return Ok(match self.last_header.filter(|_| self.header_valid) {
                    Some(header) => Response {
                        interrupt: 3,
                        bytes: header.to_vec(),
                    },
                    None => self.parameter_error(0x80),
                })
            }
            0x11 => {
                let toc = self
                    .toc
                    .as_ref()
                    .ok_or("CD GetlocP requires explicit TOC metadata")?;
                return Ok(match toc.position(self.next_lba)? {
                    Some(position) => Response {
                        interrupt: 3,
                        bytes: position.to_vec(),
                    },
                    None => self.parameter_error(0x80),
                });
            }
            0x13 | 0x14 => {
                let toc = self
                    .toc
                    .as_ref()
                    .ok_or("CD drive: GetTN/GetTD require explicit TOC metadata")?;
                let bytes = if command.opcode == 0x13 {
                    let tracks = toc.tracks.len() as u8;
                    vec![self.status(), 1, ((tracks / 10) << 4) | (tracks % 10)]
                } else {
                    let bcd = command.parameters[0];
                    let track = usize::from(bcd >> 4) * 10 + usize::from(bcd & 15);
                    if bcd >> 4 > 9 || bcd & 15 > 9 || track > toc.tracks.len() {
                        return Ok(self.parameter_error(0x10));
                    }
                    let position = if track == 0 {
                        toc.lead_out
                    } else {
                        toc.tracks[track - 1].start
                    };
                    let [minute, second, _] = position.bcd();
                    vec![self.status(), minute, second]
                };
                return Ok(Response {
                    interrupt: 3,
                    bytes,
                });
            }
            _ => unreachable!(),
        }
        Ok(response)
    }

    /// Complete a supplied seek/pause/reset boundary. No duration is assigned here.
    /// Located Play needs two boundaries: seek start updates position, then
    /// seek completion starts playback and its report clock. Neither emits INT2.
    /// Read also needs two boundaries; the second makes sector delivery ready.
    /// Explicit SeekL/SeekP retain position until their second boundary, with INT2.
    pub fn complete(&mut self, cycle: u32) -> Result<Option<Response>> {
        match self.activity {
            Activity::PreparingRead { target } => {
                self.speed_changed = false;
                self.next_lba = target;
                self.activity = Activity::SeekingRead { target };
                Ok(None)
            }
            Activity::SeekingRead { .. } => {
                if let Some(code) = self.media_error_code(self.next_lba, false) {
                    self.activity = Activity::Paused;
                    return Ok(Some(self.media_error(code)));
                }
                self.activity = Activity::Reading;
                Ok(None)
            }
            Activity::PreparingSeek {
                target, physical, ..
            } => {
                self.speed_changed = false;
                self.activity = Activity::Seeking { target, physical };
                Ok(None)
            }
            Activity::Seeking { target, physical } => {
                self.next_lba = target - if physical { (target + 150) / 32768 } else { 0 };
                self.activity = Activity::Paused;
                self.header_valid = false;
                Ok(Some(match self.media_error_code(target, physical) {
                    Some(code) => self.media_error(code),
                    None => self.response(2),
                }))
            }
            Activity::PreparingPlay { target } => {
                self.speed_changed = false;
                self.next_lba = target;
                self.activity = Activity::SeekingPlay { target };
                Ok(None)
            }
            Activity::SeekingPlay { target } => {
                self.next_lba = target;
                self.start_playing(cycle);
                Ok(None)
            }
            Activity::Pausing | Activity::Resetting => {
                self.activity = Activity::Paused;
                Ok(Some(self.response(2)))
            }
            _ => Err("CD drive: no mechanical transition to complete".into()),
        }
    }

    fn start_playing(&mut self, cycle: u32) {
        self.activity = Activity::Playing;
        self.play_track = 0;
        self.play_start_cycle = cycle;
    }

    /// Select backing only for an audio program sector. End/autopause callbacks,
    /// data tracks and program tails need no backing read.
    pub(crate) fn play_media_lba(&self) -> Result<Option<i32>> {
        let toc = self
            .toc
            .as_ref()
            .ok_or("CD playback requires explicit TOC")?;
        if self.next_lba >= toc.lead_out.lba() {
            return Ok(None);
        }
        let q = toc
            .position(self.next_lba)?
            .ok_or("CD playback has no position")?;
        if self.auto_pause && self.play_track != 0 && q[0] != self.play_track {
            return Ok(None);
        }
        let track = &toc.tracks[toc.physical_track(self.next_lba)];
        if track.kind == TrackKind::Audio && self.next_lba < track.start.lba() {
            return Err("CD playback before an audio first track is not implemented".into());
        }
        Ok(
            (track.kind == TrackKind::Audio && self.next_lba < track.end.lba())
                .then_some(self.next_lba),
        )
    }

    /// Process one caller-supplied CDDA boundary at its explicit CPU cycle.
    /// None represents a failed media read and plays silence. Data tracks and
    /// program tails also play silence. End/autopause do not consume a sector.
    /// The caller owns feed wait completion, IRQ publication and all scheduling.
    pub fn play_sector(
        &mut self,
        position: CdPosition,
        raw: Option<&[u8]>,
        cycle: u32,
    ) -> Result<PlayDelivery> {
        if self.status() & 0xe0 != 0x80 || position.lba() != self.next_lba {
            return Err("CD playback: require playing state and current position".into());
        }
        let toc = self
            .toc
            .as_ref()
            .ok_or("CD playback requires explicit TOC")?;
        if self.next_lba >= toc.lead_out.lba() {
            if matches!(self.activity, Activity::PreparingSeek { .. }) {
                return Err(
                    "CD playback: end during explicit seek preparation is not implemented".into(),
                );
            }
            self.activity = Activity::Paused;
            let response = self.response(4);
            self.motor_on = false;
            return Ok(PlayDelivery {
                audio: None,
                response: Some(response),
            });
        }
        let q = toc
            .position(self.next_lba)?
            .ok_or("CD playback has no position")?;
        if self.auto_pause && self.play_track != 0 && q[0] != self.play_track {
            if matches!(self.activity, Activity::PreparingSeek { .. }) {
                return Err(
                    "CD playback: autopause during explicit seek preparation is not implemented"
                        .into(),
                );
            }
            self.activity = Activity::Paused;
            return Ok(PlayDelivery {
                audio: None,
                response: Some(self.response(4)),
            });
        }
        if raw.is_some_and(|bytes| bytes.len() != 2352) {
            return Err(
                "CD playback requires a whole raw sector or an explicit failed read".into(),
            );
        }
        let track = &toc.tracks[toc.physical_track(self.next_lba)];
        if track.kind == TrackKind::Audio && self.next_lba < track.start.lba() {
            return Err("CD playback before an audio first track is not implemented".into());
        }
        let samples = if track.kind == TrackKind::Audio && self.next_lba < track.end.lba() {
            raw.unwrap_or(&[0; 2352])
        } else {
            &[0; 2352]
        };
        let next_lba = self
            .next_lba
            .checked_add(1)
            .ok_or("CD playback position overflow")?;
        CdPosition::from_lba(next_lba)?;
        let right = !self
            .peak_right
            .ok_or("CD playback requires explicit peak phase")?;
        let mut response = None;
        if self.report && cycle.wrapping_sub(self.play_start_cycle) >= 25_401_600 {
            let frame = (q[7] >> 4) * 10 + (q[7] & 15);
            if frame.is_multiple_of(10) {
                let peak = samples
                    .as_chunks::<4>()
                    .0
                    .iter()
                    .map(|frame| {
                        let at = if right { 2 } else { 0 };
                        i16::from_le_bytes([frame[at], frame[at + 1]])
                            .unsigned_abs()
                            .min(0x7fff)
                    })
                    .max()
                    .unwrap_or(0)
                    | if right { 0x8000 } else { 0 };
                let time = if (frame / 10) & 1 == 0 {
                    [q[5], q[6], q[7]]
                } else {
                    [q[2], q[3] | 0x80, q[4]]
                };
                response = Some(Response {
                    interrupt: 1,
                    bytes: vec![
                        self.status(),
                        q[0],
                        q[1],
                        time[0],
                        time[1],
                        time[2],
                        peak as u8,
                        (peak >> 8) as u8,
                    ],
                });
            }
        }
        let audio = (!self.muted).then(|| samples.to_vec());
        self.peak_right = Some(right);
        self.play_track = if self.play_track == 0 {
            q[0]
        } else {
            self.play_track
        };
        self.next_lba = next_lba;
        Ok(PlayDelivery { audio, response })
    }

    /// Supply one Read boundary. TOC errors precede any backing read and accept
    /// None; a valid data position requires a whole raw Mode-2 sector. Actual
    /// failed data reads, overruns and deferred retry remain unsupported.
    /// Input checks precede data state changes, including EDC and routing checks.
    pub fn sector(&mut self, raw: Option<&[u8]>) -> Result<Delivery> {
        if self.status() & 0xe0 != 0x20 {
            return Err("CD drive: sector arrival requires completed read seek".into());
        }
        if let Some(code) = self.media_error_code(self.next_lba, false) {
            if matches!(self.activity, Activity::PreparingSeek { .. }) {
                return Err(
                    "CD drive: media error during explicit seek preparation is not implemented"
                        .into(),
                );
            }
            self.activity = Activity::Paused;
            return Ok(Delivery::Error(self.media_error(code)));
        }
        let raw = raw.ok_or("CD drive: failed data media reads are not implemented")?;
        if raw.len() != 2352
            || raw[..12] != [0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0]
            || raw[15] != 2
            || raw[16..20] != raw[20..24]
        {
            return Err("CD drive: expected raw Mode-2 sector with matching subheaders".into());
        }
        let position = CdPosition::from_bcd(raw[12..15].try_into()?)?;
        if position.lba() != self.next_lba {
            return Err(format!(
                "CD drive: sector LBA {} differs from expected {}",
                position.lba(),
                self.next_lba
            )
            .into());
        }
        let next_lba = self
            .next_lba
            .checked_add(1)
            .ok_or("CD drive LBA overflow")?;
        CdPosition::from_lba(next_lba)?;
        if raw[18] & 0x20 != 0 {
            edc::check_form2(&raw[16..])?;
        } else if edc::checksum(&raw[16..2072]) != u32::from_le_bytes(raw[2072..2076].try_into()?) {
            return Err("CD drive: Form-1 EDC mismatch; ECC repair is not implemented".into());
        }
        let audio = raw[18] & 0x44 == 0x44;
        let filtered = self.mode & 8 != 0;
        let delivery = if audio && (filtered || self.mode & 0x40 != 0) {
            if self.mode & 0x40 == 0
                || self.xa_ended
                || (filtered && raw[16..18] != self.filter)
                || raw[17] == 255
            {
                Delivery::Filtered
            } else {
                self.xa_ended = raw[18] & 0x81 == 0x81;
                Delivery::Xa(raw[16..].to_vec())
            }
        } else {
            let bytes = if self.mode & 0x20 != 0 {
                &raw[12..]
            } else {
                &raw[24..2072]
            };
            Delivery::Data {
                bytes: bytes.to_vec(),
                response: self.response(1),
            }
        };
        self.last_header = Some(raw[12..20].try_into()?);
        self.header_valid = true;
        self.next_lba = next_lba;
        Ok(delivery)
    }
}

fn check_mode(mode: u8) -> Result<()> {
    if mode & !0xee != 0 {
        return Err(format!(
            "CD drive: CDDA-read/2328-byte mode bits {:#04x} unimplemented",
            mode & !0xee
        )
        .into());
    }
    Ok(())
}

fn bcd(value: usize) -> u8 {
    (((value / 10) << 4) | (value % 10)) as u8
}
