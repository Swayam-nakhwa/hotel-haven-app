#pragma once

#include <string>
#include <memory>
#include <vector>

class Room {
public:
    Room(int id, const std::string& name, int capacity, double nightlyRate,
         const std::string& description, const std::vector<std::string>& amenities);
    virtual ~Room();

    int id() const;
    const std::string& name() const;
    int capacity() const;
    double nightlyRate() const;
    const std::string& description() const;
    const std::vector<std::string>& amenities() const;
    virtual const std::string& category() const = 0;

private:
    int id_;
    std::string name_;
    int capacity_;
    double nightlyRate_;
    std::string description_;
    std::vector<std::string> amenities_;
};

class StandardRoom : public Room {
public:
    explicit StandardRoom(int id);
    const std::string& category() const override;
};

class DeluxeRoom : public Room {
public:
    explicit DeluxeRoom(int id);
    const std::string& category() const override;
};

class SuiteRoom : public Room {
public:
    explicit SuiteRoom(int id);
    const std::string& category() const override;
};

std::vector<std::shared_ptr<Room> > createRoomCatalog();
