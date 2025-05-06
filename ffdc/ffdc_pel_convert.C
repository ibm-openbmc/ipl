#include <ffdc_pel_convert.H>
extern "C"
{
#include <libpdbg.h>
}
#include <attributes_info.H>
#include <libekb.H>

#include <log.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <format>
#include <map>
#include <optional>
#include <ranges>
#include <string>

namespace ffdc
{
namespace internal
{
/**
 * @brief Convert pHAL priority string to PEL priority format.
 *
 * @param phalPriority pHAL priority string
 * @return Corresponding PEL priority string
 */
static std::string getPelPriority(std::string_view phalPriority)
{
    static const std::unordered_map<std::string_view, std::string_view>
        priorityMap = {
            {"HIGH", "H"}, {"MEDIUM", "M"}, {"LOW", "L"}, {"NONE", "L"}};

    if (auto it = priorityMap.find(phalPriority); it != priorityMap.end())
    {
        return std::string{it->second};
    }

    logger::error(
        "Unsupported phal priority({}) is given to get pel priority format",
        phalPriority);
    return "H";
}
} // namespace internal

int convertFAPItoPELformat(
    const FFDC& ffdc, json& callout,
    std::vector<std::pair<std::string, std::string>>& userData)
{
    using namespace std::string_literals;

    if (ffdc.ffdc_type != FFDC_TYPE_NONE &&
        ffdc.ffdc_type != FFDC_TYPE_UNSUPPORTED)
    {
        logger::error("Unsupported phal FFDC type to create PEL. MSG: {}",
                      ffdc.message);
        return 1;
    }

    // Basic HWP info
    userData.emplace_back("HWP_RC", ffdc.hwp_errorinfo.rc);
    userData.emplace_back("HWP_RC_DESC", ffdc.hwp_errorinfo.rc_desc);

    // Raw HWP FFDC data
    for (const auto& [key, value] : ffdc.hwp_errorinfo.ffdcs_data)
    {
        userData.emplace_back("HWP_FFDC_" + key, value);
    }

    // HW callouts
    int calloutCount = 0;
    for (const auto& hwCallout : ffdc.hwp_errorinfo.hwcallouts)
    {
        std::string prefix = std::format("HWP_HW_CO_{:02}_", ++calloutCount);

        userData.emplace_back(prefix + "HW_ID", hwCallout.hwid);
        userData.emplace_back(prefix + "PRIORITY", hwCallout.callout_priority);

        auto target = pdbg::util::findTargetByPhysBinPath(
            pdbg_target_root(), hwCallout.target_entity_path);
        if (!target)
        {
            logger::error("Failed to find target by entity path");
            continue;
        }

        ATTR_LOCATION_CODE_Type locCode;
        pdbg::util::getLocationCode(target, locCode);
        userData.emplace_back(prefix + "LOC_CODE", locCode);

        ATTR_PHYS_DEV_PATH_Type physDevPath;
        if (DT_GET_PROP(ATTR_PHYS_DEV_PATH, target, physDevPath) == 0)
        {
            userData.emplace_back(prefix + "PHYS_PATH", physDevPath);
        }

        userData.emplace_back(prefix + "CLK_POS",
                              std::to_string(hwCallout.clkPos));
        userData.emplace_back(prefix + "CALLOUT_PLANAR",
                              hwCallout.isPlanarCallout ? "true" : "false");

        if (hwCallout.isPlanarCallout)
        {
            callout.push_back(
                {{"InventoryPath",
                  "/xyz/openbmc_project/inventory/system/chassis/motherboard"},
                 {"Deconfigured", false},
                 {"Guarded", false},
                 {"Priority",
                  internal::getPelPriority(hwCallout.callout_priority)}});
        }
    }

    // CDG targets
    calloutCount = 0;
    for (const auto& cdg : ffdc.hwp_errorinfo.cdg_targets)
    {
        std::string prefix = std::format("HWP_CDG_TGT_{:02}_", ++calloutCount);

        auto target = pdbg::util::findTargetByPhysBinPath(
            pdbg_target_root(), cdg.target_entity_path);
        if (!target)
        {
            logger::info("Failed to find target by entity path");
            continue;
        }

        ATTR_LOCATION_CODE_Type locCode;
        pdbg::util::getLocationCode(target, locCode);
        userData.emplace_back(prefix + "LOC_CODE", locCode);

        ATTR_PHYS_DEV_PATH_Type physDevPath;
        if (DT_GET_PROP(ATTR_PHYS_DEV_PATH, target, physDevPath) == 0)
        {
            userData.emplace_back(prefix + "PHYS_PATH", physDevPath);
        }

        userData.emplace_back(prefix + "CO_REQ",
                              cdg.callout ? "true" : "false");
        userData.emplace_back(prefix + "CO_PRIORITY", cdg.callout_priority);
        userData.emplace_back(prefix + "DECONF_REQ",
                              cdg.deconfigure ? "true" : "false");
        userData.emplace_back(prefix + "GUARD_REQ",
                              cdg.guard ? "true" : "false");
        userData.emplace_back(prefix + "GUARD_TYPE", cdg.guard_type);

        json calloutJson = {
            {"Priority", internal::getPelPriority(cdg.callout_priority)},
            {"Deconfigured", cdg.deconfigure},
            {"Guarded", cdg.guard},
            {"GuardType", cdg.guard_type},
            {"EntityPath", cdg.target_entity_path}};

        ATTR_MRU_ID_Type mruId;
        if (DT_GET_PROP(ATTR_MRU_ID, target, mruId) == 0)
        {
            calloutJson["MRUs"] = json::array(
                {{{"ID", mruId}, {"Priority", calloutJson["Priority"]}}});
        }

        callout.emplace_back(std::move(calloutJson));
    }

    // Procedure callouts
    calloutCount = 0;
    for (const auto& proc : ffdc.hwp_errorinfo.procedures_callout)
    {
        std::string prefix = std::format("HWP_PROC_CO_{:02}_", ++calloutCount);

        userData.emplace_back(prefix + "PRIORITY", proc.callout_priority);
        userData.emplace_back(prefix + "MAINT_PROCEDURE", proc.proc_callout);

        callout.push_back(
            {{"Procedure", proc.proc_callout},
             {"Priority", internal::getPelPriority(proc.callout_priority)}});
    }

    return 0;
}
} // namespace ffdc
