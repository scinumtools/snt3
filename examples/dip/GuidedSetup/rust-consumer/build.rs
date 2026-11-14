use scinumtools3_build::{Config, RustType};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    Config::new("../DIPfile")
        .rust_struct("crate::BuildSettings")
        .const_name("BUILD_SETTINGS")
        .field("max_threads", "build.max_threads", RustType::Usize)
        .field("trace", "build.trace", RustType::Bool)
        .generate("build_settings.rs")?;
    Ok(())
}
