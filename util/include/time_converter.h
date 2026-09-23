//
// Created by Arjun on 18/09/2026.
//

#ifndef DUNDERDB_TIME_CONVERTER_H
#define DUNDERDB_TIME_CONVERTER_H
#include <chrono>
#include <string>

class TimeConverter {
public:
    static int64_t utc_to_epoch_ms(const std::string& input) {
        // Accepted:
        // YYYY-MM-DDTHH:MM:SSZ
        // YYYY-MM-DDTHH:MM:SS.sssZ

        if (input.size() != 20 && input.size() != 24)
            throw std::invalid_argument("Invalid timestamp");

        if (input[4] != '-' || input[7] != '-' ||
            input[10] != 'T' || input[13] != ':' ||
            input[16] != ':' || input.back() != 'Z')
            throw std::invalid_argument("Invalid timestamp");

        auto number = [&](const size_t pos, const size_t len) -> int {
            int value = 0;
            for (size_t i = 0; i < len; ++i) {
                if (input[pos + i] < '0' || input[pos + i] > '9')
                    throw std::invalid_argument("Invalid timestamp");

                value = value * 10 + (input[pos + i] - '0');
            }
            return value;
        };

        const int year = number(0, 4);
        const int month = number(5, 2);
        const int day = number(8, 2);
        const int hour = number(11, 2);
        const int minute = number(14, 2);
        const int second = number(17, 2);

        int milliseconds = 0;

        if (input.size() == 24) {
            if (input[19] != '.')
                throw std::invalid_argument("Invalid timestamp");

            milliseconds = number(20, 3);
        }

        if (hour > 23 || minute > 59 || second > 59 || milliseconds > 999)
            throw std::invalid_argument("Invalid timestamp");

        const std::chrono::year_month_day date{
            std::chrono::year{year},
            std::chrono::month{static_cast<unsigned>(month)},
            std::chrono::day{static_cast<unsigned>(day)}
        };

        if (!date.ok())
            throw std::invalid_argument("Invalid timestamp");

        const auto day_point = std::chrono::sys_days{date};

        const auto time =
            day_point +
            std::chrono::hours{hour} +
            std::chrono::minutes{minute} +
            std::chrono::seconds{second} +
            std::chrono::milliseconds{milliseconds};

        return time.time_since_epoch().count();
    }

    static std::string epoch_ms_to_iso(int64_t epoch_ms) {
        using namespace std::chrono;

        const sys_time<milliseconds> time{milliseconds{epoch_ms}};

        const auto day_point = floor<days>(time);
        const year_month_day date{day_point};

        const auto time_of_day = time - day_point;

        const auto hours = duration_cast<std::chrono::hours>(time_of_day);
        const auto minutes =
            duration_cast<std::chrono::minutes>(time_of_day - hours);
        const auto seconds =
            duration_cast<std::chrono::seconds>(
                time_of_day - hours - minutes);
        const auto milliseconds =
            duration_cast<std::chrono::milliseconds>(
                time_of_day - hours - minutes - seconds);

        std::ostringstream out;

        out << std::setfill('0')
            << std::setw(4) << static_cast<int>(date.year()) << '-'
            << std::setw(2) << static_cast<unsigned>(date.month()) << '-'
            << std::setw(2) << static_cast<unsigned>(date.day()) << 'T'
            << std::setw(2) << hours.count() << ':'
            << std::setw(2) << minutes.count() << ':'
            << std::setw(2) << seconds.count() << '.'
            << std::setw(3) << milliseconds.count()
            << 'Z';

        return out.str();
    }
};

#endif //DUNDERDB_TIME_CONVERTER_H
