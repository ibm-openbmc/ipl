#pragma once

#include <attributes_info.H>
#include <errl_handle.H>
#include <target_types.H>

#include <optional>
#include <string>

extern "C"
{
#include <libpdbg.h>
}

namespace sbeffdc
{
/** @brief Process and generate error log for SBE FFDC data.
 *
 *  This function processes Self Boot Engine (SBE) First Failure Data Capture
 * (FFDC) for the specified target and generates an appropriate error log entry.
 *
 *  @param[in] target - Hardware target associated with the SBE FFDC.
 *  @param[in] chipId - Identifier of the chip on the target.
 *  @param[in] chipType - Type of the chip (e.g., processor, memory buffer).
 *  @param[in] errMsg - Descriptive error message to be associated with the log.
 *  @param[in] chipPos - Position of the chip in the system.
 *  @return std::optional containing a unique_ptr to `errl::ErrlHandle` if FFDC
 * is processed, std::nullopt if there is no FFDC or an error occurs during
 * processing.
 */
std::optional<std::unique_ptr<errl::ErrlHandle>> processSbeFfdc(
    pdbg_target* target, ATTR_CHIP_ID_Type chipId, ATTR_TYPE_Type chipType,
    const std::string& errMsg, uint32_t chipPos);
} // namespace sbeffdc
