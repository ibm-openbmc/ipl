#include <ffdc_converter.H>
#include <sbe_boot_failure.H>
#include <sbe_intf.H>
#include <target.H>

#include <string>
extern "C"
{
#include <libpdbg.h>
}
namespace errl
{
namespace sbe_boot_failure
{
std::optional<std::unique_ptr<ErrlHandle>> create(pdbg_target* target) noexcept
{
    if (!target)
    {
        logger::error("SBEBootFailureError::create - Null target");
        return std::nullopt;
    }

    std::string errMsg = "org.open_power.Processor.Error.SbeBootFailure";
    uint32_t chipPos = pdbg_target_index(target);

    auto handle = std::make_unique<ErrlHandle>();
    ATTR_CHIP_ID_type chipId;
    if (DT_GET_PROP(ATTR_CHIP_ID_type, target, chipId))
    {
        logger::error("Failed to read chip ID from target");
        return std::nullopt;
    }

    ATTR_TYPE_type chipType;
    if (DT_GET_PROP(ATTR_TYPE_type, target, chipType))
    {
        logger::error("Failed to read chip type from target");
        return std::nullopt;
    }

    auto handle =
        sbeffdc::processSbeFfdc(target, chipId, chipType, errMsg, chipPos);
    if (!handle)
    {
        logger::error("SBEBootFailureError: Failed to process FFDC");
        return std::nullopt;
    }
    return handle;
}
}
} // namespace errl
