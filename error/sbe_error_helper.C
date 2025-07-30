#include <attributes_info.H>
#include <errl_util.H>
#include <libphal.H>
#include <phal_exception.H>
#include <sbe_boot_failure.H>
#include <target.H>

#include <log.hpp>

#include <string>
extern "C"
{
#include <libpdbg.h>
}
namespace errl::sbe_err_helper
{
std::optional<std::unique_ptr<ErrlHandle>>
    create(std::string_view msg, pdbg_target* target)
{
    assert(target && "sbe_boot_failure create: target is null");
    uint32_t chipPos = pdbg_target_index(target);

    ATTR_CHIP_ID_Type chipId;
    if (DT_GET_PROP(ATTR_CHIP_ID, target, chipId))
    {
        logger::error("sbe_boot_failure: failed to read chip ID from target");
        return std::nullopt;
    }

    ATTR_TYPE_Type chipType;
    if (DT_GET_PROP(ATTR_TYPE, target, chipType))
    {
        logger::error("sbe_boot_failure :failed to read chip type from target");
        return std::nullopt;
    }

    if (chipType != ENUM_ATTR_TYPE_PROC)
    {
        logger::error("sbe_boot_failure: not a proc chip");
        return std::nullopt;
    }

    std::vector<openpower::phal::sbe::FFDCEntry> lientry;
    openpower::phal::sbeError_t error = capturePfutFFDC(target, lientry);
    if (!((error.errType() == openpower::phal::exception::SBE_CMD_TIMEOUT) ||
          (error.errType() == openpower::phal::exception::SBE_CMD_FAILED) ||
          (error.errType() ==
           openpower::phal::exception::SBE_INTERNAL_FFDC_DATA)))
    {
        logger::error("sbe_boot_failure: failure in captureProcFFDC");
        return std::nullopt;
    }
    auto handle = std::make_unique<errl::ErrlHandle>();

    for (const auto& entry : lientry)
    {
        std::unordered_map<std::string, std::string> ffdcUserData;
        if (entry.fapiRc == fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA)
        {
            logger::error("sbe_boot_failure: plat error ignore");
            continue;
        }

        FFDC parsedFfdc;
        (void)libekb_parse_sbe_ffdc_pkt(entry.fapiRc, entry.data, chipPos,
                                        chipType, parsedFfdc);

        json callout = json::array();
        std::unordered_map<std::string, std::string> effdcUserData;

        errl::util::convertFAPItoPELformat(parsedFfdc, callout, effdcUserData);

        ffdcUserData.insert(std::make_move_iterator(effdcUserData.begin()),
                            std::make_move_iterator(effdcUserData.end()));
        ffdcUserData.emplace("SRC6", std::to_string(chipPos << 16));
        ffdcUserData.emplace("_PID", std::to_string(getpid()));
        auto errl = std::make_unique<errl::ErrlEntry>(
            msg, ffdcUserData, std::make_optional(std::move(callout)),
            std::make_optional(std::move(entry.data)));
        handle->addEntry(std::move(errl));
    }
    return handle;
}
} // namespace errl::sbe_err_helper
