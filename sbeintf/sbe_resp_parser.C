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
#include <sbe_constants.H>
#include <sbe_resp_parser.H>

#include <log.hpp>

#include <cstring>

namespace sbei::internal
{
// FFDC Package Format
//
//               Byte 0       |   Byte 1      |   Byte 2      |   Byte 3
// ---------------------------------------------------------------------------
// Word 0  :   Magic Bytes: 0xFBAD            | Length in words (N+5)
// Word 1  :   Sequence ID                    | Command Class | Command
// Word 2  :   SLID                           | Severity      | Chip ID
// Word 3  :   Return Code (bits 0–31)
// Word 4  :   FFDC Data – Word 0
// Word 5  :   FFDC Data – Word 1
// ...
// Word N+4:   FFDC Data – Word N
int parseSBEFFDC(const std::vector<std::byte>& buf, size_t offset,
                 size_t endOffset, FfdcMap& ffdc)
{
    ffdc.clear();
    const size_t buflen = buf.size();

    while (offset + sizeof(sbei::pozFfdcHeader) <= endOffset)
    {
        // Parse header
        sbei::pozFfdcHeader hdr{};
        std::memcpy(&hdr, &buf[offset], sizeof(hdr));

        const uint16_t magic = be16toh(hdr.magicByte);
        const uint16_t lengthWords = be16toh(hdr.lengthInWords);

        if (magic != FFDC_MAGIC || lengthWords < FFDC_HEADER_LEN)
        {
            logger::error("Invalid FFDC magic or length: magic=0x{:04x}, len={}",
                          magic, lengthWords);
            return EPROTO;
        }

        const size_t packageBytes = lengthWords * sizeof(uint32_t);
        if (offset + packageBytes > endOffset)
        {
            logger::error("FFDC package overruns buffer: offset={}, size={}, buflen={}",
                          offset, packageBytes, buflen);
            return EPROTO;
        }

        const sbei::Slid slid = be16toh(hdr.slid);
        const uint32_t fapiRc = be32toh(hdr.fapiRc);
        const auto severity = static_cast<fapi2::errlSeverity_t>(hdr.severity);

        // FFDC data starts after header
        const size_t headerBytes = sizeof(sbei::pozFfdcHeader);
        const size_t dataBytes = packageBytes - headerBytes;
        const size_t dataOffset = offset + headerBytes;

        std::vector<std::byte> ffdcData(buf.begin() + dataOffset,
                                        buf.begin() + dataOffset + dataBytes);

        ffdc[slid].emplace_back(sbei::FfdcEntry{
            .data = std::move(ffdcData),
            .fapiRc = fapiRc,
            .severity = severity
        });

        offset += packageBytes;
    }
    return 0;
}


int parseSBEResponse(const std::vector<std::byte>& buf,
                     std::vector<std::byte>& value,
                     sbei::sbePrimResponse& primary,
                     sbei::sbeSecondaryResponse& secondary, FFDCMapOpt* ffdc)
{
    const size_t buflen = buf.size();

    if (buflen < SBEFIFO_MIN_RESP_LEN)
    {
        logger::error("parseSBEResponse: buffer too small: {}", buflen);
        return EPROTO;
    }

    uint32_t offsetWord = 0;
    std::memcpy(&offsetWord, &buf[buflen - WORD_SIZE], sizeof(offsetWord));
    offsetWord = be32toh(offsetWord);

    const size_t headerOffset = buflen - (offsetWord * WORD_SIZE);
    if (headerOffset + 2 * WORD_SIZE > buflen)
    {
        logger::error("parseSBEResponse: invalid header offset: {}",
                      headerOffset);
        return EPROTO;
    }

    uint32_t header = 0;
    std::memcpy(&header, &buf[headerOffset], sizeof(header));
    header = be32toh(header);

    if ((header & MAGIC_MASK) != MAGIC_HEADER)
    {
        logger::error("parseSBEResponse: invalid magic header 0x{:08x}",
                      header);
        return EPROTO;
    }

    uint32_t status = 0;
    std::memcpy(&status, &buf[headerOffset + WORD_SIZE], sizeof(status));
    status = be32toh(status);

    sbei::status::StatusWord sw{.full = status};
    primary = static_cast<sbei::sbePrimResponse>(sw.primary);
    secondary = static_cast<sbei::sbeSecondaryResponse>(sw.secondary);

    value.clear();
    if (headerOffset > 0)
    {
        value.insert(value.end(), buf.begin(), buf.begin() + headerOffset);
    }

    if (ffdc && ffdc->has_value() && (headerOffset + 2 * WORD_SIZE < buflen))
    {
        const size_t offset = headerOffset + 2 * WORD_SIZE;
        const size_t endOffset = buflen - WORD_SIZE;

        auto& ffdcMap = ffdc->value();
        const int rc = parseSBEFFDC(buf, offset, endOffset, ffdcMap);
        if (rc)
        {
            logger::error("parseSBEResponse: ffdc invalid format rc 0x{:08x}",
                          rc);
            return rc;
        }
    }
    return 0;
}
} // namespace sbei::internal
