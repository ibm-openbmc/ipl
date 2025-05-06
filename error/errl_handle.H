#pragma once

#include <errl_entry.H>

#include <memory>
#include <vector>

class ErrlHandle
{
  public:
    ErrlHandle() noexcept = default;
    ~ErrlHandle() noexcept = default;

    // Delete copy constructor and copy assignment (cannot copy unique_ptr)
    ErrlHandle(const ErrlHandle&) = delete;
    ErrlHandle& operator=(const ErrlHandle&) = delete;

    // Default move constructor and move assignment
    ErrlHandle(ErrlHandle&&) noexcept = default;
    ErrlHandle& operator=(ErrlHandle&&) noexcept = default;

    void addEntry(std::unique_ptr<ErrlEntry> entry) noexcept;
    bool hasEntries() const noexcept;
    const std::vector<std::unique_ptr<ErrlEntry>>& getEntries() const noexcept;

  private:
    std::vector<std::unique_ptr<ErrlEntry>> entries;
};
