@echo off
setlocal
cd /d "%~dp0"
g++ -std=c++14 -Wall -Wextra -pedantic -Iinclude tests\booking_service_tests.cpp src\Room.cpp src\Booking.cpp src\CsvBookingRepository.cpp src\BookingService.cpp -o tests\booking-service-tests.exe
if errorlevel 1 exit /b 1
tests\booking-service-tests.exe
set TEST_RESULT=%ERRORLEVEL%
del tests\booking-service-tests.exe
exit /b %TEST_RESULT%
