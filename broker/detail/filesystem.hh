#ifndef BROKER_DETAIL_FILESYSTEM_HH
#define BROKER_DETAIL_FILESYSTEM_HH

#include <string>

namespace broker {
namespace detail {

using path = std::string;

/// Checks whether a given filename exists.
/// @param p The path to examine.
/// @returns `true` if the given path or file status corresponds to an existing
/// file or directory, `false` otherwise.
bool exists(const path& p);

/// Removes a file or empty directory.
/// @param p The path to remove.
/// @returns `true` iff *p* was deleted successfully.
bool remove(const path& p);

/// Deletes the contents of a path (if it is a directory) and the contents of
/// all its subdirectories, recursively, then deletes the path itself as if by
/// repeatedly applying the POSIX remove.
/// @param p The path to remove.
/// @returns `true` iff *p* was deleted successfully.
bool remove_all(const path& p);

} // namespace detail
} // namespace broker

#endif // BROKER_DETAIL_FILE_SYSTEM_HH
