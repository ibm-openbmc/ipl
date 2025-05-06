#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>
using json = nlohmann::json;

namespace errl
{
enum class Level
{
    Emergency,
    Alert,
    Critical,
    Error,
    Warning,
    Notice,
    Informational,
    Debug,
};
class ErrlEntry
{
  public:
    ErrlEntry(std::string_view msg, Level sev,
              std::vector<std::pair<std::string, std::string>> data,
              std::optional<json> callout = std::nullopt);

    ~ErrlEntry() noexcept = default;

    ErrlEntry(const ErrlEntry&) = default;
    ErrlEntry& operator=(const ErrlEntry&) = default;
    ErrlEntry(ErrlEntry&&) noexcept = default;
    ErrlEntry& operator=(ErrlEntry&&) noexcept = default;

    const std::string& getMessage() const noexcept
    {
        return message;
    }
    Level getSeverity() const noexcept
    {
        return severity;
    }
    const std::vector<std::pair<std::string, std::string>>&
        getAdditionalData() const noexcept
    {
        return additionalData;
    }
    const std::optional<json>& getCallout() const noexcept
    {
        return callout;
    }

  private:
    std::string message;
    Level severity;
    std::vector<std::string, std::string> additionalData;
    std::optional<json> callout;
};
}; // namespace errl
