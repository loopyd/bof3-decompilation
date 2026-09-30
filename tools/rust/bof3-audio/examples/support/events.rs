//! Explicit cue/poll/release schedule for the development runtime probe.
use bof3_audio::Result;
use serde::Serialize;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
#[serde(tag = "kind", rename_all = "snake_case")]
pub enum Action {
    Cue { row: usize },
    Poll,
    Release,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize)]
pub struct Event {
    pub frame: u64,
    pub action: Action,
}

pub fn parse(value: &str, body_frames: u64) -> Result<Vec<Event>> {
    let mut result = Vec::new();
    for item in value.split(',') {
        let (action, frame) = item
            .split_once('@')
            .ok_or("events require ROW@FRAME, poll@FRAME or off@FRAME")?;
        let frame = frame.parse::<u64>()?;
        if frame >= body_frames || result.last().is_some_and(|e: &Event| e.frame > frame) {
            return Err("events must be ordered and strictly before the body endpoint".into());
        }
        let action = match action {
            "poll" => Action::Poll,
            "off" => Action::Release,
            row => Action::Cue { row: row.parse()? },
        };
        if result.len() == 64 {
            return Err("probe schedule exceeds 64 events".into());
        }
        result.push(Event { frame, action });
    }
    Ok(result)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn simultaneous_events_preserve_explicit_order() {
        let events = parse("0@0,poll@1024,1@1024,off@2048", 4096).unwrap();
        assert_eq!(
            events.iter().map(|e| e.frame).collect::<Vec<_>>(),
            [0, 1024, 1024, 2048]
        );
        assert_eq!(
            events.iter().map(|e| e.action).collect::<Vec<_>>(),
            [
                Action::Cue { row: 0 },
                Action::Poll,
                Action::Cue { row: 1 },
                Action::Release,
            ]
        );
    }
    #[test]
    fn invalid_or_unbounded_schedules_fail_instead_of_reordering() {
        for value in [
            "",
            "0",
            "0@",
            "@0",
            "0@-1",
            "-1@0",
            "0@4",
            "0@2,poll@1",
            "0@1,",
            "0@18446744073709551616",
        ] {
            assert!(parse(value, 4).is_err(), "{value}");
        }
        assert!(parse(&vec!["0@0"; 65].join(","), 4).is_err());
    }
}
