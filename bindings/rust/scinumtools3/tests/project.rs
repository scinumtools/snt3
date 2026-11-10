use scinumtools3::{compare_diph5, Project};
use std::{
    fs,
    path::PathBuf,
    time::{SystemTime, UNIX_EPOCH},
};

fn temporary_dir() -> PathBuf {
    let stamp = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let path = std::env::temp_dir().join(format!("snt3-rust-{}-{stamp}", std::process::id()));
    fs::create_dir_all(&path).unwrap();
    path
}

#[test]
fn load_source_and_archive_with_typed_values() {
    let directory = temporary_dir();
    let source = directory.join("parameters.dip");
    let alternate = directory.join("parameters.dipl");
    let archive = directory.join("parameters.diph5");
    fs::write(&source, "simulation\n  steps int = 42\n    ?descr \"step count\"\n  enabled bool = true\n  title str = \"test run\"\n  interval float = 0.5 s\n  samples int[3] = [1, 2, 3]\n  weights float[2] = [1.5, 2.5]\n  flags bool[2] = [true, false]\n  labels str[2] = [\"first\", \"second\"]\n").unwrap();
    fs::copy(&source, &alternate).unwrap();

    let project = Project::load(&source).unwrap();
    assert_eq!(
        Project::load(&alternate)
            .unwrap()
            .node("simulation.steps")
            .unwrap()
            .as_i64()
            .unwrap(),
        42
    );
    assert_eq!(
        project.node("simulation.steps").unwrap().as_i64().unwrap(),
        42
    );
    assert_eq!(
        project
            .node("simulation.steps")
            .unwrap()
            .metadata("descr")
            .unwrap()
            .as_deref(),
        Some("step count")
    );
    assert_eq!(
        project
            .node("simulation.steps")
            .unwrap()
            .provenance()
            .unwrap()
            .source_line,
        2
    );
    assert!(project
        .node("simulation.enabled")
        .unwrap()
        .as_bool()
        .unwrap());
    assert_eq!(
        project
            .node("simulation.title")
            .unwrap()
            .as_string()
            .unwrap(),
        "test run"
    );
    assert_eq!(
        project
            .node("simulation.interval")
            .unwrap()
            .units()
            .unwrap()
            .as_deref(),
        Some("s")
    );
    let samples = project.node("simulation.samples").unwrap();
    assert_eq!(samples.shape().unwrap(), vec![3]);
    assert_eq!(samples.as_i64_array().unwrap(), vec![1, 2, 3]);
    assert_eq!(
        project
            .node("simulation.labels")
            .unwrap()
            .as_string_array()
            .unwrap(),
        vec!["first", "second"]
    );
    assert_eq!(
        project
            .node("simulation.weights")
            .unwrap()
            .as_f64_array()
            .unwrap(),
        vec![1.5, 2.5]
    );
    assert_eq!(
        project
            .node("simulation.flags")
            .unwrap()
            .as_bool_array()
            .unwrap(),
        vec![true, false]
    );
    let simulation = project.node("simulation").unwrap();
    let names: Vec<_> = simulation
        .entries()
        .unwrap()
        .into_iter()
        .map(|(name, _)| name)
        .collect();
    assert!(names.contains(&"steps".to_owned()));

    project.save(&archive).unwrap();
    let restored = Project::load(&archive).unwrap();
    assert_eq!(
        restored.node("simulation.steps").unwrap().as_i64().unwrap(),
        42
    );
    assert_eq!(
        restored
            .node("simulation.samples")
            .unwrap()
            .as_i64_array()
            .unwrap(),
        vec![1, 2, 3]
    );
    assert_eq!(compare_diph5(&archive, &archive).unwrap().changed, 0);
    let changed_archive = directory.join("changed.diph5");
    let mut changed = Project::new().unwrap();
    changed.add_file(&source).unwrap();
    changed.add_override("simulation.steps = 43\n").unwrap();
    changed.parse().unwrap();
    changed.save(&changed_archive).unwrap();
    let comparison = compare_diph5(&archive, &changed_archive).unwrap();
    assert_eq!(comparison.changed, 1);
    assert_eq!(comparison.differences[0].path, "simulation.steps");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn overrides_and_errors() {
    let mut project = Project::new().unwrap();
    project.add_source("answer int = 1\n").unwrap();
    project.add_override("answer = 2\n").unwrap();
    assert!(project.node("answer").is_err());
    project.parse().unwrap();
    let answer = project.node("answer").unwrap();
    assert_eq!(answer.as_i64().unwrap(), 2);
    assert!(answer.is_overridden().unwrap());
    assert!(!answer.provenance().unwrap().override_code.is_empty());
    assert!(project.node("missing").is_err());
    drop(answer);
    assert!(project.add_override("answer = 3\n").is_err());
}

#[test]
fn load_dipfile_project_and_reject_invalid_input() {
    let directory = temporary_dir();
    fs::write(directory.join("DIPfile"), "code[]\n  file = \"main.dip\"\n").unwrap();
    fs::write(directory.join("main.dip"), "answer int = 7\n").unwrap();
    let project = Project::load(&directory).unwrap();
    assert_eq!(project.node("answer").unwrap().as_i64().unwrap(), 7);
    let sources = project.source_files().unwrap();
    assert!(!sources.is_empty());
    assert!(sources.iter().any(|p| p.ends_with("main.dip")));
    assert!(Project::load(directory.join("missing.txt")).is_err());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn traverse_named_collection_items() {
    let mut project = Project::new().unwrap();
    project
        .add_source("samples[first]\n  count int = 3\nsamples[second]\n  count int = 5\n")
        .unwrap();
    project.parse().unwrap();
    let samples = project.node("samples").unwrap();
    let entries = samples.entries().unwrap();
    let names: Vec<_> = entries.iter().map(|(name, _)| name.as_str()).collect();
    assert_eq!(names, vec!["first", "second"]);
    assert_eq!(entries[1].1.path().unwrap(), "samples[second]");
    assert_eq!(
        project
            .node("samples[second].count")
            .unwrap()
            .as_i64()
            .unwrap(),
        5
    );
}
