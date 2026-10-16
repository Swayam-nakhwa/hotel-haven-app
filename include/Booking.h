#pragma once

#include <string>

enum class BookingStatus {
    Confirmed,
    Cancelled
};

class Booking {
public:
    Booking(const std::string& id, int roomId, int guestCount, const std::string& guestName,
            const std::string& email, const std::string& checkIn,
            const std::string& checkOut, double totalPrice,
            BookingStatus status = BookingStatus::Confirmed);

    const std::string& id() const;
    int roomId() const;
    int guestCount() const;
    const std::string& guestName() const;
    const std::string& email() const;
    const std::string& checkIn() const;
    const std::string& checkOut() const;
    double totalPrice() const;
    BookingStatus status() const;
    void cancel();

private:
    std::string id_;
    int roomId_;
    int guestCount_;
    std::string guestName_;
    std::string email_;
    std::string checkIn_;
    std::string checkOut_;
    double totalPrice_;
    BookingStatus status_;
};

std::string bookingStatusText(BookingStatus status);
