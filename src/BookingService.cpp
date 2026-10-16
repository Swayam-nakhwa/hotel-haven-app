#include "BookingService.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace {
bool isLeapYear(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int daysInMonth(int year, int month) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return days[month - 1];
}

bool parseDateParts(const string& date, int& year, int& month, int& day) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    for (string::size_type i = 0; i < date.size(); ++i) {
        if (i != 4 && i != 7 && !isdigit(static_cast<unsigned char>(date[i]))) {
            return false;
        }
    }
    year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
           (date[2] - '0') * 10 + date[3] - '0';
    month = (date[5] - '0') * 10 + date[6] - '0';
    day = (date[8] - '0') * 10 + date[9] - '0';
    return year >= 1900 && month >= 1 && month <= 12 &&
           day >= 1 && day <= daysInMonth(year, month);
}

string trim(const string& value) {
    string::size_type first = 0;
    while (first < value.size() && isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }
    string::size_type last = value.size();
    while (last > first && isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }
    return value.substr(first, last - first);
}

string lower(const string& value) {
    string result = value;
    transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char character) { return static_cast<char>(tolower(character)); });
    return result;
}

bool datesOverlap(const Booking& booking, const string& checkIn,
                  const string& checkOut) {
    return booking.status() == BookingStatus::Confirmed &&
           booking.checkIn() < checkOut && checkIn < booking.checkOut();
}
}

bool isValidDate(const string& date) {
    int year = 0;
    int month = 0;
    int day = 0;
    return parseDateParts(date, year, month, day);
}

long long dateOrdinal(const string& date) {
    int year = 0;
    int month = 0;
    int day = 0;
    if (!parseDateParts(date, year, month, day)) {
        throw invalid_argument("Enter dates in YYYY-MM-DD format.");
    }
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned int yearOfEra = static_cast<unsigned int>(year - era * 400);
    const unsigned int adjustedMonth = static_cast<unsigned int>(month + (month > 2 ? -3 : 9));
    const unsigned int dayOfYear = (153 * adjustedMonth + 2) / 5 + static_cast<unsigned int>(day) - 1;
    const unsigned int dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    return static_cast<long long>(era) * 146097 + static_cast<long long>(dayOfEra);
}

string currentDate() {
    const time_t now = time(NULL);
    const tm* local = localtime(&now);
    if (local == NULL) {
        throw runtime_error("Could not determine the current local date.");
    }
    ostringstream date;
    date << setfill('0') << setw(4) << local->tm_year + 1900 << '-'
         << setw(2) << local->tm_mon + 1 << '-'
         << setw(2) << local->tm_mday;
    return date.str();
}

BookingService::BookingService(BookingRepository& repository,
                               const vector<shared_ptr<Room> >& rooms)
    : repository_(repository), rooms_(rooms), bookings_(repository.load()),
      nextBookingNumber_(1) {
    for (vector<Booking>::const_iterator it = bookings_.begin(); it != bookings_.end(); ++it) {
        const string prefix = "HTL-";
        if (it->id().compare(0, prefix.size(), prefix) == 0) {
            const unsigned long number = strtoul(it->id().substr(prefix.size()).c_str(), NULL, 10);
            if (number >= nextBookingNumber_) {
                nextBookingNumber_ = static_cast<unsigned int>(number + 1);
            }
        }
    }
}

const vector<shared_ptr<Room> >& BookingService::rooms() const {
    return rooms_;
}

vector<shared_ptr<Room> > BookingService::availableRooms(
    const string& checkIn, const string& checkOut) const {
    if (!isValidDate(checkIn) || !isValidDate(checkOut)) {
        throw invalid_argument("Enter valid check-in and check-out dates.");
    }
    if (checkIn < currentDate()) {
        throw invalid_argument("Check-in must be today or a future date.");
    }
    if (dateOrdinal(checkOut) <= dateOrdinal(checkIn)) {
        throw invalid_argument("Check-out must be after check-in.");
    }

    vector<shared_ptr<Room> > available;
    for (vector<shared_ptr<Room> >::const_iterator room = rooms_.begin();
         room != rooms_.end(); ++room) {
        bool occupied = false;
        for (vector<Booking>::const_iterator booking = bookings_.begin();
             booking != bookings_.end(); ++booking) {
            if (booking->roomId() == (*room)->id() &&
                datesOverlap(*booking, checkIn, checkOut)) {
                occupied = true;
                break;
            }
        }
        if (!occupied) {
            available.push_back(*room);
        }
    }
    return available;
}

vector<Booking> BookingService::bookingsForEmail(const string& email) const {
    const string normalizedEmail = lower(trim(email));
    vector<Booking> matches;
    for (vector<Booking>::const_iterator it = bookings_.begin(); it != bookings_.end(); ++it) {
        if (lower(it->email()) == normalizedEmail) {
            matches.push_back(*it);
        }
    }
    return matches;
}

Booking BookingService::createBooking(int roomId, int guestCount, const string& guestName,
                                      const string& email, const string& checkIn,
                                      const string& checkOut) {
    const string name = trim(guestName);
    const string normalizedEmail = lower(trim(email));
    if (name.size() < 2 || name.size() > 80 ||
        name.find_first_of("\r\n") != string::npos) {
        throw invalid_argument("Enter a name between 2 and 80 characters.");
    }
    const string::size_type at = normalizedEmail.find('@');
    if (normalizedEmail.size() > 254 || at == string::npos || at == 0 ||
        at != normalizedEmail.rfind('@') ||
        normalizedEmail.find_first_of(" \t\r\n") != string::npos ||
        normalizedEmail.find('.', at + 1) == string::npos ||
        normalizedEmail[normalizedEmail.size() - 1] == '.') {
        throw invalid_argument("Enter a valid email address.");
    }

    const Room& room = roomById(roomId);
    if (guestCount < 1 || guestCount > room.capacity()) {
        throw invalid_argument("Guest count must be between 1 and " +
                                    to_string(room.capacity()) + " for this room.");
    }
    const vector<shared_ptr<Room> > available = availableRooms(checkIn, checkOut);
    bool roomIsAvailable = false;
    for (vector<shared_ptr<Room> >::const_iterator it = available.begin();
         it != available.end(); ++it) {
        if ((*it)->id() == roomId) {
            roomIsAvailable = true;
            break;
        }
    }
    if (!roomIsAvailable) {
        throw invalid_argument("That room is no longer available for the selected dates.");
    }

    const long long nights = dateOrdinal(checkOut) - dateOrdinal(checkIn);
    ostringstream id;
    id << "HTL-" << setfill('0') << setw(6) << nextBookingNumber_;
    const Booking booking(id.str(), roomId, guestCount, name, normalizedEmail, checkIn, checkOut,
                          room.nightlyRate() * static_cast<double>(nights));

    vector<Booking> updatedBookings = bookings_;
    updatedBookings.push_back(booking);
    repository_.save(updatedBookings);
    bookings_.swap(updatedBookings);
    ++nextBookingNumber_;
    return booking;
}

void BookingService::cancelBooking(const string& bookingId, const string& email) {
    const string normalizedEmail = lower(trim(email));
    vector<Booking> updatedBookings = bookings_;
    for (vector<Booking>::iterator it = updatedBookings.begin(); it != updatedBookings.end(); ++it) {
        if (it->id() == trim(bookingId) && lower(it->email()) == normalizedEmail) {
            if (it->status() == BookingStatus::Cancelled) {
                throw invalid_argument("This reservation has already been cancelled.");
            }
            it->cancel();
            repository_.save(updatedBookings);
            bookings_.swap(updatedBookings);
            return;
        }
    }
    throw invalid_argument("No reservation matched that booking ID and email.");
}

const Room& BookingService::roomById(int roomId) const {
    for (vector<shared_ptr<Room> >::const_iterator it = rooms_.begin();
         it != rooms_.end(); ++it) {
        if ((*it)->id() == roomId) {
            return **it;
        }
    }
    throw invalid_argument("Choose a valid room.");
}
