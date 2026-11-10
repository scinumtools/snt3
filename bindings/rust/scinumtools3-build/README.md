# `scinumtools3-build`

Compile-time DIPL evaluation and Rust struct initializer generation for Cargo build scripts.

## Overview

`scinumtools3-build` evaluates and validates DIPL configurations during Cargo's build-script phase (`build.rs`), then generates Rust initializer expressions directly for structs and enums defined by the consuming application.

The consuming application builds using standard `cargo build` or `cargo build --release` without requiring a separately installed SNT CLI, Python interpreter, or Python bindings.

## Architecture

| Component | Responsibility |
|---|---|
| **DIPL Project & Schemas** | Declares configuration parameters, overrides, and units; validates constraints |
| **`scinumtools3-build`** | Loads evaluated DIPL values, performs checked type conversions, emits Rust initializer expressions |
| **Application Rust Code** | Owns destination structs and enums; determines which parameters are compiled |
| **`rustc`** | Type-checks generated initializers against existing application definitions |

> **Key Rule:** Build dependencies cannot depend on the application crate being built without creating a cyclic dependency. `scinumtools3-build` never imports or inspects consumer ASTs; it generates source code referencing target paths (e.g. `crate::settings::GuiDefaults`) and lets `rustc` enforce type compatibility.

## Usage

### 1. `Cargo.toml`

```toml
[build-dependencies]
scinumtools3-build = "0.10.0"
```

### 2. Application Structs (`src/settings.rs`)

```rust
#[derive(Debug, PartialEq)]
pub enum Colormap {
    Viridis,
    Plasma,
    Grayscale,
}

#[derive(Debug, PartialEq)]
pub struct GuiDefaults {
    pub sidebar_width: f32,
    pub sidebar_min_width: f32,
    pub sidebar_max_width: f32,
    pub histogram_bins: usize,
    pub colormap: Colormap,
}
```

### 3. Build Script (`build.rs`)

```rust
use scinumtools3_build::{Config, RustType};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    Config::new("settings/gui.dip")
        .rust_struct("crate::settings::GuiDefaults")
        .const_name("GUI_DEFAULTS")
        .field("sidebar_width", "viewer.layout.sidebar_width", RustType::F32)
        .field("sidebar_min_width", "viewer.layout.sidebar_min_width", RustType::F32)
        .field("sidebar_max_width", "viewer.layout.sidebar_max_width", RustType::F32)
        .field("histogram_bins", "viewer.statistics.histogram_bins", RustType::Usize)
        .enum_field(
            "colormap",
            "viewer.views.slice_2d.colormap",
            &[
                ("viridis", "crate::settings::Colormap::Viridis"),
                ("plasma", "crate::settings::Colormap::Plasma"),
                ("grayscale", "crate::settings::Colormap::Grayscale"),
            ],
        )
        .generate("gui_defaults.rs")?;
    Ok(())
}
```

### 4. Inclusion in Application (`src/main.rs`)

```rust
mod settings;

mod compiled_settings {
    include!(concat!(env!("OUT_DIR"), "/gui_defaults.rs"));
}

fn main() {
    let defaults = compiled_settings::GUI_DEFAULTS;
    println!("Sidebar width: {}", defaults.sidebar_width);
}
```

## Generation Modes

### Const Mode (`const_name`)
Emits compile-time constant initializers:
```rust
pub const GUI_DEFAULTS: crate::settings::GuiDefaults =
    crate::settings::GuiDefaults {
        sidebar_width: 300.0_f32,
        ...
    };
```
Ideal for primitives, string slices (`&'static str`), and simple enum variants.

### Factory Mode (`factory_name`)
Emits constructor functions:
```rust
pub fn make_app_settings() -> crate::AppSettings {
    crate::AppSettings {
        name: ::std::string::String::from("Scientific Studio"),
        ...
    }
}
```
Required for heap-allocated types such as owned `String` or collections.

## Supported Types & Conversions

* **Booleans:** DIPL boolean $\to$ `bool`
* **Signed Integers:** `i8`, `i16`, `i32`, `i64`, `i128`, `isize` (bounds-checked against target limits)
* **Unsigned Integers:** `u8`, `u16`, `u32`, `u64`, `u128`, `usize` (bounds-checked; negative DIPL values rejected)
* **Floats:** `f32`, `f64` (checked for non-finite values and target overflow)
* **Strings:**
  * `RustType::Str` $\to$ properly escaped `&'static str`
  * `RustType::String` $\to$ `::std::string::String::from(...)` (factory mode only)
* **Enums:** Explicit mapping from DIPL string values to qualified Rust enum paths (`.enum_field(...)`)

## Transitive Dependency Tracking & Reproducibility

* **Cargo Tracking:** Automatically emits `cargo:rerun-if-changed` for all contributing DIPL files, including the root `DIPfile`, imported sources, and schemas.
* **Deterministic Code Generation:** Field order is preserved as configured. Numeric literals and string escapes are deterministically formatted.
* **Atomic Writes:** Generated files are written to temporary files and atomically renamed. If the existing file matches byte-for-byte, rewriting is skipped to preserve modification timestamps and prevent unnecessary downstream recompilation.
