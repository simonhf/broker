#include "broker/detail/generator_file_writer.hh"

#include <caf/error.hpp>
#include <caf/sec.hpp>

#include "broker/detail/meta_command_writer.hh"
#include "broker/detail/meta_data_writer.hh"
#include "broker/error.hh"
#include "broker/logger.hh"
#include "broker/message.hh"

namespace broker {
namespace detail {

generator_file_writer::generator_file_writer()
  : sink_(nullptr, buf_), flush_threshold_(1024) {
  buf_.reserve(2028);
}

generator_file_writer::~generator_file_writer() {
  if (auto err = flush())
    BROKER_ERROR("flushing file in destructor failed:" << err);
}

caf::error generator_file_writer::open(std::string file_name) {
  if (auto err = flush()) {
    // Log the error, but ignore it otherwise.
    BROKER_ERROR("flushing previous file failed:" << err);
  }
  f_.open(file_name, std::ofstream::binary);
  if (!f_.is_open())
    return make_error(ec::cannot_open_file, file_name);
  auto magic = format::magic;
  auto version = format::version;
  char header[sizeof(magic) + sizeof(version)];
  memcpy(header, &magic, sizeof(magic));
  memcpy(header + sizeof(magic), &version, sizeof(version));
  if (!f_.write(header, sizeof(header))) {
    BROKER_ERROR("unable to write to file:" << file_name);
    f_.close();
    return make_error(ec::cannot_write_file, file_name);
  }
  if (!f_.flush()) {
    BROKER_ERROR("unable to write to file (flush failed):" << file_name);
    f_.close();
    return make_error(ec::cannot_write_file, file_name);
  }
  file_name_ = std::move(file_name);
  return caf::none;
}

caf::error generator_file_writer::flush() {
  if (!f_.is_open() || buf_.empty())
    return caf::none;
  if (!f_.write(buf_.data(), buf_.size()))
    return make_error(ec::cannot_write_file, file_name_);
  buf_.clear();
  sink_.seek(0);
  return caf::none;
}

caf::error generator_file_writer::write(const data_message& x) {
  meta_data_writer writer{sink_};
  uint16_t tid;
  auto entry = format::entry_type::data_message;
  BROKER_TRY(topic_id(get_topic(x), tid), sink_(entry, tid),
             writer(get_data(x)));
  if (buf_.size() >= flush_threshold())
    return flush();
  return caf::none;
}

caf::error generator_file_writer::write(const command_message& x) {
  meta_command_writer writer{sink_};
  uint16_t tid;
  auto entry = format::entry_type::command_message;
  BROKER_TRY(topic_id(get_topic(x), tid), sink_(entry, tid),
             writer(get_command(x)));
  if (buf_.size() >= flush_threshold())
    return flush();
  return caf::none;
}

caf::error generator_file_writer::write(const data_or_command_message& x) {
  if (caf::holds_alternative<data_message>(x))
    return write(caf::get<data_message>(x));
  return write(caf::get<command_message>(x));
}

caf::error generator_file_writer::topic_id(const topic& x, uint16_t& id) {
  auto e = topic_table_.end();
  auto i = std::find(topic_table_.begin(), e, x);
  if (i == e) {
    // Write the new topic to file first.
    auto entry = format::entry_type::new_topic;
    if (auto err = sink_(entry, x.string()))
      return err;
    id = static_cast<uint16_t>(topic_table_.size());
    topic_table_.emplace_back(x);
    return caf::none;
  }
  id = static_cast<uint16_t>(std::distance(topic_table_.begin(), i));
  return caf::none;
}

bool generator_file_writer::operator!() const {
  return !f_;
}

generator_file_writer::operator bool() const {
  return static_cast<bool>(f_);
}

generator_file_writer_ptr make_generator_file_writer(const std::string& fname) {
  generator_file_writer_ptr result{new generator_file_writer};
  if (result->open(fname) != caf::none)
    return nullptr;
  return result;
}

generator_file_writer& operator<<(generator_file_writer& out,
                                  const data_message& x) {
  out.write(x);
  return out;
}

} // namespace detail
} // namespace broker
