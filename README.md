# dunderdb

Log ingestion database.

### Development Roadmap

[x] ZMQ thread receives incoming messages and push to queue \
[x] Parse JSON messages with RapidJSON, validate schema \
[x] Push the validated messages to a temporary buffer and on reaching threshold, move to a disk buffer \
[x] Disk Writer dequeues disk buffer, serialize messages, and write to disk \
[x] Index on time range \
[] Create, store schemas, indexes on disk and load on start \
[] Seal .tmp file on start \
[] Queries API for range based query \
[] Client API with ZMQ for pushing data \
[] Unit tests \
[] Delete query

---

[] Dockerize \
[] Thread pool for disk writer \
[] Pagination for retrieval queries \
[] Schema versioning and evolution \
[] Chunking smaller segments into larger files \
[] Queries with filters on values of columns \
[] Standing window queries \
[] Specialized indexes