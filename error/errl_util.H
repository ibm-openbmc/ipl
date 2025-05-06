#pragma once

#include <libekb.H>

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>
#include <utility>

namespace errl::util
{
using json = nlohmann::json;
/**
 * @brief Converts FFDC (First Failure Data Capture) in FAPI format into
 *        Platform Event Log (PEL) format.
 *
 * @param[in]  ffdc   The FFDC data captured by the HWP.
 * @param[out] callout JSON array to be populated with callout entries
 *                     in PEL-compatible format.
 * @param[out] userData  Key-value pairs of additional FFDC user data
 *                       to be added to the PEL.
 * @return 0 on success 1 on failure
 */
int convertFAPItoPELformat(
    const FFDC& ffdc, json& callout,
    std::unordered_map<std::string, std::string>& userData);

} // namespace errl::util
