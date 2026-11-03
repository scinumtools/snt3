#ifndef SNT_DIP_HDF5_IO_PRIVATE_H
#define SNT_DIP_HDF5_IO_PRIVATE_H

#include "io.h"
#include <hdf5.h>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace snt::dip::hdf5::detail {
        /**
         * A checksum or archive-structure failure, distinct from an I/O failure.
         */
        class IntegrityError : public Error {
          public:
            using Error::Error;
        };

        /**
         * Own one HDF5 handle and close it on scope exit.
         */
        class Id {
          public:
            using Closer = herr_t (*)(hid_t);

            Id() = default;
            /** @param value Handle to own. @param closer Matching HDF5 close function.
             *  @param operation Error text if opening the handle failed. */
            Id(hid_t value, Closer closer, const char* operation) : value_(value), closer_(closer) {
                if (value_ < 0)
                    throw Error(operation);
            }
            Id(const Id&) = delete;
            Id& operator=(const Id&) = delete;
            Id(Id&& other) noexcept : value_(other.value_), closer_(other.closer_) { other.value_ = H5I_INVALID_HID; }
            Id& operator=(Id&& other) noexcept {
                if (this != &other) {
                    close();
                    value_ = other.value_;
                    closer_ = other.closer_;
                    other.value_ = H5I_INVALID_HID;
                }
                return *this;
            }
            ~Id() { close(); }

            operator hid_t() const { return value_; }

          private:
            void close() noexcept {
                if (value_ >= 0 && closer_)
                    closer_(value_);
                value_ = H5I_INVALID_HID;
            }

            hid_t value_ = H5I_INVALID_HID; ///< Owned HDF5 handle, or invalid after a move.
            Closer closer_ = nullptr;        ///< Function that releases the matching handle type.
        };

    /**
     * Throw on a failed HDF5 call.
     * @param status HDF5 result.
     * @param operation Error context.
     */
    void check(herr_t status, const char* operation);
    /**
     * Convert a stored count safely.
     * @param value Stored count.
     * @param field Name for errors.
     */
    size_t checked_size(uint64_t value, const char* field);
    /** Multiply allocation dimensions safely. @param count Items. @param width Bytes per item.
     *  @param field Name for errors. */
    size_t checked_product(size_t count, size_t width, const char* field);
    /**
     * Create a UTF-8 HDF5 string type.
     * @param size Storage width in bytes.
     */
    Id string_type(size_t size);
    /**
     * Check for an attribute.
     * @param object HDF5 owner.
     * @param name Attribute name.
     */
    bool has_attribute(hid_t object, std::string_view name);
    /**
     * Remove an existing attribute, if present.
     * @param object HDF5 owner.
     * @param name Attribute name.
     */
    void remove_attribute(hid_t object, std::string_view name);
    /** Store a string attribute. @param object HDF5 owner. @param name Attribute name.
     *  @param value UTF-8 text. */
    void write_string(hid_t object, std::string_view name, const std::string& value);
    /** Read a string attribute. @param object HDF5 owner. @param name Attribute name.
     *  @param fallback Value when the attribute is absent. */
    std::string read_string(hid_t object, std::string_view name, const std::string& fallback = {});
        /** Store a scalar attribute. @param object HDF5 owner. @param name Attribute name.
         *  @param type Matching HDF5 native type. @param value Value to store. */
        template <typename T> void write_scalar(hid_t object, std::string_view name, hid_t type, const T& value) {
            remove_attribute(object, name);
            Id space(H5Screate(H5S_SCALAR), H5Sclose, "Unable to create an HDF5 attribute dataspace");
            Id attribute(
                H5Acreate2(object, std::string(name).c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT),
                H5Aclose,
                "Unable to create an HDF5 scalar attribute"
            );
            check(H5Awrite(attribute, type, &value), "Unable to write an HDF5 scalar attribute");
        }

        /** Read a scalar attribute. @param object HDF5 owner. @param name Attribute name.
         *  @param type Matching HDF5 native type. @param fallback Value when absent. */
        template <typename T> T read_scalar(hid_t object, std::string_view name, hid_t type, T fallback = {}) {
            if (!has_attribute(object, name))
                return fallback;
            Id attribute(
                H5Aopen(object, std::string(name).c_str(), H5P_DEFAULT), H5Aclose, "Unable to open an HDF5 attribute"
            );
            Id space(H5Aget_space(attribute), H5Sclose, "Unable to inspect an HDF5 scalar attribute shape");
            if (H5Sget_simple_extent_ndims(space) != 0)
                throw Error("Invalid HDF5 scalar attribute shape");
            T value{};
            check(H5Aread(attribute, type, &value), "Unable to read an HDF5 scalar attribute");
            return value;
        }


    /** Write an ordered string list. @param object HDF5 owner. @param name Attribute name.
     *  @param values Strings to store. */
    void write_strings(hid_t object, std::string_view name, const std::vector<std::string>& values);
    /**
     * Read an ordered string list.
     * @param object HDF5 owner.
     * @param name Attribute name.
     */
    std::vector<std::string> read_strings(hid_t object, std::string_view name);
    /**
     * Count direct children.
     * @param group HDF5 group.
     */
    hsize_t object_count(hid_t group);
    /**
     * Read one child name.
     * @param group HDF5 group.
     * @param index Child index.
     */
    std::string object_name(hid_t group, hsize_t index);
    /**
     * Make a fixed-width archive entry name.
     * @param index Zero-based entry number.
     */
    std::string numbered(size_t index);
    /**
     * Return the current UTC timestamp in archive form.
     */
    std::string utc_now();
    /**
     * Generate a record identifier.
     */
    std::string uuid();
    /**
     * Read a required UTF-8 attribute.
     * @param object HDF5 owner.
     * @param field Attribute name.
     */
    std::string required_utf8(hid_t object, std::string_view field);
        /** Read a required scalar. @param object HDF5 owner. @param field Attribute name.
         *  @param type Matching HDF5 native type. */
        template <typename T> T required_scalar(hid_t object, std::string_view field, hid_t type) {
            if (!has_attribute(object, field))
                throw Error("Missing DIPH5 v3 attribute: " + std::string(field));
            return read_scalar<T>(object, field, type);
        }


    /**

     * Check whether a path exists.

     * @param object HDF5 location.

     * @param path Relative or absolute path.

     */
    bool has_link(hid_t object, const char* path);
    /**
     * Convert an experiment status to its archive spelling.
     * @param status Status value.
     */
    const char* status_name(ExperimentStatus status);
    /**
     * Parse an archive status.
     * @param value Stored status spelling.
     */
    ExperimentStatus parse_status(const std::string& value);
    /**
     * Validate argument-list allocation size.
     * @param args Command arguments.
     */
    size_t checked_argv_width(const std::vector<std::string>& args);
    /** Write a command argument list. @param group Owner group. @param name Child name.
     *  @param args Ordered arguments. */
    void write_argv(hid_t group, const char* name, const std::vector<std::string>& args);
    /**
     * Read a command argument list.
     * @param group Owner group.
     * @param name Child name.
     */
    std::vector<std::string> read_argv(hid_t group, const char* name);
    /**
     * Hash exact file bytes.
     * @param path File to read.
     */
    std::string hash_file(const std::filesystem::path& path);
    /**
     * Open and validate an experiment archive for updates.
     * @param file Archive path.
     */
    Id open_experiment_file(const std::filesystem::path& file);
    /**
     * Read the execution record.
     * @param input Open archive.
     */
    ExperimentOutcome inspect_execution(hid_t input);
    /**
     * Read the results registry.
     * @param archive Open archive.
     */
    std::vector<ExperimentResult> inspect_results(hid_t archive);
    /**
     * Compute a candidate canonical configuration fingerprint, or no value when unsupported.
     */
    std::optional<std::string> configuration_fingerprint(hid_t archive);
} // namespace snt::dip::hdf5::detail

#endif
