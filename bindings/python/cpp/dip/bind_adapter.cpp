#include <optional>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <snt/dip/adapter.h>
#include <snt/dip/environment.h>
#include "../val/bind_to_value.h"

namespace py = pybind11;

namespace snt::bind::python {
    namespace {
        class PyAdapter : public dip::Adapter {
          public:
            using dip::Adapter::Adapter;
            void describe_outputs(const dip::Environment& env, dip::OutputPlan& outputs) const override {
                py::gil_scoped_acquire gil;
                py::function override = py::get_override(this, "describe_outputs");
                if (override)
                    override(py::cast(&env, py::return_value_policy::reference),
                             py::cast(&outputs, py::return_value_policy::reference));
            }
            void plan(const dip::Environment& env, dip::AdapterContext& context) const override {
                py::gil_scoped_acquire gil;
                py::function override = py::get_override(this, "plan");
                if (!override)
                    throw std::runtime_error("Python Adapter subclass must implement plan(env, context)");
                override(
                    py::cast(&env, py::return_value_policy::reference),
                    py::cast(&context, py::return_value_policy::reference)
                );
            }
            void plan_resolved(const dip::Environment& env, const dip::OutputPlan& outputs,
                               dip::AdapterContext& context) const override {
                py::gil_scoped_acquire gil;
                py::function override = py::get_override(this, "plan_resolved");
                if (override) {
                    override(py::cast(&env, py::return_value_policy::reference),
                             py::cast(&outputs, py::return_value_policy::reference),
                             py::cast(&context, py::return_value_policy::reference));
                } else {
                    plan(env, context);
                }
            }
        };
    } // namespace

    void init_adapter(py::module_& m) {
        py::class_<dip::OutputMapping>(m, "OutputMapping", "One resolved adapter output mapping.")
            .def_readonly("id", &dip::OutputMapping::id)
            .def_readonly("target", &dip::OutputMapping::target)
            .def_readonly("key", &dip::OutputMapping::key)
            .def_readonly("source_path", &dip::OutputMapping::source_path)
            .def_readonly("rule", &dip::OutputMapping::rule)
            .def_readonly("origin", &dip::OutputMapping::origin)
            .def_readonly("replacements", &dip::OutputMapping::replacements)
            .def_readonly("dependencies", &dip::OutputMapping::dependencies)
            .def_readonly("active", &dip::OutputMapping::active)
            .def_property_readonly("value", [](const dip::OutputMapping& mapping) {
                return to_python_value(mapping.value->value);
            })
            .def_property_readonly("value_node", [](const dip::OutputMapping& mapping) {
                return mapping.value;
            });

        py::class_<dip::OutputPlan>(m, "OutputPlan", "Typed, ordered adapter output mappings.")
            .def(py::init<>())
            .def("add_node", &dip::OutputPlan::add_node, py::arg("env"), py::arg("id"),
                 py::arg("target"), py::arg("key"), py::arg("source_path"), py::arg("active") = true,
                 py::arg("rule") = "", py::arg("origin") = "",
                 py::arg("dependencies") = std::vector<std::string>{})
            .def("replace_node", &dip::OutputPlan::replace_node, py::arg("env"), py::arg("id"),
                 py::arg("target"), py::arg("key"), py::arg("source_path"), py::arg("active") = true,
                 py::arg("rule") = "", py::arg("origin") = "",
                 py::arg("dependencies") = std::vector<std::string>{})
            .def("add_value", &dip::OutputPlan::add_value, py::arg("id"), py::arg("target"),
                 py::arg("key"), py::arg("value"), py::arg("active") = true,
                 py::arg("rule") = "", py::arg("origin") = "",
                 py::arg("dependencies") = std::vector<std::string>{})
            .def("replace_value", &dip::OutputPlan::replace_value, py::arg("id"), py::arg("target"),
                 py::arg("key"), py::arg("value"), py::arg("active") = true,
                 py::arg("rule") = "", py::arg("origin") = "",
                 py::arg("dependencies") = std::vector<std::string>{})
            .def("select", &dip::OutputPlan::select, py::arg("target"), py::arg("active_only") = false)
            .def_property_readonly("mappings", &dip::OutputPlan::mappings,
                                   py::return_value_policy::reference_internal);

        py::enum_<dip::ExistingOutputPolicy>(m, "ExistingOutputPolicy")
            .value("Reject", dip::ExistingOutputPolicy::Reject)
            .value("ReplaceRegistered", dip::ExistingOutputPolicy::ReplaceRegistered)
            .value("SyncRegistered", dip::ExistingOutputPolicy::SyncRegistered);

        py::class_<dip::AdapterContext>(m, "AdapterContext", "Outputs planned for one adapter run.")
            .def(
                "add_text",
                &dip::AdapterContext::add_text,
                py::arg("path"),
                py::arg("text"),
                "Register application-defined text at a relative output path."
            )
            .def(
                "add_binary",
                [](dip::AdapterContext& context, const std::filesystem::path& path, py::bytes data) {
                    const std::string bytes = data;
                    context.add_binary(path, {bytes.begin(), bytes.end()});
                },
                py::arg("path"),
                py::arg("data"),
                "Register raw bytes without encoding or newline changes."
            )
            .def(
                "add_stream",
                [](dip::AdapterContext& context, const std::filesystem::path& path, py::function callback) {
                    context.add_stream(path, [callback = std::move(callback)](std::ostream& stream) {
                        py::gil_scoped_acquire gil;
                        const auto active = std::make_shared<bool>(true);
                        py::cpp_function write([&stream, active](py::bytes data) {
                            if (!*active)
                                throw std::runtime_error("The adapter stream is no longer active");
                            const std::string bytes = data;
                            stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
                            if (!stream)
                                throw std::runtime_error("Writing the adapter stream failed");
                        });
                        try {
                            callback(write);
                        } catch (...) {
                            *active = false;
                            throw;
                        }
                        *active = false;
                    });
                },
                py::arg("path"),
                py::arg("callback"),
                "Register a callback called after path validation with a write(bytes) function."
            );

        py::class_<dip::Adapter, PyAdapter>(m, "Adapter", "Subclass and implement plan or plan_resolved.")
            .def(py::init<>())
            .def("describe_outputs", &dip::Adapter::describe_outputs, py::arg("env"), py::arg("outputs"))
            .def(
                "plan",
                &dip::Adapter::plan,
                py::arg("env"),
                py::arg("context"),
                "Select values, validate them for the target application, and register its files."
            )
            .def("plan_resolved", &dip::Adapter::plan_resolved, py::arg("env"), py::arg("outputs"),
                 py::arg("context"));

        m.def("resolve_output_plan", &dip::resolve_output_plan, py::arg("env"), py::arg("adapter"),
              "Resolve and validate an adapter's output plan without writing files.");

        m.def(
            "run_adapter",
            [](const dip::Environment& env,
               const dip::Adapter& adapter,
               const std::filesystem::path& output_dir,
               const std::optional<std::filesystem::path>& snapshot,
               dip::ExistingOutputPolicy policy) {
                return dip::run_adapter(env, adapter, output_dir, snapshot.value_or(std::filesystem::path{}), policy);
            },
            py::arg("env"),
            py::arg("adapter"),
            py::arg("output_dir"),
            py::arg("snapshot") = py::none(),
            py::arg("existing_output_policy") = dip::ExistingOutputPolicy::Reject,
            "Plan and write outputs from an existing Environment. Paths must be relative. "
            "Returns written paths in registration order, followed by an optional DIPH5 snapshot."
        );
        m.def(
            "run_adapter_project",
            [](const std::filesystem::path& project,
               const dip::Adapter& adapter,
               const std::filesystem::path& output_dir,
               const std::optional<std::filesystem::path>& snapshot,
               dip::ExistingOutputPolicy policy) {
                return dip::run_adapter_project(
                    project, adapter, output_dir, snapshot.value_or(std::filesystem::path{}), policy
                );
            },
            py::arg("project"),
            py::arg("adapter"),
            py::arg("output_dir"),
            py::arg("snapshot") = py::none(),
            py::arg("existing_output_policy") = dip::ExistingOutputPolicy::Reject,
            "Parse a DIPfile, then plan and write adapter outputs. Returns written paths."
        );
        m.def(
            "run_adapter_snapshot",
            [](const std::filesystem::path& input,
               const dip::Adapter& adapter,
               const std::filesystem::path& output_dir,
               const std::optional<std::filesystem::path>& snapshot,
               dip::ExistingOutputPolicy policy) {
                return dip::run_adapter_snapshot(
                    input, adapter, output_dir, snapshot.value_or(std::filesystem::path{}), policy
                );
            },
            py::arg("input"),
            py::arg("adapter"),
            py::arg("output_dir"),
            py::arg("snapshot") = py::none(),
            py::arg("existing_output_policy") = dip::ExistingOutputPolicy::Reject,
            "Load a DIPH5 snapshot, then plan and write adapter outputs. Returns written paths."
        );
    }
} // namespace snt::bind::python
