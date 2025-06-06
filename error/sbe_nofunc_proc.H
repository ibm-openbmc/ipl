#pragma once
#include <errl_handle.H>

#include <memory>
extern "C"
{
#include <libpdbg.h>
}
namespace errl::sbe_nofunc_proc
{
/** @brief Create an no functional primary proc SBE boot failure.
 *
 *  Constructs and returns an error log handle specific to a Self Boot Engine
 * (SBE) boot failure scenario on the provided hardware target.
 *
 *  @return std::optional containing a unique_ptr to ErrlHandle if creation
 * succeeds, std::nullopt otherwise.
 */
std::optional<std::unique_ptr<ErrlHandle>> create();
} // namespace errl::sbe_nofunc_proc
