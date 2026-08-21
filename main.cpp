#include <iostream>
#include <atomic>
#include <csignal>

#include "common_queue.h"
#include "network_receiver.h"
#include "schema.h"
#include "unvalidated_message.h"
#include "validator.h"
#include "service_buffer.h"
#include "buffer_map.h"
#include "disk_writer.h"
#include "index_loader.h"
#include "schema_loader.h"

std::atomic<bool> shutdown_requested{false};

void handle_signal(int signal)
{
    if (signal == SIGTERM || signal == SIGINT) {
        shutdown_requested.store(true);
    }
}

int main() {
    std::signal(SIGTERM, handle_signal);
    std::signal(SIGINT, handle_signal);

    std::cout << "Starting __DunderDB__" << std::endl;
    CommonQueue<UnvalidatedMessage> insertion_queue;

    // load schemas
    std::string services_directory = "/home/arjunsiva/dunderdb/data/services";
    std::string indexes_directory = "/home/arjunsiva/dunderdb/data/indexes";
    std::string schemas_directory = "/home/arjunsiva/dunderdb/data/schemas";

    std::vector<Schema> schemas_from_disk = SchemaLoader::load_schemas(std::filesystem::path{schemas_directory});

    for (const auto& schema : schemas_from_disk) {
        std::cout<< schema.to_string()<< std::endl;
    }

    std::cout << "Schemas loaded" << std::endl;

    // create buffer map
    BufferMap buffer_map;
    // create unique pointers of service buffers and add em to buffer map
    for (const auto& schema : schemas_from_disk) {
        const auto service_name = schema.get_service_name();
        buffer_map.add_buffer(service_name, std::make_unique<ServiceBuffer>(service_name, 512, 32));
    }

    std::cout << "Buffer Map loaded" << std::endl;

    // create Flush job queue
    CommonQueue<FlushJob> disk_queue;

    // load validator with schemas and validate them. validator has a map service -> schema
    Validator validator{insertion_queue, buffer_map, disk_queue};

    for (const auto& schema : schemas_from_disk) {
        validator.add_schema(schema);
    }


    NetworkReceiver receiver{insertion_queue};
    // starts network receiver in a new thread
    receiver.start();

    std::cout << "Receiver started" << std::endl;

    validator.start();
    std::cout << "Validator started" << std::endl;



    // auto sales_index = ServiceIndex{"sales", indexes_directory};
    // IndexMap index_map;
    // index_map.add_index("sales", std::make_unique<ServiceIndex>("sales", indexes_directory));

    // load indexes from disk
    IndexMap index_map = IndexLoader::load_indexes(indexes_directory);
    std::cout << "Indexes loaded" << std::endl;

    DiskWriter disk_writer{disk_queue, services_directory, index_map};
    disk_writer.start();

    std::cout << "Disk Writer started" << std::endl;

    while (!shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // shutdown
    std::cout << "Shutdown requested" << std::endl;

    // 1. Stop accepting new network requests
    receiver.stop();
    receiver.join();
    std::cout << "Network receiver stopped" << std::endl;

    // 2. Close insertion queue between network and validator
    insertion_queue.close();

    // 3. Validator drains remaining messages and exits
    validator.join();
    std::cout << "Validator stopped" << std::endl;

    // 4. Close disk job queue
    disk_queue.close();

    //5. DiskWriter drains remaining flush jobs, including the forced ones
    disk_writer.join();
    std::cout << "Disk Writer stopped" << std::endl;

    std::cout << "Stopped __DunderDB__" << std::endl;
    return 0;
}
