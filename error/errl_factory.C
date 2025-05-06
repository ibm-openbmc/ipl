#include <errl_factory.H>
#include <sbe_boot_failure.H>

namespace errl::factory
{
std::optional<std::unique_ptr<ErrlHandle>>
    createSbeBootFailure(pdbg_target* target)
{
    return errl::sbe_boot_failure::create(target);
}
} // namespace errl::factory
