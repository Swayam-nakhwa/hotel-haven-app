#include "Room.h"

#include <memory>

using namespace std;

Room::Room(int id, const string& name, int capacity, double nightlyRate,
           const string& description, const vector<string>& amenities)
    : id_(id), name_(name), capacity_(capacity), nightlyRate_(nightlyRate),
      description_(description), amenities_(amenities) {}

Room::~Room() {}

int Room::id() const { return id_; }
const string& Room::name() const { return name_; }
int Room::capacity() const { return capacity_; }
double Room::nightlyRate() const { return nightlyRate_; }
const string& Room::description() const { return description_; }
const vector<string>& Room::amenities() const { return amenities_; }

StandardRoom::StandardRoom(int id)
    : Room(id, "Classic King", 2, 119.00,
           "A calm, comfortable room made for a restful city stay.",
           vector<string>{"Queen bed", "Wi-Fi", "Work desk", "Rain shower"}) {}

const string& StandardRoom::category() const {
    static const string value = "Standard";
    return value;
}

DeluxeRoom::DeluxeRoom(int id)
    : Room(id, "Deluxe Garden", 3, 179.00,
           "A little more room, with a quiet garden outlook and lounge chair.",
           vector<string>{"King bed", "Garden view", "Wi-Fi", "Lounge chair"}) {}

const string& DeluxeRoom::category() const {
    static const string value = "Deluxe";
    return value;
}

SuiteRoom::SuiteRoom(int id)
    : Room(id, "The Corner Suite", 4, 259.00,
           "A generous suite with a separate living space for longer stays.",
           vector<string>{"King bed", "Living area", "Breakfast", "Bath tub"}) {}

const string& SuiteRoom::category() const {
    static const string value = "Suite";
    return value;
}

vector<shared_ptr<Room> > createRoomCatalog() {
    vector<shared_ptr<Room> > rooms;
    rooms.push_back(shared_ptr<Room>(new StandardRoom(101)));
    rooms.push_back(shared_ptr<Room>(new StandardRoom(102)));
    rooms.push_back(shared_ptr<Room>(new StandardRoom(103)));
    rooms.push_back(shared_ptr<Room>(new DeluxeRoom(201)));
    rooms.push_back(shared_ptr<Room>(new DeluxeRoom(202)));
    rooms.push_back(shared_ptr<Room>(new SuiteRoom(301)));
    return rooms;
}
