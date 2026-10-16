@echo off
setlocal
cd /d "%~dp0"
g++ -std=c++14 -Wall -Wextra -pedantic -Iinclude src\main.cpp src\Room.cpp src\Booking.cpp src\CsvBookingRepository.cpp src\BookingService.cpp src\HttpServer.cpp -o hotel-reservation.exe -lws2_32
if errorlevel 1 exit /b 1
echo Build successful: hotel-reservation.exe
