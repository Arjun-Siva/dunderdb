#include <iostream>

#include "common_queue.h"
#include "network_receiver.h"
#include "schema.h"
#include "unvalidated_message.h"
#include "validator.h"
#include "service_buffer.h"
#include "buffer_map.h"

int main() {
    std::cout << "Starting __DunderDB__" << std::endl;
    CommonQueue<UnvalidatedMessage> insertion_queue;

    // load schemas
    Schema sales("sales");
    sales.add_column(Column("item", ColumnType::STRING, 20, false) );
    sales.add_column(Column("price", ColumnType::NUMBER, false) );
    sales.add_column(Column("category", ColumnType::STRING, 10, true));

    std::cout << "Schemas loaded" << std::endl;

    // create buffer map
    BufferMap buffer_map;
    auto service_name = std::string("sales");
    // create unique pointers of service buffers and add em to buffer map
    buffer_map.add_buffer(service_name, std::make_unique<ServiceBuffer>(service_name, 512, 32));

    std::cout << "Buffer Map loaded" << std::endl;

    // create Flush job queue
    CommonQueue<FlushJob> disk_queue;

    // load validator with schemas and validate them. validator has a map service -> schema
    Validator validator{insertion_queue, buffer_map, disk_queue};
    validator.add_schema(sales);

    NetworkReceiver receiver{insertion_queue};
    // starts network receiver in a new thread
    receiver.start();

    std::cout << "Receiver started" << std::endl;

    // while (true) {
    //     auto msg = insertion_queue.dequeue();  // ideally blocking
    //     std::cout << msg.service << " " << msg.payload << "\n";
    // }

    validator.start();
    std::cout << "Validator started" << std::endl;

    while (true) {}

    return 0;
}
