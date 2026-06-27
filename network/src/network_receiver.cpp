//
// Created by Arjun on 12/06/2026.
//

#include "network_receiver.h"
#include <chrono>
#include <zmq.hpp>
#include <iostream>

[[noreturn]] void NetworkReceiver::run() const {
    zmq::context_t context(1);

    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");

    std::cout << "Server listening on port 5555...\n";

    while (true) {
        std::vector<std::string> frames;

        while (true) {
            zmq::message_t msg;
            socket.recv(msg);

            frames.emplace_back(
                static_cast<char *>(msg.data()),
                msg.size());

            if (!socket.get(zmq::sockopt::rcvmore))
                break;
        }

        if (frames.size() < 2) {
            socket.send(
                zmq::buffer("1"),
                zmq::send_flags::none);

            continue;
        }

        const std::string &request_type = frames[0];
        const std::string &service_name = frames[1];

        // push request
        if (request_type == "0") {
            // push messages to queue
            for (size_t i = 2; i < frames.size(); ++i) {
                this->insertion_queue_.enqueue(UnvalidatedMessage{
                    .service = service_name,
                    .payload = std::move(frames[i]),
                    .timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                    ).count()
                });
            }
        }

        socket.send(
            zmq::buffer("0"),
            zmq::send_flags::none);
    }
}


void NetworkReceiver::start() {
    this->thread_ = std::thread(&NetworkReceiver::run, this);
}

void NetworkReceiver::join() {
    if (this->thread_.joinable()) this->thread_.join();
}