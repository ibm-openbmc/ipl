#pragma once

#include <errl_handle.H>

#include <memory>
extern "C"
{
#include <libpdbg.h>
}

namespace errl::factory
{
/** @brief Create an error log for an SBE boot failure.
 *
 *  Constructs and returns an error log entry specific to SBE (Self Boot Engine)
 *  boot failure scenarios on the given target.
 *
 *  @param[in] target - The hardware target (pdbg_target) associated with the
 * failure.
 *  @return std::optional containing a unique_ptr to ErrlHandle if creation
 * succeeds, std::nullopt otherwise.
 */
std::optional<std::unique_ptr<ErrlHandle>>
    createSbeBootFailure(pdbg_target* target);

/** @brief Create an error log for an POZ SBE chipop failure.
 *
 *  Constructs and returns an error log entry specific to SBE (Self Boot Engine)
 *  chipop failure scenarios on the given target.
 *
 *  @param[in] target - The hardware target (pdbg_target) associated with the
 * failure.
 *  @return std::optional containing a unique_ptr to ErrlHandle if creation
 * succeeds, std::nullopt otherwise.
 */
std::optional<std::unique_ptr<ErrlHandle>>
    createPozSbeChipOpFailure(pdbg_target* target);

/** @brief Create an error log for an SBE non functional processor
 *
 *  Constructs and returns an error log entry specific to SBE (Self Boot Engine)
 *  non functional processor scenarios.
 *
 *  @return std::optional containing a unique_ptr to ErrlHandle if creation
 * succeeds, std::nullopt otherwise.
 */
std::optional<std::unique_ptr<ErrlHandle>> createSbeNoFuncProc();

/** @brief Create an error log for an SBE HWP failure.
 *
 *  Constructs and returns an error log entry specific to SBE (Self Boot Engine)
 *  HWP failure scenarios on the given target.
 *
 *  @param[in] target - The hardware target (pdbg_target) associated with the
 * failure.
 *  @return std::optional containing a unique_ptr to ErrlHandle if creation
 * succeeds, std::nullopt otherwise.
 */
std::optional<std::unique_ptr<ErrlHandle>> createSbeHWPFailure();

} // namespace errl::factory
