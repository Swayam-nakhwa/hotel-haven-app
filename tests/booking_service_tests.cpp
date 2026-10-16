#include "BookingService.h"
#include "Room.h"

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

class InMemoryBookingRepository : public BookingRepository {
public:
    vector<Booking> load() override { return bookings; }
    void save(const vector<Booking>& updated) override { bookings = updated; }

    vector<Booking> bookings;
};

void require(bool condition, const string& message) {
    if (!condition) {
        throw runtime_error(message);
    }
}

template <typename Function>
void requireInvalid(Function action, const string& message) {
    bool rejected = false;
    try {
        action();
    } catch (const invalid_argument&) {
        rejected = true;
    }
    require(rejected, message);
}

int main() {
    try {
        require(isValidDate("2028-02-29"), "Leap day should be a valid date.");
        require(!isValidDate("2027-02-29"), "An invalid leap day should be rejected.");
        require(!isValidDate("2028-13-01"), "An invalid month should be rejected.");
        require(dateOrdinal("2099-01-03") - dateOrdinal("2099-01-01") == 2,
                "Date arithmetic should count the nights between dates.");

        InMemoryBookingRepository repository;
        const vector<shared_ptr<Room> > rooms = createRoomCatalog();
        BookingService service(repository, rooms);
        require(rooms.size() == 6, "The hotel catalog should contain six rooms.");

        const Booking first = service.createBooking(
            101, 2, "Alex, Guest", "Alex@example.com", "2099-01-01", "2099-01-03");
        require(first.id() == "HTL-000001", "The first booking should receive the first reference.");
        require(first.totalPrice() == 238.00, "The booking total should use the room rate and stay length.");
        require(repository.bookings.size() == 1, "A successful booking should be persisted.");
        require(service.bookingsForEmail(" alex@EXAMPLE.com ").size() == 1,
                "Reservation lookup should ignore email case and surrounding spaces.");

        const vector<shared_ptr<Room> > adjacent =
            service.availableRooms("2099-01-03", "2099-01-04");
        require(adjacent.size() == rooms.size(),
                "A room should be available when a previous stay checks out that day.");
        const vector<shared_ptr<Room> > overlapping =
            service.availableRooms("2099-01-02", "2099-01-04");
        require(overlapping.size() == rooms.size() - 1,
                "An overlapping stay should make its room unavailable.");

        requireInvalid([&service]() {
            service.createBooking(101, 1, "Second Guest", "second@example.com",
                                  "2099-01-02", "2099-01-04");
        }, "The service should reject overlapping room reservations.");
        requireInvalid([&service]() {
            service.createBooking(102, 1, "Second Guest", "second@example.com",
                                  "2099-01-03", "2099-01-03");
        }, "The service should reject a stay with no nights.");
        requireInvalid([&service]() {
            service.createBooking(102, 1, "Second Guest", "not-an-email",
                                  "2099-01-03", "2099-01-04");
        }, "The service should reject an invalid email address.");
        requireInvalid([&service]() {
            service.createBooking(101, 3, "Second Guest", "second@example.com",
                                  "2099-01-03", "2099-01-04");
        }, "The service should reject more guests than a room can accommodate.");

        requireInvalid([&service]() {
            service.cancelBooking("HTL-000001", "wrong@example.com");
        }, "A reservation should not be cancellable with a different email.");
        service.cancelBooking("HTL-000001", "alex@example.com");
        require(repository.bookings[0].status() == BookingStatus::Cancelled,
                "Cancellation should be persisted.");
        require(service.availableRooms("2099-01-01", "2099-01-03").size() == rooms.size(),
                "A cancelled room should become available again.");
        requireInvalid([&service]() {
            service.cancelBooking("HTL-000001", "alex@example.com");
        }, "A cancelled reservation should not be cancellable again.");

        const string csvPath = "tests\\booking-roundtrip.csv";
        CsvBookingRepository csvRepository(csvPath);
        csvRepository.save(repository.bookings);
        const vector<Booking> loaded = csvRepository.load();
        remove(csvPath.c_str());
        require(loaded.size() == 1, "The CSV repository should reload saved bookings.");
        require(loaded[0].guestName() == "Alex, Guest",
                "The CSV repository should correctly round-trip commas in names.");
        require(loaded[0].guestCount() == 2,
                "The CSV repository should preserve the number of guests.");
        require(loaded[0].status() == BookingStatus::Cancelled,
                "The CSV repository should preserve cancellation status.");

        cout << "All booking service tests passed." << endl;
        return 0;
    } catch (const exception& error) {
        cerr << "Test failed: " << error.what() << endl;
        return 1;
    }
}
