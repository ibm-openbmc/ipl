#include <errl_entry.H>

#include <log.hpp>
namespace errl
{
// errl_entry.C
ErrlEntry::ErrlEntry(std::string_view msg, Level sev,
                     std::vector<std::pair<std::string, std::string>> data,
                     std::optional<json> calloutData,
                     std::optional<std::vector<std::byte>> blobData) :
    message{msg}, severity{sev}, additionalData{std::move(data)},
    callout{std::move(calloutData)}, blob{std::move(blobData)}
{
    logger::info("ErrlEntry constructor msg {}", msg);
}
} // namespace errl
