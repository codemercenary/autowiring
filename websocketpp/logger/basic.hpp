/*
 * Copyright (c) 2013, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef WEBSOCKETPP_LOGGER_BASIC_HPP
#define WEBSOCKETPP_LOGGER_BASIC_HPP

/* Need a way to print a message to the log
 *
 * - timestamps
 * - channels
 * - thread safe
 * - output to stdout or file
 * - selective output channels, both compile time and runtime
 * - named channels
 * - ability to test whether a log message will be printed at compile time
 *
 */

#include <ctime>
#include <iostream>
#include <iomanip>

#include <websocketpp/common/cpp11.hpp>
#include <websocketpp/common/stdint.hpp>
#include <websocketpp/logger/levels.hpp>

namespace websocketpp {
namespace log {

/// Basic logger that outputs to an ostream
template <typename concurrency, typename names>
class basic {
public:
    basic<concurrency,names>(channel_type_hint::value h = 
        channel_type_hint::access)
      : m_static_channels(0xffffffff)
      , m_dynamic_channels(0)
      , m_out(h == channel_type_hint::error ? &std::cerr : &std::cout) {}
      
    basic<concurrency,names>(std::ostream * out)
      : m_static_channels(0xffffffff)
      , m_dynamic_channels(0)
      , m_out(out) {}

    basic<concurrency,names>(level c, channel_type_hint::value h = 
        channel_type_hint::access)
      : m_static_channels(c)
      , m_dynamic_channels(0)
      , m_out(h == channel_type_hint::error ? &std::cerr : &std::cout) {}
    
    basic<concurrency,names>(level c, std::ostream * out)
      : m_static_channels(c)
      , m_dynamic_channels(0)
      , m_out(out) {}

    void set_ostream(std::ostream * out = &std::cout) {
        m_out = out;
    }

    void set_channels(level channels) {
        if (channels == names::none) {
            clear_channels(names::all);
            return;
        }

        scoped_lock_type lock(m_lock);
        m_dynamic_channels |= (channels & m_static_channels);
    }

    void clear_channels(level channels) {
        scoped_lock_type lock(m_lock);
        m_dynamic_channels &= ~channels;
    }

    void write(level channel, std::string const & msg) {
        scoped_lock_type lock(m_lock);
        if (!this->dynamic_test(channel)) { return; }
        *m_out << "[" << timestamp << "] "
                  << "[" << names::channel_name(channel) << "] "
                  << msg << "\n";
        m_out->flush();
    }

    void write(level channel, char const * msg) {
        scoped_lock_type lock(m_lock);
        if (!this->dynamic_test(channel)) { return; }
        *m_out << "[" << timestamp << "] "
                  << "[" << names::channel_name(channel) << "] "
                  << msg << "\n";
        m_out->flush();
    }

    _WEBSOCKETPP_CONSTEXPR_TOKEN_ bool static_test(level channel) const {
        return ((channel & m_static_channels) != 0);
    }

    bool dynamic_test(level channel) {
        return ((channel & m_dynamic_channels) != 0);
    }
private:
    typedef typename concurrency::scoped_lock_type scoped_lock_type;
    typedef typename concurrency::mutex_type mutex_type;

    // The timestamp does not include the time zone, because on Windows with the
    // default registry settings, the time zone would be written out in full,
    // which would be obnoxiously verbose.
    //
    // TODO: find a workaround for this or make this format user settable
    static std::ostream & timestamp(std::ostream & os) {
        std::time_t t = std::time(NULL);
        std::tm* lt = std::localtime(&t);
        #ifdef _WEBSOCKETPP_CPP11_CHRONO_
            return os << std::put_time(lt,"%Y-%m-%d %H:%M:%S");
        #else // Falls back to strftime, which requires a temporary copy of the string.
            char buffer[20];
            std::strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",lt);
            return os << buffer;
        #endif
    }

    mutex_type m_lock;

    level const m_static_channels;
    level m_dynamic_channels;
    std::ostream * m_out;
};

} // log
} // websocketpp

#endif // WEBSOCKETPP_LOGGER_BASIC_HPP
