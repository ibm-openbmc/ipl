#include <errl_factory.H>
#include <poz_sbe_chipop_failure.H>
#include <sbe_boot_failure.H>
#include <sbe_hwp_failure.H>
#include <sbe_nofunc_proc.H>

namespace errl::factory
{
std::optional<std::unique_ptr<ErrlHandle>>
    createSbeBootFailure(pdbg_target* target)
{
    return errl::sbe_boot_failure::create(target);
}

std::optional<std::unique_ptr<ErrlHandle>> createSbeHWPFailure()
{
    return errl::sbe_hwp_failure::create();
}

std::optional<std::unique_ptr<ErrlHandle>>
    createPozSbeChipOpFailure(pdbg_target* target)
{
    return errl::poz_sbe_chipop_failure::create(target);
}
std::optional<std::unique_ptr<ErrlHandle>> createSbeNoFuncProc()
{
    return errl::sbe_nofunc_proc::create();
}
} // namespace errl::factory
