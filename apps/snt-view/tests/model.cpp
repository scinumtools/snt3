#include "data_page.h"
#include "source_view.h"
#include "viewer_model.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <snt/val/values_array.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    void check(bool condition, const char* message) {
        if (!condition)
            throw std::runtime_error(message);
    }

    void check(bool condition, const std::string& message) {
        if (!condition)
            throw std::runtime_error(message);
    }

    void write(const std::filesystem::path& path, const std::string& contents) {
        std::ofstream file(path);
        file << contents;
        if (!file)
            throw std::runtime_error("Unable to write test artifact");
    }

    bool has_span(const snt::view::HighlightedLine& line, snt::view::SyntaxKind kind, const std::string& text) {
        for (const auto& span : line.spans)
            if (span.kind == kind && span.text.find(text) != std::string::npos)
                return true;
        return false;
    }
} // namespace

int main() {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() / ("snt-view-model-" + suffix + ".dip");
    const auto snapshot_path = std::filesystem::temp_directory_path() / ("snt-view-model-" + suffix + ".diph5");
    const auto inline_dir = std::filesystem::temp_directory_path() / ("snt-view-inline-" + suffix);
    try {
        const auto standalone_dir = inline_dir / "standalone";
        std::filesystem::create_directories(standalone_dir);
        const auto standalone_file = standalone_dir / "test.dip";
        write(standalone_file, "value int = 1\n");
        snt::view::ViewerModel standalone(standalone_file);
        const auto* standalone_source = standalone.object("@code?0");
        const auto* standalone_files = standalone.object("@files");
        const auto* standalone_file_entry = standalone.object("@project-file?test.dip");
        check(standalone_files && standalone_file_entry && standalone_file_entry->parent == "@files" &&
                  standalone.object("")->children.front() == "@files" && standalone_file_entry->children.empty() &&
                  standalone.project_files().size() == 1 && standalone.project_files().front().links.size() == 1 &&
                  standalone.project_files().front().size_bytes == 14 &&
                  standalone.project_files().front().modified_at.has_value() &&
                  standalone.source_targets(*standalone_file_entry).size() == 1 &&
                  standalone.source_targets(*standalone_file_entry).front().file == standalone_file &&
                  standalone.source_object_path(standalone.project_files().front().links.front()) == "@code?0" &&
                  standalone.select(standalone_file_entry->path),
              "Standalone .dip files should appear in Files and link to their source in the inspector");
        check(standalone.object("@local") && standalone_source && standalone_source->parent == "@local" &&
                  standalone_source->label == "test.dip" && standalone_source->source_kind == "code",
              "A standalone DIPL file should appear under Local sources");
        const auto standalone_targets = standalone.source_targets(*standalone_source);
        check(standalone_targets.size() == 1 && standalone_targets.front().file == standalone_file &&
                  standalone.source_object_path(standalone.source_targets(*standalone.object("value")).front()) ==
                      "@code?0" &&
                  standalone.editable_document_path(standalone_file) == "test.dip" && !standalone.can_save_as(),
              "Standalone source links should open the editable file");
        check(standalone.preview_document("test.dip", "value int = 2\n").candidate_valid &&
                  !standalone.preview_document("test.dip", "value int =\n").candidate_valid &&
                  standalone.save_document("test.dip", "value int = 2\n") &&
                  standalone.environment()["value"].as<int>() == 2,
              "A standalone source should check, save, and reload edited values");
        write(standalone_file, "value int = 3\n");
        check(!standalone.save_document("test.dip", "value int = 4\n") &&
                  standalone.environment()["value"].as<int>() == 2,
              "Saving must not overwrite a standalone file changed outside the editor");
        const auto dipl_file = standalone_dir / "test.dipl";
        write(dipl_file, "answer int = 5\n");
        snt::view::ViewerModel dipl(dipl_file);
        check(dipl.object("@code?0") && dipl.object("@project-file?test.dipl") &&
                  dipl.project_files().size() == 1 &&
                  dipl.source_object_path(dipl.project_files().front().links.front()) == "@code?0" &&
                  dipl.editable_document_path(dipl_file) == "test.dipl" &&
                  dipl.save_document("test.dipl", "answer int = 6\n") && dipl.environment()["answer"].as<int>() == 6,
              "Standalone .dipl files should support the same editing workflow");
        write(standalone_dir / "child.dip", "base int = 3\n");
        const auto referencing_file = standalone_dir / "referencing.dip";
        write(referencing_file, "$source lookup = \"child.dip\"\nvalue int = {lookup?base}\n");
        snt::view::ViewerModel referencing(referencing_file);
        check(referencing.preview_document(
                  "referencing.dip", "$source lookup = \"child.dip\"\nvalue int = ({lookup?base} + 1)\n"
              ).candidate_valid,
              "Checking a standalone edit should resolve sources relative to the original file");
        const auto convertible_file = standalone_dir / "convert.dip";
        write(convertible_file, "converted int = 9\n");
        snt::view::ViewerModel convertible(convertible_file);
        check(convertible.can_convert_to_project() && convertible.convert_to_project() &&
                  convertible.artifact() == standalone_dir / "DIPfile" && convertible.can_save_as() &&
                  !convertible.can_convert_to_project() && convertible.object("@dipfile-source") &&
                  convertible.environment()["converted"].as<int>() == 9 &&
                  convertible.editable_document_path(convertible_file) == "convert.dip",
              "Converting a standalone file should open its new editable project");
        const auto nested_dir = inline_dir / "nested-project";
        std::filesystem::create_directories(nested_dir / "config");
        write(nested_dir / "DIPfile", "code[]\n  file = \"config/settings.dip\"\n");
        write(nested_dir / "config/settings.dip", "setting int = 4\n");
        snt::view::ViewerModel nested_project(nested_dir / "DIPfile");
        const auto* folder = nested_project.object("@folder?config");
        const auto* nested_file = nested_project.object("@project-file?config/settings.dip");
        check(folder && folder->parent == "@files" && nested_file && nested_file->parent == folder->path &&
                  nested_project.project_file_target(*nested_file).has_value() &&
                  !nested_project.source_object_path(*nested_project.project_file_target(*nested_file)).empty(),
              "Project files should retain folder structure and open their source entries");
        check(!dipl.convert_to_project() && dipl.can_convert_to_project() &&
                  dipl.artifact() == dipl_file && dipl.environment()["answer"].as<int>() == 6,
              "Conversion must preserve a standalone viewer when a DIPfile already exists");
        write(path, "a int = 1\nb int = 2\ngroup\n  boundary[inlet]\n    speed int = 4\n");
        snt::view::ViewerModel model(path);
        check(!model.can_export_diph5(), "Standalone source files should not expose project snapshot export");
        check(model.input_path() == path.string(), "Absolute input path was changed for display");
        check(
            model.display_file_path(path.string()) == path.filename().string(),
            "A declaration in the opened file should show its filename"
        );
        check(model.revision() == 0, "Initial viewer revision is incorrect");
        check(model.object("a") && model.object("b"), "Values are missing from the browser");
        check(!model.object("@overrides"), "Unmodified artifact should not have an Overrides branch");
        check(
            !model.object("@schemas") && !model.object("@units"),
            "Artifact without registered schemas or units has empty registry branches"
        );
        const auto* collection = model.object("group.boundary");
        const auto* item = model.object("group.boundary[inlet]");
        check(
            collection && collection->hierarchy_kind == snt::dip::Path::Kind::Map,
            "Map collection is missing from the browser"
        );
        check(
            item && item->parent == "group.boundary" && item->label == "[inlet]",
            "Collection item is not nested beneath its map"
        );
        check(
            model.object("group.boundary[inlet].speed") && model.object("group.boundary[inlet].speed")->has_value,
            "Collection value is missing from the browser"
        );
        const auto group_source = model.source_targets(*model.object("group"));
        const auto item_source = model.source_targets(*item);
        check(
            group_source.size() == 1 && group_source.front().line == 3 && item_source.size() == 1 &&
                item_source.front().line == 4,
            "Explicit groups and collection items need source locations"
        );
        check(model.select("a") && model.select("b"), "Selection failed");
        check(model.back() && model.selection() == "a", "Back navigation failed");
        check(
            model.can_forward() && model.select("a") && model.can_forward(),
            "Selecting the current node discarded forward history"
        );
        check(model.forward() && model.selection() == "b", "Forward navigation failed");
        check(
            model.select("group.boundary[inlet].speed") && model.back() && model.selection() == "b" &&
                model.can_forward() && model.forward() && model.selection() == "group.boundary[inlet].speed",
            "Back and forward did not preserve browsing history"
        );
        check(
            model.back() && model.select("a") && !model.can_forward() && model.back() && model.selection() == "b" &&
                model.forward() && model.selection() == "a",
            "Selecting another node did not replace the forward branch"
        );
        check(model.select("b"), "Selection failed after history navigation");
        model.set_search("a");

        write(path, "a int = 3\nb int = 4\n");
        check(
            model.reload() && model.selection() == "b" && model.search() == "a", "Reload did not preserve viewer state"
        );
        check(model.revision() == 1, "Successful reload did not invalidate inspection data");
        check(model.environment().get_node("a")->value->to_string() == "3", "Reload kept the old value");

        write(path, "a int = invalid\n");
        check(!model.reload() && model.stale(), "Invalid reload should retain the last valid model");
        check(model.revision() == 1, "Failed reload invalidated the last valid inspection data");
        check(model.environment().get_node("a")->value->to_string() == "3", "Invalid reload changed the model");

        const auto example = std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) / "examples/dip/ParameterViewer/DIPfile";
        const auto relative_example = std::filesystem::relative(example);
        snt::view::ViewerModel project(relative_example);
        const snt::dip::Inspector view{project.environment()};
        const auto cube = view.value_summary("experiment.response_cube");
        check(
            cube.shape == (snt::val::Array::ShapeType{2, 2, 3}) && cube.elements == 12,
            "The example cube has the wrong inspection shape"
        );
        auto cube_page = view.value_slice("experiment.response_cube", {{1, 1}, {0, 1}, {1, 2}});
        const auto* cube_values = dynamic_cast<const snt::val::ArrayValue<double>*>(cube_page.get());
        check(
            cube_values && cube_values->get_size() == 4 && cube_values->get_value(0) == 2.1 &&
                cube_values->get_value(3) == 2.5,
            "The example cube cannot be read as a bounded slice"
        );
        check(
            cube_page->slice({{1, 1}, {1, 1}})->to_string() == "2.5",
            "A displayed array cell was not formatted from its bounded page"
        );
        const auto cube_cells = snt::view::read_array_page(
            project.environment(), "experiment.response_cube", cube.shape, {0, 0, 0}, 1, 2, 1, 1, 24, 8
        );
        check(
            cube_cells.size() == 1 && cube_cells[0].size() == 2 && cube_cells[0][0] == "1.4" &&
                cube_cells[0][1] == "1.5",
            "The cube page does not display the expected values with a fixed axis"
        );
        const auto transposed_cells = snt::view::read_array_page(
            project.environment(), "experiment.response_cube", cube.shape, {0, 1, 0}, 2, 0, 1, 0, 2, 2
        );
        check(
            transposed_cells.size() == 2 && transposed_cells[0].size() == 2 && transposed_cells[0][0] == "1.4" &&
                transposed_cells[0][1] == "2.4" && transposed_cells[1][0] == "1.5" && transposed_cells[1][1] == "2.5",
            "The cube page does not follow its selected row and column axes"
        );
        const auto series = view.value_summary("experiment.sample_series");
        const auto series_cells = snt::view::read_array_page(
            project.environment(), "experiment.sample_series", series.shape, {0}, 0, 0, 24, 0, 24, 8
        );
        check(
            series_cells.size() == 12 && series_cells.front()[0] == "24" && series_cells.back()[0] == "35",
            "The series does not display its final page"
        );
        const auto named_curve = view.value_summary("reference?calibration_curve");
        auto named_page = view.value_slice("reference?calibration_curve", {{1, 2}});
        const auto* named_values = dynamic_cast<const snt::val::ArrayValue<double>*>(named_page.get());
        check(
            named_curve.shape == (snt::val::Array::ShapeType{4}) && named_values && named_values->get_value(0) == 1.0 &&
                named_values->get_value(1) == 1.02,
            "Named-source arrays cannot be inspected through the bounded slice API"
        );
        const auto named_cells = snt::view::read_array_page(
            project.environment(), "reference?calibration_curve", named_curve.shape, {0}, 0, 0, 1, 0, 2, 1
        );
        check(
            named_cells.size() == 2 && named_cells[0][0] == "1" && named_cells[1][0] == "1.02",
            "The named-source array page shows incorrect values"
        );
        const auto table = view.table("experiment.readings");
        check(
            project.object("experiment.readings") && project.object("experiment.readings")->has_table,
            "The browser does not identify the example table"
        );
        for (const auto& path :
             {"experiment.readings",
              "experiment.readings.time",
              "experiment.reference_readings",
              "experiment.reference_readings.time"}) {
            const auto* object = project.object(path);
            const auto sources = object ? project.source_targets(*object) : std::vector<snt::view::SourceTarget>{};
            check(
                object && !sources.empty() && sources.front().file.filename() == "observations.dip",
                "A table or column does not lead to its DIPL declaration"
            );
        }
        check(
            table.rows == 4 && table.columns.size() == 3 && table.columns.front().name == "time" &&
                table.columns.back().name == "pressure",
            "The example table lost its rows or column order"
        );
        const auto table_cells = snt::view::read_table_page(project.environment(), table, 1, 0, 2, 3);
        check(
            table_cells.size() == 2 && table_cells[0].size() == 3 && table_cells[0][0] == "1" &&
                table_cells[1][0] == "2",
            "The table page does not follow its row offset and column order"
        );
        const auto extended = view.table("experiment.extended_readings");
        check(extended.rows == 48 && extended.columns.size() == 8, "The example's large table has the wrong shape");
        const auto extended_page = snt::view::read_table_page(project.environment(), extended, 32, 0, 32, 8);
        check(
            extended_page.size() == 16 && extended_page.front()[0] == "32" && extended_page.back()[0] == "47" &&
                extended_page.front().size() == 8,
            "The example's large table cannot be read on its second page"
        );
        check(
            view.capabilities("experiment.duration").hasReferenceGraph &&
                view.capabilities("reference?lab_name").hasReferenceGraph &&
                view.capabilities("reference?calibration_curve").hasArrayData,
            "Graph or Data capability is missing from the example browser"
        );
        check(
            project.object("@project")->label == "Resolved nodes" &&
                project.object("@dipfile")->label == "Project" &&
                project.object("@overrides")->label == "Overrides",
            "Resolved and overridden branch labels are incorrect"
        );
        check(
            project.object("")->children ==
                std::vector<std::string>{
                    "@files",
                    "@dipfile",
                    "@overrides",
                    "@project",
                    "@named-nodes",
                    "@schemas",
                    "@units",
                    "@local",
                    "@sources",
                    "@blocks",
                    "@raw_sources"
                },
            "Browser sections are not in the expected order"
        );
        const auto& entries = project.environment().project_entries();
        const auto* files_section = project.object("@files");
        const auto* source_folder = project.object("@folder?sources");
        const auto* schema_folder = project.object("@folder?schemas");
        const auto* code_folder = project.object("@folder?code");
        const auto* file_entry = project.object("@project-file?sources/reference.dip");
        check(files_section && source_folder && schema_folder && code_folder &&
                  source_folder->parent == "@files" && schema_folder->parent == "@files" &&
                  code_folder->parent == "@files" &&
                  std::find(files_section->children.begin(), files_section->children.end(),
                            "@project-file?sources/reference.dip") == files_section->children.end() &&
                  file_entry && file_entry->parent == source_folder->path &&
                  project.source_targets(*file_entry).size() == 1 &&
                  project.source_targets(*file_entry).front().file.filename() == "reference.dip" &&
                  std::find(source_folder->children.begin(), source_folder->children.end(),
                            "@project-file?sources/catalog.dip") != source_folder->children.end() &&
                  std::find(code_folder->children.begin(), code_folder->children.end(),
                            "@project-file?code/observations.dip") != code_folder->children.end() &&
                  project.project_file_target(*file_entry).has_value() &&
                  project.source_object_path(*project.project_file_target(*file_entry)) == "reference?",
              "Project file tree should group nested files and link them to their source entries");
        const auto& project_files = project.project_files();
        check(!project_files.empty() && project_files.front().path == "DIPfile",
              "Files tab should include the project manifest first");
        for (const auto& file : project_files) {
            check(!file.links.empty(), "Every loaded project file needs a source link: " + file.path);
            for (const auto& link : file.links)
                check(!project.source_object_path(link).empty(),
                      "Loaded project file link has no source entry: " + file.path);
        }
        const auto has_project_file = [&](const std::string& path) {
            return std::any_of(project_files.begin(), project_files.end(), [&](const auto& file) {
                return file.path == path;
            });
        };
        check(has_project_file("sources/reference.dip") && has_project_file("sources/catalog.dip") &&
                  has_project_file("schemas/probe.dip") && has_project_file("code/observations.dip") &&
                  has_project_file("samples.txt") && has_project_file("parameters.dip") &&
                  has_project_file("table_samples.dipt"),
              "Files tab should include named, schema, raw, table, and code sources");
        const auto* table_file = project.object("@project-file?table_samples.dipt");
        check(table_file && project.source_targets(*table_file).size() == 1 &&
                  project.source_targets(*table_file).front().format == snt::view::SourceFormat::Table &&
                  project.editable_document_path(project.source_targets(*table_file).front().file) ==
                      "table_samples.dipt",
              "Table files should open the table source in the shared editor");
        check(
            entries.size() == 10 && entries.front().kind == snt::dip::ProjectEntry::Kind::Unit &&
                entries.front().line == 3 && entries.back().kind == snt::dip::ProjectEntry::Kind::Source &&
                entries.back().line == 32,
            "DIPfile entries should retain source order and line numbers"
        );
        const auto* source_entry = project.object("@dipfile?Sources?1");
        check(
            source_entry && source_entry->label == "reference" && source_entry->manifest_index &&
                std::filesystem::path(entries[*source_entry->manifest_index].resolved_path).filename() ==
                    "reference.dip",
            "Named source registration is missing from the DIPfile branch"
        );
        const auto registration = project.source_targets(*source_entry);
        check(
            registration.size() == 1 && registration.front().file.filename() == "DIPfile" &&
                registration.front().line == 7,
            "DIPfile registration does not open its assignment line"
        );
        const auto* code_source = project.object("@code?5");
        const auto* second_code_source = project.object("@code?6");
        check(
            code_source && code_source->parent == "@local" && code_source->label == "parameters.dip" &&
                second_code_source && second_code_source->label == "code/observations.dip",
            "Project code files are missing from Local sources"
        );
        const auto code_targets = project.source_targets(*code_source);
        check(
            code_targets.size() == 1 && code_targets.front().file.filename() == "parameters.dip" &&
                code_targets.front().line == 1,
            "Project code file does not open at its first line"
        );
        check(
            project.input_path() == relative_example.string() &&
                project.artifact() == std::filesystem::absolute(relative_example),
            "Relative input path was expanded for display"
        );
        const auto provenance = project.environment()["experiment.geometry.length"].get_provenance();
        check(
            provenance.source && provenance.override_source &&
                project.display_file_path(provenance.source->path) == "parameters.dip" &&
                project.display_file_path(provenance.override_source->path) == "overrides.dip" &&
                project.display_file_path(project.environment().sources.at("reference").path) ==
                    "sources/reference.dip",
            "Inspector file paths are not relative to the opened project"
        );
        const auto* combined = project.object("experiment.repeats");
        const auto* materials = project.object("experiment.materials");
        const auto* probes = project.object("experiment.probes");
        check(
            combined && combined->has_value && !combined->children.empty(),
            "Value with children is missing from the example browser"
        );
        check(materials && materials->hierarchy_kind == snt::dip::Path::Kind::Map, "Example map kind is incorrect");
        check(probes && probes->hierarchy_kind == snt::dip::Path::Kind::List, "Example list kind is incorrect");
        const auto* optional_note = project.object("experiment.optional_note");
        check(optional_note && !optional_note->has_value &&
                  optional_note->node_dtype == snt::dip::NodeDtype::String &&
                  optional_note->value_origin == snt::dip::ValueOrigin::None &&
                  optional_note->children.empty(),
              "Explicit none values should be distinguished from groups in the browser");
        const auto* override = project.object("@override?experiment.geometry.length");
        check(
            project.object("@overrides") && override && override->has_value && override->parent == "@overrides" &&
                override->node_path == "experiment.geometry.length" && project.value_node(*override)->override,
            "Overridden node is missing from its separate branch"
        );
        check(
            project.object("experiment.probes[1].accuracy") &&
                project.object("experiment.probes[1].accuracy")->has_value,
            "Schema value is missing from the example browser"
        );
        const auto* schema = project.object("@schema?probe");
        const auto* unit = project.object("@unit?sample_tick");
        check(
            project.object("@schemas") && schema && schema->role == snt::view::ObjectRole::Schema &&
                schema->parent == "@schemas" && schema->node_path == "probe",
            "Registered schema is missing from its branch"
        );
        const auto manifest = project.environment().get_schema_manifest();
        check(
            manifest.size() == 1 && manifest.front().name == schema->node_path && manifest.front().source &&
                project.display_file_path(manifest.front().source->path) == "schemas/probe.dip" &&
                project.environment().get_node("experiment.instrument.model")->schema_id == manifest.front().id,
            "Schema declaration or contributed value cannot be inspected"
        );
        check(
            project.object("@units") && unit && unit->role == snt::view::ObjectRole::Unit && unit->parent == "@units" &&
                unit->node_path == "sample_tick" &&
                project.environment().units.at(unit->node_path).definition == "0.25*s",
            "Registered custom unit is missing from its branch"
        );
        const auto unit_target = project.source_targets(*unit);
        check(
            unit_target.size() == 1 && std::filesystem::equivalent(unit_target.front().file, example) &&
                unit_target.front().line == 3,
            "Custom unit does not lead to its DIPfile line"
        );
        const auto schema_target = project.source_targets(*schema);
        check(
            schema_target.size() == 2 && schema_target.front().file.filename() == "probe.dip" &&
                schema_target.front().line > 0 && schema_target.back().file.filename() == "DIPfile",
            "Schema needs both its definition and DIPfile registration"
        );
        const auto value_targets = project.source_targets(*project.object("experiment.geometry.length"));
        check(
            value_targets.size() == 2 && value_targets.front().file.filename() == "overrides.dip" &&
                value_targets.back().file.filename() == "parameters.dip",
            "Overridden parameter needs both its effective and declared source"
        );
        const auto group_targets = project.source_targets(*project.object("experiment.geometry"));
        check(
            group_targets.size() == 1 && group_targets.front().file.filename() == "parameters.dip" &&
                group_targets.front().line > 0,
            "Declared group does not lead to its source line"
        );
        const auto modified_targets = project.source_targets(*project.object("experiment.target_temperature"));
        check(
            modified_targets.size() == 2 && modified_targets.front().line == 17 && modified_targets.back().line == 15,
            "Modified parameter needs both its latest change and original declaration"
        );
        const auto source_targets = project.source_targets(
            *project.object(project.named_source_node_path("reference", "lab_name"))
        );
        check(
            source_targets.size() == 1 && source_targets.front().file.filename() == "reference.dip",
            "Named-source value does not lead to its source file"
        );
        const auto source_group = project.source_targets(
            *project.object(project.named_source_node_path("catalog", "devices[thermometer]"))
        );
        check(
            source_group.size() == 1 && source_group.front().file.filename() == "catalog.dip" &&
                source_group.front().line == 2,
            "Named-source collection item does not lead to its declaration"
        );
        snt::view::SourceView source_view;
        std::string source_error;
        check(
            source_view.open(unit_target.front().file, unit_target.front().line, source_error) &&
                source_error.empty() && source_view.lines().size() >= 3 &&
                has_span(source_view.lines()[2], snt::view::SyntaxKind::String, "0.25*s"),
            "Read-only source viewer did not load or highlight the unit definition"
        );
        const auto fixture =
            std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) / "docs/dipl/highlight/highlighting-test.dipl";
        check(source_view.open(fixture, 1, source_error), "DIPL highlighting fixture could not be opened");
        const auto find_fixture_line = [&](const std::string& text) {
            return std::find_if(source_view.lines().begin(), source_view.lines().end(), [&](const auto& line) {
                return line.text.find(text) != std::string::npos;
            });
        };
        const auto unit_directive = find_fixture_line("$unit local_length");
        check(
            has_span(source_view.lines()[0], snt::view::SyntaxKind::Comment, "DIPL") &&
                unit_directive != source_view.lines().end() &&
                has_span(*unit_directive, snt::view::SyntaxKind::Keyword, "$unit") &&
                has_span(*unit_directive, snt::view::SyntaxKind::Name, "local_length"),
            "DIPL source highlighting lost comment or directive categories"
        );
        const auto scalar = find_fixture_line("length float = 2.5 m");
        const auto reference = find_fixture_line("reference_direct float");
        const auto metadata = find_fixture_line("?descr \"A highlighted");
        const auto multiline = find_fixture_line("value without interpolation and # text");
        const auto formatted = find_fixture_line("formatted str = f\"Length");
        check(
            scalar != source_view.lines().end() && has_span(*scalar, snt::view::SyntaxKind::Name, "length") &&
                has_span(*scalar, snt::view::SyntaxKind::Type, "float") &&
                has_span(*scalar, snt::view::SyntaxKind::Number, "2.5") &&
                has_span(*scalar, snt::view::SyntaxKind::Unit, "m") && reference != source_view.lines().end() &&
                has_span(*reference, snt::view::SyntaxKind::Reference, "{?length}") &&
                metadata != source_view.lines().end() &&
                has_span(*metadata, snt::view::SyntaxKind::Metadata, "?descr") &&
                multiline != source_view.lines().end() &&
                has_span(*multiline, snt::view::SyntaxKind::String, "# text") &&
                formatted != source_view.lines().end() &&
                has_span(*formatted, snt::view::SyntaxKind::Reference, "{{?length}}"),
            "DIPL highlighting lost a source token category"
        );
        const auto source_id = project.named_source_node_path("reference", "lab_name");
        const auto* source = project.object(source_id);
        check(
            project.object("@project") && project.object("@sources") && project.object("reference?") && source &&
                source->has_value,
            "Named source is missing from the example browser"
        );
        check(
            source->parent == "@named-nodes?reference" && source->source_name == "reference" &&
                source->node_path == "lab_name",
            "Named source path has the wrong identity"
        );
        check(project.value_node(*source)->value->to_string() == "\"North Lab\"", "Named source value is unavailable");
        const auto reads = view.graph().dependencies("?experiment.lab_name");
        check(
            reads.size() == 1 && reads.front().target == "reference?lab_name" && project.object(source_id),
            "Named source dependency is not navigable"
        );
        const auto* catalog = project.object(project.named_source_node_path("catalog", "devices"));
        const auto* family = project.object(project.named_source_node_path("catalog", "devices[thermometer].family"));
        check(
            catalog && catalog->hierarchy_kind == snt::dip::Path::Kind::Map && family && family->has_value,
            "Second source map is not browsable"
        );
        const auto catalog_reads = view.graph().dependencies("?experiment.instrument_family");
        check(
            catalog_reads.size() == 1 && catalog_reads.front().target == "catalog?devices[thermometer].family" &&
                project.object(project.named_source_node_path("catalog", "devices[thermometer].family")),
            "Second source dependency is not navigable"
        );
        const auto* raw_source = project.object("raw_samples?");
        check(
            raw_source && raw_source->children.empty() && project.environment().sources.at("raw_samples").raw_text,
            "Raw source is missing from the browser"
        );
        const auto raw_targets = project.source_targets(*raw_source);
        check(
            raw_targets.size() == 1 && raw_targets.front().format == snt::view::SourceFormat::Plain &&
                raw_targets.front().file.filename() == "samples.txt",
            "Raw source does not open as plain text"
        );
        snt::view::SourceView raw_view;
        std::string raw_error;
        check(
            raw_view.open(raw_targets.front().file, raw_targets.front().line, raw_error, raw_targets.front().format) &&
                raw_error.empty() && raw_view.lines().front().spans.size() == 1 &&
                raw_view.lines().front().spans.front().kind == snt::view::SyntaxKind::Text,
            "Raw source text should not receive DIPL syntax colors"
        );
        const auto raw_reads = view.graph().dependencies("?experiment.sample_values");
        check(
            raw_reads.size() == 1 && raw_reads.front().target == "raw_samples?" &&
                project.object(raw_reads.front().target),
            "Raw source dependency is not navigable"
        );
        const auto& parameter_relations = project.source_relations(*code_source);
        const auto& raw_relations = project.source_relations(*raw_source);
        check(std::find(parameter_relations.depends_on.begin(), parameter_relations.depends_on.end(),
                        "raw_samples?") != parameter_relations.depends_on.end() &&
                  std::find(raw_relations.used_by.begin(), raw_relations.used_by.end(), code_source->path) !=
                      raw_relations.used_by.end() &&
                  raw_relations.reader_targets.at(code_source->path).line == 74 &&
                  parameter_relations.dependency_targets.at("raw_samples?").line == 1,
              "Source graph should show both sides of a raw-source dependency");
        const auto& reference_relations = project.source_relations(*project.object("reference?"));
        check(std::find(reference_relations.used_by.begin(), reference_relations.used_by.end(),
                        code_source->path) != reference_relations.used_by.end(),
              "Named source graph should link back to dependent source files");
        const auto& observation_relations = project.source_relations(*second_code_source);
        check(std::find(observation_relations.depends_on.begin(), observation_relations.depends_on.end(),
                        code_source->path) != observation_relations.depends_on.end() &&
                  std::find(parameter_relations.used_by.begin(), parameter_relations.used_by.end(),
                            second_code_source->path) != parameter_relations.used_by.end(),
              "Source graph should show cross-file dependencies in both directions");
        const auto* schema_file = project.object("@project-file?schemas/probe.dip");
        check(schema_file &&
                  std::find(project.source_relations(*schema_file).depends_on.begin(),
                            project.source_relations(*schema_file).depends_on.end(), code_source->path) !=
                      project.source_relations(*schema_file).depends_on.end() &&
                  project.source_relations(*schema_file).dependency_targets.at(code_source->path).line > 1 &&
                  project.source_object_path(
                      project.source_relations(*schema_file).dependency_targets.at(code_source->path)
                  ) == code_source->path,
              "Schema file source graph should show its dependency on the parameter source");
        check(raw_source->parent == "@raw_sources", "Raw array source is not in Raw named sources");
        const auto* inline_table = project.object("@block?experiment.readings");
        check(
            inline_table && inline_table->parent == "@blocks" &&
                project.environment().block_inputs().at("experiment.readings").code.find("---") != std::string::npos,
            "Inline table text is missing from Block value sources"
        );
        const auto inline_table_targets = project.source_targets(*inline_table);
        check(
            inline_table_targets.size() == 1 && inline_table_targets.front().block_path &&
                inline_table_targets.front().format == snt::view::SourceFormat::Table &&
                inline_table_targets.front().file.filename() == "observations.dip",
            "Inline table source does not open its table text"
        );
        snt::view::SourceView table_view;
        std::string table_error;
        check(
            table_view.open_text(
                inline_table_targets.front().file,
                inline_table_targets.front().line,
                project.environment().block_inputs().at(*inline_table_targets.front().block_path).code,
                inline_table_targets.front().format,
                table_error
            ) && table_error.empty() &&
                has_span(table_view.lines().front(), snt::view::SyntaxKind::Type, "float") &&
                table_view.lines()[3].text == "---" &&
                table_view.lines()[4].spans.front().kind == snt::view::SyntaxKind::Text,
            "Table header and data highlighting are incorrect"
        );
        const auto* raw_table = project.object("table_samples?");
        check(
            raw_table && raw_table->parent == "@raw_sources" &&
                project.environment().sources.at("table_samples").table_text,
            "Raw table source is missing from Raw named sources"
        );
        const auto raw_table_targets = project.source_targets(*raw_table);
        check(
            raw_table_targets.size() == 1 && raw_table_targets.front().format == snt::view::SourceFormat::Table &&
                raw_table_targets.front().file.filename() == "table_samples.dipt",
            "Raw table source does not open with table highlighting"
        );
        const auto* inline_array = project.object("@block?experiment.sample_grid");
        check(
            inline_array && inline_array->parent == "@blocks" &&
                project.environment().block_inputs().at("experiment.sample_grid").kind ==
                    snt::dip::BlockInput::Kind::Array,
            "Array string block is missing from Block value sources"
        );
        const auto inline_array_targets = project.source_targets(*inline_array);
        check(
            inline_array_targets.size() == 1 && inline_array_targets.front().block_path &&
                inline_array_targets.front().format == snt::view::SourceFormat::Plain,
            "Array string block does not open as plain text"
        );
        check(
            project.select(source_id) && project.reload() && project.selection() == source_id &&
                project.input_path() == relative_example.string(),
            "Reload lost the source selection or original input path"
        );
        check(
            project.select("@override?experiment.geometry.length") && project.reload() &&
                project.selection() == "@override?experiment.geometry.length",
            "Reload lost the override selection"
        );
        check(
            project.select("@schema?probe") && project.reload() && project.selection() == "@schema?probe" &&
                project.select("@unit?sample_tick") && project.reload() && project.selection() == "@unit?sample_tick",
            "Reload lost a registered definition selection"
        );
        project.environment().save(snapshot_path);
        snt::view::ViewerModel snapshot(snapshot_path);
        const auto snapshot_table = snt::dip::Inspector{snapshot.environment()}.table("experiment.extended_readings");
        check(
            snapshot_table.rows == 48 && snapshot_table.columns.size() == 8,
            "The large example table was not retained in the snapshot"
        );
        check(
            snapshot.object("experiment.geometry.length") && !snapshot.object("@dipfile") &&
                snapshot.environment().project_entries().empty() &&
                snapshot.source_targets(*snapshot.object("experiment.geometry.length")).empty() &&
                snapshot.object("@unit?sample_tick") &&
                snapshot.source_targets(*snapshot.object("@unit?sample_tick")).empty(),
            "Source browsing must remain disabled for .diph5 snapshots"
        );
        std::filesystem::create_directory(inline_dir);
        write(inline_dir / "DIPfile", "code[]\n  string = \"\"\"\nanswer int = 7\n\"\"\"\n");
        snt::view::ViewerModel inline_project(inline_dir / "DIPfile");
        check(inline_project.object("answer"), "Embedded project value is missing");
        const auto exported_snapshot = inline_dir / "exported.diph5";
        check(inline_project.can_export_diph5() && inline_project.export_diph5(exported_snapshot) &&
                  std::filesystem::exists(exported_snapshot),
              "A live project should export a DIPH5 snapshot");
        snt::dip::Environment exported_environment;
        exported_environment.load(exported_snapshot);
        check(exported_environment.get_node("answer")->value->to_string() == "7" &&
                  inline_project.artifact() == inline_dir / "DIPfile",
              "The exported snapshot should retain evaluated values without replacing the open project");
        check(!inline_project.export_diph5(exported_snapshot) &&
                  inline_project.error().find("Refusing to replace") != std::string::npos &&
                  !inline_project.export_diph5(inline_dir / "wrong.txt"),
              "Snapshot export should reject existing files and non-DIPH5 names");
        check(
            inline_project.environment().project_entries().size() == 1 &&
                inline_project.environment().project_entries().front().resolved_path.empty() &&
                inline_project.object("@dipfile?Code?0") && inline_project.object("@code?0") &&
                inline_project.object("@code?0")->parent == "@local",
            "Embedded code is missing from the DIPfile or Local sources branch"
        );
        const auto inline_code_target = inline_project.source_targets(*inline_project.object("@code?0"));
        check(
            inline_code_target.size() == 1 && inline_code_target.front().file == inline_dir / "DIPfile" &&
                inline_code_target.front().line == 2,
            "Inline local source does not open at its DIPfile registration"
        );
        const auto inline_target = inline_project.source_targets(*inline_project.object("answer"));
        check(
            inline_target.size() == 1 && inline_target.front().file == inline_dir / "DIPfile" &&
                inline_target.front().line == 2 && inline_target.front().label == "Embedded source registration",
            "Embedded project code must link to its DIPfile registration"
        );
        std::filesystem::create_directory(inline_dir / "empty");
        write(inline_dir / "empty/DIPfile", "");
        snt::view::ViewerModel empty_project(inline_dir / "empty/DIPfile");
        check(
            empty_project.object("@dipfile") && empty_project.object("@dipfile")->children.empty() &&
                empty_project.object("")->children.front() == "@files" &&
                empty_project.object("@project-file?DIPfile"),
            "An empty DIPfile should have an inspectable branch"
        );
        check(empty_project.create_code_source("new-source.dip") &&
                  empty_project.selection() == "@code?0" &&
                  empty_project.object("@code?0") &&
                  empty_project.document_text("new-source.dip") == "# New source\n" &&
                  std::filesystem::exists(inline_dir / "empty/new-source.dip"),
              "Creating a source should register and open it in the project");
        check(!empty_project.create_code_source("new-source.dip") &&
                  !empty_project.create_code_source("../outside.dip") &&
                  !empty_project.create_code_source("invalid\".dip") &&
                  empty_project.environment().project_entries().size() == 1,
              "Creating a source must reject duplicate and unsafe filenames");
        write(inline_dir / "empty/existing.dip", "existing int = 1\n");
        check(!empty_project.create_code_source("existing.dip") &&
                  !empty_project.object("@code?1") &&
                  std::filesystem::file_size(inline_dir / "empty/existing.dip") > 0,
              "Creating a source must preserve existing project files");
        check(empty_project.create_named_source("reference", "reference.dip") &&
                  empty_project.selection() == "reference?" &&
                  empty_project.object("reference?") &&
                  empty_project.document_text("reference.dip") == "# New named source\n" &&
                  std::filesystem::exists(inline_dir / "empty/reference.dip") &&
                  empty_project.environment().project_entries().size() == 2,
              "Creating a named source should register and open it in the project");
        check(!empty_project.create_named_source("reference", "another.dip") &&
                  !empty_project.create_named_source("bad name", "another.dip") &&
                  !empty_project.create_named_source("other", "reference.dip") &&
                  !std::filesystem::exists(inline_dir / "empty/another.dip") &&
                  empty_project.environment().project_entries().size() == 2,
              "Creating a named source must reject duplicate names, paths, and invalid identifiers");
        const auto setup_dir = inline_dir / "setup";
        std::filesystem::create_directory(setup_dir);
        write(setup_dir / "DIPfile", "code[]\n  file = \"settings.dip\"\noverrides[]\n  file = \"manual.dip\"\n");
        write(setup_dir / "settings.dip", "count int\n  !setup\nname str = \"old\"\n  !setup\nuntouched int = 1\n");
        write(setup_dir / "manual.dip", "untouched = 2\n");
        snt::view::ViewerModel setup_project(setup_dir / "DIPfile");
        check(setup_project.object("@setup") && setup_project.setup_questions().size() == 2,
              "Incomplete project should expose all setup questions");
        check(std::find(setup_project.object("")->children.begin(), setup_project.object("")->children.end(),
                        "@setup") == setup_project.object("")->children.end() && setup_project.select("@setup"),
              "Setup should open from the top menu without appearing in the browser tree");
        const auto settings_path = setup_project.editable_document_path(setup_dir / "settings.dip");
        check(settings_path && *settings_path == "settings.dip" && setup_project.can_save_as(),
              "An incomplete setup source should remain editable");
        const auto edited_settings = setup_project.document_text(*settings_path) + "# setup source edit\n";
        check(setup_project.preview_document(*settings_path, edited_settings).candidate_valid &&
                  setup_project.save_document(*settings_path, edited_settings) &&
                  setup_project.document_text(*settings_path) == edited_settings,
              "Editing a setup source should work before required values are filled");
        check(!setup_project.commit_setup({{"count", "invalid"}}, {}, "answers.dip") &&
                  setup_project.setup_error_path() == "count" &&
                  !std::filesystem::exists(setup_dir / "answers.dip"),
              "Invalid setup must not write an override");
        check(!setup_project.commit_setup({{"name", "new"}}, {}, "answers.dip") &&
                  setup_project.setup_error_path() == "count",
              "A missing required answer should identify its setup field");
        check(!setup_project.commit_setup({{"count", "4"}}, {}, "answers.txt") &&
                  setup_project.setup_error_path() == "@setup-file",
              "An invalid setup filename should identify the filename field");
        check(!setup_project.can_remove_setup_override(),
              "Remove override should be unavailable before setup creates an override");
        check(setup_project.commit_setup({{"count", "4"}, {"name", "new"}}, {}, "answers.dip"),
              "Complete setup should commit successfully");
        check(setup_project.can_remove_setup_override() && std::filesystem::exists(setup_dir / "answers.dip") &&
                  setup_project.environment().get_node("count")->value->to_string() == "4" &&
                  setup_project.environment()["name"].as<std::string>() == "new" &&
                  setup_project.can_save_as(),
              "Committed setup values should appear immediately");
        snt::view::ViewerModel reopened_setup(setup_dir / "DIPfile");
        check(reopened_setup.environment().get_node("count")->value->to_string() == "4" &&
                  reopened_setup.environment()["name"].as<std::string>() == "new",
              "Reopened project should retain setup values");
        check(reopened_setup.can_remove_setup_override() && reopened_setup.remove_setup_override() &&
                  !std::filesystem::exists(setup_dir / "answers.dip") &&
                  std::filesystem::exists(setup_dir / "manual.dip") &&
                  !reopened_setup.environment().get_node("count")->value &&
                  reopened_setup.environment()["name"].as<std::string>() == "old" &&
                  reopened_setup.environment().get_node("untouched")->value->to_string() == "2" &&
                  reopened_setup.setup_questions().size() == 2,
              "Removing guided setup should restore prior values and preserve unrelated overrides");
        std::size_t manual_override_index = reopened_setup.environment().project_entries().size();
        for (std::size_t index = 0; index < reopened_setup.environment().project_entries().size(); ++index)
            if (reopened_setup.environment().project_entries()[index].kind ==
                    snt::dip::ProjectEntry::Kind::Override)
                manual_override_index = index;
        check(manual_override_index < reopened_setup.environment().project_entries().size() &&
                  reopened_setup.delete_override_source(manual_override_index) &&
                  !std::filesystem::exists(setup_dir / "manual.dip") &&
                  std::filesystem::exists(setup_dir / "settings.dip") &&
                  reopened_setup.environment().get_node("untouched")->value->to_string() == "1",
              "Deleting an override source should remove only its file and registration");
        snt::view::ViewerModel stale_setup(setup_dir / "DIPfile");
        check(!stale_setup.can_remove_setup_override() &&
                  reopened_setup.commit_setup({{"count", "9"}}, {}, "second-answers.dip") &&
                  stale_setup.remove_setup_override() &&
                  !std::filesystem::exists(setup_dir / "second-answers.dip"),
              "Remove override should detect a committed setup even if the viewer state is stale");
        write(setup_dir / "DIPfile", "code[]\n  file = \"settings.dip\"\noverrides[]\n  file = \"setup-overrides.dip\"\n");
        write(setup_dir / "setup-overrides.dip", "count = 7\n");
        snt::view::ViewerModel registered_setup(setup_dir / "DIPfile");
        check(!registered_setup.can_remove_setup_override() &&
                  registered_setup.remove_setup_override("setup-overrides.dip") &&
                  !std::filesystem::exists(setup_dir / "setup-overrides.dip") &&
                  registered_setup.environment().project_entries().size() == 1 &&
                  !registered_setup.environment().get_node("count")->value,
              "Setup removal should use the visible project registration even without a generated-file header");
        const auto guided_example = std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) / "examples/dip/GuidedSetup/DIPfile";
        const auto guided_copy = inline_dir / "guided";
        std::filesystem::create_directory(guided_copy);
        write(guided_copy / "DIPfile", "code[]\n  file = \"settings.dip\"\n");
        std::filesystem::copy_file(guided_example.parent_path() / "settings.dip", guided_copy / "settings.dip");
        snt::view::ViewerModel guided_project(guided_copy / "DIPfile");
        check(!guided_project.commit_setup({{"simulation.timestep", "3 m"},
                                            {"simulation.random_seed", "12"}}, {}, "invalid-setup.dip") &&
                  guided_project.setup_error_path() == "simulation.timestep",
              "A setup unit error should identify the input that caused it");
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
