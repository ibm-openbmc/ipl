#pragma once
#include <errl_handle.H>

#include <memory>

namespace errl::sbe_hwp_failure
{
/** @brief Create an error handle for a hardware procedure failure.
 *
 *  Constructs and returns an error log handle specific to a hardware
 *  procedure failure during boot scenario on the provided hardware target.
 *
 *  @return std::optional containing a unique_ptr to ErrlHandle if creation
 * succeeds, std::nullopt otherwise.
 */
std::optional<std::unique_ptr<ErrlHandle>> create();
} // namespace errl::sbe_hwp_failure
