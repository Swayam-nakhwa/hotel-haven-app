#include "BookingRepository.h"

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace {
string csvEscape(const string& value) {
    if (value.find_first_of(",\"\r\n") == string::npos) {
        return value;
    }
    string escaped = "\"";
    for (string::const_iterator it = value.begin(); it != value.end(); ++it) {
        if (*it == '"') {
            escaped += "\"\"";
        } else {
            escaped += *it;
        }
    }
    return escaped + "\"";
}

vector<string> parseCsvRow(const string& line) {
    vector<string> fields;
    string field;
    bool quoted = false;
    for (string::size_type i = 0; i < line.size(); ++i) {
        const char character = line[i];
        if (quoted && character == '"' && i + 1 < line.size() && line[i + 1] == '"') {
            field += '"';
            ++i;
        } else if (character == '"') {
            quoted = !quoted;
        } else if (character == ',' && !quoted) {
            fields.push_back(field);
            field.clear();
        } else {
            field += character;
        }
    }
    if (quoted) {
        throw runtime_error("A booking row in the CSV file has an unclosed quote.");
    }
    fields.push_back(field);
    return fields;
}
}

BookingRepository::~BookingRepository() {}

CsvBookingRepository::CsvBookingRepository(const string& filePath)
    : filePath_(filePath) {}

vector<Booking> CsvBookingRepository::load() {
    ifstream input(filePath_.c_str());
    if (!input) {
        ofstream createFile(filePath_.c_str(), ios::app);
        if (!createFile) {
            throw runtime_error("Could not open or create the booking data file: " + filePath_);
        }
        return vector<Booking>();
    }

    vector<Booking> bookings;
    string line;
    bool firstLine = true;
    while (getline(input, line)) {
        if (firstLine) {
            firstLine = false;
            if (line == "id,room_id,guest_count,guest_name,email,check_in,check_out,status,total_price") {
                continue;
            }
        }
        if (line.empty()) {
            continue;
        }
        const vector<string> fields = parseCsvRow(line);
        if (fields.size() != 9) {
            throw runtime_error("A booking row in the CSV file has an unexpected number of fields.");
        }
        char* end = NULL;
        const long roomId = strtol(fields[1].c_str(), &end, 10);
        if (end == fields[1].c_str() || *end != '\0') {
            throw runtime_error("A booking row contains an invalid room number.");
        }
        const long guestCount = strtol(fields[2].c_str(), &end, 10);
        if (end == fields[2].c_str() || *end != '\0' || guestCount < 1) {
            throw runtime_error("A booking row contains an invalid guest count.");
        }
        const double totalPrice = strtod(fields[8].c_str(), &end);
        if (end == fields[8].c_str() || *end != '\0') {
            throw runtime_error("A booking row contains an invalid total price.");
        }
        BookingStatus status;
        if (fields[7] == "Confirmed") {
            status = BookingStatus::Confirmed;
        } else if (fields[7] == "Cancelled") {
            status = BookingStatus::Cancelled;
        } else {
            throw runtime_error("A booking row contains an unknown booking status.");
        }
        bookings.push_back(Booking(fields[0], static_cast<int>(roomId),
                                   static_cast<int>(guestCount), fields[3], fields[4],
                                   fields[5], fields[6], totalPrice, status));
    }
    if (input.bad()) {
        throw runtime_error("Could not read the booking data file: " + filePath_);
    }
    return bookings;
}

void CsvBookingRepository::save(const vector<Booking>& bookings) {
    ofstream output(filePath_.c_str(), ios::trunc);
    if (!output) {
        throw runtime_error("Could not write the booking data file: " + filePath_);
    }
    output << "id,room_id,guest_count,guest_name,email,check_in,check_out,status,total_price\n";
    for (vector<Booking>::const_iterator it = bookings.begin(); it != bookings.end(); ++it) {
        output << csvEscape(it->id()) << ',' << it->roomId() << ',' << it->guestCount() << ','
               << csvEscape(it->guestName()) << ',' << csvEscape(it->email()) << ','
               << csvEscape(it->checkIn()) << ',' << csvEscape(it->checkOut()) << ','
               << bookingStatusText(it->status()) << ',' << fixed << setprecision(2)
               << it->totalPrice() << '\n';
    }
    output.flush();
    if (!output) {
        throw runtime_error("Could not finish writing the booking data file: " + filePath_);
    }
}
