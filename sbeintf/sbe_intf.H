/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* Host Firmware for POWER Systems Project                                */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020,2025                        */
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
#include <sbe_constants.H>
#include <sbe_ffdc_common.H>

#include <cstdint>
#include <vector>
namespace sbei
{
// SBE GetScom Command & Response Format
// Request (to SBE):
// Command Length in words 0x04
// Reserved     | Chip ID   | 0xA2  | 0x01
// Register Address (bits 0..31)
// Register Address (bits 32..63)

// Response (from SBE):
// 0 : Register Data (bits 0..31)
// 1 : Register Data (bits 32..63)
// 2 : 0xC0DE0000   | 0xA2  | 0x01
// 3 : Primary Status | Secondary Status
// 4..N-2 : FFDC Packages (optional)
// N-1 : Distance (in words) to status header (Word 2 with 0xC0DE)
/**@brief Read a SCOM register from a chip
 * @tparam T - target type
 * @param [in] target - HW target to operate on
 * @param [in] chipId - Id of the target chip
 * @param [in] address - SCOM register address to read data from
 * @param [out] value - Value read from the Hardware target
 * @param [out] primary - primary sbe status response
 * @param [out] secondary - secondary sbe status response
 * @param [out] ffdc - FFDC data if requested and if received
 * @return  fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
 */
template <fapi2::TargetType T>
fapi2::ReturnCode
    getScom(const fapi2::Target<T>& target, const uint8_t chipId,
            const uint64_t address, uint64_t& value,
            sbei::sbePrimResponse& primary,
            sbei::sbeSecondaryResponse& secondary, FFDCMapOpt* ffdc = nullptr);

// SBE Execute Istep Command & Response Format
//
//  Request (to SBE):
//  Command Length in words 0x03
//  Reserved     | Chip ID   | 0xA1  | 0x01
//  Reserved     | major |   reserved | minor
//  Response (from SBE):
//  2 : 0xC0DE0000 | 0xA1 | 0x01
//  3 : Primary Status | Secondary Status
//  4..N-2 : FFDC Packages (optional)
//  N-1 : Distance (in words) to status header (Word 2 with 0xC0DE)
//
/**@brief excute istep on a chip
 * @tparam T - target type
 * @param [in] target - HW target to operate on
 * @param [in] chipId - Id of the target chip
 * @param [in] major - major istep number
 * @param [in] minor - minor istep number
 * @param [out] primary - primary sbe status response
 * @param [out] secondary - secondary sbe status response
 * @param [out] ffdc - FFDC data if requested and if received
 * @return  fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
 */
template <fapi2::TargetType T>
fapi2::ReturnCode executeIstep(
    const fapi2::Target<T>& target, const uint8_t chipId, const uint8_t major,
    const uint8_t minor, sbei::sbePrimResponse& primary,
    sbei::sbeSecondaryResponse& secondary, FFDCMapOpt* ffdc = nullptr);

// Get SBE FFDC command and response format
//
//  Request (to SBE):
//  Command Length in words 0x02
//  Reserved     | Chip ID   | 0xA8  | 0x01
//
//  Response (from SBE):
//  0 : FFDC package 0
//  N : FFDC package N
//  2 : 0xC0DE0000   | 0xA8  | 0x01
//  3 : Primary Status | Secondary Status
//  N-1 : Distance (in words) to status header (Word 2 with 0xC0DE)
/**@brief Get SBE ffdc data
 * @tparam T - target type
 * @param [in] target - HW target to operate on
 * @param [in] chipId - Id of the target chip
 * @param [out] ffdc - FFDC data
 * @param [out] primary - primary sbe status response
 * @param [out] secondary - secondary sbe status response
 * @return  fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
 */
template <fapi2::TargetType T>
fapi2::ReturnCode getFFDC(const fapi2::Target<T>& target, uint8_t chipId,
                          FFDCMap& ffdc, sbei::sbePrimResponse& primary,
                          sbei::sbeSecondaryResponse& secondary);
} // namespace sbei

// template instantiation
#include "sbe_get_ffdc.H"
#include "sbe_istep.H"
#include "sbe_scom.H"
