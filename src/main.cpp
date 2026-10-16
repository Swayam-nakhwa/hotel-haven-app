#include "BookingRepository.h"
#include "BookingService.h"
#include "HttpServer.h"
#include "Room.h"

#include <windows.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace std;

int main(int argc, char* argv[]) {
    unsigned short port = 8080;
    if (argc > 1) {
        char* end = NULL;
        const long requestedPort = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || requestedPort < 1 || requestedPort > 65535) {
            cerr << "Usage: hotel-reservation.exe [port between 1 and 65535]" << endl;
            return 1;
        }
        port = static_cast<unsigned short>(requestedPort);
    }

    if (!CreateDirectoryA("data", NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        cerr << "Could not create the data folder for saved reservations." << endl;
        return 1;
    }

    try {
        CsvBookingRepository repository("data\\bookings.csv");
        const vector<shared_ptr<Room> > rooms = createRoomCatalog();
        BookingService bookingService(repository, rooms);
        HttpServer server(bookingService);
        server.run(port);
    } catch (const exception& error) {
        cerr << "Hotel Haven stopped: " << error.what() << endl;
        return 1;
    }
    return 0;
}
