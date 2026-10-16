#include "Booking.h"

using namespace std;

Booking::Booking(const string& id, int roomId, int guestCount, const string& guestName,
                 const string& email, const string& checkIn,
                 const string& checkOut, double totalPrice, BookingStatus status)
    : id_(id), roomId_(roomId), guestCount_(guestCount), guestName_(guestName), email_(email),
      checkIn_(checkIn), checkOut_(checkOut), totalPrice_(totalPrice), status_(status) {}

const string& Booking::id() const { return id_; }
int Booking::roomId() const { return roomId_; }
int Booking::guestCount() const { return guestCount_; }
const string& Booking::guestName() const { return guestName_; }
const string& Booking::email() const { return email_; }
const string& Booking::checkIn() const { return checkIn_; }
const string& Booking::checkOut() const { return checkOut_; }
double Booking::totalPrice() const { return totalPrice_; }
BookingStatus Booking::status() const { return status_; }
void Booking::cancel() { status_ = BookingStatus::Cancelled; }

string bookingStatusText(BookingStatus status) {
    return status == BookingStatus::Confirmed ? "Confirmed" : "Cancelled";
}
