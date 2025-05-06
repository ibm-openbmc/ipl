#include "no_func_target_error.H"

#include <string> // For string literals, if needed.
extern "C"
{
#include <libpdbg.h>
}
namespace no_func_target
{
std::optional<std::unique_ptr<ErrlHandle>> create(pdbg_target* target)
{
    (void)target; // If not used yet, silence unused parameter warning.

    return std::make_unique<ErrlEntry>(
        "org.open_power.PHAL.Error.NoFunctionalBootProc", fapi2::INFORMATIONAL,
        std::unordered_map<std::string, std::string>{},
        );
}
}
