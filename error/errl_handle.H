#pragma once

#include <errl_entry.H>

#include <memory>
#include <vector>
namespace errl
{
/** @brief Container for managing a collection of error log entries.
 *
 *  This class owns and manages one or more `ErrlEntry` objects, providing
 *  methods to add, query, and access the stored entries. It is non-copyable
 *  to preserve unique ownership semantics.
 */
class ErrlHandle
{
  public:
    /** @brief Default constructor. */
    ErrlHandle() noexcept = default;

    /** @brief Default destructor. */
    ~ErrlHandle() noexcept = default;

    /** @brief Deleted copy constructor to enforce unique ownership. */
    ErrlHandle(const ErrlHandle&) = delete;

    /** @brief Deleted copy assignment operator to enforce unique ownership. */
    ErrlHandle& operator=(const ErrlHandle&) = delete;

    /** @brief Default move constructor. */
    ErrlHandle(ErrlHandle&&) noexcept = default;

    /** @brief Default move assignment operator. */
    ErrlHandle& operator=(ErrlHandle&&) noexcept = default;

    /** @brief Add a new error log entry to the handle.
     *
     *  @param[in] entry - A unique pointer to an `ErrlEntry` to be added.
     */
    void addEntry(std::unique_ptr<ErrlEntry> entry) noexcept;

    /** @brief Check if the handle contains any error log entries.
     *
     *  @return true if one or more entries exist, false otherwise.
     */
    bool hasEntries() const noexcept;

    /** @brief Get the list of error log entries.
     *
     *  @return A reference to a vector of unique pointers to `ErrlEntry`
     * objects.
     */
    const std::vector<std::unique_ptr<ErrlEntry>>& getEntries() const noexcept;

  private:
    std::vector<std::unique_ptr<ErrlEntry>>
        entries; ///< Stored error log entries.
};
} // namespace errl
