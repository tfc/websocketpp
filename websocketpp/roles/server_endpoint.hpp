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

#ifndef WEBSOCKETPP_SERVER_ENDPOINT_HPP
#define WEBSOCKETPP_SERVER_ENDPOINT_HPP

#include <websocketpp/endpoint.hpp>
#include <websocketpp/logger/levels.hpp>

#include <iostream>

namespace websocketpp {


/// Server endpoint role based on the given config
/**
 *
 */
template <typename config>
class server : public endpoint<connection<config>,config> {
public:
    /// Type of this endpoint
    typedef server<config> type;

    /// Type of the endpoint concurrency component
    typedef typename config::concurrency_type concurrency_type;
    /// Type of the endpoint transport component
    typedef typename config::transport_type transport_type;

    /// Type of the connections this server will create
    typedef connection<config> connection_type;
    /// Type of a shared pointer to the connections this server will create
    typedef typename connection_type::ptr connection_ptr;

    /// Type of the connection transport component
    typedef typename transport_type::transport_con_type transport_con_type;
    /// Type of a shared pointer to the connection transport component
    typedef typename transport_con_type::ptr transport_con_ptr;

    /// Type of the endpoint component of this server
    typedef endpoint<connection_type,config> endpoint_type;


    // TODO: clean up these types

    explicit server() : endpoint_type(true)
    {
        endpoint_type::m_alog.write(log::alevel::devel, "server constructor");
    }

    // return an initialized connection_ptr. Call start() on this object to
    // begin the processing loop.
    connection_ptr get_connection() {
        connection_ptr con = endpoint_type::create_connection();

        return con;
    }

    // Starts the server's async connection acceptance loop.
    void start_accept() {
        connection_ptr con = get_connection();
        
        lib::error_code ec;
        
        transport_type::async_accept(
            lib::static_pointer_cast<transport_con_type>(con),
            lib::bind(
                &type::handle_accept,
                this,
                con,
                lib::placeholders::_1
            ),
            ec
        );
        
        if (ec == error::async_accept_not_listening) {
            endpoint_type::m_elog.write(log::elevel::info,
                "Stopping acceptance of new connections because the underlying transport is no longer listening.");
        } else if (ec) {
            endpoint_type::m_elog.write(log::elevel::rerror,
                "handle_accept error: "+ec.message());
        }
    }

    void handle_accept(connection_ptr con, lib::error_code const & ec) {
        if (ec) {
            con->terminate(ec);

            if (ec == error::operation_canceled) {
                endpoint_type::m_elog.write(log::elevel::info,
                    "handle_accept error: "+ec.message());
            } else {
                endpoint_type::m_elog.write(log::elevel::rerror,
                    "handle_accept error: "+ec.message());
            }
        } else {
            con->start();
        }

        // TODO: are there cases where we should terminate this loop?
        start_accept();
    }
private:
};

} // namespace websocketpp

#endif //WEBSOCKETPP_SERVER_ENDPOINT_HPP
