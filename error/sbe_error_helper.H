#pragma once
#include <errl_handle.H>

#include <memory>
extern "C"
{
#include <libpdbg.h>
}
namespace errl::sbe_err_helper
{
/** @brief Create an error handle for an SBE failure.
 *
 *  Constructs and returns an error log handle specific to a Self Boot Engine
 * (SBE) failure scenario on the provided hardware target.
 *  @param[in] msg - error event/message
 *  @param[in] target - The hardware target (pdbg_target) associated with the
 * failure.
 *  @return std::optional containing a unique_ptr to ErrlHandle if creation
 * succeeds, std::nullopt otherwise.
 */
std::optional<std::unique_ptr<ErrlHandle>>
    create(std::string_view msg, pdbg_target* target);
} // namespace errl::sbe_err_helper
