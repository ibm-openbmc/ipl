#include "errl_entry.H"
namespace errl
{
ErrlEntry::ErrlEntry(std::string_view msg, Level sev,
                     std::vector<std::pair<std::string, std::string>> data,
                     std::optional<json> calloutData) :
    message{msg}, severity{sev}, additionalData{std::move(data)},
    callout{std::move(calloutData)}
{}

} // namespace errl
