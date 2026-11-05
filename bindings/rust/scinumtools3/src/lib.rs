//! Safe, owned access to SNT3 DIPL projects and DIPH5 archives.

mod comparison;
mod node;
mod project;
mod shared;

pub use comparison::{compare_diph5, Comparison, Difference, DifferenceKind};
pub use node::{Node, NodeKind, Provenance, ValueType};
pub use project::Project;
pub use shared::{Error, Result};
