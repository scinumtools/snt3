//! `scinumtools3-build` — Compile-Time DIPL Settings for Rust.
//!
//! Evaluates and validates DIPL configurations during Cargo's build phase and
//! generates Rust expressions that initialize structs defined by the consuming
//! application.

mod conversion;
mod error;
mod mapping;
mod rust_codegen;

pub use error::{BuildError, Result};
pub use mapping::{EnumMapping, FieldKind, FieldMapping, GenerationMode, RustType};

use scinumtools3::Project;
use std::{
    collections::HashSet,
    env,
    path::{Path, PathBuf},
};

/// Builder configuration for compile-time DIPL evaluation and Rust struct initializer generation.
pub struct Config {
    manifest_path: PathBuf,
    rust_struct: Option<String>,
    mode: Option<GenerationMode>,
    fields: Vec<FieldMapping>,
    emit_rerun_directives: bool,
    out_dir: Option<PathBuf>,
}

impl Config {
    /// Create a new configuration pointing to a `DIPfile` manifest or DIPL source file.
    pub fn new(manifest_path: impl AsRef<Path>) -> Self {
        Self {
            manifest_path: manifest_path.as_ref().to_path_buf(),
            rust_struct: None,
            mode: None,
            fields: Vec::new(),
            emit_rerun_directives: true,
            out_dir: None,
        }
    }

    /// Set the destination Rust struct path (e.g. `"crate::settings::GuiDefaults"`).
    pub fn rust_struct(mut self, name: impl Into<String>) -> Self {
        self.rust_struct = Some(name.into());
        self
    }

    /// Configure const generation mode with the given constant item name.
    ///
    /// Generates `pub const NAME: TargetType = TargetType { ... };`.
    pub fn const_name(mut self, name: impl Into<String>) -> Self {
        self.mode = Some(GenerationMode::Const(name.into()));
        self
    }

    /// Configure factory generation mode with the given constructor function name.
    ///
    /// Generates `pub fn make_settings() -> TargetType { TargetType { ... } }`.
    pub fn factory_name(mut self, name: impl Into<String>) -> Self {
        self.mode = Some(GenerationMode::Factory(name.into()));
        self
    }

    /// Set the generation mode explicitly.
    pub fn mode(mut self, mode: GenerationMode) -> Self {
        self.mode = Some(mode);
        self
    }

    /// Explicitly override the output directory where generated code is written.
    /// Defaults to Cargo's `OUT_DIR` environment variable.
    pub fn out_dir(mut self, path: impl AsRef<Path>) -> Self {
        self.out_dir = Some(path.as_ref().to_path_buf());
        self
    }

    /// Control whether `cargo:rerun-if-changed` directives are automatically printed.
    /// Defaults to `true`.
    pub fn emit_rerun_directives(mut self, emit: bool) -> Self {
        self.emit_rerun_directives = emit;
        self
    }

    /// Map an application struct field to a DIPL path with a target primitive or string type.
    pub fn field(
        mut self,
        field_name: impl Into<String>,
        dipl_path: impl Into<String>,
        rust_type: RustType,
    ) -> Self {
        self.fields.push(FieldMapping {
            field_name: field_name.into(),
            dipl_path: dipl_path.into(),
            kind: FieldKind::Scalar(rust_type),
        });
        self
    }

    /// Map an application struct field to a DIPL symbolic string mapped to Rust enum variants.
    pub fn enum_field<S1, S2>(
        mut self,
        field_name: impl Into<String>,
        dipl_path: impl Into<String>,
        variants: &[(S1, S2)],
    ) -> Self
    where
        S1: AsRef<str>,
        S2: AsRef<str>,
    {
        let mappings = variants
            .iter()
            .map(|(dipl_val, rust_var)| EnumMapping {
                dipl_value: dipl_val.as_ref().to_owned(),
                rust_variant: rust_var.as_ref().to_owned(),
            })
            .collect();
        self.fields.push(FieldMapping {
            field_name: field_name.into(),
            dipl_path: dipl_path.into(),
            kind: FieldKind::Enum(mappings),
        });
        self
    }

    /// Evaluate the DIPL configuration and write the generated Rust source file.
    ///
    /// If `output_file` is relative, it is resolved against `out_dir` or `$OUT_DIR`.
    pub fn generate(self, output_file: impl AsRef<Path>) -> Result<PathBuf> {
        let rust_struct = self
            .rust_struct
            .ok_or(BuildError::MissingConfiguration("rust_struct"))?;
        rust_codegen::validate_rust_path(&rust_struct, "rust_struct")?;

        let mode = self
            .mode
            .unwrap_or_else(|| GenerationMode::Const("SETTINGS".to_owned()));
        match &mode {
            GenerationMode::Const(name) => rust_codegen::validate_identifier(name, "const_name")?,
            GenerationMode::Factory(name) => {
                rust_codegen::validate_identifier(name, "factory_name")?
            }
        }

        let base_dir = env::var_os("CARGO_MANIFEST_DIR")
            .map(PathBuf::from)
            .unwrap_or_else(|| env::current_dir().unwrap_or_else(|_| PathBuf::from(".")));
        let resolved_manifest = if self.manifest_path.is_relative() {
            base_dir.join(&self.manifest_path)
        } else {
            self.manifest_path.clone()
        };

        let project = Project::load(&resolved_manifest)?;

        if self.emit_rerun_directives {
            println!("cargo:rerun-if-changed={}", resolved_manifest.display());
            if let Ok(sources) = project.source_files() {
                for source in sources {
                    println!("cargo:rerun-if-changed={}", source.display());
                }
            }
        }

        let mut seen_fields = HashSet::new();
        let mut initializers = Vec::with_capacity(self.fields.len());

        for field in &self.fields {
            if !seen_fields.insert(&field.field_name) {
                return Err(BuildError::DuplicateField(field.field_name.clone()));
            }
            rust_codegen::validate_identifier(&field.field_name, "field_name")?;

            let node = project
                .node(&field.dipl_path)
                .map_err(|e| BuildError::MissingPath {
                    field: field.field_name.clone(),
                    path: field.dipl_path.clone(),
                    message: e.to_string(),
                })?;

            let expr = conversion::convert_node_value(&field.field_name, &field.kind, &node, &mode)?;
            initializers.push((field.field_name.clone(), expr));
        }

        let source = rust_codegen::generate_source(&rust_struct, &mode, &initializers);

        let dest_path = if output_file.as_ref().is_relative() {
            let out_dir = self
                .out_dir
                .or_else(|| env::var_os("OUT_DIR").map(PathBuf::from))
                .unwrap_or_else(|| base_dir.clone());
            out_dir.join(output_file.as_ref())
        } else {
            output_file.as_ref().to_path_buf()
        };

        rust_codegen::write_if_changed(&dest_path, &source)?;
        Ok(dest_path)
    }
}
