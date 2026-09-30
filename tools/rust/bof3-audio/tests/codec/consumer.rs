//! Independent Linux mpg123 oracle, loaded only by explicitly requested tests.
//! No native decoder is linked into the audio library or executable.

use std::{
    ffi::{c_char, c_int, c_long, c_void, CStr, CString},
    os::unix::ffi::OsStrExt,
    path::Path,
};

#[link(name = "dl")]
unsafe extern "C" {
    fn dlopen(name: *const c_char, flags: c_int) -> *mut c_void;
    fn dlsym(handle: *mut c_void, name: *const c_char) -> *mut c_void;
    fn dlclose(handle: *mut c_void) -> c_int;
    fn dlerror() -> *const c_char;
}

struct Library(*mut c_void);
impl Drop for Library {
    fn drop(&mut self) {
        unsafe {
            if !self.0.is_null() {
                dlclose(self.0);
            }
        }
    }
}

struct Decoder {
    handle: *mut c_void,
    close: unsafe extern "C" fn(*mut c_void) -> c_int,
    delete: unsafe extern "C" fn(*mut c_void),
    exit: unsafe extern "C" fn(),
}
impl Drop for Decoder {
    fn drop(&mut self) {
        unsafe {
            if !self.handle.is_null() {
                (self.close)(self.handle);
                (self.delete)(self.handle);
            }
            (self.exit)();
        }
    }
}

pub struct Decoded {
    pub rate: u32,
    pub channels: u16,
    pub pcm: Vec<i16>,
}

pub fn decode(path: &Path) -> Decoded {
    // Initialization and teardown belong to the library's process-wide state.
    // Independent Rust tests may call this oracle concurrently.
    static DECODING: std::sync::Mutex<()> = std::sync::Mutex::new(());
    let _guard = DECODING.lock().expect("mpg123 oracle lock poisoned");
    // Signatures follow the public mpg123 API. Every pointer remains valid for
    // its synchronous call; decoder cleanup precedes unloading the library.
    unsafe {
        let library = Library(dlopen(c"libmpg123.so.0".as_ptr(), 2));
        if library.0.is_null() {
            let error = dlerror();
            let message = if error.is_null() {
                "unknown loader error".into()
            } else {
                CStr::from_ptr(error).to_string_lossy()
            };
            panic!("independent MP3 check requires installed mpg123: {message}");
        }
        macro_rules! function {
            ($name:literal, $type:ty) => {{
                let name = CString::new(concat!("mpg123_", $name)).unwrap();
                let address = dlsym(library.0, name.as_ptr());
                assert!(!address.is_null(), "missing oracle function {}", $name);
                std::mem::transmute::<*mut c_void, $type>(address)
            }};
        }
        let init = function!("init", unsafe extern "C" fn() -> c_int);
        let new = function!(
            "new",
            unsafe extern "C" fn(*const c_char, *mut c_int) -> *mut c_void
        );
        let open = function!(
            "open",
            unsafe extern "C" fn(*mut c_void, *const c_char) -> c_int
        );
        let format = function!(
            "getformat",
            unsafe extern "C" fn(*mut c_void, *mut c_long, *mut c_int, *mut c_int) -> c_int
        );
        let read = function!(
            "read",
            unsafe extern "C" fn(*mut c_void, *mut c_void, usize, *mut usize) -> c_int
        );
        let close = function!("close", unsafe extern "C" fn(*mut c_void) -> c_int);
        let delete = function!("delete", unsafe extern "C" fn(*mut c_void));
        let exit = function!("exit", unsafe extern "C" fn());
        assert_eq!(init(), 0);
        let mut error = 0;
        let decoder = Decoder {
            handle: new(std::ptr::null(), &mut error),
            close,
            delete,
            exit,
        };
        assert!(
            !decoder.handle.is_null() && error == 0,
            "mpg123 creation failed: {error}"
        );
        let path = CString::new(path.as_os_str().as_bytes()).unwrap();
        assert_eq!(
            open(decoder.handle, path.as_ptr()),
            0,
            "mpg123 rejected input"
        );
        let (mut rate, mut channels, mut encoding) = (0, 0, 0);
        assert_eq!(
            format(decoder.handle, &mut rate, &mut channels, &mut encoding),
            0
        );
        assert_eq!(encoding, 0xd0, "oracle requires signed 16-bit native PCM");
        assert!(matches!(channels, 1 | 2));
        let mut pcm = Vec::new();
        let mut buffer = [0i16; 32768];
        loop {
            let mut bytes = 0;
            let status = read(
                decoder.handle,
                buffer.as_mut_ptr().cast(),
                std::mem::size_of_val(&buffer),
                &mut bytes,
            );
            assert!(
                matches!(status, 0 | -12),
                "mpg123 decode failed or format changed: {status}"
            );
            assert!(bytes <= std::mem::size_of_val(&buffer) && bytes % 2 == 0);
            pcm.extend_from_slice(&buffer[..bytes / 2]);
            if status == -12 {
                break;
            }
            assert_ne!(bytes, 0, "mpg123 made no progress");
        }
        assert_eq!(pcm.len() % channels as usize, 0);
        Decoded {
            rate: rate.try_into().unwrap(),
            channels: channels.try_into().unwrap(),
            pcm,
        }
    }
}
