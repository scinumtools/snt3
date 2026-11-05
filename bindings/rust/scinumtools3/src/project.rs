use crate::{
    node::Node,
    shared::{check, path_text, text, Error, Result},
};
use scinumtools3_sys as ffi;
use std::{
    marker::PhantomData,
    path::Path,
    ptr::{self, NonNull},
};

/// An evaluated DIPL project or a loaded DIPH5 environment.
pub struct Project {
    raw: NonNull<ffi::snt_dip>,
    ready: bool,
}
impl Drop for Project {
    fn drop(&mut self) {
        unsafe { ffi::snt_dip_parser_free(self.raw.as_ptr()) }
    }
}
impl Project {
    pub fn new() -> Result<Self> {
        let mut raw = ptr::null_mut();
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe { ffi::snt_dip_parser_create(&mut raw, &mut error) },
            error,
        )?;
        Ok(Self {
            raw: NonNull::new(raw).ok_or_else(|| Error("C ABI returned a null parser".into()))?,
            ready: false,
        })
    }

    /// Load DIPfile, a directory containing DIPfile, .dip, .dipl, or .diph5.
    pub fn load(path: impl AsRef<Path>) -> Result<Self> {
        let path = path.as_ref();
        let mut project = Self::new()?;
        let artifact = if path.is_dir() {
            path.join("DIPfile")
        } else {
            path.to_path_buf()
        };
        let name = artifact.file_name().and_then(|s| s.to_str()).unwrap_or("");
        if name == "DIPfile" {
            project.add_project(&artifact)?;
            project.parse()?;
        } else if artifact
            .extension()
            .and_then(|s| s.to_str())
            .is_some_and(|s| s.eq_ignore_ascii_case("diph5"))
        {
            let cpath = path_text(&artifact)?;
            let mut error = ffi::snt_dip_error::default();
            check(
                unsafe {
                    ffi::snt_dip_environment_load(project.raw.as_ptr(), cpath.as_ptr(), &mut error)
                },
                error,
            )?;
            project.ready = true;
        } else if artifact
            .extension()
            .and_then(|s| s.to_str())
            .is_some_and(|s| s.eq_ignore_ascii_case("dip") || s.eq_ignore_ascii_case("dipl"))
        {
            project.add_file(&artifact)?;
            project.parse()?;
        } else {
            return Err(Error(
                "expected DIPfile, a .dip/.dipl source, or a .diph5 archive".into(),
            ));
        }
        Ok(project)
    }

    pub fn add_source(&mut self, source: &str) -> Result<()> {
        self.ensure_unparsed()?;
        let source = text(source)?;
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe {
                ffi::snt_dip_parser_add_string(self.raw.as_ptr(), source.as_ptr(), &mut error)
            },
            error,
        )?;
        Ok(())
    }
    pub fn add_file(&mut self, path: impl AsRef<Path>) -> Result<()> {
        self.ensure_unparsed()?;
        let path = path_text(path.as_ref())?;
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe { ffi::snt_dip_parser_add_file(self.raw.as_ptr(), path.as_ptr(), &mut error) },
            error,
        )?;
        Ok(())
    }
    pub fn add_project(&mut self, path: impl AsRef<Path>) -> Result<()> {
        self.ensure_unparsed()?;
        let path = path_text(path.as_ref())?;
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe {
                ffi::snt_dip_parser_add_project(self.raw.as_ptr(), path.as_ptr(), &mut error)
            },
            error,
        )?;
        Ok(())
    }
    pub fn add_schema(&mut self, name: &str, body: &str) -> Result<()> {
        self.ensure_unparsed()?;
        let (name, body) = (text(name)?, text(body)?);
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe {
                ffi::snt_dip_parser_add_schema_string(
                    self.raw.as_ptr(),
                    name.as_ptr(),
                    body.as_ptr(),
                    &mut error,
                )
            },
            error,
        )?;
        Ok(())
    }
    pub fn add_override(&mut self, body: &str) -> Result<()> {
        self.ensure_unparsed()?;
        let body = text(body)?;
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe {
                ffi::snt_dip_parser_add_override_string(
                    self.raw.as_ptr(),
                    body.as_ptr(),
                    &mut error,
                )
            },
            error,
        )?;
        Ok(())
    }
    pub fn parse(&mut self) -> Result<()> {
        self.ensure_unparsed()?;
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe { ffi::snt_dip_parser_parse(self.raw.as_ptr(), &mut error) },
            error,
        )?;
        self.ready = true;
        Ok(())
    }
    pub fn save(&self, path: impl AsRef<Path>) -> Result<()> {
        self.ensure_ready()?;
        let path = path_text(path.as_ref())?;
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe { ffi::snt_dip_environment_save(self.raw.as_ptr(), path.as_ptr(), &mut error) },
            error,
        )
    }
    pub fn node(&self, path: &str) -> Result<Node<'_>> {
        self.ensure_ready()?;
        let path = text(path)?;
        let mut raw = ptr::null_mut();
        let mut error = ffi::snt_dip_error::default();
        check(
            unsafe {
                ffi::snt_dip_environment_get_node(
                    self.raw.as_ptr(),
                    path.as_ptr(),
                    &mut raw,
                    &mut error,
                )
            },
            error,
        )?;
        Ok(Node {
            raw: NonNull::new(raw).ok_or_else(|| Error("C ABI returned a null node".into()))?,
            _project: PhantomData,
        })
    }
    fn ensure_ready(&self) -> Result<()> {
        if self.ready {
            Ok(())
        } else {
            Err(Error("parse or load the project first".into()))
        }
    }
    fn ensure_unparsed(&self) -> Result<()> {
        if self.ready {
            Err(Error(
                "project is already evaluated; create a new Project for a different configuration"
                    .into(),
            ))
        } else {
            Ok(())
        }
    }
}
