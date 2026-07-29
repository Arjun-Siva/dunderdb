# dunderdb
Log ingestion database.
<br>


### Development Roadmap
MVP \
[x] ZMQ thread receives incoming messages and push to queue \
[x] Parse JSON messages with RapidJSON, validate schema \
[x] Push the validated messages to a temporary buffer and on reaching threshold, move to a disk buffer \
[] Disk Writer dequeues disk buffer, serialize messages, and write to disk \
[] Index on time range \
[] Queries API for range based query \
[] Create, store schemas, indexes on disk and load on start \
[] Client API with ZMQ for pushing data \
[] Unit tests
---
[] Dockerize \
[] Thread pool for disk writer \
[] Pagination for retrieval queries \
[] Delete query \
[] Chunking smaller segments into larger files \
[] Queries with filters on values of columns \
[] Standing window queries \
[] Specialized indexes