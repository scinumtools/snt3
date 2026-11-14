use scinumtools3::Project;

pub struct BuildSettings {
    pub max_threads: usize,
    pub trace: bool,
}

mod compiled_settings {
    include!(concat!(env!("OUT_DIR"), "/build_settings.rs"));
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let build = compiled_settings::BUILD_SETTINGS;
    let project = Project::load(concat!(env!("CARGO_MANIFEST_DIR"), "/../DIPfile"))?;
    let steps = project.node("simulation.steps")?.as_i64()?;

    println!("Build: {} threads, tracing {}", build.max_threads, build.trace);
    println!("Runtime: {steps} simulation steps");
    Ok(())
}
