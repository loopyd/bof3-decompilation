use std::fmt;

#[derive(Debug)]
pub enum Error {
    Io(std::io::Error),
    Json(serde_json::Error),
    Archive(emi_ex_v2::Error),
    Invalid(String),
    UnsupportedRelease(String),
    StaleIndex(String),
    Conflict(String),
    Codec(String),
    Incomplete(String),
}

pub type Result<T> = std::result::Result<T, Error>;

impl Error {
    pub fn code(&self) -> &'static str {
        match self {
            Self::Io(_) => "io-error",
            Self::Json(_) | Self::Invalid(_) | Self::Archive(_) => "invalid-input",
            Self::UnsupportedRelease(_) => "unsupported-release",
            Self::StaleIndex(_) => "stale-index",
            Self::Conflict(_) => "edit-conflict",
            Self::Codec(_) => "codec-error",
            Self::Incomplete(_) => "incomplete",
        }
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Io(error) => write!(f, "{error}"),
            Self::Json(error) => write!(f, "{error}"),
            Self::Archive(error) => write!(f, "{error}"),
            Self::Invalid(message)
            | Self::UnsupportedRelease(message)
            | Self::StaleIndex(message)
            | Self::Conflict(message)
            | Self::Codec(message)
            | Self::Incomplete(message) => f.write_str(message),
        }
    }
}

impl std::error::Error for Error {}

impl From<std::io::Error> for Error {
    fn from(error: std::io::Error) -> Self {
        Self::Io(error)
    }
}
impl From<serde_json::Error> for Error {
    fn from(error: serde_json::Error) -> Self {
        Self::Json(error)
    }
}
impl From<emi_ex_v2::Error> for Error {
    fn from(error: emi_ex_v2::Error) -> Self {
        Self::Archive(error)
    }
}
