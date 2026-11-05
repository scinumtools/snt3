use crate::shared::{check, path_text, Error, Result};
use scinumtools3_sys as ffi;
use std::{
    ffi::{c_char, CStr},
    path::Path,
    ptr::{self, NonNull},
};

/// Summary of changes between two DIPH5 configurations.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Comparison {
    pub added: usize,
    pub removed: usize,
    pub changed: usize,
    pub differences: Vec<Difference>,
}
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Difference {
    pub path: String,
    pub category: String,
    pub kind: DifferenceKind,
    pub fields: String,
    pub before: String,
    pub after: String,
    pub changed_elements: usize,
    pub example_indices: Vec<usize>,
}
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DifferenceKind {
    Added,
    Removed,
    Changed,
}

/// Compare the effective resolved values in two DIPH5 files.
pub fn compare_diph5(before: impl AsRef<Path>, after: impl AsRef<Path>) -> Result<Comparison> {
    let (before, after) = (path_text(before.as_ref())?, path_text(after.as_ref())?);
    let mut raw = ptr::null_mut();
    let mut error = ffi::snt_dip_error::default();
    check(
        unsafe {
            ffi::snt_dip_compare_files(before.as_ptr(), after.as_ptr(), 0, 5, &mut raw, &mut error)
        },
        error,
    )?;
    struct Owned(NonNull<ffi::snt_dip_comparison>);
    impl Drop for Owned {
        fn drop(&mut self) {
            unsafe { ffi::snt_dip_comparison_free(self.0.as_ptr()) }
        }
    }
    let raw =
        Owned(NonNull::new(raw).ok_or_else(|| Error("C ABI returned a null comparison".into()))?);
    let (mut added, mut removed, mut changed) = (0, 0, 0);
    let mut error = ffi::snt_dip_error::default();
    check(
        unsafe {
            ffi::snt_dip_comparison_summary(
                raw.0.as_ptr(),
                &mut added,
                &mut removed,
                &mut changed,
                &mut error,
            )
        },
        error,
    )?;
    let mut differences = Vec::new();
    for index in 0..unsafe { ffi::snt_dip_comparison_count(raw.0.as_ptr()) } {
        let mut entry = ffi::snt_dip_difference::default();
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe { ffi::snt_dip_comparison_get(raw.0.as_ptr(), index, &mut entry, &mut error) },
            error,
        )?;
        let field = |pointer: *const c_char| -> String {
            if pointer.is_null() {
                String::new()
            } else {
                unsafe { CStr::from_ptr(pointer).to_string_lossy().into_owned() }
            }
        };
        let kind = match entry.kind {
            0 => DifferenceKind::Added,
            1 => DifferenceKind::Removed,
            2 => DifferenceKind::Changed,
            _ => return Err(Error("unknown difference kind".into())),
        };
        let indices = if entry.example_count == 0 {
            Vec::new()
        } else {
            if entry.example_indices.is_null() {
                return Err(Error("C ABI returned null example indices".into()));
            }
            unsafe { std::slice::from_raw_parts(entry.example_indices, entry.example_count) }
                .to_vec()
        };
        differences.push(Difference {
            path: field(entry.path),
            category: field(entry.category),
            kind,
            fields: field(entry.fields),
            before: field(entry.before),
            after: field(entry.after),
            changed_elements: entry.changed_elements,
            example_indices: indices,
        });
    }
    Ok(Comparison {
        added,
        removed,
        changed,
        differences,
    })
}
