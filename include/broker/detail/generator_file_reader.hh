#pragma once

#include <memory>
#include <cstddef>

#include <caf/binary_deserializer.hpp>

#include "broker/detail/data_generator.hh"
#include "broker/fwd.hh"
#include "broker/topic.hh"

namespace broker {
namespace detail {

class generator_file_reader {
public:
  using value_type = caf::variant<data_message, command_message>;

  generator_file_reader(int fd, void* addr, size_t file_size);

  generator_file_reader(generator_file_reader&&) = delete;

  generator_file_reader(const generator_file_reader&) = delete;

  generator_file_reader& operator=(generator_file_reader&&) = delete;

  generator_file_reader& operator=(const generator_file_reader&) = delete;

  ~generator_file_reader();

  bool at_end() const;

  /// @pre `at_end()`
  void rewind();

  caf::error read(value_type& x);

  caf::error skip();

  caf::error skip_to_end();

  const std::vector<topic>& topics() const noexcept {
    return topic_table_;
  }

  size_t entries() const noexcept {
    return data_entries_ + command_entries_;
  }

  size_t data_entries() const noexcept {
    return data_entries_;
  }

  size_t command_entries() const noexcept {
    return command_entries_;
  }

private:
  int fd_;
  void* addr_;
  size_t file_size_;
  caf::binary_deserializer source_;
  data_generator generator_;
  std::vector<topic> topic_table_;
  size_t data_entries_ = 0;
  size_t command_entries_ = 0;
  bool sealed_ = false;
};

using generator_file_reader_ptr = std::unique_ptr<generator_file_reader>;

generator_file_reader_ptr make_generator_file_reader(const std::string& fname);

} // namespace detail
} // namespace broker
