use std::fmt;

/// Result alias for build-helper operations.
pub type Result<T> = std::result::Result<T, BuildError>;

/// Errors encountered while evaluating DIPL, converting types, or generating Rust source.
#[derive(Debug)]
pub enum BuildError {
    /// Failure during DIPL parsing, validation, or node retrieval.
    Dipl(scinumtools3::Error),
    /// I/O error during file reading or code generation output.
    Io(std::io::Error),
    /// A requested DIPL parameter path does not exist in the evaluated environment.
    MissingPath {
        field: String,
        path: String,
        message: String,
    },
    /// A field was mapped more than once in the builder configuration.
    DuplicateField(String),
    /// A provided identifier is not a valid Rust identifier.
    InvalidIdentifier {
        context: &'static str,
        name: String,
    },
    /// A provided path is not a valid Rust path.
    InvalidRustPath {
        context: &'static str,
        path: String,
    },
    /// A DIPL node kind or type could not be converted to the requested Rust type.
    TypeMismatch {
        field: String,
        expected: &'static str,
        found: String,
    },
    /// An integer value is out of range for the destination Rust integer type.
    IntegerOutOfRange {
        field: String,
        value: String,
        target: &'static str,
    },
    /// A numeric float value is NaN or infinite.
    FloatNonFinite {
        field: String,
        value: f64,
    },
    /// A numeric float value overflows the destination floating-point type.
    FloatOverflow {
        field: String,
        value: f64,
        target: &'static str,
    },
    /// A DIPL value did not match any variant listed in the enum mapping.
    UnknownEnumVariant {
        field: String,
        value: String,
        valid_variants: Vec<String>,
    },
    /// An owned or allocated type was requested in const mode.
    UnsupportedConstType {
        field: String,
        type_name: &'static str,
    },
    /// Required builder configuration is missing before code generation.
    MissingConfiguration(&'static str),
}

impl fmt::Display for BuildError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Dipl(err) => write!(f, "DIPL evaluation failed: {err}"),
            Self::Io(err) => write!(f, "I/O error during code generation: {err}"),
            Self::MissingPath { field, path, message } => {
                write!(f, "field `{field}`: DIPL path `{path}` was not found: {message}")
            }
            Self::DuplicateField(field) => {
                write!(f, "field `{field}` is mapped more than once")
            }
            Self::InvalidIdentifier { context, name } => {
                write!(f, "invalid Rust identifier for {context}: `{name}`")
            }
            Self::InvalidRustPath { context, path } => {
                write!(f, "invalid Rust path for {context}: `{path}`")
            }
            Self::TypeMismatch { field, expected, found } => {
                write!(f, "field `{field}`: expected {expected}, found {found}")
            }
            Self::IntegerOutOfRange { field, value, target } => {
                write!(f, "field `{field}`: integer `{value}` is out of range for target type `{target}`")
            }
            Self::FloatNonFinite { field, value } => {
                write!(f, "field `{field}`: non-finite float value `{value}` cannot be emitted")
            }
            Self::FloatOverflow { field, value, target } => {
                write!(f, "field `{field}`: float value `{value}` overflows target type `{target}`")
            }
            Self::UnknownEnumVariant { field, value, valid_variants } => {
                write!(
                    f,
                    "field `{field}`: DIPL value `{value}` does not match any valid enum variant (options: {})",
                    valid_variants.join(", ")
                )
            }
            Self::UnsupportedConstType { field, type_name } => {
                write!(
                    f,
                    "field `{field}`: type `{type_name}` requires allocation and cannot be used in const mode; consider factory mode or RustType::Str"
                )
            }
            Self::MissingConfiguration(item) => {
                write!(f, "missing required configuration: {item}")
            }
        }
    }
}

impl std::error::Error for BuildError {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        match self {
            Self::Dipl(err) => Some(err),
            Self::Io(err) => Some(err),
            _ => None,
        }
    }
}

impl From<scinumtools3::Error> for BuildError {
    fn from(err: scinumtools3::Error) -> Self {
        Self::Dipl(err)
    }
}

impl From<std::io::Error> for BuildError {
    fn from(err: std::io::Error) -> Self {
        Self::Io(err)
    }
}
