# DunderDB
Ingestion-optimized Time Series Database

### Architecture
<img width="1654" height="612" alt="Dunderdb drawio (1)" src="https://github.com/user-attachments/assets/4d4c1022-b115-4d84-a7a8-1a6cbe790390" /> 

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
[x] Handle schema DDL and changes in validator, index map, and disk writer \
[x] Index lookup for time range, load files, form reply JSON \
[x] Eventual visibility - Query handler signals BufferMap for APPEND when it requires tmp file \
[x] Client API with ZMQ for pushing data and querying \
[x] Sequence numbers for messages, update headers \
[] Delete query - Tombstones on disk, and Query handler reads them

---
[] Unit tests \
[] Conan for dependencies \
[] Random out-of-order insertion \
[] Timestamps from client \
[] Thread pool for disk writer \
[] Pagination for retrieval queries \
[] Segment LRU cache \
[] Stats on indexes \
[] Schema versioning \
[] Chunking smaller segments into larger files \
[] Queries with filters on values of columns \
[] Dead letter queues \
[] Standing window queries \
[] Specialized indexes

---
