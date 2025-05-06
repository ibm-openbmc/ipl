#include <pdbg_util.H>

#include <cassert>
#include <cstring>
#include <optional>

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

std::optional<ATTR_LOCATION_CODE_Type>
    getLocationCode(struct pdbg_target* target)
{
    assert(target && "getLocationCode: target is null");

    ATTR_LOCATION_CODE_Type locationCode;
    if (DT_GET_PROP(ATTR_LOCATION_CODE, target, locationCode) == 0)
    {
        return locationCode;
    }

    return getLocationCode(pdbg_target_parent(nullptr, target));
}

} // namespace pdbg::util
#endif
