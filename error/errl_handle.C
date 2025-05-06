#include <errl_handle.H>

namespace errl
{
void ErrlHandle::addEntry(std::unique_ptr<ErrlEntry> entry) noexcept
{
    entries.push_back(std::move(entry));
}

bool ErrlHandle::hasEntries() const noexcept
{
    return !entries.empty();
}

const std::vector<std::unique_ptr<ErrlEntry>>&
    ErrlHandle::getEntries() const noexcept
{
    return entries;
}
} // namespace errl
