/// Supported primitive and string types for consumer struct fields.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RustType {
    /// Rust `bool`.
    Bool,
    /// Rust 8-bit signed integer `i8`.
    I8,
    /// Rust 16-bit signed integer `i16`.
    I16,
    /// Rust 32-bit signed integer `i32`.
    I32,
    /// Rust 64-bit signed integer `i64`.
    I64,
    /// Rust 128-bit signed integer `i128`.
    I128,
    /// Rust pointer-width signed integer `isize`.
    Isize,
    /// Rust 8-bit unsigned integer `u8`.
    U8,
    /// Rust 16-bit unsigned integer `u16`.
    U16,
    /// Rust 32-bit unsigned integer `u32`.
    U32,
    /// Rust 64-bit unsigned integer `u64`.
    U64,
    /// Rust 128-bit unsigned integer `u128`.
    U128,
    /// Rust pointer-width unsigned integer `usize`.
    Usize,
    /// Rust 32-bit floating point `f32`.
    F32,
    /// Rust 64-bit floating point `f64`.
    F64,
    /// Rust string slice `&'static str`. Supported in both const and factory modes.
    Str,
    /// Owned `String`. Requires allocation; only valid in factory mode.
    String,
}

impl RustType {
    /// Return the canonical type name as a string slice.
    pub fn type_name(&self) -> &'static str {
        match self {
            Self::Bool => "bool",
            Self::I8 => "i8",
            Self::I16 => "i16",
            Self::I32 => "i32",
            Self::I64 => "i64",
            Self::I128 => "i128",
            Self::Isize => "isize",
            Self::U8 => "u8",
            Self::U16 => "u16",
            Self::U32 => "u32",
            Self::U64 => "u64",
            Self::U128 => "u128",
            Self::Usize => "usize",
            Self::F32 => "f32",
            Self::F64 => "f64",
            Self::Str => "&'static str",
            Self::String => "String",
        }
    }
}

/// Mapping between a DIPL string value and an application enum variant.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EnumMapping {
    pub dipl_value: String,
    pub rust_variant: String,
}

/// Specification for how a field value is extracted and emitted.
#[derive(Debug, Clone)]
pub enum FieldKind {
    /// Direct scalar conversion to a Rust primitive or string.
    Scalar(RustType),
    /// Symbolic mapping to an application enum variant.
    Enum(Vec<EnumMapping>),
}

/// Mapping from an application struct field to a DIPL path.
#[derive(Debug, Clone)]
pub struct FieldMapping {
    pub field_name: String,
    pub dipl_path: String,
    pub kind: FieldKind,
}

/// Generation mode for the emitted Rust initializer.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum GenerationMode {
    /// Generates `pub const NAME: TargetType = TargetType { ... };`.
    Const(String),
    /// Generates `pub fn make_settings() -> TargetType { TargetType { ... } }`.
    Factory(String),
}
