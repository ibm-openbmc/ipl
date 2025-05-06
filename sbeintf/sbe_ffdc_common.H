#pragma once

#include <cstddef> // for std::byte
#include <cstdint>
#include <map>
#include <optional>
#include <vector>
#include <error_info_defs.H>
namespace sbei
{
using Slid = uint16_t;
struct FfdcEntry
{
    std::vector<std::byte> data;
    uint32_t fapiRc;
    fapi2::errlSeverity_t severity;
};
using FfdcMap = std::map<Slid, std::vector<FfdcEntry>>;
using FfdcMapOpt = std::optional<FfdcMap>;

struct pozFfdcHeader
{
    uint32_t magicByte:16;
    uint32_t lengthInWords:16;
    uint32_t seqId:16;
    uint32_t cmdClass:8;
    uint32_t cmd:8;
    uint32_t slid:16;
    uint32_t severity:8;
    uint32_t chipId:8;
    uint32_t fapiRc;
} __attribute__((packed));
} // namespace sbei
