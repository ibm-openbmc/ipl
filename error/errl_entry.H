#pragma once

#include <unistd.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
using json = nlohmann::json;

namespace errl
{
/**
 * @brief format of the ffdc file
 * @detail taken from /xyz/openbmc_project/Logging/Create/common.hpp
 */
enum class FFDCFormat
{
    JSON,
    CBOR,
    Text,
    Custom,
};

/**
 * @enum UserDataFormatVersion
 * @brief Represents different formats supported for user data FFDC (First
 * Failure Data Capture).
 */
enum class UserDataFormatVersion
{
    json = 1, ///< User data is encoded in JSON format.
    cbor = 1, ///< User data is encoded in CBOR (Concise Binary Object
              ///< Representation) format.
    text = 1, ///< User data is stored as plain text.
    custom = 1
};

/**
 * @brief Contains information about an FFDC file.
 * @detail taken from phosphor-logging/pel.hpp
 */
struct PelFFDCfile
{
    FFDCFormat format;
    uint8_t subType = 0x1;
    uint8_t version = 0x1;
    int fd;
};

/** @brief Represents an error log entry with associated metadata.
 *
 *  This class encapsulates a single error log entry, including
 *  severity level, a message, additional key-value metadata,
 *  optional callout information in JSON format, and an optional binary blob.
 */
class ErrlEntry
{
  public:
    /** @brief Constructs an ErrlEntry instance.
     *
     *  @param[in] i_msg - Descriptive error message.
     *  @param[in] i_sev - Severity level of the error.
     *  @param[in] i_data - Key-value pairs of additional debug data.
     *  @param[in] i_callout - Optional JSON-formatted callout information.
     *  @param[in] i_blob - Optional binary data associated with the error.
     */
    ErrlEntry(std::string_view i_msg,
              std::unordered_map<std::string, std::string> i_data,
              std::optional<json> i_callout = std::nullopt,
              std::optional<std::vector<uint8_t>> i_blob = std::nullopt);

    /** @brief Default destructor. */
    ~ErrlEntry() noexcept;

    /** @brief Copy constructor. */
    ErrlEntry(const ErrlEntry&) = delete;

    /** @brief Copy assignment operator. disable*/
    ErrlEntry& operator=(const ErrlEntry&) = delete;

    /** @brief Move constructor. */
    ErrlEntry(ErrlEntry&&) noexcept = default;

    /** @brief Move assignment operator. */
    ErrlEntry& operator=(ErrlEntry&&) noexcept = default;

    /** @brief Get the human-readable error message.
     *  @return A reference to the message string.
     */
    const std::string& getMessage() const noexcept
    {
        return message;
    }

    /** @brief Get the additional debug data.
     *  @return A reference to a vector of key-value string pairs.
     */
    const std::unordered_map<std::string, std::string>&
        getAdditionalData() const noexcept
    {
        return additionalData;
    }

    /** @brief Get the optional callout data.
     *  @return A reference to an optional JSON object containing callout
     * information.
     */
    const std::optional<json>& getCallout() const noexcept
    {
        return callout;
    }

    /** @brief Get the file descriptor and path of the blob, if available.
     *  @return Optional pair of file descriptor and filesystem path.
     */
    const std::optional<
        std::vector<std::pair<PelFFDCfile, std::filesystem::path>>>&
        getFfdcFiles() const noexcept
    {
        return ffdcFiles;
    }

  private:
    std::string message;                      ///< Error message
    std::unordered_map<std::string, std::string> additionalData;
    std::optional<json> callout;              ///< Optional callout info in JSON
    std::optional<std::vector<uint8_t>> blob; ///< Optional binary data
    std::optional<std::vector<std::pair<PelFFDCfile, std::filesystem::path>>>
        ffdcFiles;
};
}; // namespace errl
