use std::{env, path::PathBuf, process::Command};

fn main() {
    for name in [
        "SNT3_PREFIX",
        "SNT3_BUILD_DIR",
        "SNT3_HDF5_LIB_DIR",
        "SNT3_HDF5_LIB_NAME",
    ] {
        println!("cargo:rerun-if-env-changed={name}");
    }
    if env::var_os("DOCS_RS").is_some() {
        return;
    }

    if let Some(build) = env::var_os("SNT3_BUILD_DIR") {
        let build = PathBuf::from(build);
        println!(
            "cargo:rustc-link-search=native={}",
            build.join("bindings/c").display()
        );
        println!(
            "cargo:rustc-link-search=native={}",
            build.join("lib").display()
        );
    } else {
        let prefix = env::var_os("SNT3_PREFIX")
            .expect("Set SNT3_PREFIX to an installed SNT3, or SNT3_BUILD_DIR to a CMake build");
        let prefix = PathBuf::from(prefix);
        let lib = if prefix.join("lib").is_dir() {
            prefix.join("lib")
        } else {
            prefix.join("lib64")
        };
        println!("cargo:rustc-link-search=native={}", lib.display());
    }

    // snt-c is static, so the dependent archives must follow it on the link line.
    for name in [
        "snt-c", "snt-dip", "snt-puq", "snt-val", "snt-exs", "snt-core",
    ] {
        println!("cargo:rustc-link-lib=static={name}");
    }
    if let Some(dir) = env::var_os("SNT3_HDF5_LIB_DIR") {
        println!(
            "cargo:rustc-link-search=native={}",
            PathBuf::from(dir).display()
        );
        println!(
            "cargo:rustc-link-lib={}",
            env::var("SNT3_HDF5_LIB_NAME").unwrap_or_else(|_| "hdf5".into())
        );
    } else {
        let result = Command::new("pkg-config")
            .args(["--libs", "hdf5"])
            .output()
            .expect("pkg-config is needed to find HDF5; set SNT3_HDF5_LIB_DIR to override it");
        assert!(result.status.success(), "pkg-config could not find HDF5");
        for flag in String::from_utf8_lossy(&result.stdout).split_whitespace() {
            if let Some(path) = flag.strip_prefix("-L") {
                println!("cargo:rustc-link-search=native={path}");
            } else if let Some(name) = flag.strip_prefix("-l") {
                println!("cargo:rustc-link-lib={name}");
            }
        }
    }
    match env::var("CARGO_CFG_TARGET_OS").as_deref() {
        Ok("macos") => println!("cargo:rustc-link-lib=c++"),
        Ok("linux") => println!("cargo:rustc-link-lib=stdc++"),
        _ => {}
    }
}
