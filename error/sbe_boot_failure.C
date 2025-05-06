#include <attributes_info.H>
#include <errl_util.H>
#include <libphal.H>
#include <phal_exception.H>
#include <sbe_boot_failure.H>
#include <sbe_error_helper.H>
#include <target.H>

#include <log.hpp>

#include <string>
extern "C"
{
#include <libpdbg.h>
}
namespace errl::sbe_boot_failure
{
std::optional<std::unique_ptr<ErrlHandle>> create(pdbg_target* target)
{
    assert(target && "sbe_boot_failure create: target is null");

    std::string errMsg = "org.open_power.Processor.Error.SbeBootFailure";
    return errl::sbe_err_helper::create(errMsg, target);
}
} // namespace errl::sbe_boot_failure
