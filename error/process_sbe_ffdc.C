#include <errl_handle.H>
#include <ffdc_pel_convert.H>
#include <process_sbe_ffdc.H>
#include <sbe_ffdc_common.H>
#include <sbe_intf.H>

#include <log.hpp>

namespace sbeffdc
{
std::optional<std::unique_ptr<errl::ErrlHandle>> processSbeFfdc(
    pdbg_target* target, ATTR_CHIP_ID_Type chipId, ATTR_TYPE_Type chipType,
    const std::string& errMsg, uint32_t chipPos)
{
    sbei::FFDCMap ffdcMap;
    sbei::sbePrimResponse primary;
    sbei::sbeSecondaryResponse secondary;

    auto rc = sbeintf::getFFDC(target, static_cast<uint8_t>(chipId), ffdcMap,
                               primary, secondary);
    if (rc)
    {
        logger::error("processSbeFfdc: getFFDC failed: {}", rc);
        return std::nullopt;
    }

    if (ffdcMap.empty())
    {
        logger::error("processSbeFfdc: no FFDC data found");
        return std::nullopt;
    }

    auto handle = std::make_unique<errl::ErrlHandle>();

    for (const auto& [slid, entries] : ffdcMap)
    {
        json callout = json::array();
        std::vector<std::pair<std::string, std::string>> ffdcUserData;
        fapi2::errlSeverity_t severity = fapi2::FAPI2_ERRL_SEV_UNDEFINED;

        for (const auto& entry : entries)
        {
            if (entry.fapiRc != fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA)
            {
                FFDC parsedFfdc;
                libekb_get_sbe_ffdc(parsedFfdc, entry.fapiRc, entry.data,
                                    chipPos, chipType);

                json ecallout;
                std::vector<std::pair<std::string, std::string>> effdcUserData;

                ffdc::convertFfdcToPel(parsedFfdc, effdcUserData, ecallout);

                callout.push_back(std::move(ecallout));
                ffdcUserData.insert(
                    ffdcUserData.end(),
                    std::make_move_iterator(effdcUserData.begin()),
                    std::make_move_iterator(effdcUserData.end()));

                if (severity > entry.severity)
                {
                    severity = entry.severity;
                }
            }
        }

        ffdcUserData.emplace_back("SRC6", std::to_string(chipPos << 16));
        auto errl = std::make_unique<errl::ErrlEntry>(errMsg, severity,
                                                      ffdcUserData, callout);
        handle->addEntry(std::move(errl));
    }

    return handle;
}
} // namespace sbeffdc
