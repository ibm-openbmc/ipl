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
#include <sbe_oper.H>
#include <sys/ioctl.h>

#include <cstdint>
#include <cstring>
#include <vector>

#ifdef PHAL
extern "C"
{
#include <libpdbg.h>
}

namespace sbei::oper
{

enum class SBEIoctl : std::uint32_t
{
    SbefifoReadTimeout = _IOW('s', 0x00, unsigned int),
    // Future ioctls can be added here
};

namespace internal
{

inline int sbefifoRead(int fd, std::vector<std::byte>& out)
{
    out.resize(MAX_SBE_RESP_SIZE);
    ssize_t n = read(fd, out.data(), out.size());

    if (n < 0)
    {
        logger::error("sbefifoRead: read error n={} errno={} {}", n, errno,
                      strerror(errno));
        return EIO;
    }

    out.resize(n); // Trim buffer to actual size read
    logger::info("sbefifoRead: read {} bytes", out.size());
    return 0;
}

inline int sbefifoWrite(int fd, const std::vector<std::byte>& cmd)
{
    ssize_t n = write(fd, cmd.data(), cmd.size());

    if (n < 0)
    {
        logger::error("sbefifoWrite: write error n={} errno={} {}", n, errno,
                      strerror(errno));
        return EIO;
    }

    if (static_cast<size_t>(n) != cmd.size())
    {
        logger::error("sbefifoWrite: partial write n={} expected={}", n,
                      cmd.size());
        return EIO;
    }

    return 0;
}
} // namespace internal

template <fapi2::TargetType T>
fapi2::ReturnCode transport(const fapi2::Target<T>& target,
                            const std::vector<std::byte>& cmd, int timeout,
                            std::vector<std::byte>& out)
{
    const int fd = pdbg_get_backend_fd(target);
    if (fd == -1)
    {
        logger::error("transport: invalid backend fd for target {}",
                      pdbg_target_path(target));
        return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
    }

    if (int rc = ioctl(fd,
                       static_cast<unsigned long>(SBEIoctl::SbefifoReadTimeout),
                       &timeout);
        rc != 0)
    {
        logger::error(
            "transport: ioctl set timeout failed rc={} timeout={} errno={} {}",
            rc, timeout, errno, strerror(errno));
        return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
    }

    if (int rc = internal::sbefifoWrite(fd, cmd); rc != 0)
    {
        logger::error("transport: sbefifoWrite failed rc={}", rc);
        return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
    }

    out.clear();
    if (int rc = internal::sbefifoRead(fd, out); rc != 0)
    {
        logger::error("transport: sbefifoRead failed rc={}", rc);
        return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
    }
    return fapi2::FAPI2_RC_SUCCESS;
}
} // namespace sbei::oper
#endif // PHAL
