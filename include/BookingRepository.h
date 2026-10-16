#pragma once

#include "Booking.h"

#include <string>
#include <vector>

class BookingRepository {
public:
    virtual ~BookingRepository();
    virtual std::vector<Booking> load() = 0;
    virtual void save(const std::vector<Booking>& bookings) = 0;
};

class CsvBookingRepository : public BookingRepository {
public:
    explicit CsvBookingRepository(const std::string& filePath);
    std::vector<Booking> load() override;
    void save(const std::vector<Booking>& bookings) override;

private:
    std::string filePath_;
};
