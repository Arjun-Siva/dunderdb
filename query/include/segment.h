//
// Created by Arjun on 17/09/2026.
//

#ifndef DUNDERDB_SEGMENT_H
#define DUNDERDB_SEGMENT_H

#include <vector>

#include "validated_message.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "parsed_select_query.h"

class Segment
{
public:
    Segment() = delete;
    Segment(Segment& other) = delete;
    Segment(Segment&& other) = delete;
    Segment& operator=(const Segment& other) = delete;
    Segment& operator=(Segment&& other) = delete;

    explicit Segment(const std::vector<std::byte>& segment_binary);
    void append_query_results(const ParsedSelectQuery& query, rapidjson::Writer<rapidjson::StringBuffer>& writer,
                              bool skip_time_filter = false) const;

private:
    Schema schema_;
    std::vector<ValidatedMessage> messages_vector_;
};

#endif //DUNDERDB_SEGMENT_H
