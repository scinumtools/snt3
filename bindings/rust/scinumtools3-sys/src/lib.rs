//! Raw declarations for the supported part of the SNT3 C ABI.
#![allow(non_camel_case_types)]

use std::ffi::{c_char, c_int};

#[repr(C)]
pub struct snt_dip {
    _private: [u8; 0],
}
#[repr(C)]
pub struct snt_dip_node {
    _private: [u8; 0],
}
#[repr(C)]
pub struct snt_dip_comparison {
    _private: [u8; 0],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct snt_dip_error {
    pub code: c_int,
    pub message: *const c_char,
}
impl Default for snt_dip_error {
    fn default() -> Self {
        Self {
            code: 0,
            message: std::ptr::null(),
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct snt_dip_difference {
    pub path: *const c_char,
    pub category: *const c_char,
    pub kind: c_int,
    pub fields: *const c_char,
    pub before: *const c_char,
    pub after: *const c_char,
    pub changed_elements: usize,
    pub example_indices: *const usize,
    pub example_count: usize,
}
impl Default for snt_dip_difference {
    fn default() -> Self {
        unsafe { std::mem::zeroed() }
    }
}

extern "C" {
    pub fn snt_dip_parser_create(out: *mut *mut snt_dip, error: *mut snt_dip_error) -> c_int;
    pub fn snt_dip_parser_free(dip: *mut snt_dip);
    pub fn snt_dip_parser_add_string(
        dip: *mut snt_dip,
        source: *const c_char,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_parser_add_file(
        dip: *mut snt_dip,
        path: *const c_char,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_parser_add_project(
        dip: *mut snt_dip,
        path: *const c_char,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_parser_add_schema_string(
        dip: *mut snt_dip,
        name: *const c_char,
        source: *const c_char,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_parser_add_override_string(
        dip: *mut snt_dip,
        source: *const c_char,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_parser_parse(dip: *mut snt_dip, error: *mut snt_dip_error) -> c_int;
    pub fn snt_dip_environment_load(
        dip: *mut snt_dip,
        path: *const c_char,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_environment_save(
        dip: *mut snt_dip,
        path: *const c_char,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_environment_get_source_count(
        dip: *const snt_dip,
        result: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_environment_get_source_path(
        dip: *const snt_dip,
        index: usize,
        buffer: *mut c_char,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_environment_get_source_name(
        dip: *const snt_dip,
        index: usize,
        buffer: *mut c_char,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_environment_get_node(
        dip: *const snt_dip,
        path: *const c_char,
        out: *mut *mut snt_dip_node,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_free(node: *mut snt_dip_node);
    pub fn snt_dip_node_get_size(
        node: *const snt_dip_node,
        out: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_kind(
        node: *const snt_dip_node,
        out: *mut c_int,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_path(
        node: *const snt_dip_node,
        buffer: *mut c_char,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_entry(
        node: *const snt_dip_node,
        index: usize,
        name: *mut c_char,
        capacity: usize,
        required: *mut usize,
        out: *mut *mut snt_dip_node,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_type(
        node: *const snt_dip_node,
        out: *mut c_int,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_shape(
        node: *const snt_dip_node,
        dimensions: *mut usize,
        capacity: usize,
        rank: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_value_size(
        node: *const snt_dip_node,
        out: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_copy_bool_array(
        node: *const snt_dip_node,
        values: *mut c_int,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_copy_int64_array(
        node: *const snt_dip_node,
        values: *mut i64,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_copy_uint64_array(
        node: *const snt_dip_node,
        values: *mut u64,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_copy_double_array(
        node: *const snt_dip_node,
        values: *mut f64,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_copy_string_array(
        node: *const snt_dip_node,
        offsets: *mut usize,
        offsets_capacity: usize,
        bytes: *mut c_char,
        bytes_capacity: usize,
        count: *mut usize,
        bytes_required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_bool(
        node: *const snt_dip_node,
        out: *mut c_int,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_int64(
        node: *const snt_dip_node,
        out: *mut i64,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_uint64(
        node: *const snt_dip_node,
        out: *mut u64,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_double(
        node: *const snt_dip_node,
        out: *mut f64,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_string(
        node: *const snt_dip_node,
        buffer: *mut c_char,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_has_units(
        node: *const snt_dip_node,
        out: *mut c_int,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_units(
        node: *const snt_dip_node,
        buffer: *mut c_char,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_provenance(
        node: *const snt_dip_node,
        source_name: *mut c_char,
        source_name_capacity: usize,
        source_name_required: *mut usize,
        source_line: *mut usize,
        source_code: *mut c_char,
        source_code_capacity: usize,
        source_code_required: *mut usize,
        override_source_name: *mut c_char,
        override_source_name_capacity: usize,
        override_source_name_required: *mut usize,
        override_line: *mut usize,
        override_code: *mut c_char,
        override_code_capacity: usize,
        override_code_required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_is_overridden(
        node: *const snt_dip_node,
        out: *mut c_int,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_has_metadata(
        node: *const snt_dip_node,
        name: *const c_char,
        out: *mut c_int,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_node_get_metadata(
        node: *const snt_dip_node,
        name: *const c_char,
        buffer: *mut c_char,
        capacity: usize,
        required: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_compare_files(
        before: *const c_char,
        after: *const c_char,
        scope: c_int,
        max_examples: usize,
        out: *mut *mut snt_dip_comparison,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_comparison_free(result: *mut snt_dip_comparison);
    pub fn snt_dip_comparison_summary(
        result: *const snt_dip_comparison,
        added: *mut usize,
        removed: *mut usize,
        changed: *mut usize,
        error: *mut snt_dip_error,
    ) -> c_int;
    pub fn snt_dip_comparison_count(result: *const snt_dip_comparison) -> usize;
    pub fn snt_dip_comparison_get(
        result: *const snt_dip_comparison,
        index: usize,
        out: *mut snt_dip_difference,
        error: *mut snt_dip_error,
    ) -> c_int;
}
