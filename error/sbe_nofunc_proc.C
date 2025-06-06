#include <attributes_info.H>
#include <errl_util.H>
#include <libphal.H>
#include <phal_exception.H>
#include <sbe_nofunc_proc.H>
#include <target.H>

#include <log.hpp>

#include <string>
extern "C"
{
#include <libpdbg.h>
}
namespace errl::sbe_nofunc_proc
{
std::optional<std::unique_ptr<ErrlHandle>> create()
{
    auto handle = std::make_unique<errl::ErrlHandle>();
    std::unordered_map<std::string, std::string> additionalData;
    additionalData.emplace("_PID", std::to_string(getpid()));

    json jsonCalloutDataList;
    jsonCalloutDataList = json::array();
    json jsonCalloutData;
    jsonCalloutData["Procedure"] = "BMC0001";
    jsonCalloutData["Priority"] = "H";
    jsonCalloutDataList.emplace_back(jsonCalloutData);
    auto errl = std::make_unique<errl::ErrlEntry>(
        "org.open_power.Processor.Error.SbeBootFailure", additionalData,
        std::make_optional(std::move(jsonCalloutDataList)), std::nullopt);
    handle->addEntry(std::move(errl));
    return handle;
}
} // namespace errl::sbe_nofunc_proc
