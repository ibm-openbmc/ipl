#pragma once
extern "C"
{
#include <libpdbg.h>
}
#include <optional>
#include <attributes_info.H>
namespace pdbg::util
{

/** @brief Find a hardware target by physical binary path.
 *
 *  Searches under the specified parent target for a child whose physical binary path
 *  matches the provided reference path.
 *
 *  @param[in] parent - The parent hardware target to search under.
 *  @param[in] refPath - The reference physical binary path to match.
 *  @return std::optional containing a pointer to the matching target if found,
 *          std::nullopt otherwise.
 */
std::optional<pdbg_target*> findTargetByPhysBinPath(
    struct pdbg_target* parent, const ATTR_PHYS_BIN_PATH_Type& refPath);

/** @brief Retrieve location code for a hardware target recursive till found
 *
 *  Fetches the location code for the specified target and prints it to stdout.
 *
 *  @param[in] target - The hardware target whose location code is to be retrieved.
 *  @param[out] locCode - The location code attribute used for lookup.
 */
void getLocationCode(struct pdbg_target* target, ATTR_LOCATION_CODE_Type& locCode);
} // namespace pdbg::util
