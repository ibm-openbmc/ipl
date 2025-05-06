#pragma once

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

std::optional<std::unique_ptr<ErrlHandle>> processSbeFfdc(
    pdbg_target* target, ATTR_CHIP_ID_type chipId, ATTR_TYPE_type chipType,
    const std::string& errMsg, uint32_t chipPos);

} // namespace sbeffdc
