#pragma once

#include <errl_handle.H>
extern "C"
{
#include <libpdbg.h>
}

namespace errl
{
namespace factory
{
ErrlHandle createNoFunctionalTarget(pdbg_target* target);
}
} // namespace errl
