#include <iostream>

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

int main() {
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

    while (true) {}

    return 0;
}
