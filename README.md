# dunderdb

Log ingestion/Time series database

### Architecture


                         ┌──────────────────────┐
                         │        Client        │
                         │      C++ API         │
                         └──────────┬───────────┘
                                    ▲ 
                              ZeroMQ / JSON
                                    │
                                    ▼
                         ┌──────────────────────┐
                         │   Network Receiver   │
                         └──────────┬───────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │                               │
                  INSERT                          SELECT
                    │                               │
                    ▼                               ▼
             ┌─────────────┐              ┌─────────────────┐
             │ Common Queue│              │ Select Thread   │
             └──────┬──────┘              └────────┬────────┘
                    │                              │
                    ▼                              ▼
             ┌─────────────┐              ┌─────────────────┐
             │  Validator  │              │ Query Resolver  │
             └──────┬──────┘              └────────┬────────┘
                    │                              │
                    ▼                              │
             ┌─────────────┐                       │
             │   Service   │                       │
             │   Buffers   │                       │
             └──────┬──────┘                       │
                    │                              │
                    ▼                              ▼
             ┌─────────────┐                ┌─────────────┐
             │ Disk Queue  │                │    Index    │
             └──────┬──────┘                └──────┬───┬──┘
                    │                              ▲   |
                    ▼                              │   |
             ┌─────────────┐                       │   |
             │ Disk Writer │──────────update───────┘   |
             └──────┬──────┘                           |
                    │                                  |
                    ▼                                  |
             ┌─────────────┐                           |
             │   Storage   │◄──────────query────────── +
             │   Segments  │               
             └─────────────┘               

### Development Roadmap
#### MVP
[x] ZMQ thread receives incoming messages and push to queue \
[x] Parse JSON messages with RapidJSON, validate schema \
[x] Push the validated messages to a temporary buffer and on reaching threshold, move to a disk buffer \
[x] Disk Writer dequeues disk buffer, serialize messages, and write to disk \
[x] Index on time range \
[x] Create, store schemas, indexes on disk and load on start \
[x] Graceful shutdown - wait for validator to finish, force flush from buffers \
[x] Separate socket and thread for queries (SELECT, DELETE v INSERT, DDL) \
[x] Master Schema map to handle DDL in ingestion handler \
[x] Attach Schema binary to file headers \
[x] Differentiate insert and DDL messages in validator \
[] Handle schema DDL and changes in validator, index map, and disk writer \
[x] Index lookup for time range, load files, form reply JSON \
[] Client API with ZMQ for pushing data and querying \
[] Delete query

---
[] Unit tests \
[] Dockerize \
[] Thread pool for disk writer \
[] Pagination for retrieval queries \
[] Stats on indexes \
[] Schema versioning \
[] Chunking smaller segments into larger files \
[] Queries with filters on values of columns \
[] Dead letter queues \
[] Standing window queries \
[] Specialized indexes
