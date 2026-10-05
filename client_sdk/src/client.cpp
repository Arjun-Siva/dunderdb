#include "dunderdb/client.h"

#include <stdexcept>
#include <utility>

namespace dunderdb
{
    Client::Client(
        std::string host,
        const int insert_port,
        const int query_port
    )
        : host_(std::move(host)),
          insert_port_(insert_port),
          query_port_(query_port),
          context_(1),
          insert_socket_(context_, zmq::socket_type::req),
          query_socket_(context_, zmq::socket_type::req) {}

    Client::~Client() = default;

    void Client::connect() {
        if (connected_) {
            throw std::runtime_error("Client is already connected");
        }

        const std::string insert_endpoint =
            "tcp://" + host_ + ":" + std::to_string(insert_port_);

        const std::string query_endpoint =
            "tcp://" + host_ + ":" + std::to_string(query_port_);

        insert_socket_.connect(insert_endpoint);
        query_socket_.connect(query_endpoint);

        connected_ = true;
    }

    Reply Client::send(const SendRequest& request) const {
        if (!connected_) {
            throw std::runtime_error("Client is not connected");
        }

        // TODO:
        // 1. Serialize SendRequest
        // 2. Send multipart ZMQ message
        // 3. Receive server response
        // 4. Parse response into Reply

        throw std::runtime_error("Client::send() not implemented");
    }

    Reply Client::query(const QueryRequest& request) const {
        if (!connected_) {
            throw std::runtime_error("Client is not connected");
        }

        // TODO:
        // 1. Serialize QueryRequest
        // 2. Send ZMQ request
        // 3. Receive server response
        // 4. Parse response into Reply

        throw std::runtime_error("Client::query() not implemented");
    }
} // namespace dunderdb
