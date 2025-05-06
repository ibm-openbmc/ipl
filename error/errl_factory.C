#include <errl_factory.H>

namespace errl::factory
{

ErrlHandle createNoFunctionalTarget(pdbg_target* target)
{
    ErrlHandle handle;
    auto entry = errl::no_func_target::create(target);
    handle.addEntry(std::move(entry));
    return handle;
}

} // namespace errl::factory
