//
// Created by Arjun on 12/06/2026.
//

#include "ingestion_receiver.h"
#include <chrono>
#include <zmq.hpp>
#include <iostream>

void IngestionReceiver::run() const
{
    zmq::context_t context(1);

    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");
    std::cout << "Ingestion handler listening on port 5555...\n";

    while (active_.load()) {

        // Wait for the next complete ZeroMQ multipart-message
        // NOTE: Assuming ZeroMQ delivers only after the complete message is received, needs verification
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
            socket.recv(msg);

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

        if (frames.size() < 2) {
            socket.send(
                zmq::buffer("1"),
                zmq::send_flags::none);

            continue;
        }

        const std::string& request_type = frames[0];
        const std::string& service_name = frames[1];

        // request type 0 is exclusively for ingestion
        // other types must be sent to a different socket
        if (request_type == "0") {

            for (size_t i = 2; i < frames.size(); ++i) {
                insertion_queue_.enqueue(
                    UnvalidatedMessage{
                        .service = service_name,
                        .payload = std::move(frames[i]),
                        .timestamp_ms =
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()
                            ).count()
                    }
                );
            } // end-for
        } // end-if

        socket.send(
            zmq::buffer("0"),
            zmq::send_flags::none);
    }
}


void IngestionReceiver::start() {
    this->thread_ = std::thread(&IngestionReceiver::run, this);
}

void IngestionReceiver::stop() {
    active_.store(false);
}

void IngestionReceiver::join() {
    if (this->thread_.joinable()) this->thread_.join();
}
