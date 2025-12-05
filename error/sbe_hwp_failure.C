#include <errl_util.H>
#include <libekb.H>
#include <sbe_hwp_failure.H>

#include <string>
namespace errl::sbe_hwp_failure
{
std::optional<std::unique_ptr<ErrlHandle>> create()
{
    FFDC ffdc;
    libekb_get_ffdc(ffdc);

    json callout = json::array();
    std::unordered_map<std::string, std::string> effdcUserData;
    errl::util::convertFAPItoPELformat(ffdc, callout, effdcUserData);

    std::string errMsg = "org.open_power.PHAL.Error.Boot";
    auto handle = std::make_unique<errl::ErrlHandle>();
    auto errl = std::make_unique<errl::ErrlEntry>(
        errMsg, effdcUserData, std::make_optional(std::move(callout)));
    handle->addEntry(std::move(errl));
    return handle;
}
} // namespace errl::sbe_hwp_failure
