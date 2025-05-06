/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* Host Firmware for POWER Systems Project                                */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2025                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#pragma once
#include <return_code.H>
#include <target.H>
#include <target_types.H>

#include <vector>

namespace sbei::oper
{
/**@brief Transport the command to SBE and recv response
 * @tparam T - target type
 * @param [in] target - HW target to operate on
 * @param [in] cmd - SBE Fifo command
 * @param [in] timeout - max time to wait to read the response
 * @param [out] out - response read from the hardware target
 * @return  fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
 */
template <fapi2::TargetType T>
fapi2::ReturnCode transport(const fapi2::Target<T>& target,
                            const std::vector<std::byte>& cmd,
                            const int timeout, std::vector<std::byte>& out);
} // namespace sbei::oper

#include <plat_sbe_oper.H>
