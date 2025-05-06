#pragma once
#include <errl_handle.H>

#include <memory>
extern "C"
{
#include <libpdbg.h>
}
namespace errl::poz_sbe_chipop_failure
{
/** @brief Create an error handle for an SBE chipop failure.
 *
 *  Constructs and returns an error log handle specific to a Self Boot Engine
 * (SBE) chipop failure scenario on the provided hardware target.
 *
 *  @param[in] target - The hardware target (pdbg_target) associated with the
 * failure.
 *  @return std::optional containing a unique_ptr to ErrlHandle if creation
 * succeeds, std::nullopt otherwise.
 */
std::optional<std::unique_ptr<ErrlHandle>> create(pdbg_target* target);
} // namespace errl::poz_sbe_chipop_failure
