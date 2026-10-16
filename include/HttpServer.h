#pragma once

#include "BookingService.h"

class HttpServer {
public:
    explicit HttpServer(BookingService& bookingService);
    void run(unsigned short port);

private:
    BookingService& bookingService_;
};
