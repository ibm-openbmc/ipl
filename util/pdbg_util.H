#pragma once
extern "C"
{
#include <libpdbg.h>
}
#include <attributes_info.H>
namespace pdbg::util
{

std::optional<pdbg_target*> findTargetByPhysBinPath(
    struct pdbg_target* parent, const ATTR_PHYS_BIN_PATH_Type& refPath);

std::optional<ATTR_LOCATION_CODE_Type>
    getLocationCode(struct pdbg_target* target);
} // namespace pdbg::util
