#pragma once

#include <libekb.H>

#include <nlohmann/json.hpp>

#include <string>
#include <utility>
#include <vector>

namespace ffdc
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
 */
void convertFAPItoPELformat(
    const FFDC& ffdc, json& callout,
    std::vector<std::pair<std::string, std::string>>& userData);

} // namespace ffdc
