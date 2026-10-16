#pragma once

#include "Booking.h"
#include "BookingRepository.h"
#include "Room.h"

#include <memory>
#include <string>
#include <vector>

class BookingService {
public:
    BookingService(BookingRepository& repository,
                   const std::vector<std::shared_ptr<Room> >& rooms);

    const std::vector<std::shared_ptr<Room> >& rooms() const;
    std::vector<std::shared_ptr<Room> > availableRooms(
        const std::string& checkIn, const std::string& checkOut) const;
    std::vector<Booking> bookingsForEmail(const std::string& email) const;
    Booking createBooking(int roomId, int guestCount, const std::string& guestName,
                          const std::string& email, const std::string& checkIn,
                          const std::string& checkOut);
    void cancelBooking(const std::string& bookingId, const std::string& email);

private:
    BookingRepository& repository_;
    std::vector<std::shared_ptr<Room> > rooms_;
    std::vector<Booking> bookings_;
    unsigned int nextBookingNumber_;

    const Room& roomById(int roomId) const;
};

bool isValidDate(const std::string& date);
long long dateOrdinal(const std::string& date);
std::string currentDate();
