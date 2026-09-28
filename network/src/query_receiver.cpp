//
// Created by Arjun on 28/08/2026.
//

#include "query_receiver.h"
#include <chrono>
#include <zmq.hpp>
#include <iostream>

QueryReceiver::QueryReceiver(SelectHandler& select_handler) : select_handler_(select_handler) {}

void QueryReceiver::run() const
{
    zmq::context_t context(1);

    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("tcp://*:5556");
    std::cout << "Query handler listening on port 5556...\n";

    while (active_.load()) {

        // Wait for the next complete ZeroMQ multipart-message
        zmq::pollitem_t item{
            socket,
            0,
            ZMQ_POLLIN,
            0
        };

        zmq::poll(
            &item,
            1,
            std::chrono::milliseconds(100)
        );

        // No message arrived during this poll interval
        // Go back and check active_.
        if (!(item.revents & ZMQ_POLLIN)) {
            continue;
        }

        std::vector<std::string> frames;

        // The complete multipart message is available
        while (true) {
            zmq::message_t msg;

            // get one frame
            auto recv_res_ = socket.recv(msg);

            frames.emplace_back(
                static_cast<char*>(msg.data()),
                msg.size()
            );

            if (!socket.get(zmq::sockopt::rcvmore)) {
                break;
            }
        }

        // Shutdown may have been requested while we were
        // receiving this message. Discard the entire message.
        if (!active_.load()) {
            break;
        }

        // For now, limit the inbound query messages to be only of 2 frames 1. request type, 2. json payload
        if (frames.size() != 2) {
            socket.send(
                zmq::buffer("-1"),
                zmq::send_flags::none);

            continue;
        }

        const std::string& request_type = frames[0];
        const std::string& json_payload = frames[1];

        // request type 1 is exclusively for SELECT query
        // ingestion must be sent to a different socket
        if (request_type == "1") {
            std::string result = this->select_handler_.get_query_result(json_payload);
            socket.send(zmq::buffer(result), zmq::send_flags::none);
        } // end-if
        else {
            socket.send(
            zmq::buffer("Error: Invalid Request Type"),
            zmq::send_flags::none);
        }
    }
}


void QueryReceiver::start() {
    this->thread_ = std::thread(&QueryReceiver::run, this);
}

void QueryReceiver::stop() {
    active_.store(false);
}

void QueryReceiver::join() {
    if (this->thread_.joinable()) this->thread_.join();
}
