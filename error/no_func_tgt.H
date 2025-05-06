#pragma once

#include "errl_entry.H"

#include <memory>

struct pdbg_target; // Forward declaration

namespace no_func_target
{
    std::optional<std::unique_ptr<ErrlHandle>> create(pdbg_target* target);
};
