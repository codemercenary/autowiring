//
// impl/read.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef AUTOBOOST_ASIO_IMPL_READ_HPP
#define AUTOBOOST_ASIO_IMPL_READ_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <algorithm>
#include <autoboost/asio/buffer.hpp>
#include <autoboost/asio/completion_condition.hpp>
#include <autoboost/asio/detail/array_fwd.hpp>
#include <autoboost/asio/detail/base_from_completion_cond.hpp>
#include <autoboost/asio/detail/bind_handler.hpp>
#include <autoboost/asio/detail/consuming_buffers.hpp>
#include <autoboost/asio/detail/dependent_type.hpp>
#include <autoboost/asio/detail/handler_alloc_helpers.hpp>
#include <autoboost/asio/detail/handler_cont_helpers.hpp>
#include <autoboost/asio/detail/handler_invoke_helpers.hpp>
#include <autoboost/asio/detail/handler_type_requirements.hpp>
#include <autoboost/asio/detail/throw_error.hpp>
#include <autoboost/asio/error.hpp>

#include <autoboost/asio/detail/push_options.hpp>

namespace autoboost {
namespace asio {

template <typename SyncReadStream, typename MutableBufferSequence,
    typename CompletionCondition>
std::size_t read(SyncReadStream& s, const MutableBufferSequence& buffers,
    CompletionCondition completion_condition, autoboost::system::error_code& ec)
{
  ec = autoboost::system::error_code();
  autoboost::asio::detail::consuming_buffers<
    mutable_buffer, MutableBufferSequence> tmp(buffers);
  std::size_t total_transferred = 0;
  tmp.prepare(detail::adapt_completion_condition_result(
        completion_condition(ec, total_transferred)));
  while (tmp.begin() != tmp.end())
  {
    std::size_t bytes_transferred = s.read_some(tmp, ec);
    tmp.consume(bytes_transferred);
    total_transferred += bytes_transferred;
    tmp.prepare(detail::adapt_completion_condition_result(
          completion_condition(ec, total_transferred)));
  }
  return total_transferred;
}

template <typename SyncReadStream, typename MutableBufferSequence>
inline std::size_t read(SyncReadStream& s, const MutableBufferSequence& buffers)
{
  autoboost::system::error_code ec;
  std::size_t bytes_transferred = read(s, buffers, transfer_all(), ec);
  autoboost::asio::detail::throw_error(ec, "read");
  return bytes_transferred;
}

template <typename SyncReadStream, typename MutableBufferSequence>
inline std::size_t read(SyncReadStream& s, const MutableBufferSequence& buffers,
    autoboost::system::error_code& ec)
{
  return read(s, buffers, transfer_all(), ec);
}

template <typename SyncReadStream, typename MutableBufferSequence,
    typename CompletionCondition>
inline std::size_t read(SyncReadStream& s, const MutableBufferSequence& buffers,
    CompletionCondition completion_condition)
{
  autoboost::system::error_code ec;
  std::size_t bytes_transferred = read(s, buffers, completion_condition, ec);
  autoboost::asio::detail::throw_error(ec, "read");
  return bytes_transferred;
}

#if !defined(AUTOBOOST_ASIO_NO_IOSTREAM)

template <typename SyncReadStream, typename Allocator,
    typename CompletionCondition>
std::size_t read(SyncReadStream& s,
    autoboost::asio::basic_streambuf<Allocator>& b,
    CompletionCondition completion_condition, autoboost::system::error_code& ec)
{
  ec = autoboost::system::error_code();
  std::size_t total_transferred = 0;
  std::size_t max_size = detail::adapt_completion_condition_result(
        completion_condition(ec, total_transferred));
  std::size_t bytes_available = read_size_helper(b, max_size);
  while (bytes_available > 0)
  {
    std::size_t bytes_transferred = s.read_some(b.prepare(bytes_available), ec);
    b.commit(bytes_transferred);
    total_transferred += bytes_transferred;
    max_size = detail::adapt_completion_condition_result(
          completion_condition(ec, total_transferred));
    bytes_available = read_size_helper(b, max_size);
  }
  return total_transferred;
}

template <typename SyncReadStream, typename Allocator>
inline std::size_t read(SyncReadStream& s,
    autoboost::asio::basic_streambuf<Allocator>& b)
{
  autoboost::system::error_code ec;
  std::size_t bytes_transferred = read(s, b, transfer_all(), ec);
  autoboost::asio::detail::throw_error(ec, "read");
  return bytes_transferred;
}

template <typename SyncReadStream, typename Allocator>
inline std::size_t read(SyncReadStream& s,
    autoboost::asio::basic_streambuf<Allocator>& b,
    autoboost::system::error_code& ec)
{
  return read(s, b, transfer_all(), ec);
}

template <typename SyncReadStream, typename Allocator,
    typename CompletionCondition>
inline std::size_t read(SyncReadStream& s,
    autoboost::asio::basic_streambuf<Allocator>& b,
    CompletionCondition completion_condition)
{
  autoboost::system::error_code ec;
  std::size_t bytes_transferred = read(s, b, completion_condition, ec);
  autoboost::asio::detail::throw_error(ec, "read");
  return bytes_transferred;
}

#endif // !defined(AUTOBOOST_ASIO_NO_IOSTREAM)

namespace detail
{
  template <typename AsyncReadStream, typename MutableBufferSequence,
      typename CompletionCondition, typename ReadHandler>
  class read_op
    : detail::base_from_completion_cond<CompletionCondition>
  {
  public:
    read_op(AsyncReadStream& stream, const MutableBufferSequence& buffers,
        CompletionCondition completion_condition, ReadHandler& handler)
      : detail::base_from_completion_cond<
          CompletionCondition>(completion_condition),
        stream_(stream),
        buffers_(buffers),
        start_(0),
        total_transferred_(0),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler))
    {
    }

#if defined(AUTOBOOST_ASIO_HAS_MOVE)
    read_op(const read_op& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        buffers_(other.buffers_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(other.handler_)
    {
    }

    read_op(read_op&& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        buffers_(other.buffers_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(other.handler_))
    {
    }
#endif // defined(AUTOBOOST_ASIO_HAS_MOVE)

    void operator()(const autoboost::system::error_code& ec,
        std::size_t bytes_transferred, int start = 0)
    {
      switch (start_ = start)
      {
        case 1:
        buffers_.prepare(this->check_for_completion(ec, total_transferred_));
        for (;;)
        {
          stream_.async_read_some(buffers_,
              AUTOBOOST_ASIO_MOVE_CAST(read_op)(*this));
          return; default:
          total_transferred_ += bytes_transferred;
          buffers_.consume(bytes_transferred);
          buffers_.prepare(this->check_for_completion(ec, total_transferred_));
          if ((!ec && bytes_transferred == 0)
              || buffers_.begin() == buffers_.end())
            break;
        }

        handler_(ec, static_cast<const std::size_t&>(total_transferred_));
      }
    }

  //private:
    AsyncReadStream& stream_;
    autoboost::asio::detail::consuming_buffers<
      mutable_buffer, MutableBufferSequence> buffers_;
    int start_;
    std::size_t total_transferred_;
    ReadHandler handler_;
  };

  template <typename AsyncReadStream,
      typename CompletionCondition, typename ReadHandler>
  class read_op<AsyncReadStream, autoboost::asio::mutable_buffers_1,
      CompletionCondition, ReadHandler>
    : detail::base_from_completion_cond<CompletionCondition>
  {
  public:
    read_op(AsyncReadStream& stream,
        const autoboost::asio::mutable_buffers_1& buffers,
        CompletionCondition completion_condition, ReadHandler& handler)
      : detail::base_from_completion_cond<
          CompletionCondition>(completion_condition),
        stream_(stream),
        buffer_(buffers),
        start_(0),
        total_transferred_(0),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler))
    {
    }

#if defined(AUTOBOOST_ASIO_HAS_MOVE)
    read_op(const read_op& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        buffer_(other.buffer_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(other.handler_)
    {
    }

    read_op(read_op&& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        buffer_(other.buffer_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(other.handler_))
    {
    }
#endif // defined(AUTOBOOST_ASIO_HAS_MOVE)

    void operator()(const autoboost::system::error_code& ec,
        std::size_t bytes_transferred, int start = 0)
    {
      std::size_t n = 0;
      switch (start_ = start)
      {
        case 1:
        n = this->check_for_completion(ec, total_transferred_);
        for (;;)
        {
          stream_.async_read_some(
              autoboost::asio::buffer(buffer_ + total_transferred_, n),
              AUTOBOOST_ASIO_MOVE_CAST(read_op)(*this));
          return; default:
          total_transferred_ += bytes_transferred;
          if ((!ec && bytes_transferred == 0)
              || (n = this->check_for_completion(ec, total_transferred_)) == 0
              || total_transferred_ == autoboost::asio::buffer_size(buffer_))
            break;
        }

        handler_(ec, static_cast<const std::size_t&>(total_transferred_));
      }
    }

  //private:
    AsyncReadStream& stream_;
    autoboost::asio::mutable_buffer buffer_;
    int start_;
    std::size_t total_transferred_;
    ReadHandler handler_;
  };

  template <typename AsyncReadStream, typename Elem,
      typename CompletionCondition, typename ReadHandler>
  class read_op<AsyncReadStream, autoboost::array<Elem, 2>,
      CompletionCondition, ReadHandler>
    : detail::base_from_completion_cond<CompletionCondition>
  {
  public:
    read_op(AsyncReadStream& stream, const autoboost::array<Elem, 2>& buffers,
        CompletionCondition completion_condition, ReadHandler& handler)
      : detail::base_from_completion_cond<
          CompletionCondition>(completion_condition),
        stream_(stream),
        buffers_(buffers),
        start_(0),
        total_transferred_(0),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler))
    {
    }

#if defined(AUTOBOOST_ASIO_HAS_MOVE)
    read_op(const read_op& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        buffers_(other.buffers_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(other.handler_)
    {
    }

    read_op(read_op&& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        buffers_(other.buffers_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(other.handler_))
    {
    }
#endif // defined(AUTOBOOST_ASIO_HAS_MOVE)

    void operator()(const autoboost::system::error_code& ec,
        std::size_t bytes_transferred, int start = 0)
    {
      typename autoboost::asio::detail::dependent_type<Elem,
          autoboost::array<autoboost::asio::mutable_buffer, 2> >::type bufs = {{
        autoboost::asio::mutable_buffer(buffers_[0]),
        autoboost::asio::mutable_buffer(buffers_[1]) }};
      std::size_t buffer_size0 = autoboost::asio::buffer_size(bufs[0]);
      std::size_t buffer_size1 = autoboost::asio::buffer_size(bufs[1]);
      std::size_t n = 0;
      switch (start_ = start)
      {
        case 1:
        n = this->check_for_completion(ec, total_transferred_);
        for (;;)
        {
          bufs[0] = autoboost::asio::buffer(bufs[0] + total_transferred_, n);
          bufs[1] = autoboost::asio::buffer(
              bufs[1] + (total_transferred_ < buffer_size0
                ? 0 : total_transferred_ - buffer_size0),
              n - autoboost::asio::buffer_size(bufs[0]));
          stream_.async_read_some(bufs, AUTOBOOST_ASIO_MOVE_CAST(read_op)(*this));
          return; default:
          total_transferred_ += bytes_transferred;
          if ((!ec && bytes_transferred == 0)
              || (n = this->check_for_completion(ec, total_transferred_)) == 0
              || total_transferred_ == buffer_size0 + buffer_size1)
            break;
        }

        handler_(ec, static_cast<const std::size_t&>(total_transferred_));
      }
    }

  //private:
    AsyncReadStream& stream_;
    autoboost::array<Elem, 2> buffers_;
    int start_;
    std::size_t total_transferred_;
    ReadHandler handler_;
  };

#if defined(AUTOBOOST_ASIO_HAS_STD_ARRAY)

  template <typename AsyncReadStream, typename Elem,
      typename CompletionCondition, typename ReadHandler>
  class read_op<AsyncReadStream, std::array<Elem, 2>,
      CompletionCondition, ReadHandler>
    : detail::base_from_completion_cond<CompletionCondition>
  {
  public:
    read_op(AsyncReadStream& stream, const std::array<Elem, 2>& buffers,
        CompletionCondition completion_condition, ReadHandler& handler)
      : detail::base_from_completion_cond<
          CompletionCondition>(completion_condition),
        stream_(stream),
        buffers_(buffers),
        start_(0),
        total_transferred_(0),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler))
    {
    }

#if defined(AUTOBOOST_ASIO_HAS_MOVE)
    read_op(const read_op& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        buffers_(other.buffers_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(other.handler_)
    {
    }

    read_op(read_op&& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        buffers_(other.buffers_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(other.handler_))
    {
    }
#endif // defined(AUTOBOOST_ASIO_HAS_MOVE)

    void operator()(const autoboost::system::error_code& ec,
        std::size_t bytes_transferred, int start = 0)
    {
      typename autoboost::asio::detail::dependent_type<Elem,
          std::array<autoboost::asio::mutable_buffer, 2> >::type bufs = {{
        autoboost::asio::mutable_buffer(buffers_[0]),
        autoboost::asio::mutable_buffer(buffers_[1]) }};
      std::size_t buffer_size0 = autoboost::asio::buffer_size(bufs[0]);
      std::size_t buffer_size1 = autoboost::asio::buffer_size(bufs[1]);
      std::size_t n = 0;
      switch (start_ = start)
      {
        case 1:
        n = this->check_for_completion(ec, total_transferred_);
        for (;;)
        {
          bufs[0] = autoboost::asio::buffer(bufs[0] + total_transferred_, n);
          bufs[1] = autoboost::asio::buffer(
              bufs[1] + (total_transferred_ < buffer_size0
                ? 0 : total_transferred_ - buffer_size0),
              n - autoboost::asio::buffer_size(bufs[0]));
          stream_.async_read_some(bufs, AUTOBOOST_ASIO_MOVE_CAST(read_op)(*this));
          return; default:
          total_transferred_ += bytes_transferred;
          if ((!ec && bytes_transferred == 0)
              || (n = this->check_for_completion(ec, total_transferred_)) == 0
              || total_transferred_ == buffer_size0 + buffer_size1)
            break;
        }

        handler_(ec, static_cast<const std::size_t&>(total_transferred_));
      }
    }

  //private:
    AsyncReadStream& stream_;
    std::array<Elem, 2> buffers_;
    int start_;
    std::size_t total_transferred_;
    ReadHandler handler_;
  };

#endif // defined(AUTOBOOST_ASIO_HAS_STD_ARRAY)

  template <typename AsyncReadStream, typename MutableBufferSequence,
      typename CompletionCondition, typename ReadHandler>
  inline void* asio_handler_allocate(std::size_t size,
      read_op<AsyncReadStream, MutableBufferSequence,
        CompletionCondition, ReadHandler>* this_handler)
  {
    return autoboost_asio_handler_alloc_helpers::allocate(
        size, this_handler->handler_);
  }

  template <typename AsyncReadStream, typename MutableBufferSequence,
      typename CompletionCondition, typename ReadHandler>
  inline void asio_handler_deallocate(void* pointer, std::size_t size,
      read_op<AsyncReadStream, MutableBufferSequence,
        CompletionCondition, ReadHandler>* this_handler)
  {
    autoboost_asio_handler_alloc_helpers::deallocate(
        pointer, size, this_handler->handler_);
  }

  template <typename AsyncReadStream, typename MutableBufferSequence,
      typename CompletionCondition, typename ReadHandler>
  inline bool asio_handler_is_continuation(
      read_op<AsyncReadStream, MutableBufferSequence,
        CompletionCondition, ReadHandler>* this_handler)
  {
    return this_handler->start_ == 0 ? true
      : autoboost_asio_handler_cont_helpers::is_continuation(
          this_handler->handler_);
  }

  template <typename Function, typename AsyncReadStream,
      typename MutableBufferSequence, typename CompletionCondition,
      typename ReadHandler>
  inline void asio_handler_invoke(Function& function,
      read_op<AsyncReadStream, MutableBufferSequence,
        CompletionCondition, ReadHandler>* this_handler)
  {
    autoboost_asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }

  template <typename Function, typename AsyncReadStream,
      typename MutableBufferSequence, typename CompletionCondition,
      typename ReadHandler>
  inline void asio_handler_invoke(const Function& function,
      read_op<AsyncReadStream, MutableBufferSequence,
        CompletionCondition, ReadHandler>* this_handler)
  {
    autoboost_asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }
} // namespace detail

template <typename AsyncReadStream, typename MutableBufferSequence,
    typename CompletionCondition, typename ReadHandler>
inline AUTOBOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
    void (autoboost::system::error_code, std::size_t))
async_read(AsyncReadStream& s, const MutableBufferSequence& buffers,
    CompletionCondition completion_condition,
    AUTOBOOST_ASIO_MOVE_ARG(ReadHandler) handler)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a ReadHandler.
  AUTOBOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

  detail::async_result_init<
    ReadHandler, void (autoboost::system::error_code, std::size_t)> init(
      AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler));

  detail::read_op<AsyncReadStream, MutableBufferSequence,
    CompletionCondition, AUTOBOOST_ASIO_HANDLER_TYPE(
      ReadHandler, void (autoboost::system::error_code, std::size_t))>(
        s, buffers, completion_condition, init.handler)(
          autoboost::system::error_code(), 0, 1);

  return init.result.get();
}

template <typename AsyncReadStream, typename MutableBufferSequence,
    typename ReadHandler>
inline AUTOBOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
    void (autoboost::system::error_code, std::size_t))
async_read(AsyncReadStream& s, const MutableBufferSequence& buffers,
    AUTOBOOST_ASIO_MOVE_ARG(ReadHandler) handler)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a ReadHandler.
  AUTOBOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

  detail::async_result_init<
    ReadHandler, void (autoboost::system::error_code, std::size_t)> init(
      AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler));

  detail::read_op<AsyncReadStream, MutableBufferSequence,
    detail::transfer_all_t, AUTOBOOST_ASIO_HANDLER_TYPE(
      ReadHandler, void (autoboost::system::error_code, std::size_t))>(
        s, buffers, transfer_all(), init.handler)(
          autoboost::system::error_code(), 0, 1);

  return init.result.get();
}

#if !defined(AUTOBOOST_ASIO_NO_IOSTREAM)

namespace detail
{
  template <typename AsyncReadStream, typename Allocator,
      typename CompletionCondition, typename ReadHandler>
  class read_streambuf_op
    : detail::base_from_completion_cond<CompletionCondition>
  {
  public:
    read_streambuf_op(AsyncReadStream& stream,
        basic_streambuf<Allocator>& streambuf,
        CompletionCondition completion_condition, ReadHandler& handler)
      : detail::base_from_completion_cond<
          CompletionCondition>(completion_condition),
        stream_(stream),
        streambuf_(streambuf),
        start_(0),
        total_transferred_(0),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler))
    {
    }

#if defined(AUTOBOOST_ASIO_HAS_MOVE)
    read_streambuf_op(const read_streambuf_op& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        streambuf_(other.streambuf_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(other.handler_)
    {
    }

    read_streambuf_op(read_streambuf_op&& other)
      : detail::base_from_completion_cond<CompletionCondition>(other),
        stream_(other.stream_),
        streambuf_(other.streambuf_),
        start_(other.start_),
        total_transferred_(other.total_transferred_),
        handler_(AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(other.handler_))
    {
    }
#endif // defined(AUTOBOOST_ASIO_HAS_MOVE)

    void operator()(const autoboost::system::error_code& ec,
        std::size_t bytes_transferred, int start = 0)
    {
      std::size_t max_size, bytes_available;
      switch (start_ = start)
      {
        case 1:
        max_size = this->check_for_completion(ec, total_transferred_);
        bytes_available = read_size_helper(streambuf_, max_size);
        for (;;)
        {
          stream_.async_read_some(streambuf_.prepare(bytes_available),
              AUTOBOOST_ASIO_MOVE_CAST(read_streambuf_op)(*this));
          return; default:
          total_transferred_ += bytes_transferred;
          streambuf_.commit(bytes_transferred);
          max_size = this->check_for_completion(ec, total_transferred_);
          bytes_available = read_size_helper(streambuf_, max_size);
          if ((!ec && bytes_transferred == 0) || bytes_available == 0)
            break;
        }

        handler_(ec, static_cast<const std::size_t&>(total_transferred_));
      }
    }

  //private:
    AsyncReadStream& stream_;
    autoboost::asio::basic_streambuf<Allocator>& streambuf_;
    int start_;
    std::size_t total_transferred_;
    ReadHandler handler_;
  };

  template <typename AsyncReadStream, typename Allocator,
      typename CompletionCondition, typename ReadHandler>
  inline void* asio_handler_allocate(std::size_t size,
      read_streambuf_op<AsyncReadStream, Allocator,
        CompletionCondition, ReadHandler>* this_handler)
  {
    return autoboost_asio_handler_alloc_helpers::allocate(
        size, this_handler->handler_);
  }

  template <typename AsyncReadStream, typename Allocator,
      typename CompletionCondition, typename ReadHandler>
  inline void asio_handler_deallocate(void* pointer, std::size_t size,
      read_streambuf_op<AsyncReadStream, Allocator,
        CompletionCondition, ReadHandler>* this_handler)
  {
    autoboost_asio_handler_alloc_helpers::deallocate(
        pointer, size, this_handler->handler_);
  }

  template <typename AsyncReadStream, typename Allocator,
      typename CompletionCondition, typename ReadHandler>
  inline bool asio_handler_is_continuation(
      read_streambuf_op<AsyncReadStream, Allocator,
        CompletionCondition, ReadHandler>* this_handler)
  {
    return this_handler->start_ == 0 ? true
      : autoboost_asio_handler_cont_helpers::is_continuation(
          this_handler->handler_);
  }

  template <typename Function, typename AsyncReadStream,
      typename Allocator, typename CompletionCondition, typename ReadHandler>
  inline void asio_handler_invoke(Function& function,
      read_streambuf_op<AsyncReadStream, Allocator,
        CompletionCondition, ReadHandler>* this_handler)
  {
    autoboost_asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }

  template <typename Function, typename AsyncReadStream,
      typename Allocator, typename CompletionCondition, typename ReadHandler>
  inline void asio_handler_invoke(const Function& function,
      read_streambuf_op<AsyncReadStream, Allocator,
        CompletionCondition, ReadHandler>* this_handler)
  {
    autoboost_asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
  }
} // namespace detail

template <typename AsyncReadStream, typename Allocator,
    typename CompletionCondition, typename ReadHandler>
inline AUTOBOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
    void (autoboost::system::error_code, std::size_t))
async_read(AsyncReadStream& s,
    autoboost::asio::basic_streambuf<Allocator>& b,
    CompletionCondition completion_condition,
    AUTOBOOST_ASIO_MOVE_ARG(ReadHandler) handler)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a ReadHandler.
  AUTOBOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

  detail::async_result_init<
    ReadHandler, void (autoboost::system::error_code, std::size_t)> init(
      AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler));

  detail::read_streambuf_op<AsyncReadStream, Allocator,
    CompletionCondition, AUTOBOOST_ASIO_HANDLER_TYPE(
      ReadHandler, void (autoboost::system::error_code, std::size_t))>(
        s, b, completion_condition, init.handler)(
          autoboost::system::error_code(), 0, 1);

  return init.result.get();
}

template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
inline AUTOBOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
    void (autoboost::system::error_code, std::size_t))
async_read(AsyncReadStream& s,
    autoboost::asio::basic_streambuf<Allocator>& b,
    AUTOBOOST_ASIO_MOVE_ARG(ReadHandler) handler)
{
  // If you get an error on the following line it means that your handler does
  // not meet the documented type requirements for a ReadHandler.
  AUTOBOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

  detail::async_result_init<
    ReadHandler, void (autoboost::system::error_code, std::size_t)> init(
      AUTOBOOST_ASIO_MOVE_CAST(ReadHandler)(handler));

  detail::read_streambuf_op<AsyncReadStream, Allocator,
    detail::transfer_all_t, AUTOBOOST_ASIO_HANDLER_TYPE(
      ReadHandler, void (autoboost::system::error_code, std::size_t))>(
        s, b, transfer_all(), init.handler)(
          autoboost::system::error_code(), 0, 1);

  return init.result.get();
}

#endif // !defined(AUTOBOOST_ASIO_NO_IOSTREAM)

} // namespace asio
} // namespace autoboost

#include <autoboost/asio/detail/pop_options.hpp>

#endif // AUTOBOOST_ASIO_IMPL_READ_HPP