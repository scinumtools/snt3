use scinumtools3_sys as ffi;
use std::{
    ffi::{c_char, c_int, CStr, CString},
    fmt,
    path::Path,
    ptr,
};

/// An SNT3 diagnostic or an invalid Rust input to the C ABI.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Error(pub String);
impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&self.0)
    }
}
impl std::error::Error for Error {}
pub type Result<T> = std::result::Result<T, Error>;

pub(crate) fn text(value: &str) -> Result<CString> {
    CString::new(value).map_err(|_| Error("input contains a NUL byte".into()))
}
pub(crate) fn path_text(path: &Path) -> Result<CString> {
    text(
        path.to_str()
            .ok_or_else(|| Error("path is not valid Unicode".into()))?,
    )
}
pub(crate) fn check(status: c_int, error: ffi::snt_dip_error) -> Result<()> {
    if status == 0 {
        return Ok(());
    }
    let message = if error.message.is_null() {
        format!("SNT3 C ABI error {status}")
    } else {
        unsafe { CStr::from_ptr(error.message).to_string_lossy().into_owned() }
    };
    Err(Error(message))
}
pub(crate) fn read_string(
    mut call: impl FnMut(*mut c_char, usize, *mut usize, *mut ffi::snt_dip_error) -> c_int,
) -> Result<String> {
    let mut required = 0;
    let mut error = ffi::snt_dip_error::default();
    check(call(ptr::null_mut(), 0, &mut required, &mut error), error)?;
    if required == 0 {
        return Err(Error("C ABI returned a zero string size".into()));
    }
    let mut bytes = vec![0u8; required];
    let mut error = ffi::snt_dip_error::default();
    check(
        call(
            bytes.as_mut_ptr().cast(),
            bytes.len(),
            &mut required,
            &mut error,
        ),
        error,
    )?;
    if bytes.last() != Some(&0) {
        return Err(Error("C ABI returned an unterminated string".into()));
    }
    bytes.pop();
    String::from_utf8(bytes).map_err(|_| Error("C ABI returned invalid UTF-8".into()))
}

pub(crate) fn decode_buffer(mut bytes: Vec<u8>) -> Result<String> {
    if bytes.pop() != Some(0) {
        return Err(Error("C ABI returned an unterminated string".into()));
    }
    String::from_utf8(bytes).map_err(|_| Error("C ABI returned invalid UTF-8".into()))
}
