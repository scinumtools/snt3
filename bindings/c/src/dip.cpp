#include "snt/c/dip.h"

#include <cstring>
#include <stdexcept>
#include <string>

#include <snt/dip/dip.h>
#include <snt/dip/report/report.h>

struct snt_dip {
    snt::dip::DIP parser;
    snt::dip::Environment env;
    bool parsed = false;
};

namespace {
thread_local std::string last_error;

int fail(snt_dip_error* error, const std::exception& exception) {
    last_error = exception.what();
    if (error) {
        error->code = 1;
        error->message = last_error.c_str();
    }
    return 1;
}

void ok(snt_dip_error* error) {
    if (error) {
        error->code = 0;
        error->message = nullptr;
    }
}

snt::dip::ExportFormat export_format(snt_dip_export_format format) {
    switch (format) {
    case SNT_DIP_EXPORT_CPP:
        return snt::dip::ExportFormat::CPP;
    case SNT_DIP_EXPORT_C:
        return snt::dip::ExportFormat::C;
    case SNT_DIP_EXPORT_FORTRAN:
        return snt::dip::ExportFormat::FORTRAN;
    case SNT_DIP_EXPORT_RUST:
        return snt::dip::ExportFormat::RUST;
    case SNT_DIP_EXPORT_R:
        return snt::dip::ExportFormat::R;
    case SNT_DIP_EXPORT_JULIA:
        return snt::dip::ExportFormat::JULIA;
    case SNT_DIP_EXPORT_JSON:
        return snt::dip::ExportFormat::JSON;
    case SNT_DIP_EXPORT_TOML:
        return snt::dip::ExportFormat::TOML;
    case SNT_DIP_EXPORT_YAML:
        return snt::dip::ExportFormat::YAML;
    default:
        throw std::invalid_argument("invalid DIP output format");
    }
}

snt::dip::report::ReportFormat report_format(snt_dip_report_format format) {
    switch (format) {
    case SNT_DIP_REPORT_TEX: return snt::dip::report::ReportFormat::Tex;
    case SNT_DIP_REPORT_PDF: return snt::dip::report::ReportFormat::Pdf;
    case SNT_DIP_REPORT_MD: return snt::dip::report::ReportFormat::Markdown;
    case SNT_DIP_REPORT_RST: return snt::dip::report::ReportFormat::Rst;
    case SNT_DIP_REPORT_HTML: return snt::dip::report::ReportFormat::Html;
    case SNT_DIP_REPORT_TYP: return snt::dip::report::ReportFormat::Typst;
    case SNT_DIP_REPORT_TXT: return snt::dip::report::ReportFormat::Text;
    case SNT_DIP_REPORT_JSON: return snt::dip::report::ReportFormat::Json;
    default: throw std::invalid_argument("invalid DIP report format");
    }
}
} // namespace

extern "C" int snt_dip_parser_create(snt_dip** result, snt_dip_error* error) {
    try {
        if (!result)
            throw std::invalid_argument("output is required");
        *result = new snt_dip;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_string(snt_dip* dip, const char* source, snt_dip_error* error) {
    try {
        if (!dip || !source)
            throw std::invalid_argument("DIP and source text are required");
        dip->parser.add_string(source);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_file(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and filename are required");
        dip->parser.add_file(path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_override_string(snt_dip* dip, const char* source, snt_dip_error* error) {
    try {
        if (!dip || !source)
            throw std::invalid_argument("DIP and override text are required");
        dip->parser.add_override_string(source);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_override_file(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and override file path are required");
        dip->parser.add_override_file(path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_is_overridden(
    const snt_dip* dip, const char* path, int* result, snt_dip_error* error
) {
    try {
        if (!dip || !dip->parsed || !path || !result)
            throw std::invalid_argument("parsed DIP, path, and output are required");
        *result = dip->env.get_node(path)->override ? 1 : 0;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_project(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and project filename are required");
        dip->parser.add_project(path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_parse(snt_dip* dip, snt_dip_error* error) {
    try {
        if (!dip)
            throw std::invalid_argument("DIP is required");
        dip->env = dip->parser.parse();
        dip->parsed = true;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_get(
    const snt_dip* dip, const char* path, char* buffer, size_t capacity, snt_dip_error* error
) {
    try {
        if (!dip || !dip->parsed || !path || !buffer || !capacity)
            throw std::invalid_argument("parsed DIP, path, buffer, and capacity are required");
        auto value = dip->env.request_node_data(std::string("?") + path);
        if (!value.value)
            throw std::runtime_error("DIPL path has no value");
        auto output = value.value->to_string();
        if (output.size() + 1 > capacity)
            throw std::invalid_argument("output buffer is too small");
        std::memcpy(buffer, output.c_str(), output.size() + 1);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_environment_load(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and filename are required");
        dip->env.load(path);
        dip->parsed = true;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_environment_save(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and filename are required");
        dip->env.save(path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_environment_generate(
    snt_dip* dip, snt_dip_export_format format, const char* path, snt_dip_error* error
) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and filename are required");
        dip->env.generate(export_format(format), path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_environment_generate_report(
    snt_dip* dip, snt_dip_report_format format, const char* path,
    const char* input_label, const char* intro_file, const char* tex_compiler,
    snt_dip_error* error
) {
    const snt_dip_report_options options{input_label, intro_file, tex_compiler, nullptr, nullptr, nullptr, nullptr};
    return snt_dip_environment_generate_report_with_options(dip, format, path, &options, error);
}

extern "C" int snt_dip_environment_generate_report_with_options(
    snt_dip* dip, snt_dip_report_format format, const char* path,
    const snt_dip_report_options* cover, snt_dip_error* error
) {
    try {
        if (!dip || !dip->parsed || !path || !*path)
            throw std::invalid_argument("parsed DIP and output path are required");
        snt::dip::report::ReportOptions options;
        if (cover) {
            if (cover->input_label) options.input_label = cover->input_label;
            if (cover->intro_file) options.introduction_file = cover->intro_file;
            if (cover->tex_compiler) options.tex_compiler = cover->tex_compiler;
            if (cover->title) options.title = cover->title;
            if (cover->author) options.author = cover->author;
            if (cover->date) options.date = cover->date;
            if (cover->version) options.version = cover->version;
        }
        snt::dip::report::generate(dip->env, report_format(format), path, options);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" void snt_dip_parser_free(snt_dip* dip) {
    delete dip;
}

extern "C" int snt_dip_parser_add_schema_string(snt_dip* dip, const char* name, const char* source, snt_dip_error* error) {
    try {
        if (!dip || !name || !source)
            throw std::invalid_argument("DIP, schema name, and schema input are required");
        dip->parser.add_schema_string(name, source);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_schema_file(snt_dip* dip, const char* name, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !name || !path)
            throw std::invalid_argument("DIP, schema name, and schema input are required");
        dip->parser.add_schema_file(name, path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}
