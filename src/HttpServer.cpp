#include "HttpServer.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

namespace {
const size_t MAX_REQUEST_BYTES = 32768;

struct HttpRequest {
    string method;
    string target;
    string body;
};

string lower(const string& value) {
    string result = value;
    transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char character) { return static_cast<char>(tolower(character)); });
    return result;
}

int hexValue(char value) {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

string urlDecode(const string& value) {
    string decoded;
    for (string::size_type i = 0; i < value.size(); ++i) {
        if (value[i] == '+') {
            decoded += ' ';
        } else if (value[i] == '%') {
            if (i + 2 >= value.size()) {
                throw invalid_argument("The request contains an invalid URL-encoded value.");
            }
            const int high = hexValue(value[i + 1]);
            const int low = hexValue(value[i + 2]);
            if (high < 0 || low < 0) {
                throw invalid_argument("The request contains an invalid URL-encoded value.");
            }
            decoded += static_cast<char>(high * 16 + low);
            i += 2;
        } else {
            decoded += value[i];
        }
    }
    return decoded;
}

map<string, string> parseParameters(const string& values) {
    map<string, string> parameters;
    string::size_type start = 0;
    while (start < values.size()) {
        const string::size_type end = values.find('&', start);
        const string pair = values.substr(start, end == string::npos ? end : end - start);
        const string::size_type equals = pair.find('=');
        const string key = urlDecode(pair.substr(0, equals));
        const string value = equals == string::npos
            ? string() : urlDecode(pair.substr(equals + 1));
        if (!key.empty()) {
            parameters[key] = value;
        }
        if (end == string::npos) {
            break;
        }
        start = end + 1;
    }
    return parameters;
}

string requiredValue(const map<string, string>& values,
                          const string& key) {
    const map<string, string>::const_iterator found = values.find(key);
    if (found == values.end() || found->second.empty()) {
        throw invalid_argument("Please provide " + key + ".");
    }
    return found->second;
}

string jsonEscape(const string& value) {
    ostringstream escaped;
    for (string::const_iterator it = value.begin(); it != value.end(); ++it) {
        const unsigned char character = static_cast<unsigned char>(*it);
        switch (character) {
            case '"': escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\b': escaped << "\\b"; break;
            case '\f': escaped << "\\f"; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default:
                if (character < 0x20) {
                    const char digits[] = "0123456789abcdef";
                    escaped << "\\u00" << digits[character >> 4] << digits[character & 0x0f];
                } else {
                    escaped << *it;
                }
        }
    }
    return escaped.str();
}

string readFile(const string& path) {
    ifstream input(path.c_str(), ios::binary);
    if (!input) {
        throw runtime_error("Could not open the web app asset: " + path);
    }
    ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

string roomJson(const Room& room) {
    ostringstream json;
    json << "{\"id\":" << room.id()
         << ",\"name\":\"" << jsonEscape(room.name())
         << "\",\"category\":\"" << jsonEscape(room.category())
         << "\",\"capacity\":" << room.capacity()
         << ",\"nightlyRate\":" << room.nightlyRate()
         << ",\"description\":\"" << jsonEscape(room.description())
         << "\",\"amenities\":[";
    for (vector<string>::size_type i = 0; i < room.amenities().size(); ++i) {
        if (i != 0) json << ',';
        json << '"' << jsonEscape(room.amenities()[i]) << '"';
    }
    json << "]}";
    return json.str();
}

string bookingJson(const Booking& booking, const BookingService& service) {
    string roomName = "Room " + to_string(booking.roomId());
    for (vector<shared_ptr<Room> >::const_iterator room = service.rooms().begin();
         room != service.rooms().end(); ++room) {
        if ((*room)->id() == booking.roomId()) {
            roomName = (*room)->name();
            break;
        }
    }
    ostringstream json;
    json << "{\"id\":\"" << jsonEscape(booking.id())
         << "\",\"roomId\":" << booking.roomId()
         << ",\"guestCount\":" << booking.guestCount()
         << ",\"roomName\":\"" << jsonEscape(roomName)
         << "\",\"guestName\":\"" << jsonEscape(booking.guestName())
         << "\",\"email\":\"" << jsonEscape(booking.email())
         << "\",\"checkIn\":\"" << jsonEscape(booking.checkIn())
         << "\",\"checkOut\":\"" << jsonEscape(booking.checkOut())
         << "\",\"totalPrice\":" << booking.totalPrice()
         << ",\"status\":\"" << bookingStatusText(booking.status()) << "\"}";
    return json.str();
}

string bookingsJson(const vector<Booking>& bookings,
                         const BookingService& service) {
    ostringstream json;
    json << "{\"bookings\":[";
    for (vector<Booking>::size_type i = 0; i < bookings.size(); ++i) {
        if (i != 0) json << ',';
        json << bookingJson(bookings[i], service);
    }
    json << "]}";
    return json.str();
}

void sendResponse(SOCKET client, int status, const string& contentType,
                  const string& body, const string& cacheControl = "no-store") {
    const char* reason = status == 200 ? "OK" :
                         status == 201 ? "Created" :
                         status == 400 ? "Bad Request" :
                         status == 404 ? "Not Found" :
                         status == 405 ? "Method Not Allowed" : "Internal Server Error";
    ostringstream response;
    response << "HTTP/1.1 " << status << ' ' << reason << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Cache-Control: " << cacheControl << "\r\n"
             << "X-Content-Type-Options: nosniff\r\n"
             << "Connection: close\r\n\r\n"
             << body;
    const string message = response.str();
    string::size_type sent = 0;
    while (sent < message.size()) {
        const int result = send(client, message.data() + sent,
                                static_cast<int>(message.size() - sent), 0);
        if (result == SOCKET_ERROR || result == 0) {
            break;
        }
        sent += static_cast<string::size_type>(result);
    }
}

HttpRequest readRequest(SOCKET client) {
    string received;
    size_t headerEnd = string::npos;
    size_t contentLength = 0;
    bool headersParsed = false;
    char buffer[4096];
    while (true) {
        if (headersParsed && received.size() >= headerEnd + 4 + contentLength) {
            break;
        }
        if (received.size() >= MAX_REQUEST_BYTES) {
            throw invalid_argument("The request is too large.");
        }
        const int count = recv(client, buffer, sizeof(buffer), 0);
        if (count == SOCKET_ERROR || count == 0) {
            throw invalid_argument("The connection ended before the request was complete.");
        }
        received.append(buffer, static_cast<size_t>(count));
        if (!headersParsed) {
            headerEnd = received.find("\r\n\r\n");
            if (headerEnd != string::npos) {
                istringstream headers(received.substr(0, headerEnd));
                string requestLine;
                if (!getline(headers, requestLine)) {
                    throw invalid_argument("The request line is missing.");
                }
                if (!requestLine.empty() && requestLine[requestLine.size() - 1] == '\r') {
                    requestLine.erase(requestLine.size() - 1);
                }
                HttpRequest request;
                istringstream requestParts(requestLine);
                string version;
                if (!(requestParts >> request.method >> request.target >> version)) {
                    throw invalid_argument("The request line is invalid.");
                }

                string header;
                while (getline(headers, header)) {
                    if (!header.empty() && header[header.size() - 1] == '\r') {
                        header.erase(header.size() - 1);
                    }
                    const string::size_type colon = header.find(':');
                    if (colon == string::npos) continue;
                    const string name = lower(header.substr(0, colon));
                    string value = header.substr(colon + 1);
                    while (!value.empty() && value[0] == ' ') value.erase(0, 1);
                    if (name == "content-length") {
                        char* end = NULL;
                        const unsigned long parsed = strtoul(value.c_str(), &end, 10);
                        if (end == value.c_str() || *end != '\0' || parsed > MAX_REQUEST_BYTES) {
                            throw invalid_argument("The request body is too large or invalid.");
                        }
                        contentLength = static_cast<size_t>(parsed);
                    }
                }
                if (headerEnd + 4 + contentLength > MAX_REQUEST_BYTES) {
                    throw invalid_argument("The request is too large.");
                }
                headersParsed = true;
                if (received.size() >= headerEnd + 4 + contentLength) {
                    request.body = received.substr(headerEnd + 4, contentLength);
                    return request;
                }
                continue;
            }
        }
    }

    istringstream headers(received.substr(0, headerEnd));
    string requestLine;
    getline(headers, requestLine);
    if (!requestLine.empty() && requestLine[requestLine.size() - 1] == '\r') {
        requestLine.erase(requestLine.size() - 1);
    }
    HttpRequest request;
    istringstream requestParts(requestLine);
    string version;
    if (!(requestParts >> request.method >> request.target >> version)) {
        throw invalid_argument("The request line is invalid.");
    }
    request.body = received.substr(headerEnd + 4, contentLength);
    return request;
}

string contentTypeFor(const string& path) {
    if (path == "/styles.css") return "text/css; charset=utf-8";
    if (path == "/app.js") return "application/javascript; charset=utf-8";
    if (path == "/images/standard-room.jpg" || path == "/images/deluxe-room.jpg" ||
        path == "/images/suite-room.jpg") return "image/jpeg";
    return "text/html; charset=utf-8";
}

void handleRequest(SOCKET client, BookingService& service) {
    try {
        const HttpRequest request = readRequest(client);
        const string::size_type queryStart = request.target.find('?');
        const string path = request.target.substr(0, queryStart);
        const map<string, string> query = queryStart == string::npos
            ? map<string, string>()
            : parseParameters(request.target.substr(queryStart + 1));

        if (request.method == "GET" && path == "/") {
            sendResponse(client, 200, contentTypeFor(path), readFile("public\\index.html"));
            return;
        }
        if (request.method == "GET" && path == "/styles.css") {
            sendResponse(client, 200, contentTypeFor(path), readFile("public\\styles.css"),
                         "public, max-age=86400, immutable");
            return;
        }
        if (request.method == "GET" && path == "/app.js") {
            sendResponse(client, 200, contentTypeFor(path), readFile("public\\app.js"),
                         "public, max-age=86400, immutable");
            return;
        }
        if (request.method == "GET" && path == "/images/standard-room.jpg") {
            sendResponse(client, 200, contentTypeFor(path), readFile("public\\images\\standard-room.jpg"),
                         "public, max-age=2592000, immutable");
            return;
        }
        if (request.method == "GET" && path == "/images/deluxe-room.jpg") {
            sendResponse(client, 200, contentTypeFor(path), readFile("public\\images\\deluxe-room.jpg"),
                         "public, max-age=2592000, immutable");
            return;
        }
        if (request.method == "GET" && path == "/images/suite-room.jpg") {
            sendResponse(client, 200, contentTypeFor(path), readFile("public\\images\\suite-room.jpg"),
                         "public, max-age=2592000, immutable");
            return;
        }
        if (request.method == "GET" && path == "/api/health") {
            sendResponse(client, 200, "application/json; charset=utf-8", "{\"status\":\"ok\"}");
            return;
        }
        if (request.method == "GET" && path == "/api/rooms") {
            const string checkIn = requiredValue(query, "checkIn");
            const string checkOut = requiredValue(query, "checkOut");
            const vector<shared_ptr<Room> > rooms = service.availableRooms(checkIn, checkOut);
            ostringstream json;
            json << "{\"rooms\":[";
            for (vector<shared_ptr<Room> >::size_type i = 0; i < rooms.size(); ++i) {
                if (i != 0) json << ',';
                json << roomJson(*rooms[i]);
            }
            json << "]}";
            sendResponse(client, 200, "application/json; charset=utf-8", json.str());
            return;
        }
        if (request.method == "GET" && path == "/api/bookings") {
            const string email = requiredValue(query, "email");
            sendResponse(client, 200, "application/json; charset=utf-8",
                         bookingsJson(service.bookingsForEmail(email), service));
            return;
        }
        if (request.method == "POST" && path == "/api/bookings") {
            const map<string, string> form = parseParameters(request.body);
            const string roomIdValue = requiredValue(form, "roomId");
            const string guestCountValue = requiredValue(form, "guestCount");
            char* end = NULL;
            const long roomId = strtol(roomIdValue.c_str(), &end, 10);
            if (end == roomIdValue.c_str() || *end != '\0' || roomId < 1 || roomId > 100000) {
                throw invalid_argument("Choose a valid room.");
            }
            const long guestCount = strtol(guestCountValue.c_str(), &end, 10);
            if (end == guestCountValue.c_str() || *end != '\0' || guestCount < 1 || guestCount > 100) {
                throw invalid_argument("Enter a valid whole number of guests.");
            }
            const Booking booking = service.createBooking(
                static_cast<int>(roomId), static_cast<int>(guestCount),
                requiredValue(form, "guestName"),
                requiredValue(form, "email"), requiredValue(form, "checkIn"),
                requiredValue(form, "checkOut"));
            sendResponse(client, 201, "application/json; charset=utf-8",
                         "{\"booking\":" + bookingJson(booking, service) + "}");
            return;
        }
        if (request.method == "POST" && path == "/api/bookings/cancel") {
            const map<string, string> form = parseParameters(request.body);
            service.cancelBooking(requiredValue(form, "bookingId"), requiredValue(form, "email"));
            sendResponse(client, 200, "application/json; charset=utf-8",
                         "{\"message\":\"Reservation cancelled.\"}");
            return;
        }

        const bool knownPath = path == "/" || path == "/styles.css" || path == "/app.js" ||
                               path.compare(0, 8, "/images/") == 0 ||
                               path.compare(0, 5, "/api/") == 0;
        if (knownPath) {
            sendResponse(client, 405, "application/json; charset=utf-8",
                         "{\"error\":\"Method not allowed.\"}");
        } else {
            sendResponse(client, 404, "application/json; charset=utf-8",
                         "{\"error\":\"Page not found.\"}");
        }
    } catch (const invalid_argument& error) {
        sendResponse(client, 400, "application/json; charset=utf-8",
                     "{\"error\":\"" + jsonEscape(error.what()) + "\"}");
    } catch (const exception& error) {
        cerr << "Request failed: " << error.what() << endl;
        sendResponse(client, 500, "application/json; charset=utf-8",
                     "{\"error\":\"The server could not complete this request.\"}");
    }
}
}

HttpServer::HttpServer(BookingService& bookingService)
    : bookingService_(bookingService) {}

void HttpServer::run(unsigned short port) {
    WSADATA data;
    const int startup = WSAStartup(MAKEWORD(2, 2), &data);
    if (startup != 0) {
        throw runtime_error("Winsock initialization failed with code " + to_string(startup));
    }

    const SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) {
        WSACleanup();
        throw runtime_error("Could not create the local web server socket.");
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        closesocket(listener);
        WSACleanup();
        throw runtime_error("Could not bind to 127.0.0.1:" + to_string(port) +
                                 ". Try another port if this one is already in use.");
    }
    if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listener);
        WSACleanup();
        throw runtime_error("Could not start listening for browser connections.");
    }

    cout << "Hotel Haven is ready at http://127.0.0.1:" << port << endl;
    cout << "Keep this window open while using the app. Press Ctrl+C to stop." << endl;
    while (true) {
        const SOCKET client = accept(listener, NULL, NULL);
        if (client == INVALID_SOCKET) {
            const int error = WSAGetLastError();
            closesocket(listener);
            WSACleanup();
            throw runtime_error("Accepting a browser connection failed with code " +
                                     to_string(error));
        }
        handleRequest(client, bookingService_);
        shutdown(client, SD_SEND);
        closesocket(client);
    }
}
