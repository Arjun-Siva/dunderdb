#include <iostream>
#include <atomic>
#include <csignal>

#include "common_queue.h"
#include "ingestion_receiver.h"
#include "schema.h"
#include "unvalidated_message.h"
#include "validator.h"
#include "service_buffer.h"
#include "buffer_map.h"
#include "disk_writer.h"
#include "index_loader.h"
#include "schema_loader.h"
#include "lock_map.h"
#include "query_receiver.h"
#include "select_handler.h"

std::atomic<bool> shutdown_requested{false};

void handle_signal(const int signal)
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
    // Map of locks for .tmp files
    LockMap tmp_file_lock_map;
    // create unique pointers of service buffers and add em to buffer map
    for (const auto& schema : schemas_from_disk) {
        const auto service_name = schema.get_service_name();
        buffer_map.add_buffer(service_name, std::make_unique<ServiceBuffer>(schema, 512, 32));
        tmp_file_lock_map.create(service_name);
    }

    std::cout << "Buffer Map loaded" << std::endl;

    // create Flush job queue
    CommonQueue<FlushJob> disk_queue;

    // load validator with schemas and validate them. validator has a map service -> schema
    Validator validator{insertion_queue, buffer_map, disk_queue};

    for (const auto& schema : schemas_from_disk) {
        validator.add_schema(schema);
    }

    IngestionReceiver ingestion_receiver{insertion_queue};
    // starts network receiver in a new thread
    ingestion_receiver.start();

    std::cout << "Ingestion Receiver started" << std::endl;

    validator.start();
    std::cout << "Validator started" << std::endl;


    // load indexes from disk
    IndexMap index_map; // passed as reference and the indexes are loaded into the map
    IndexLoader::load_indexes(indexes_directory, index_map);
    std::cout << "Indexes loaded" << std::endl;

    // TODO: add empty indexes to index map for services that are present in schema map, but not in index map
    // TEMPORARY
    // index_map.add_index("sales", std::make_unique<ServiceIndex>("sales", indexes_directory));
    // index_map.add_index("employee", std::make_unique<ServiceIndex>("employee", indexes_directory));
    // END

    DiskWriter disk_writer{disk_queue, services_directory, index_map, tmp_file_lock_map};
    disk_writer.start();

    std::cout << "Disk Writer started" << std::endl;

    SelectHandler select_handler{index_map, tmp_file_lock_map, services_directory};
    QueryReceiver query_receiver{select_handler};
    query_receiver.start();

    std::cout << "Query Receiver started" << std::endl;

    while (!shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // shutdown
    std::cout << "Shutdown requested" << std::endl;

    // 1. Stop accepting new network requests
    ingestion_receiver.stop();
    ingestion_receiver.join();
    std::cout << "Ingestion receiver stopped" << std::endl;

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
