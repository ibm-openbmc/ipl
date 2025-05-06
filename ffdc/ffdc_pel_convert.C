extern "C"
{
#include <libpdbg.h>
}

#include <ffdc_converter.H>

namespace ffdc
{

namespace internal
{
/**
 * @brief GET PEL priority from pHAL priority
 *
 * The pHAL callout priority is in different format than PEL format
 * so, this api is used to return current phal supported priority into
 * PEL expected format.
 *
 * @param[in] phalPriority used to pass phal priority format string
 *
 * @return pel priority format string else empty if failure
 *
 * @note For "NONE" returning "L" (LOW)
 */
static std::string getPelPriority(const std::string& phalPriority)
{
    const std::map<std::string, std::string> priorityMap = {
        {"HIGH", "H"}, {"MEDIUM", "M"}, {"LOW", "L"}, {"NONE", "L"}};

    auto it = priorityMap.find(phalPriority);
    if (it == priorityMap.end())
    {
        logger::error(
            "Unsupported phal priority({}) is given to get pel priority format",
            phalPriority);
        return "H";
    }

    return it->second;
}
} // namespace internal

int convertFfdcToPel(const FFDC& ffdc, json& callout,
                     std::vector<std::pair<std::string, std::string>>& userData)
{
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

    // Add raw HWP FFDC data
    for (const auto& [key, value] : ffdc.hwp_errorinfo.ffdcs_data)
    {
        userData.emplace_back("HWP_FFDC_" + key, value);
    }

    // HW callouts
    int calloutCount = 0;
    for (const auto& hwCallout : ffdc.hwp_errorinfo.hwcallouts)
    {
        ++calloutCount;
        std::string prefix = std::format("HWP_HW_CO_{:02}_", calloutCount);

        userData.emplace_back(prefix + "HW_ID", hwCallout.hwid);
        userData.emplace_back(prefix + "PRIORITY", hwCallout.callout_priority);

        auto target = findTargetByPhysBinPath(pdbg_target_root(),
                                              hwCallout.target_entity_path);
        if (!target)
        {
            logger::error("Failed to find target by entity path");
            continue;
        }

        auto locCodeOpt = pdbg::util::getLocationCode(target);
        if (locCodeOpt)
        {
            userData.emplace_back(prefix + "LOC_CODE", *locCodeOpt);
        }
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
            std::string pelPriority =
                getPelPriority(hwCallout.callout_priority);
            json jsonCalloutData;

            // Inventory path for planar
            jsonCalloutData["InventoryPath"] =
                "/xyz/openbmc_project/inventory/system/chassis/motherboard";
            jsonCalloutData["Deconfigured"] = false;
            jsonCalloutData["Guarded"] = false;
            jsonCalloutData["Priority"] = pelPriority;
            callout.emplace_back(std::move(jsonCalloutData));
        }
    }

    // CDG targets
    calloutCount = 0;
    for (const auto& cdg : ffdc.hwp_errorinfo.cdg_targets)
    {
        ++calloutCount;
        std::string prefix = std::format("HWP_CDG_TGT_{:02}_", calloutCount);

        auto target =
            findTargetByPhysBinPath(pdbg_target_root(), cdg.target_entity_path);
        if (!target)
        {
            logger::info("Failed to find target by entity path");
            continue;
        }

        auto locCodeOpt = pdbg::util::getLocationCode(target);
        if (locCodeOpt)
        {
            userData.emplace_back(prefix + "LOC_CODE", *locCodeOpt);
        }
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

        json calloutJson;
        if (locCodeOpt)
        {
            calloutJson["LocationCode"] = *locCodeOpt;
        }
        calloutJson["Priority"] = getPelPriority(cdg.callout_priority);

        ATTR_MRU_ID_Type mruId;
        if (DT_GET_PROP(ATTR_MRU_ID, target, mruId) == 0)
        {
            calloutJson["MRUs"] = json::array({
                {{"ID", mruId}, {"Priority", calloutJson["Priority"]}},
            });
        }

        calloutJson["Deconfigured"] = cdg.deconfigure;
        calloutJson["Guarded"] = cdg.guard;
        calloutJson["GuardType"] = cdg.guard_type;
        calloutJson["EntityPath"] = cdg.target_entity_path;

        callout.emplace_back(std::move(calloutJson));
    }

    // Procedure callouts
    calloutCount = 0;
    for (const auto& proc : ffdc.hwp_errorinfo.procedures_callout)
    {
        ++calloutCount;
        std::string prefix = std::format("HWP_PROC_CO_{:02}_", calloutCount);

        userData.emplace_back(prefix + "PRIORITY", proc.callout_priority);
        userData.emplace_back(prefix + "MAINT_PROCEDURE", proc.proc_callout);

        json calloutJson;
        calloutJson["Procedure"] = proc.proc_callout;
        calloutJson["Priority"] = getPelPriority(proc.callout_priority);
        callout.emplace_back(std::move(calloutJson));
    }
    return 0;
}
} // namespace ffdc
