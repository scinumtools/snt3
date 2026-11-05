use crate::{
    project::Project,
    shared::{check, decode_buffer, read_string, text, Error, Result},
};
use scinumtools3_sys as ffi;
use std::{
    ffi::c_int,
    marker::PhantomData,
    ptr::{self, NonNull},
};

/// A read-only node. Its project remains borrowed for the node's lifetime.
pub struct Node<'a> {
    pub(crate) raw: NonNull<ffi::snt_dip_node>,
    pub(crate) _project: PhantomData<&'a Project>,
}

/// Source locations and text recorded for a resolved value.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Provenance {
    pub source_name: String,
    pub source_line: usize,
    pub source_code: String,
    pub override_source_name: Option<String>,
    pub override_line: usize,
    pub override_code: String,
}

/// Structural kind of a DIPL hierarchy node.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NodeKind {
    None,
    Empty,
    Group,
    Map,
    List,
    Item,
    Root,
}

/// DIPL primitive type code, as defined by the C ABI.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ValueType(pub i32);
impl ValueType {
    pub const BOOL: Self = Self(1);
    pub const STRING: Self = Self(2);
    pub const CHAR: Self = Self(4);
    pub const INT8: Self = Self(16384);
    pub const INT16: Self = Self(8);
    pub const INT32: Self = Self(16);
    pub const INT64: Self = Self(32);
    pub const UINT8: Self = Self(32768);
    pub const UINT16: Self = Self(128);
    pub const UINT32: Self = Self(256);
    pub const UINT64: Self = Self(512);
    pub const FLOAT32: Self = Self(1024);
    pub const FLOAT64: Self = Self(2048);
}
impl Drop for Node<'_> {
    fn drop(&mut self) {
        unsafe { ffi::snt_dip_node_free(self.raw.as_ptr()) }
    }
}
impl Node<'_> {
    pub fn path(&self) -> Result<String> {
        read_string(|b, n, r, e| unsafe {
            ffi::snt_dip_node_get_path(self.raw.as_ptr(), b, n, r, e)
        })
    }
    pub fn kind(&self) -> Result<NodeKind> {
        let mut value = 0;
        self.scalar(|e| unsafe { ffi::snt_dip_node_get_kind(self.raw.as_ptr(), &mut value, e) })?;
        match value {
            0 => Ok(NodeKind::None),
            1 => Ok(NodeKind::Empty),
            2 => Ok(NodeKind::Group),
            3 => Ok(NodeKind::Map),
            4 => Ok(NodeKind::List),
            5 => Ok(NodeKind::Item),
            6 => Ok(NodeKind::Root),
            _ => Err(Error("C ABI returned an unknown node kind".into())),
        }
    }
    pub fn value_type(&self) -> Result<ValueType> {
        let mut value = 0;
        self.scalar(|e| unsafe { ffi::snt_dip_node_get_type(self.raw.as_ptr(), &mut value, e) })?;
        Ok(ValueType(value))
    }
    pub fn as_bool(&self) -> Result<bool> {
        let mut value = 0;
        self.scalar(|e| unsafe { ffi::snt_dip_node_get_bool(self.raw.as_ptr(), &mut value, e) })?;
        Ok(value != 0)
    }
    pub fn as_i64(&self) -> Result<i64> {
        let mut value = 0;
        self.scalar(|e| unsafe { ffi::snt_dip_node_get_int64(self.raw.as_ptr(), &mut value, e) })?;
        Ok(value)
    }
    pub fn as_u64(&self) -> Result<u64> {
        let mut value = 0;
        self.scalar(|e| unsafe { ffi::snt_dip_node_get_uint64(self.raw.as_ptr(), &mut value, e) })?;
        Ok(value)
    }
    pub fn as_f64(&self) -> Result<f64> {
        let mut value = 0.0;
        self.scalar(|e| unsafe { ffi::snt_dip_node_get_double(self.raw.as_ptr(), &mut value, e) })?;
        Ok(value)
    }
    pub fn as_string(&self) -> Result<String> {
        read_string(|b, n, r, e| unsafe {
            ffi::snt_dip_node_get_string(self.raw.as_ptr(), b, n, r, e)
        })
    }
    pub fn units(&self) -> Result<Option<String>> {
        let mut present = 0;
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_has_units(self.raw.as_ptr(), &mut present, e)
        })?;
        if present == 0 {
            Ok(None)
        } else {
            read_string(|b, n, r, e| unsafe {
                ffi::snt_dip_node_get_units(self.raw.as_ptr(), b, n, r, e)
            })
            .map(Some)
        }
    }
    pub fn is_overridden(&self) -> Result<bool> {
        let mut value = 0;
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_is_overridden(self.raw.as_ptr(), &mut value, e)
        })?;
        Ok(value != 0)
    }
    pub fn metadata(&self, name: &str) -> Result<Option<String>> {
        let name = text(if name == "descr" { "description" } else { name })?;
        let mut present = 0;
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_has_metadata(self.raw.as_ptr(), name.as_ptr(), &mut present, e)
        })?;
        if present == 0 {
            Ok(None)
        } else {
            read_string(|b, n, r, e| unsafe {
                ffi::snt_dip_node_get_metadata(self.raw.as_ptr(), name.as_ptr(), b, n, r, e)
            })
            .map(Some)
        }
    }
    pub fn provenance(&self) -> Result<Provenance> {
        let (mut name_size, mut code_size, mut override_name_size, mut override_code_size) =
            (0, 0, 0, 0);
        let (mut source_line, mut override_line) = (0, 0);
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_get_provenance(
                self.raw.as_ptr(),
                ptr::null_mut(),
                0,
                &mut name_size,
                &mut source_line,
                ptr::null_mut(),
                0,
                &mut code_size,
                ptr::null_mut(),
                0,
                &mut override_name_size,
                &mut override_line,
                ptr::null_mut(),
                0,
                &mut override_code_size,
                e,
            )
        })?;
        let mut name = vec![0u8; name_size];
        let mut code = vec![0u8; code_size];
        let mut override_name = vec![0u8; override_name_size];
        let mut override_code = vec![0u8; override_code_size];
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_get_provenance(
                self.raw.as_ptr(),
                name.as_mut_ptr().cast(),
                name.len(),
                &mut name_size,
                &mut source_line,
                code.as_mut_ptr().cast(),
                code.len(),
                &mut code_size,
                override_name.as_mut_ptr().cast(),
                override_name.len(),
                &mut override_name_size,
                &mut override_line,
                override_code.as_mut_ptr().cast(),
                override_code.len(),
                &mut override_code_size,
                e,
            )
        })?;
        let override_name = decode_buffer(override_name)?;
        Ok(Provenance {
            source_name: decode_buffer(name)?,
            source_line,
            source_code: decode_buffer(code)?,
            override_source_name: if override_name.is_empty() {
                None
            } else {
                Some(override_name)
            },
            override_line,
            override_code: decode_buffer(override_code)?,
        })
    }
    pub fn shape(&self) -> Result<Vec<usize>> {
        let mut rank = 0;
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_get_shape(self.raw.as_ptr(), ptr::null_mut(), 0, &mut rank, e)
        })?;
        let mut shape = vec![0; rank];
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_get_shape(
                self.raw.as_ptr(),
                shape.as_mut_ptr(),
                shape.len(),
                &mut rank,
                e,
            )
        })?;
        Ok(shape)
    }
    pub fn len(&self) -> Result<usize> {
        let mut count = 0;
        self.scalar(|e| unsafe { ffi::snt_dip_node_get_size(self.raw.as_ptr(), &mut count, e) })?;
        Ok(count)
    }
    pub fn is_empty(&self) -> Result<bool> {
        self.len().map(|count| count == 0)
    }
    /// Number of scalar or flattened array values in this node.
    pub fn value_len(&self) -> Result<usize> {
        let mut count = 0;
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_get_value_size(self.raw.as_ptr(), &mut count, e)
        })?;
        Ok(count)
    }
    pub fn entries(&self) -> Result<Vec<(String, Node<'_>)>> {
        let mut entries = Vec::new();
        for index in 0..self.len()? {
            let mut required = 0;
            self.scalar(|e| unsafe {
                ffi::snt_dip_node_get_entry(
                    self.raw.as_ptr(),
                    index,
                    ptr::null_mut(),
                    0,
                    &mut required,
                    ptr::null_mut(),
                    e,
                )
            })?;
            let mut name = vec![0u8; required];
            let mut raw = ptr::null_mut();
            self.scalar(|e| unsafe {
                ffi::snt_dip_node_get_entry(
                    self.raw.as_ptr(),
                    index,
                    name.as_mut_ptr().cast(),
                    name.len(),
                    &mut required,
                    &mut raw,
                    e,
                )
            })?;
            if name.pop() != Some(0) {
                return Err(Error("C ABI returned an unterminated entry name".into()));
            }
            let name = String::from_utf8(name)
                .map_err(|_| Error("C ABI returned invalid UTF-8".into()))?;
            entries.push((
                name,
                Node {
                    raw: NonNull::new(raw)
                        .ok_or_else(|| Error("C ABI returned a null child".into()))?,
                    _project: PhantomData,
                },
            ));
        }
        Ok(entries)
    }
    pub fn as_i64_array(&self) -> Result<Vec<i64>> {
        self.array(ffi::snt_dip_node_copy_int64_array)
    }
    pub fn as_u64_array(&self) -> Result<Vec<u64>> {
        self.array(ffi::snt_dip_node_copy_uint64_array)
    }
    pub fn as_f64_array(&self) -> Result<Vec<f64>> {
        self.array(ffi::snt_dip_node_copy_double_array)
    }
    pub fn as_bool_array(&self) -> Result<Vec<bool>> {
        let values: Vec<c_int> = self.array(ffi::snt_dip_node_copy_bool_array)?;
        Ok(values.into_iter().map(|value| value != 0).collect())
    }
    pub fn as_string_array(&self) -> Result<Vec<String>> {
        let (mut count, mut bytes_required) = (0, 0);
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_copy_string_array(
                self.raw.as_ptr(),
                ptr::null_mut(),
                0,
                ptr::null_mut(),
                0,
                &mut count,
                &mut bytes_required,
                e,
            )
        })?;
        let mut offsets = vec![0usize; count + 1];
        let mut bytes = vec![0u8; bytes_required];
        self.scalar(|e| unsafe {
            ffi::snt_dip_node_copy_string_array(
                self.raw.as_ptr(),
                offsets.as_mut_ptr(),
                offsets.len(),
                bytes.as_mut_ptr().cast(),
                bytes.len(),
                &mut count,
                &mut bytes_required,
                e,
            )
        })?;
        if count + 1 != offsets.len()
            || bytes_required != bytes.len()
            || offsets.first() != Some(&0)
            || offsets.last() != Some(&bytes.len())
        {
            return Err(Error(
                "C ABI returned inconsistent string array sizes".into(),
            ));
        }
        offsets
            .windows(2)
            .map(|range| {
                let (start, end) = (range[0], range[1]);
                if end < start || end > bytes.len() {
                    return Err(Error("C ABI returned invalid string offsets".into()));
                }
                std::str::from_utf8(&bytes[start..end])
                    .map(str::to_owned)
                    .map_err(|_| Error("C ABI returned invalid UTF-8".into()))
            })
            .collect()
    }
    fn scalar(&self, call: impl FnOnce(*mut ffi::snt_dip_error) -> c_int) -> Result<()> {
        let mut error = ffi::snt_dip_error::default();
        check(call(&mut error), error)
    }
    fn array<T: Default + Clone>(
        &self,
        call: unsafe extern "C" fn(
            *const ffi::snt_dip_node,
            *mut T,
            usize,
            *mut usize,
            *mut ffi::snt_dip_error,
        ) -> c_int,
    ) -> Result<Vec<T>> {
        let mut required = self.value_len()?;
        let mut values = vec![T::default(); required];
        self.scalar(|e| unsafe {
            call(
                self.raw.as_ptr(),
                values.as_mut_ptr(),
                values.len(),
                &mut required,
                e,
            )
        })?;
        if required != values.len() {
            return Err(Error("array size changed during the C ABI read".into()));
        }
        Ok(values)
    }
}
