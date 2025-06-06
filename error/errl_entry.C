#include <errl_entry.H>
#include <unistd.h>

#include <log.hpp>

#include <fstream>
#include <span>
#include <system_error>

namespace errl
{

constexpr uint8_t jsonCalloutSubtype = 0xCA;

namespace fs = std::filesystem;

namespace
{
// Write raw data to a temporary file, return fd and path
std::pair<int, fs::path> writeToTempFile(std::string_view prefix,
                                         const void* data, size_t size)
{
    auto tempPath = fs::temp_directory_path() /
                    (prefix.data() + std::string{"hostfw_XXXXXX"});
    std::string pathTemplate = tempPath.string();
    int fd = ::mkstemp(pathTemplate.data());
    if (fd == -1)
    {
        throw std::system_error(errno, std::generic_category(),
                                "writeToTempFile: mkstemp failed");
    }
    ssize_t written = ::write(fd, data, size);
    if (written < 0 || static_cast<size_t>(written) != size)
    {
        ::close(fd);
        fs::remove(pathTemplate);
        throw std::runtime_error("writeToTempFile: Incomplete write");
    }

    return {fd, fs::path(pathTemplate)};
}
} // unnamed namespace

ErrlEntry::ErrlEntry(std::string_view i_msg,
                     std::unordered_map<std::string, std::string> i_data,
                     std::optional<json> i_callout,
                     std::optional<std::vector<uint8_t>> i_blob) :
    message{i_msg}, additionalData{std::move(i_data)},
    callout{std::move(i_callout)}, blob{std::move(i_blob)}
{
    try
    {
        if (blob)
        {
            auto& vec = *blob;
            auto span = std::span<const uint8_t>(vec.data(), vec.size());
            auto [fd, path] =
                writeToTempFile("errl_blob", span.data(), span.size_bytes());

            PelFFDCfile pf;
            pf.format = FFDCFormat::Custom;
            pf.fd = fd;

            // initilize optional ffdcFiles
            if (!ffdcFiles)
            {
                ffdcFiles.emplace(
                    std::vector<
                        std::pair<PelFFDCfile, std::filesystem::path>>{});
            }
            logger::info("ErrlEntry blob fd {} path {} ", fd, path.string());
            ffdcFiles->emplace_back(std::move(pf), std::move(path));
        }

        if (callout)
        {
            const std::string jsonStr = callout->dump(2);
            auto [fd, path] =
                writeToTempFile("errl_callout", jsonStr.data(), jsonStr.size());

            PelFFDCfile pf;
            pf.format = FFDCFormat::JSON;
            pf.subType = jsonCalloutSubtype;
            pf.version = static_cast<uint8_t>(UserDataFormatVersion::json);
            pf.fd = fd;

            if (!ffdcFiles)
            {
                ffdcFiles.emplace(
                    std::vector<
                        std::pair<PelFFDCfile, std::filesystem::path>>{});
            }
            logger::info("ErrlEntry callout fd {} path {} ", fd, path.string());
            ffdcFiles->emplace_back(std::move(pf), std::move(path));
        }
    }
    catch (const std::exception& e)
    {
        logger::error("ErrlEntry: Failed to persist attachment: {}", e.what());
    }
}

ErrlEntry::~ErrlEntry() noexcept
{
    if (!ffdcFiles)
    {
        return;
    }
    for (const auto& [pelffdc, path] : *ffdcFiles)
    {
        if (pelffdc.fd >= 0)
        {
            if (::close(pelffdc.fd) == -1)
            {
                logger::error(
                    "ErrlEntry::~ErrlEntry: Failed to close fd {}: {}",
                    pelffdc.fd, std::strerror(errno));
            }
        }

        try
        {
            logger::info("~ErrlEntry removing the file {}", path.string());
            if (!path.empty() && fs::exists(path))
            {
                fs::remove(path);
            }
        }
        catch (const std::exception& e)
        {
            logger::error("ErrlEntry::~ErrlEntry: Failed to remove file {}: {}",
                          path.string(), e.what());
        }
    }
}

} // namespace errl
