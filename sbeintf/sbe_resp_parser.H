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
#include <sbe_ffdc_common.H>
#include <sbe_resp_status.H>

#include <cstddef>
#include <vector>

namespace sbei::internal
{
/**@brief Parse response buffer received from SBE
 * @param [in] buf - complete response buffer received
 * @param [out] value - value part in the response buffer
 * @param [out] primary - primary sbe status response
 * @param [out] secondary - secondary sbe status response
 * @param [out] ffdc - ffdc part in the reponse buffer
 * @return  0 on success
 */
int parseSBEResponse(const std::vector<std::byte>& buf,
                     std::vector<std::byte>& value,
                     sbei::sbePrimResponse& primary,
                     sbei::sbeSecondaryResponse& secondary, FFDCMapOpt* ffdc);

/**@brief Parse ffdc packets from the buffer
 * @detail generic method to be used for sbe commands and get ffdc command
 * @param [in] buf - complete response buffer received
 * @param [int] offset - offset from where ffdc packet is available
 * @param [int] endOffset - last location of the ffdc packet
 * @param [out] ffdc - ffdc part in the reponse buffer
 * @return  0 on success
 */
int parseSBEFFDC(const std::vector<std::byte>& buf, const size_t offset,
                 const size_t endOffset, FFDCMap& ffdc);

} // namespace sbei::internal
