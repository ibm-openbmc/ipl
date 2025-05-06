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

#include <endian.h>
#include <sbe_constants.H>
#include <sbe_ffdc_common.H>
#include <sbe_oper.H>
#include <sbe_resp_parser.H>
#include <target.H>
#include <target_types.H>

#include <log.hpp>

#include <array>
#include <cstdint>
#include <vector>

namespace sbei::internal
{

/** @brief SBE FIFO GetFFDC build command */
inline int buildGetFFDCCommand(uint8_t chipId, std::vector<std::byte>& buf)
{
    constexpr uint8_t reserved = 0x00;
    constexpr uint8_t cmdClass =
        sbei::sbeCommandClass::SBE_CMD_CLASS_GENERIC_MESSAGE;
    constexpr uint8_t command = sbei::SbeCommandGenericMsg_t::SBE_CMD_GET_FFDC;
    constexpr size_t wordCount = SBEFIFO_CMD_CLASS_GET_FFDC_LEN;

    const uint32_t cmdWord =
        (reserved << 24) | (chipId << 16) | (cmdClass << 8) | command;

    std::array<uint32_t, wordCount> msg = {
        htobe32(static_cast<uint32_t>(wordCount)),
        htobe32(cmdWord),
    };

    const auto* raw = reinterpret_cast<const std::byte*>(msg.data());
    buf.insert(buf.end(), raw, raw + sizeof(msg));
    return 0;
}

} // namespace sbei::internal

namespace sbei
{

template <fapi2::TargetType T>
fapi2::ReturnCode getFFDC(const fapi2::Target<T>& target, uint8_t chipId,
                          FFDCMap& ffdc, sbei::sbePrimResponse& primary,
                          sbei::sbeSecondaryResponse& secondary)
{
    logger::info("getFFDC chipId {}", chipId);

    std::vector<std::byte> cmdBuf;
    if (int rc = internal::buildGetFFDCCommand(chipId, cmdBuf); rc != 0)
    {
        logger::error("getFFDC: Failed to build command rc={}", rc);
        return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
    }

    std::vector<std::byte> rawResp;
    const int timeout = static_cast<int>(
        sbei::sbeCmdRespTimeout::SBE_CMD_TIMEOUT_SHORT_IN_MSEC);

    if (int rc = oper::transport(target, cmdBuf, timeout, rawResp); rc != 0)
    {
        logger::error("getFFDC: transport failed rc={}", rc);
        return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
    }

    std::vector<std::byte> parsedResp;
    if (int rc = internal::parseSBEResponse(rawResp, parsedResp, primary,
                                            secondary, nullptr);
        rc != 0)
    {
        logger::error("getFFDC: parseSBEResponse failed rc={}", rc);
        return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
    }

    const size_t endOffset =
        parsedResp.size() >= WORD_SIZE ? parsedResp.size() - WORD_SIZE : 0;

    // GET FFDC data starts from 0th location
    if (int rc = internal::parseSBEFFDC(parsedResp, 0, endOffset, ffdc);
        rc != 0)
    {
        logger::error("getFFDC: parseSBEFFDC failed rc={}", rc);
        return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
    }

    return fapi2::FAPI2_RC_SUCCESS;
}

} // namespace sbei
