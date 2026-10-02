set(parameter_viewer_inputs
    DIPfile
    catalog.dip
    observations.dip
    overrides.dip
    parameters.dip
    probe.dip
    reference.dip
    samples.txt
)

list(TRANSFORM parameter_viewer_inputs PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

set(parameter_viewer_snapshot "${CMAKE_CURRENT_SOURCE_DIR}/parameters.diph5")
add_custom_command(
    OUTPUT "${parameter_viewer_snapshot}"
    COMMAND $<TARGET_FILE:snt> dip parse
            --project "${CMAKE_CURRENT_SOURCE_DIR}/DIPfile"
            --record-graph
            --relative-source-paths
            --save "${parameter_viewer_snapshot}"
    DEPENDS snt ${parameter_viewer_inputs}
    COMMENT "Updating the ParameterViewer DIPH5 snapshot"
    VERBATIM
)
add_custom_target(parameter-viewer-snapshot ALL DEPENDS "${parameter_viewer_snapshot}")
