#pragma once

#include "errl_handle.H"

#include <memory>
namespace errl
{
namespace sbe_boot_failure
{
std::optional<std::unique_ptr<ErrlHandle>> create(pdbg_target* target);
};
} // namespace errl
