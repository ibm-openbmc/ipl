#include <pdbg_util.H>

#include <cassert>
#include <cstring>
#include <optional>

#include <log.hpp>

namespace pdbg::util
{
std::optional<pdbg_target*> findTargetByPhysBinPath(
    struct pdbg_target* target, const ATTR_PHYS_BIN_PATH_Type& refPath)
{
    assert(target && "findTargetByPhysBinPath: target is null");

    ATTR_PHYS_BIN_PATH_Type physBinPath;
    if (DT_GET_PROP(ATTR_PHYS_BIN_PATH, target, physBinPath) == 0 &&
        std::memcmp(physBinPath, refPath, sizeof(physBinPath)) == 0)
    {
        return target;
    }

    struct pdbg_target* child = nullptr;
    pdbg_for_each_child_target(target, child)
    {
        if (auto found = findTargetByPhysBinPath(child, refPath))
        {
            return found;
        }
    }

    return std::nullopt;
}

void getLocationCode(struct pdbg_target* target, ATTR_LOCATION_CODE_Type& locCode)
{
    assert(target && "getLocationCode: target is null");
    if (DT_GET_PROP(ATTR_LOCATION_CODE, target, locCode) != 0)
    {
        return;
    }
    return getLocationCode(pdbg_target_parent(nullptr, target), locCode);
}

} // namespace pdbg::util
