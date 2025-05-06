#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
using json = nlohmann::json;

namespace errl
{
/** @brief Enumeration representing the severity level of an error log entry.
 *
 *  This enum defines the standard severity levels used to classify
 *  error log entries based on their urgency and impact.
 */
enum class Level
{
    Emergency,     /**< System is unusable. Immediate attention required. */
    Alert,         /**< Action must be taken immediately. */
    Critical,      /**< Critical conditions, such as hardware failures. */
    Error,         /**< General error conditions. */
    Warning,       /**< Warning conditions that may need attention. */
    Notice,        /**< Normal but significant conditions. */
    Informational, /**< Informational messages for tracking operation. */
    Debug,         /**< Debug-level messages for troubleshooting. */
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
     *  @param[in] msg - Descriptive error message.
     *  @param[in] sev - Severity level of the error.
     *  @param[in] data - Key-value pairs of additional debug data.
     *  @param[in] callout - Optional JSON-formatted callout information.
     *  @param[in] blob - Optional binary data associated with the error.
     */
    ErrlEntry(std::string_view msg, Level sev,
              std::vector<std::pair<std::string, std::string>> data,
              std::optional<json> callout = std::nullopt,
              std::optional<std::vector<std::byte>> blob = std::nullopt);

    /** @brief Default destructor. */
    ~ErrlEntry() noexcept = default;

    /** @brief Copy constructor. */
    ErrlEntry(const ErrlEntry&) = default;

    /** @brief Copy assignment operator. */
    ErrlEntry& operator=(const ErrlEntry&) = default;

    /** @brief Move constructor. */
    ErrlEntry(ErrlEntry&&) noexcept = default;

    /** @brief Move assignment operator. */
    ErrlEntry& operator=(ErrlEntry&&) noexcept = default;

    /** @brief Get the human-readable error message.
     *  @return A reference to the message string.
     */
    const std::string& getMessage() const noexcept;

    /** @brief Get the severity level of the error.
     *  @return Severity level.
     */
    Level getSeverity() const noexcept;

    /** @brief Get the additional debug data.
     *  @return A reference to a vector of key-value string pairs.
     */
    const std::vector<std::pair<std::string, std::string>>&
        getAdditionalData() const noexcept;

    /** @brief Get the optional callout data.
     *  @return A reference to an optional JSON object containing callout
     * information.
     */
    const std::optional<json>& getCallout() const noexcept;

  private:
    std::string message;         ///< Error message
    Level severity;              ///< Severity of the error
    std::vector<std::pair<std::string, std::string>>
        additionalData;          ///< Additional debug information
    std::optional<json> callout; ///< Optional callout info in JSON
    std::optional<std::vector<std::byte>> blob; ///< Optional binary data
};
}; // namespace errl
