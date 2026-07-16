use std::fs;
use std::fs::File;
use std::io::{Read, Seek, SeekFrom};
use std::path::{Path, PathBuf};

use crate::Error;

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct CueSheet {
    pub tracks: Vec<Track>,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Track {
    pub number: u8,
    pub mode: TrackMode,
    pub file: PathBuf,
    pub index00: Option<u32>,
    pub index01: u32,
    pub pregap: Option<u32>,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum TrackMode {
    Audio,
    Mode1_2048,
    Mode1_2352,
    Mode2_2336,
    Mode2_2352,
}

pub fn read(path: &Path) -> Result<CueSheet, Error> {
    parse(
        &fs::read_to_string(path)?,
        path.parent().unwrap_or(Path::new(".")),
    )
}

pub fn extract_audio_tracks(sheet: &CueSheet, output: &Path) -> Result<Vec<PathBuf>, Error> {
    fs::create_dir_all(output)?;
    let mut written = Vec::new();
    for (position, track) in sheet.tracks.iter().enumerate() {
        if track.mode != TrackMode::Audio {
            continue;
        }
        let mut source = File::open(&track.file)?;
        let file_sectors = source.metadata()?.len() / 2352;
        let end_sector = sheet.tracks[position + 1..]
            .iter()
            .find(|next| next.file == track.file)
            .map_or(file_sectors, |next| {
                u64::from(next.index00.unwrap_or(next.index01))
            });
        let start_sector = u64::from(track.index01);
        if start_sector > end_sector || end_sector > file_sectors {
            return Err(Error::InvalidImage(
                "cue audio track range exceeds its file",
            ));
        }
        source.seek(SeekFrom::Start(start_sector * 2352))?;
        let sample_count = (end_sector - start_sector)
            .checked_mul(2352 / 2)
            .ok_or(Error::InvalidImage("cue audio sample count overflows"))?;
        let path = output.join(format!("track{:02}.wav", track.number));
        let spec = hound::WavSpec {
            channels: 2,
            sample_rate: 44_100,
            bits_per_sample: 16,
            sample_format: hound::SampleFormat::Int,
        };
        let mut writer = hound::WavWriter::create(&path, spec)
            .map_err(|error| Error::InvalidImageDetail(format!("cannot create WAV: {error}")))?;
        let mut sample = [0; 2];
        for _ in 0..sample_count {
            source.read_exact(&mut sample)?;
            writer
                .write_sample(i16::from_le_bytes(sample))
                .map_err(|error| Error::InvalidImageDetail(format!("cannot write WAV: {error}")))?;
        }
        writer
            .finalize()
            .map_err(|error| Error::InvalidImageDetail(format!("cannot finalize WAV: {error}")))?;
        written.push(path);
    }
    Ok(written)
}

pub fn parse(text: &str, root: &Path) -> Result<CueSheet, Error> {
    let mut current_file = None;
    let mut tracks: Vec<Track> = Vec::new();
    for source_line in text.lines() {
        let line = source_line.trim();
        if line.is_empty() || line.starts_with("REM ") {
            continue;
        }
        let (command, rest) = split_word(line);
        match command.to_ascii_uppercase().as_str() {
            "FILE" => {
                let (name, _) = parse_file(rest)?;
                current_file = Some(root.join(name));
            }
            "TRACK" => {
                let file = current_file
                    .clone()
                    .ok_or(Error::InvalidImage("cue TRACK appears before FILE"))?;
                let (number, mode) = parse_track(rest)?;
                if tracks.iter().any(|track| track.number == number) {
                    return Err(Error::InvalidImage("cue track number is duplicated"));
                }
                tracks.push(Track {
                    number,
                    mode,
                    file,
                    index00: None,
                    index01: u32::MAX,
                    pregap: None,
                });
            }
            "INDEX" => {
                let track = tracks
                    .last_mut()
                    .ok_or(Error::InvalidImage("cue INDEX appears before TRACK"))?;
                let (number, timestamp) = split_word(rest);
                let sector = parse_timecode(timestamp)?;
                match number {
                    "00" => track.index00 = Some(sector),
                    "01" => track.index01 = sector,
                    _ => {}
                }
            }
            "PREGAP" => {
                let track = tracks
                    .last_mut()
                    .ok_or(Error::InvalidImage("cue PREGAP appears before TRACK"))?;
                track.pregap = Some(parse_timecode(rest.trim())?);
            }
            _ => {}
        }
    }
    if tracks.is_empty() {
        return Err(Error::InvalidImage("cue sheet has no tracks"));
    }
    if tracks.iter().any(|track| track.index01 == u32::MAX) {
        return Err(Error::InvalidImage("cue track has no INDEX 01"));
    }
    Ok(CueSheet { tracks })
}

fn parse_file(value: &str) -> Result<(&str, &str), Error> {
    let value = value.trim_start();
    if let Some(quoted) = value.strip_prefix('"') {
        let end = quoted
            .find('"')
            .ok_or(Error::InvalidImage("unterminated quoted cue FILE path"))?;
        let name = &quoted[..end];
        if name.is_empty() {
            return Err(Error::InvalidImage("empty cue FILE path"));
        }
        return Ok((name, quoted[end + 1..].trim()));
    }
    let (name, kind) = split_word(value);
    if name.is_empty() {
        return Err(Error::InvalidImage("empty cue FILE path"));
    }
    Ok((name, kind))
}

fn parse_track(value: &str) -> Result<(u8, TrackMode), Error> {
    let (number, mode) = split_word(value.trim());
    let number = number
        .parse()
        .map_err(|_| Error::InvalidImage("invalid cue track number"))?;
    let mode = match mode.to_ascii_uppercase().as_str() {
        "AUDIO" => TrackMode::Audio,
        "MODE1/2048" => TrackMode::Mode1_2048,
        "MODE1/2352" => TrackMode::Mode1_2352,
        "MODE2/2336" => TrackMode::Mode2_2336,
        "MODE2/2352" => TrackMode::Mode2_2352,
        _ => return Err(Error::InvalidImage("unsupported cue track mode")),
    };
    Ok((number, mode))
}

pub fn parse_timecode(value: &str) -> Result<u32, Error> {
    let mut parts = value.split(':');
    let minutes: u32 = parse_time_part(parts.next())?;
    let seconds: u32 = parse_time_part(parts.next())?;
    let frames: u32 = parse_time_part(parts.next())?;
    if parts.next().is_some() || seconds >= 60 || frames >= 75 {
        return Err(Error::InvalidImage("invalid cue timecode"));
    }
    minutes
        .checked_mul(60 * 75)
        .and_then(|value| value.checked_add(seconds * 75 + frames))
        .ok_or(Error::InvalidImage("cue timecode overflows"))
}

pub fn format_timecode(sectors: u32) -> String {
    let minutes = sectors / (60 * 75);
    let remainder = sectors % (60 * 75);
    format!("{minutes:02}:{:02}:{:02}", remainder / 75, remainder % 75)
}

fn parse_time_part(value: Option<&str>) -> Result<u32, Error> {
    value
        .ok_or(Error::InvalidImage("invalid cue timecode"))?
        .parse()
        .map_err(|_| Error::InvalidImage("invalid cue timecode"))
}

fn split_word(value: &str) -> (&str, &str) {
    value
        .split_once(char::is_whitespace)
        .map_or((value, ""), |(word, rest)| (word, rest.trim_start()))
}
