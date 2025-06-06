#include <attributes_info.H>
#include <errl_util.H>
#include <libphal.H>
#include <phal_exception.H>
#include <poz_sbe_chipop_failure.H>
#include <sbe_error_helper.H>
#include <target.H>

#include <log.hpp>

#include <string>
extern "C"
{
#include <libpdbg.h>
}
namespace errl::poz_sbe_chipop_failure
{
std::optional<std::unique_ptr<ErrlHandle>> create(pdbg_target* target)
{
    assert(target && "poz_sbe_chipop_failure create: target is null");

    std::string errMsg = "org.open_power.OCMB.Error.SbeChipOpFailure";
    return errl::sbe_err_helper::create(errMsg, target);
}
} // namespace errl::poz_sbe_chipop_failure
