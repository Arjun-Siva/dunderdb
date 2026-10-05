#pragma once

#include "query_request.h"
#include "reply.h"
#include "send_request.h"

#include <string>
#include <zmq.hpp>

namespace dunderdb {

    class Client {
    public:
        Client(
            std::string host,
            int insert_port = 5555,
            int query_port = 5556
        );
        // TODO: implement timeout

        ~Client();

        void connect();

        Reply send(const SendRequest& request) const;
        Reply query(const QueryRequest& request) const;

    private:
        std::string host_;
        int insert_port_;
        int query_port_;

        zmq::context_t context_;
        zmq::socket_t insert_socket_;
        zmq::socket_t query_socket_;

        bool connected_{false};
    };

} // namespace dunderdb