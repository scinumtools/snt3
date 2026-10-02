#include "viewer_model.h"
#include "source_view.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void write(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream file(path);
    file << contents;
    if (!file) throw std::runtime_error("Unable to write test artifact");
}

bool has_span(const snt::view::HighlightedLine& line, snt::view::SyntaxKind kind,
              const std::string& text) {
    for (const auto& span : line.spans)
        if (span.kind == kind && span.text.find(text) != std::string::npos) return true;
    return false;
}
} // namespace

int main() {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() / ("snt-view-model-" + suffix + ".dip");
    const auto snapshot_path = std::filesystem::temp_directory_path() /
                               ("snt-view-model-" + suffix + ".diph5");
    const auto inline_dir = std::filesystem::temp_directory_path() / ("snt-view-inline-" + suffix);
    try {
        write(path, "a int = 1\nb int = 2\ngroup\n  boundary[inlet]\n    speed int = 4\n");
        snt::view::ViewerModel model(path);
        check(model.input_path() == path.string(), "Absolute input path was changed for display");
        check(model.display_file_path(path.string()) == path.filename().string(),
              "A declaration in the opened file should show its filename");
        check(model.revision() == 0, "Initial viewer revision is incorrect");
        check(model.object("a") && model.object("b"), "Values are missing from the browser");
        check(!model.object("@overrides"), "Unmodified artifact should not have an Overrides branch");
        check(!model.object("@schemas") && !model.object("@units"),
              "Artifact without registered schemas or units has empty registry branches");
        const auto* collection = model.object("group.boundary");
        const auto* item = model.object("group.boundary[inlet]");
        check(collection && collection->hierarchy_kind == snt::dip::Path::Kind::Map,
              "Map collection is missing from the browser");
        check(item && item->parent == "group.boundary" && item->label == "[inlet]",
              "Collection item is not nested beneath its map");
        check(model.object("group.boundary[inlet].speed") &&
              model.object("group.boundary[inlet].speed")->has_value,
              "Collection value is missing from the browser");
        const auto group_source = model.source_targets(*model.object("group"));
        const auto item_source = model.source_targets(*item);
        check(group_source.size() == 1 && group_source.front().line == 3 &&
              item_source.size() == 1 && item_source.front().line == 4,
              "Explicit groups and collection items need source locations");
        check(model.select("a") && model.select("b"), "Selection failed");
        check(model.back() && model.selection() == "a", "Back navigation failed");
        check(model.can_forward() && model.select("a") && model.can_forward(),
              "Selecting the current node discarded forward history");
        check(model.forward() && model.selection() == "b", "Forward navigation failed");
        check(model.select("group.boundary[inlet].speed") &&
              model.back() && model.selection() == "b" && model.can_forward() &&
              model.forward() && model.selection() == "group.boundary[inlet].speed",
              "Back and forward did not preserve browsing history");
        check(model.back() && model.select("a") && !model.can_forward() &&
              model.back() && model.selection() == "b" &&
              model.forward() && model.selection() == "a",
              "Selecting another node did not replace the forward branch");
        check(model.select("b"), "Selection failed after history navigation");
        model.set_search("a");

        write(path, "a int = 3\nb int = 4\n");
        check(model.reload() && model.selection() == "b" && model.search() == "a",
              "Reload did not preserve viewer state");
        check(model.revision() == 1, "Successful reload did not invalidate inspection data");
        check(model.environment().get_node("a")->value->to_string() == "3", "Reload kept the old value");

        write(path, "a int = invalid\n");
        check(!model.reload() && model.stale(), "Invalid reload should retain the last valid model");
        check(model.revision() == 1, "Failed reload invalidated the last valid inspection data");
        check(model.environment().get_node("a")->value->to_string() == "3", "Invalid reload changed the model");

        const auto example = std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) /
                             "examples/dip/ParameterViewer/DIPfile";
        const auto relative_example = std::filesystem::relative(example);
        snt::view::ViewerModel project(relative_example);
        check(project.object("")->children ==
                  std::vector<std::string>{"@overrides", "@dipfile", "@project", "@schemas", "@sources", "@units"},
              "Browser sections are not in the expected order");
        const auto& entries = project.environment().project_entries();
        check(entries.size() == 7 && entries.front().kind == snt::dip::ProjectEntry::Kind::Unit &&
              entries.front().line == 3 && entries.back().kind == snt::dip::ProjectEntry::Kind::Code &&
              entries.back().line == 22, "DIPfile entries should retain source order and line numbers");
        const auto* source_entry = project.object("@dipfile?Sources?1");
        check(source_entry && source_entry->label == "reference" && source_entry->manifest_index &&
              std::filesystem::path(entries[*source_entry->manifest_index].resolved_path).filename() == "reference.dip",
              "Named source registration is missing from the DIPfile branch");
        const auto registration = project.source_targets(*source_entry);
        check(registration.size() == 1 && registration.front().file.filename() == "DIPfile" &&
              registration.front().line == 7, "DIPfile registration does not open its assignment line");
        check(project.input_path() == relative_example.string() &&
              project.artifact() == std::filesystem::absolute(relative_example),
              "Relative input path was expanded for display");
        const auto provenance = project.environment()["experiment.geometry.length"].get_provenance();
        check(provenance.source && provenance.override_source &&
              project.display_file_path(provenance.source->path) == "parameters.dip" &&
              project.display_file_path(provenance.override_source->path) == "overrides.dip" &&
              project.display_file_path(project.environment().sources.at("reference").path) == "reference.dip",
              "Inspector file paths are not relative to the opened project");
        const auto* combined = project.object("experiment.repeats");
        const auto* materials = project.object("experiment.materials");
        const auto* probes = project.object("experiment.probes");
        check(combined && combined->has_value && !combined->children.empty(),
              "Value with children is missing from the example browser");
        check(materials && materials->hierarchy_kind == snt::dip::Path::Kind::Map,
              "Example map kind is incorrect");
        check(probes && probes->hierarchy_kind == snt::dip::Path::Kind::List,
              "Example list kind is incorrect");
        const auto* override = project.object("@override?experiment.geometry.length");
        check(project.object("@overrides") && override && override->has_value &&
              override->parent == "@overrides" &&
              override->node_path == "experiment.geometry.length" &&
              project.value_node(*override)->override,
              "Overridden value is missing from its separate branch");
        check(project.object("experiment.probes[1].accuracy") &&
              project.object("experiment.probes[1].accuracy")->has_value,
              "Schema value is missing from the example browser");
        const auto* schema = project.object("@schema?probe");
        const auto* unit = project.object("@unit?sample_tick");
        check(project.object("@schemas") && schema &&
              schema->role == snt::view::ObjectRole::Schema && schema->parent == "@schemas" &&
              schema->node_path == "probe", "Registered schema is missing from its branch");
        const auto manifest = project.environment().get_schema_manifest();
        check(manifest.size() == 1 && manifest.front().name == schema->node_path &&
              manifest.front().source &&
              project.display_file_path(manifest.front().source->path) == "probe.dip" &&
              project.environment().get_node("experiment.instrument.model")->schema_id == manifest.front().id,
              "Schema declaration or contributed value cannot be inspected");
        check(project.object("@units") && unit &&
              unit->role == snt::view::ObjectRole::Unit && unit->parent == "@units" &&
              unit->node_path == "sample_tick" &&
              project.environment().units.at(unit->node_path).definition == "0.25*s",
              "Registered custom unit is missing from its branch");
        const auto unit_target = project.source_targets(*unit);
        check(unit_target.size() == 1 && std::filesystem::equivalent(unit_target.front().file, example) &&
              unit_target.front().line == 3, "Custom unit does not lead to its DIPfile line");
        const auto schema_target = project.source_targets(*schema);
        check(schema_target.size() == 2 && schema_target.front().file.filename() == "probe.dip" &&
              schema_target.front().line > 0 && schema_target.back().file.filename() == "DIPfile",
              "Schema needs both its definition and DIPfile registration");
        const auto value_targets = project.source_targets(*project.object("experiment.geometry.length"));
        check(value_targets.size() == 2 &&
              value_targets.front().file.filename() == "overrides.dip" &&
              value_targets.back().file.filename() == "parameters.dip",
              "Overridden parameter needs both its effective and declared source");
        const auto group_targets = project.source_targets(*project.object("experiment.geometry"));
        check(group_targets.size() == 1 &&
              group_targets.front().file.filename() == "parameters.dip" &&
              group_targets.front().line > 0,
              "Declared group does not lead to its source line");
        const auto modified_targets = project.source_targets(*project.object("experiment.target_temperature"));
        check(modified_targets.size() == 2 && modified_targets.front().line == 10 &&
              modified_targets.back().line == 9,
              "Modified parameter needs both its latest change and original declaration");
        const auto source_targets = project.source_targets(*project.object("reference?lab_name"));
        check(source_targets.size() == 1 && source_targets.front().file.filename() == "reference.dip",
              "Named-source value does not lead to its source file");
        const auto source_group = project.source_targets(*project.object("catalog?devices[thermometer]"));
        check(source_group.size() == 1 && source_group.front().file.filename() == "catalog.dip" &&
              source_group.front().line == 2,
              "Named-source collection item does not lead to its declaration");
        snt::view::SourceView source_view;
        std::string source_error;
        check(source_view.open(unit_target.front().file, unit_target.front().line, source_error) &&
              source_error.empty() && source_view.lines().size() >= 3 &&
              has_span(source_view.lines()[2], snt::view::SyntaxKind::String, "0.25*s"),
              "Read-only source viewer did not load or highlight the unit definition");
        const auto fixture = std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) /
                             "docs/dipl/highlight/highlighting-test.dipl";
        check(source_view.open(fixture, 1, source_error) &&
              has_span(source_view.lines()[0], snt::view::SyntaxKind::Comment, "DIPL") &&
              has_span(source_view.lines()[9], snt::view::SyntaxKind::Keyword, "$unit") &&
              has_span(source_view.lines()[9], snt::view::SyntaxKind::Name, "local_length"),
              "DIPL source highlighting lost comment or directive categories");
        const auto find_fixture_line = [&](const std::string& text) {
            return std::find_if(source_view.lines().begin(), source_view.lines().end(),
                [&](const auto& line) { return line.text.find(text) != std::string::npos; });
        };
        const auto scalar = find_fixture_line("length float = 2.5 m");
        const auto reference = find_fixture_line("reference_direct float");
        const auto metadata = find_fixture_line("?descr \"A highlighted");
        const auto multiline = find_fixture_line("value without interpolation and # text");
        const auto formatted = find_fixture_line("formatted str = f\"Length");
        check(scalar != source_view.lines().end() &&
              has_span(*scalar, snt::view::SyntaxKind::Name, "length") &&
              has_span(*scalar, snt::view::SyntaxKind::Type, "float") &&
              has_span(*scalar, snt::view::SyntaxKind::Number, "2.5") &&
              has_span(*scalar, snt::view::SyntaxKind::Unit, "m") &&
              reference != source_view.lines().end() &&
              has_span(*reference, snt::view::SyntaxKind::Reference, "{?length}") &&
              metadata != source_view.lines().end() &&
              has_span(*metadata, snt::view::SyntaxKind::Metadata, "?descr") &&
              multiline != source_view.lines().end() &&
              has_span(*multiline, snt::view::SyntaxKind::String, "# text") &&
              formatted != source_view.lines().end() &&
              has_span(*formatted, snt::view::SyntaxKind::Reference, "{{?length}}"),
              "DIPL highlighting lost a source token category");
        const auto* source = project.object("reference?lab_name");
        check(project.object("@project") && project.object("@sources") &&
              project.object("reference?") && source && source->has_value,
              "Named source is missing from the example browser");
        check(source->parent == "reference?" && source->source_name == "reference" &&
              source->node_path == "lab_name", "Named source path has the wrong identity");
        check(project.value_node(*source)->value->to_string() == "\"North Lab\"",
              "Named source value is unavailable");
        const auto reads = project.environment().dependency_graph().dependencies("?experiment.lab_name");
        check(reads.size() == 1 && reads.front().target == "reference?lab_name" &&
              project.object(reads.front().target), "Named source dependency is not navigable");
        const auto* catalog = project.object("catalog?devices");
        const auto* family = project.object("catalog?devices[thermometer].family");
        check(catalog && catalog->hierarchy_kind == snt::dip::Path::Kind::Map &&
              family && family->has_value, "Second source map is not browsable");
        const auto catalog_reads = project.environment().dependency_graph().dependencies(
            "?experiment.instrument_family");
        check(catalog_reads.size() == 1 &&
              catalog_reads.front().target == "catalog?devices[thermometer].family" &&
              project.object(catalog_reads.front().target), "Second source dependency is not navigable");
        check(project.select("reference?lab_name") && project.reload() &&
              project.selection() == "reference?lab_name" &&
              project.input_path() == relative_example.string(),
              "Reload lost the source selection or original input path");
        check(project.select("@override?experiment.geometry.length") && project.reload() &&
              project.selection() == "@override?experiment.geometry.length",
              "Reload lost the override selection");
        check(project.select("@schema?probe") && project.reload() &&
              project.selection() == "@schema?probe" &&
              project.select("@unit?sample_tick") && project.reload() &&
              project.selection() == "@unit?sample_tick",
              "Reload lost a registered definition selection");
        project.environment().save(snapshot_path);
        snt::view::ViewerModel snapshot(snapshot_path);
        check(snapshot.object("experiment.geometry.length") &&
              !snapshot.object("@dipfile") && snapshot.environment().project_entries().empty() &&
              snapshot.source_targets(*snapshot.object("experiment.geometry.length")).empty() &&
              snapshot.object("@unit?sample_tick") &&
              snapshot.source_targets(*snapshot.object("@unit?sample_tick")).empty(),
              "Source browsing must remain disabled for .diph5 snapshots");
        std::filesystem::create_directory(inline_dir);
        write(inline_dir / "DIPfile", "code[]\n  string = \"\"\"\nanswer int = 7\n\"\"\"\n");
        snt::view::ViewerModel inline_project(inline_dir / "DIPfile");
        check(inline_project.object("answer"), "Embedded project value is missing");
        check(inline_project.environment().project_entries().size() == 1 &&
              inline_project.environment().project_entries().front().resolved_path.empty() &&
              inline_project.object("@dipfile?Code?0"),
              "Embedded code registration is missing from the DIPfile branch");
        const auto inline_target = inline_project.source_targets(*inline_project.object("answer"));
        check(inline_target.size() == 1 && inline_target.front().file == inline_dir / "DIPfile" &&
              inline_target.front().line == 2 &&
              inline_target.front().label == "Embedded source registration",
              "Embedded project code must link to its DIPfile registration");
        std::filesystem::create_directory(inline_dir / "empty");
        write(inline_dir / "empty/DIPfile", "");
        snt::view::ViewerModel empty_project(inline_dir / "empty/DIPfile");
        check(empty_project.object("@dipfile") &&
              empty_project.object("@dipfile")->children.empty() &&
              empty_project.object("")->children.front() == "@dipfile",
              "An empty DIPfile should have an inspectable branch");
        std::filesystem::remove(path);
        std::filesystem::remove(snapshot_path);
        std::filesystem::remove_all(inline_dir);
        return 0;
    } catch (...) {
        std::filesystem::remove(path);
        std::filesystem::remove(snapshot_path);
        std::filesystem::remove_all(inline_dir);
        throw;
    }
}
