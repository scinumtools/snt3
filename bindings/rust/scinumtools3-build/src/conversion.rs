use crate::{
    error::{BuildError, Result},
    mapping::{FieldKind, GenerationMode, RustType},
};
use scinumtools3::Node;

/// Convert a DIPL node value to a deterministic Rust expression string.
pub fn convert_node_value(
    field_name: &str,
    kind: &FieldKind,
    node: &Node<'_>,
    mode: &GenerationMode,
) -> Result<String> {
    match kind {
        FieldKind::Scalar(rust_type) => convert_scalar(field_name, *rust_type, node, mode),
        FieldKind::Enum(variants) => convert_enum(field_name, variants, node),
    }
}

fn convert_scalar(
    field_name: &str,
    rust_type: RustType,
    node: &Node<'_>,
    mode: &GenerationMode,
) -> Result<String> {
    match rust_type {
        RustType::Bool => {
            let val = node.as_bool().map_err(|e| BuildError::TypeMismatch {
                field: field_name.to_owned(),
                expected: "bool",
                found: format!("incompatible node ({e})"),
            })?;
            Ok(if val { "true" } else { "false" }.to_owned())
        }
        RustType::I8 => convert_signed_integer(field_name, node, i8::MIN as i64, i8::MAX as i64, "i8"),
        RustType::I16 => convert_signed_integer(field_name, node, i16::MIN as i64, i16::MAX as i64, "i16"),
        RustType::I32 => convert_signed_integer(field_name, node, i32::MIN as i64, i32::MAX as i64, "i32"),
        RustType::I64 => convert_signed_integer(field_name, node, i64::MIN, i64::MAX, "i64"),
        RustType::I128 => convert_signed_integer(field_name, node, i64::MIN, i64::MAX, "i128"),
        RustType::Isize => convert_signed_integer(
            field_name,
            node,
            isize::MIN as i64,
            isize::MAX as i64,
            "isize",
        ),
        RustType::U8 => convert_unsigned_integer(field_name, node, u8::MAX as u64, "u8"),
        RustType::U16 => convert_unsigned_integer(field_name, node, u16::MAX as u64, "u16"),
        RustType::U32 => convert_unsigned_integer(field_name, node, u32::MAX as u64, "u32"),
        RustType::U64 => convert_unsigned_integer(field_name, node, u64::MAX, "u64"),
        RustType::U128 => convert_unsigned_integer(field_name, node, u64::MAX, "u128"),
        RustType::Usize => convert_unsigned_integer(field_name, node, usize::MAX as u64, "usize"),
        RustType::F32 => convert_f32(field_name, node),
        RustType::F64 => convert_f64(field_name, node),
        RustType::Str => {
            let val = node.as_string().map_err(|e| BuildError::TypeMismatch {
                field: field_name.to_owned(),
                expected: "string",
                found: format!("incompatible node ({e})"),
            })?;
            Ok(escape_rust_string(&val))
        }
        RustType::String => {
            if matches!(mode, GenerationMode::Const(_)) {
                return Err(BuildError::UnsupportedConstType {
                    field: field_name.to_owned(),
                    type_name: "String",
                });
            }
            let val = node.as_string().map_err(|e| BuildError::TypeMismatch {
                field: field_name.to_owned(),
                expected: "string",
                found: format!("incompatible node ({e})"),
            })?;
            Ok(format!("::std::string::String::from({})", escape_rust_string(&val)))
        }
    }
}

fn convert_signed_integer(
    field_name: &str,
    node: &Node<'_>,
    min: i64,
    max: i64,
    target: &'static str,
) -> Result<String> {
    let val = if let Ok(s_val) = node.as_i64() {
        s_val
    } else if let Ok(u_val) = node.as_u64() {
        if u_val > max as u64 {
            return Err(BuildError::IntegerOutOfRange {
                field: field_name.to_owned(),
                value: u_val.to_string(),
                target,
            });
        }
        u_val as i64
    } else {
        return Err(BuildError::TypeMismatch {
            field: field_name.to_owned(),
            expected: target,
            found: "incompatible node (not an integer)".to_owned(),
        });
    };

    if val < min || val > max {
        return Err(BuildError::IntegerOutOfRange {
            field: field_name.to_owned(),
            value: val.to_string(),
            target,
        });
    }
    Ok(format!("{val}_{target}"))
}

fn convert_unsigned_integer(
    field_name: &str,
    node: &Node<'_>,
    max: u64,
    target: &'static str,
) -> Result<String> {
    let val = if let Ok(signed_val) = node.as_i64() {
        if signed_val < 0 {
            return Err(BuildError::IntegerOutOfRange {
                field: field_name.to_owned(),
                value: signed_val.to_string(),
                target,
            });
        }
        signed_val as u64
    } else if let Ok(u_val) = node.as_u64() {
        u_val
    } else {
        return Err(BuildError::TypeMismatch {
            field: field_name.to_owned(),
            expected: target,
            found: "incompatible node (not an integer)".to_owned(),
        });
    };

    if val > max {
        return Err(BuildError::IntegerOutOfRange {
            field: field_name.to_owned(),
            value: val.to_string(),
            target,
        });
    }
    Ok(format!("{val}_{target}"))
}

fn convert_f32(field_name: &str, node: &Node<'_>) -> Result<String> {
    let val = node.as_f64().map_err(|e| BuildError::TypeMismatch {
        field: field_name.to_owned(),
        expected: "f32",
        found: format!("incompatible node ({e})"),
    })?;
    if !val.is_finite() {
        return Err(BuildError::FloatNonFinite {
            field: field_name.to_owned(),
            value: val,
        });
    }
    if val.abs() > f32::MAX as f64 {
        return Err(BuildError::FloatOverflow {
            field: field_name.to_owned(),
            value: val,
            target: "f32",
        });
    }
    let f_val = val as f32;
    let mut text = format!("{f_val}");
    if !text.contains('.') && !text.contains('e') && !text.contains('E') {
        text.push_str(".0");
    }
    Ok(format!("{text}_f32"))
}

fn convert_f64(field_name: &str, node: &Node<'_>) -> Result<String> {
    let val = node.as_f64().map_err(|e| BuildError::TypeMismatch {
        field: field_name.to_owned(),
        expected: "f64",
        found: format!("incompatible node ({e})"),
    })?;
    if !val.is_finite() {
        return Err(BuildError::FloatNonFinite {
            field: field_name.to_owned(),
            value: val,
        });
    }
    let mut text = format!("{val}");
    if !text.contains('.') && !text.contains('e') && !text.contains('E') {
        text.push_str(".0");
    }
    Ok(format!("{text}_f64"))
}

fn convert_enum(
    field_name: &str,
    variants: &[crate::mapping::EnumMapping],
    node: &Node<'_>,
) -> Result<String> {
    let value = node.as_string().map_err(|e| BuildError::TypeMismatch {
        field: field_name.to_owned(),
        expected: "enum string identifier",
        found: format!("incompatible node ({e})"),
    })?;
    for mapping in variants {
        if mapping.dipl_value == value {
            return Ok(mapping.rust_variant.clone());
        }
    }
    let valid_variants = variants.iter().map(|m| m.dipl_value.clone()).collect();
    Err(BuildError::UnknownEnumVariant {
        field: field_name.to_owned(),
        value,
        valid_variants,
    })
}

/// Escape a string slice as a valid Rust string literal including surrounding quotes.
pub fn escape_rust_string(s: &str) -> String {
    let mut out = String::with_capacity(s.len() + 2);
    out.push('"');
    for ch in s.chars() {
        match ch {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            '\0' => out.push_str("\\0"),
            c if c.is_control() => {
                for b in c.to_string().as_bytes() {
                    use std::fmt::Write;
                    let _ = write!(out, "\\x{:02x}", b);
                }
            }
            c => out.push(c),
        }
    }
    out.push('"');
    out
}
