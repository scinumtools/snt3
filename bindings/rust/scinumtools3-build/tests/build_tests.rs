use scinumtools3_build::{BuildError, Config, RustType};
use std::{
    fs,
    path::PathBuf,
    process::Command,
    time::{SystemTime, UNIX_EPOCH},
};

static COUNTER: std::sync::atomic::AtomicU64 = std::sync::atomic::AtomicU64::new(0);

fn temporary_dir() -> PathBuf {
    let count = COUNTER.fetch_add(1, std::sync::atomic::Ordering::Relaxed);
    let stamp = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let path = std::env::temp_dir().join(format!("snt3-build-test-{}-{stamp}-{count}", std::process::id()));
    fs::create_dir_all(&path).unwrap();
    path
}

#[test]
fn test_const_mode_generates_valid_initializer() {
    let dir = temporary_dir();
    let dip_file = dir.join("settings.dip");
    fs::write(
        &dip_file,
        r#"viewer
  layout
    sidebar_width float = 300.0
    sidebar_min_width float = 180.0
    sidebar_max_width float = 800.0
  statistics
    histogram_bins int = 128
  views
    slice_2d
      colormap str = "viridis"
"#,
    )
    .unwrap();

    let out_file = dir.join("gui_defaults.rs");

    Config::new(&dip_file)
        .rust_struct("crate::settings::GuiDefaults")
        .const_name("GUI_DEFAULTS")
        .out_dir(&dir)
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
        .generate(&out_file)
        .unwrap();

    let content = fs::read_to_string(&out_file).unwrap();
    assert!(content.contains("pub const GUI_DEFAULTS: crate::settings::GuiDefaults ="));
    assert!(content.contains("sidebar_width: 300.0_f32,"));
    assert!(content.contains("sidebar_min_width: 180.0_f32,"));
    assert!(content.contains("sidebar_max_width: 800.0_f32,"));
    assert!(content.contains("histogram_bins: 128_usize,"));
    assert!(content.contains("colormap: crate::settings::Colormap::Viridis,"));

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_factory_mode_with_owned_strings() {
    let dir = temporary_dir();
    let dip_file = dir.join("app.dip");
    fs::write(
        &dip_file,
        r#"app
  name str = "Scientific Studio"
  version str = "1.0.0"
  max_threads int = 8
  debug bool = false
"#,
    )
    .unwrap();

    let out_file = dir.join("app_settings.rs");

    Config::new(&dip_file)
        .rust_struct("crate::AppSettings")
        .factory_name("make_app_settings")
        .out_dir(&dir)
        .field("name", "app.name", RustType::String)
        .field("version", "app.version", RustType::Str)
        .field("max_threads", "app.max_threads", RustType::U32)
        .field("debug", "app.debug", RustType::Bool)
        .generate(&out_file)
        .unwrap();

    let content = fs::read_to_string(&out_file).unwrap();
    assert!(content.contains("pub fn make_app_settings() -> crate::AppSettings {"));
    assert!(content.contains("name: ::std::string::String::from(\"Scientific Studio\"),"));
    assert!(content.contains("version: \"1.0.0\","));
    assert!(content.contains("max_threads: 8_u32,"));
    assert!(content.contains("debug: false,"));

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_const_mode_rejects_owned_string() {
    let dir = temporary_dir();
    let dip_file = dir.join("test.dip");
    fs::write(&dip_file, "title str = \"test\"\n").unwrap();

    let res = Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .field("title", "title", RustType::String)
        .generate("out.rs");

    match res {
        Err(BuildError::UnsupportedConstType { field, type_name }) => {
            assert_eq!(field, "title");
            assert_eq!(type_name, "String");
        }
        other => panic!("expected UnsupportedConstType, got {other:?}"),
    }

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_missing_path_fails() {
    let dir = temporary_dir();
    let dip_file = dir.join("test.dip");
    fs::write(&dip_file, "existing int = 1\n").unwrap();

    let res = Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .field("nonexistent", "missing.path", RustType::I32)
        .generate("out.rs");

    match res {
        Err(BuildError::MissingPath { field, path, .. }) => {
            assert_eq!(field, "nonexistent");
            assert_eq!(path, "missing.path");
        }
        other => panic!("expected MissingPath, got {other:?}"),
    }

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_duplicate_field_mapping_fails() {
    let dir = temporary_dir();
    let dip_file = dir.join("test.dip");
    fs::write(&dip_file, "a int = 1\nb int = 2\n").unwrap();

    let res = Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .field("same_field", "a", RustType::I32)
        .field("same_field", "b", RustType::I32)
        .generate("out.rs");

    match res {
        Err(BuildError::DuplicateField(field)) => {
            assert_eq!(field, "same_field");
        }
        other => panic!("expected DuplicateField, got {other:?}"),
    }

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_unknown_enum_variant_fails() {
    let dir = temporary_dir();
    let dip_file = dir.join("test.dip");
    fs::write(&dip_file, "color str = \"rainbow\"\n").unwrap();

    let res = Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .enum_field(
            "color",
            "color",
            &[
                ("red", "crate::Color::Red"),
                ("blue", "crate::Color::Blue"),
            ],
        )
        .generate("out.rs");

    match res {
        Err(BuildError::UnknownEnumVariant { field, value, valid_variants }) => {
            assert_eq!(field, "color");
            assert_eq!(value, "rainbow");
            assert_eq!(valid_variants, vec!["red", "blue"]);
        }
        other => panic!("expected UnknownEnumVariant, got {other:?}"),
    }

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_unsigned_integer_rejects_negative_and_overflow() {
    let dir = temporary_dir();
    let dip_file = dir.join("test.dip");
    fs::write(&dip_file, "negative int = -10\noverflow int = 300\n").unwrap();

    let res_neg = Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .field("val", "negative", RustType::Usize)
        .generate("out.rs");

    match res_neg {
        Err(BuildError::IntegerOutOfRange { field, target, .. }) => {
            assert_eq!(field, "val");
            assert_eq!(target, "usize");
        }
        other => panic!("expected IntegerOutOfRange, got {other:?}"),
    }

    let res_overflow = Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .field("val", "overflow", RustType::U8)
        .generate("out.rs");

    match res_overflow {
        Err(BuildError::IntegerOutOfRange { field, target, .. }) => {
            assert_eq!(field, "val");
            assert_eq!(target, "u8");
        }
        other => panic!("expected IntegerOutOfRange, got {other:?}"),
    }

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_string_escaping_special_characters() {
    let dir = temporary_dir();
    let dip_file = dir.join("test.dip");
    fs::write(
        &dip_file,
        "escaped str = \"Line 1\\nLine 2\\n\\\"Quoted\\\" and \\\\ backslash and Unicode: äöü 🚀\"\n",
    )
    .unwrap();

    let out_file = dir.join("escaped.rs");
    Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .field("text", "escaped", RustType::Str)
        .generate(&out_file)
        .unwrap();

    let content = fs::read_to_string(&out_file).unwrap();
    assert!(content.contains("pub const TEST: crate::Test ="));

    // Verify it compiles and executes with rustc
    let test_rs = dir.join("test_escape.rs");
    let test_source = format!(
        r#"
pub struct Test {{
    pub text: &'static str,
}}

include!({:?});

fn main() {{
    assert!(TEST.text.contains("Line 1\nLine 2"));
    assert!(TEST.text.contains("\"Quoted\""));
    assert!(TEST.text.contains("\\ backslash"));
    assert!(TEST.text.contains("Unicode: äöü 🚀"));
}}
"#,
        out_file.to_string_lossy()
    );
    fs::write(&test_rs, test_source).unwrap();

    let bin = dir.join(format!("test_escape_bin{}", std::env::consts::EXE_SUFFIX));
    let compile_status = Command::new("rustc")
        .arg(&test_rs)
        .arg("-o")
        .arg(&bin)
        .status()
        .expect("failed to invoke rustc");
    assert!(compile_status.success(), "rustc compilation failed for escaped strings");

    let run_status = Command::new(&bin)
        .status()
        .expect("failed to execute test binary");
    assert!(run_status.success(), "escaped string test binary execution failed");

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_idempotent_writes_preserves_timestamp() {
    let dir = temporary_dir();
    let dip_file = dir.join("test.dip");
    fs::write(&dip_file, "val int = 42\n").unwrap();
    let out_file = dir.join("out.rs");

    Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .field("val", "val", RustType::I32)
        .generate(&out_file)
        .unwrap();

    let mtime1 = fs::metadata(&out_file).unwrap().modified().unwrap();

    std::thread::sleep(std::time::Duration::from_millis(50));

    Config::new(&dip_file)
        .rust_struct("crate::Test")
        .const_name("TEST")
        .out_dir(&dir)
        .field("val", "val", RustType::I32)
        .generate(&out_file)
        .unwrap();

    let mtime2 = fs::metadata(&out_file).unwrap().modified().unwrap();
    assert_eq!(mtime1, mtime2, "idempotent generation must not rewrite unchanged file");

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_actual_rustc_compilation_and_execution() {
    let dir = temporary_dir();
    let dip_file = dir.join("settings.dip");
    fs::write(
        &dip_file,
        r#"viewer
  layout
    sidebar_width float = 300.0
    sidebar_min_width float = 180.0
    sidebar_max_width float = 800.0
  statistics
    histogram_bins int = 128
  views
    slice_2d
      colormap str = "plasma"
"#,
    )
    .unwrap();

    let generated_rs = dir.join("gui_defaults.rs");
    Config::new(&dip_file)
        .rust_struct("crate::settings::GuiDefaults")
        .const_name("GUI_DEFAULTS")
        .out_dir(&dir)
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
        .generate(&generated_rs)
        .unwrap();

    let main_rs = dir.join("main.rs");
    let main_source = format!(
        r#"
pub mod settings {{
    #[derive(Debug, PartialEq)]
    pub enum Colormap {{
        Viridis,
        Plasma,
        Grayscale,
    }}

    #[derive(Debug, PartialEq)]
    pub struct GuiDefaults {{
        pub sidebar_width: f32,
        pub sidebar_min_width: f32,
        pub sidebar_max_width: f32,
        pub histogram_bins: usize,
        pub colormap: Colormap,
    }}
}}

mod compiled_settings {{
    include!({:?});
}}

fn main() {{
    let defaults = compiled_settings::GUI_DEFAULTS;
    assert_eq!(defaults.sidebar_width, 300.0);
    assert_eq!(defaults.sidebar_min_width, 180.0);
    assert_eq!(defaults.sidebar_max_width, 800.0);
    assert_eq!(defaults.histogram_bins, 128);
    assert_eq!(defaults.colormap, settings::Colormap::Plasma);
    println!("SUCCESS");
}}
"#,
        generated_rs.to_string_lossy()
    );
    fs::write(&main_rs, main_source).unwrap();

    let bin_path = dir.join(format!("test_bin{}", std::env::consts::EXE_SUFFIX));
    let compile_status = Command::new("rustc")
        .arg(&main_rs)
        .arg("-o")
        .arg(&bin_path)
        .status()
        .expect("failed to invoke rustc");

    assert!(compile_status.success(), "generated code failed to compile with rustc");

    let run_status = Command::new(&bin_path)
        .status()
        .expect("failed to execute compiled test binary");
    assert!(run_status.success(), "compiled test binary failed execution");

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_rustc_fails_on_incompatible_consumer_types() {
    let dir = temporary_dir();
    let dip_file = dir.join("settings.dip");
    fs::write(&dip_file, "val int = 42\n").unwrap();

    let generated_rs = dir.join("compiled.rs");
    Config::new(&dip_file)
        .rust_struct("crate::Settings")
        .const_name("SETTINGS")
        .out_dir(&dir)
        .field("val", "val", RustType::I64)
        .generate(&generated_rs)
        .unwrap();

    // Field in struct is f32, but generated initializer is 42_i64
    let main_rs = dir.join("type_mismatch.rs");
    let main_source = format!(
        r#"
pub struct Settings {{
    pub val: f32,
}}

include!({:?});

fn main() {{}}
"#,
        generated_rs.to_string_lossy()
    );
    fs::write(&main_rs, main_source).unwrap();

    let compile_output = Command::new("rustc")
        .arg(&main_rs)
        .arg("-o")
        .arg(dir.join("should_fail_bin"))
        .output()
        .expect("failed to invoke rustc");

    assert!(
        !compile_output.status.success()
            && String::from_utf8_lossy(&compile_output.stderr).contains("error[E0308]"),
        "rustc must reject the field type: {}",
        String::from_utf8_lossy(&compile_output.stderr)
    );

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_invalid_dipl_fails_generation() {
    let dir = temporary_dir();
    let dip_file = dir.join("invalid.dip");
    // Syntax error in DIPL
    fs::write(&dip_file, "this is definitely !@# not valid dipl\n").unwrap();

    let res = Config::new(&dip_file)
        .rust_struct("crate::Settings")
        .const_name("SETTINGS")
        .out_dir(&dir)
        .field("val", "val", RustType::I32)
        .generate("compiled.rs");

    match res {
        Err(BuildError::Dipl(_)) => {}
        other => panic!("expected BuildError::Dipl on invalid DIPL syntax, got {other:?}"),
    }

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_rustc_fails_on_renamed_consumer_field() {
    let dir = temporary_dir();
    let dip_file = dir.join("settings.dip");
    fs::write(&dip_file, "sidebar_width float = 300.0\n").unwrap();

    let generated_rs = dir.join("compiled.rs");
    Config::new(&dip_file)
        .rust_struct("crate::Settings")
        .const_name("SETTINGS")
        .out_dir(&dir)
        .field("sidebar_width", "sidebar_width", RustType::F32)
        .generate(&generated_rs)
        .unwrap();

    // Application struct has renamed the field to `width`
    let main_rs = dir.join("renamed_field.rs");
    let main_source = format!(
        r#"
pub struct Settings {{
    pub width: f32,
}}

include!({:?});

fn main() {{}}
"#,
        generated_rs.to_string_lossy()
    );
    fs::write(&main_rs, main_source).unwrap();

    let compile_output = Command::new("rustc")
        .arg(&main_rs)
        .arg("-o")
        .arg(dir.join("should_fail_bin"))
        .output()
        .expect("failed to invoke rustc");

    assert!(
        !compile_output.status.success()
            && String::from_utf8_lossy(&compile_output.stderr).contains("error[E0560]"),
        "rustc must reject the missing field: {}",
        String::from_utf8_lossy(&compile_output.stderr)
    );

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_rustc_fails_on_deleted_enum_variant() {
    let dir = temporary_dir();
    let dip_file = dir.join("settings.dip");
    fs::write(&dip_file, "colormap str = \"plasma\"\n").unwrap();

    let generated_rs = dir.join("compiled.rs");
    Config::new(&dip_file)
        .rust_struct("crate::Settings")
        .const_name("SETTINGS")
        .out_dir(&dir)
        .enum_field(
            "colormap",
            "colormap",
            &[("plasma", "crate::Colormap::Plasma")],
        )
        .generate(&generated_rs)
        .unwrap();

    // Consumer removed Plasma variant from enum
    let main_rs = dir.join("deleted_variant.rs");
    let main_source = format!(
        r#"
pub enum Colormap {{
    Viridis,
}}

pub struct Settings {{
    pub colormap: Colormap,
}}

include!({:?});

fn main() {{}}
"#,
        generated_rs.to_string_lossy()
    );
    fs::write(&main_rs, main_source).unwrap();

    let compile_output = Command::new("rustc")
        .arg(&main_rs)
        .arg("-o")
        .arg(dir.join("should_fail_bin"))
        .output()
        .expect("failed to invoke rustc");

    assert!(
        !compile_output.status.success()
            && String::from_utf8_lossy(&compile_output.stderr).contains("error[E0599]"),
        "rustc must reject the missing enum variant: {}",
        String::from_utf8_lossy(&compile_output.stderr)
    );

    fs::remove_dir_all(dir).unwrap();
}

#[test]
fn test_transitive_dependency_tracking_across_imports() {
    let dir = temporary_dir();
    let dipfile = dir.join("DIPfile");
    let base_dip = dir.join("base.dip");
    let extra_dip = dir.join("extra.dip");

    fs::write(
        &dipfile,
        r#"code[]
  file = "base.dip"
code[]
  file = "extra.dip"
"#,
    )
    .unwrap();
    fs::write(&base_dip, "width float = 250.0\n").unwrap();
    fs::write(&extra_dip, "height float = 150.0\n").unwrap();

    let generated_rs = dir.join("layout.rs");
    Config::new(&dipfile)
        .rust_struct("crate::Layout")
        .const_name("LAYOUT")
        .out_dir(&dir)
        .field("width", "width", RustType::F32)
        .field("height", "height", RustType::F32)
        .generate(&generated_rs)
        .unwrap();

    let content = fs::read_to_string(&generated_rs).unwrap();
    assert!(content.contains("width: 250.0_f32,"));
    assert!(content.contains("height: 150.0_f32,"));

    // Verify Project reports all source files
    let project = scinumtools3::Project::load(&dipfile).unwrap();
    let sources = project.source_files().unwrap();
    assert!(sources.iter().any(|p| p.ends_with("base.dip")));
    assert!(sources.iter().any(|p| p.ends_with("extra.dip")));

    fs::remove_dir_all(dir).unwrap();
}
