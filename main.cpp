#include <iostream>

#include "common_queue.h"
#include "network_receiver.h"
#include "schema.h"
#include "unvalidated_message.h"
#include "validator.h"

int main() {
    std::cout << "Starting __DunderDB__" << std::endl;
    CommonQueue<UnvalidatedMessage> insertion_queue;

    // load schemas
    Schema sales("sales");
    sales.add_column(Column("item", ColumnType::STRING, 20, false) );
    sales.add_column(Column("price", ColumnType::NUMBER, false) );
    sales.add_column(Column("category", ColumnType::STRING, 10, true));

    std::cout << "Schemas loaded" << std::endl;

    // load validator with schemas and validate them. validator has a map service -> schema
    Validator validator{insertion_queue};
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
    // std::cout << "Validator started" << std::endl;

    while (true) {}

    return 0;
}
