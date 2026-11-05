use std::{env, path::PathBuf, process::Command};

fn run(command: &mut Command) {
    let status = command
        .status()
        .expect("Failed to run CMake; install CMake to build SNT3");
    assert!(status.success(), "Native SNT3 build failed with {status}");
}

fn build_native() -> PathBuf {
    let package = PathBuf::from(env::var_os("CARGO_MANIFEST_DIR").unwrap());
    let bundled = package.join("vendor/snt3");
    let checkout = package.ancestors().nth(3).unwrap().to_path_buf();
    // In the repo, use live sources. A downloaded crate uses its bundled snapshot.
    let (source, build_name) = if checkout.join("CMakeLists.txt").is_file() {
        (checkout, "snt3-checkout-build")
    } else if bundled.join("CMakeLists.txt").is_file() {
        (bundled, "snt3-bundled-build")
    } else {
        panic!("SNT3 source is missing from the crate; set SNT3_PREFIX or SNT3_BUILD_DIR");
    };
    if env::var("HOST").ok() != env::var("TARGET").ok() {
        panic!("Cross-compiling bundled SNT3 requires SNT3_PREFIX or SNT3_BUILD_DIR");
    }
    // Watch source inputs, not the whole checkout: Cargo's own target/ lives there.
    for relative in [
        "CMakeLists.txt",
        "settings.env",
        "cmake",
        "src",
        "include",
        "bindings/c",
        "external/nlohmann",
    ] {
        println!("cargo:rerun-if-changed={}", source.join(relative).display());
    }
    let build = PathBuf::from(env::var_os("OUT_DIR").unwrap()).join(build_name);
    run(Command::new("cmake")
        .arg("-S")
        .arg(&source)
        .arg("-B")
        .arg(&build)
        .arg("-DCMAKE_BUILD_TYPE=Release")
        .arg("-DENABLE_BINDING_C=ON")
        .arg("-DENABLE_BINDING_PYTHON=OFF")
        .arg("-DENABLE_UNIT_TESTS=OFF")
        .arg("-DENABLE_EXEC_APPS=OFF")
        .arg("-DENABLE_EXEC_EXAMPLES=OFF")
        .arg("-DENABLE_EXEC_BENCHMARKS=OFF")
        .arg("-DENABLE_SNT_REPORT=OFF")
        .arg("-DENABLE_MAT=OFF")
        .arg("-DENABLE_API=OFF"));
    run(Command::new("cmake")
        .arg("--build")
        .arg(&build)
        .arg("--config")
        .arg("Release")
        .arg("--target")
        .arg("snt-c")
        .arg("--parallel")
        .arg(env::var("NUM_JOBS").unwrap_or_else(|_| "1".into())));
    build
}

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

    if env::var_os("SNT3_BUILD_DIR").is_some() || env::var_os("SNT3_PREFIX").is_none() {
        let build = env::var_os("SNT3_BUILD_DIR")
            .map(PathBuf::from)
            .unwrap_or_else(build_native);
        println!(
            "cargo:rustc-link-search=native={}",
            build.join("bindings/c").display()
        );
        println!(
            "cargo:rustc-link-search=native={}",
            build.join("bindings/c/Release").display()
        );
        println!(
            "cargo:rustc-link-search=native={}",
            build.join("lib").display()
        );
        println!(
            "cargo:rustc-link-search=native={}",
            build.join("lib/Release").display()
        );
    } else {
        let prefix = env::var_os("SNT3_PREFIX").unwrap();
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
