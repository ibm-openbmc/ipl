#include <attributes_info.H>
#include <ffdc_pel_convert.H>
#include <process_sbe_ffdc.H>
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
std::optional<std::unique_ptr<ErrlHandle>> create(pdbg_target* target)
{
    assert(target && "sbe_boot_failure create: target is null");

    std::string errMsg = "org.open_power.Processor.Error.SbeBootFailure";
    uint32_t chipPos = pdbg_target_index(target);

    auto handle = std::make_unique<ErrlHandle>();
    ATTR_CHIP_ID_type chipId;
    if (DT_GET_PROP(ATTR_CHIP_ID_Type, target, chipId))
    {
        logger::error("sbe_boot_failure: failed to read chip ID from target");
        return std::nullopt;
    }

    ATTR_TYPE_type chipType;
    if (DT_GET_PROP(ATTR_TYPE_Type, target, chipType))
    {
        logger::error("sbe_boot_failure :failed to read chip type from target");
        return std::nullopt;
    }

    auto handle =
        sbeffdc::processSbeFfdc(target, chipId, chipType, errMsg, chipPos);
    if (!handle)
    {
        logger::error("sbe_boot_failure: Failed to process FFDC");
        return std::nullopt;
    }
    return handle;
}
} // namespace sbe_boot_failure
} // namespace errl
