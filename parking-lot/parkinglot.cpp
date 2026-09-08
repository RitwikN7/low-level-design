#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/*
    entities:
        - parkinglot (driver class)
        - floors
        - parking slots
        - vehicle (car, bike, truck)
        - ticket

    functions:
        - create parking lot
        - add floors
        - add slots
        - park vehicle
        - unpark vehicle
        - display free / occupied slots per vehicle type
*/

enum class VEHICLE_TYPE : uint8_t
{
    TRUCK,
    BIKE,
    CAR
};

const std::unordered_map<std::string, VEHICLE_TYPE> VEHICLE_TYPE_MAP = {
    {"TRUCK", VEHICLE_TYPE::TRUCK},
    {"BIKE",  VEHICLE_TYPE::BIKE },
    {"CAR",   VEHICLE_TYPE::CAR  },
};

struct Vehicle
{
    Vehicle(std::string registration, std::string color, VEHICLE_TYPE type)
        : registration_(std::move(registration)),
          color_(std::move(color)),
          type_(type)
    {
    }

    std::string registration_;
    std::string color_;
    VEHICLE_TYPE type_;
};

using HVehicle = std::shared_ptr<Vehicle>;

class ParkingSlot
{
public:
    ParkingSlot(VEHICLE_TYPE type, int index)
        : type_(type),
          index_(index)
    {
    }

    VEHICLE_TYPE getType() const
    {
        return type_;
    }

    void parkVehicle(HVehicle& vehicle)
    {
        parkedVehicle_ = vehicle;
    }

    void unparkVehicle()
    {
        parkedVehicle_ = nullptr;
    }

    bool isFree() const
    {
        return !parkedVehicle_;
    }

    const HVehicle& getParkedVehicle() const
    {
        return parkedVehicle_;
    }

private:
    HVehicle parkedVehicle_{nullptr};
    VEHICLE_TYPE type_{};
    int index_{};
};

using HParkingSlot = std::shared_ptr<ParkingSlot>;

class ParkingFloor
{
public:
    ParkingFloor(int slots, int index)
        : index_(index)
    {
        for (int i = 1; i <= slots; i++)
        {
            if (i == 1)
                slots_.push_back(std::make_shared<ParkingSlot>(VEHICLE_TYPE::TRUCK, i));
            else if (i <= 3)
                slots_.push_back(std::make_shared<ParkingSlot>(VEHICLE_TYPE::BIKE, i));
            else
                slots_.push_back(std::make_shared<ParkingSlot>(VEHICLE_TYPE::CAR, i));
        }
    }

    const std::vector<HParkingSlot>& getSlots() const
    {
        return slots_;
    }

    HParkingSlot getSlot(int i) const
    {
        if (i <= 0 || i > slots_.size())
            return nullptr;

        return slots_[i - 1];
    }

private:
    std::vector<HParkingSlot> slots_{};
    int index_{};
};

using HParkingFloor = std::shared_ptr<ParkingFloor>;

class ParkingSearchStrategy
{
public:
    virtual ~ParkingSearchStrategy() = default;
    virtual std::vector<std::vector<int>>
    matches(const std::vector<HParkingFloor>& floors) const = 0;
};

class SearchFreeSlots : public ParkingSearchStrategy
{
public:
    SearchFreeSlots(VEHICLE_TYPE type)
        : type_(type)
    {
    }

    std::vector<std::vector<int>> matches(const std::vector<HParkingFloor>& floors) const override
    {
        std::vector<std::vector<int>> result{};

        for (int i = 1; i <= floors.size(); i++)
        {
            result.push_back({});
            const auto& slots = floors[i - 1]->getSlots();
            for (int j = 1; j <= slots.size(); j++)
            {
                const auto& slot = slots[j - 1];
                if (slot->getType() == type_ && slot->isFree())
                    result.back().push_back(j);
            }
        }

        return result;
    }

private:
    VEHICLE_TYPE type_{};
};

class SearchOccupiedSlots : public ParkingSearchStrategy
{
public:
    SearchOccupiedSlots(VEHICLE_TYPE type)
        : type_(type)
    {
    }

    std::vector<std::vector<int>> matches(const std::vector<HParkingFloor>& floors) const override
    {
        std::vector<std::vector<int>> result{};

        for (int i = 1; i <= floors.size(); i++)
        {
            result.push_back({});
            const auto& slots = floors[i - 1]->getSlots();
            for (int j = 1; j <= slots.size(); j++)
            {
                const auto& slot = slots[j - 1];
                if (slot->getType() == type_ && !slot->isFree())
                    result.back().push_back(j);
            }
        }

        return result;
    }

private:
    VEHICLE_TYPE type_{};
};

using HParkingSearchStrategy = std::unique_ptr<ParkingSearchStrategy>;

class ParkingLot
{
public:
    ParkingLot(const std::string& parkingLotID, int numFloors, int numSlots)
        : parkingLotID_(parkingLotID),
          numFloors_(numFloors),
          numSlots_(numSlots)
    {
        for (int i = 1; i <= numFloors_; i++)
            floors_.push_back(std::make_shared<ParkingFloor>(numSlots_, i));
    }

    void parkVehicle(const std::string& registration, const std::string& color,
                     const std::string& vehicleType)
    {
        auto vehicleTypeIter = VEHICLE_TYPE_MAP.find(vehicleType);
        if (vehicleTypeIter == VEHICLE_TYPE_MAP.end())
        {
            std::cout << "Invalid vehicle type\n";
            return;
        }

        auto enVehicleType = vehicleTypeIter->second;

        parkingSearch_ = std::make_unique<SearchFreeSlots>(enVehicleType);
        auto matches = parkingSearch_->matches(floors_);
        parkingSearch_ = nullptr;

        for (int i = 1; i <= numFloors_; i++)
        {
            if (matches[i - 1].empty())
                continue;

            int slotNumber = matches[i - 1][0];
            auto slot = floors_[i - 1]->getSlot(slotNumber);

            std::string ticketID =
                parkingLotID_ + "_" + std::to_string(i) + "_" + std::to_string(slotNumber);

            HVehicle vehicle = std::make_shared<Vehicle>(registration, color, enVehicleType);
            slot->parkVehicle(vehicle);
            tickets_.insert(ticketID);
            std::cout << "Parked vehicle. Ticket ID: " << ticketID << '\n';
            return;
        }

        std::cout << "Parking Lot Full\n";
    }

    void unparkVehicle(const std::string& ticketID)
    {
        auto ticketIter = tickets_.find(ticketID);
        if (ticketIter == tickets_.end())
        {
            std::cout << "Invalid ticket ID\n";
            return;
        }

        auto pair = processTicket(ticketID);

        auto slot = floors_[pair.first - 1]->getSlot(pair.second);
        if (slot->isFree())
        {
            std::cout << "Invalid ticket ID\n";
            return;
        }

        const auto& vehicle = slot->getParkedVehicle();

        std::cout << "Unparked vehicle with Registration Number: " << vehicle->registration_
                  << " and Color: " << vehicle->color_ << '\n';

        slot->unparkVehicle();
        tickets_.erase(ticketID);
    }

    void search(const std::string& strategy, const std::string& vehicleType)
    {
        auto vehicleTypeIter = VEHICLE_TYPE_MAP.find(vehicleType);
        if (vehicleTypeIter == VEHICLE_TYPE_MAP.end())
        {
            std::cout << "Invalid vehicle type\n";
            return;
        }

        auto enVehicleType = vehicleTypeIter->second;

        if (strategy == "free_slots" || strategy == "free_count")
            parkingSearch_ = std::make_unique<SearchFreeSlots>(enVehicleType);
        else if (strategy == "occupied_slots")
            parkingSearch_ = std::make_unique<SearchOccupiedSlots>(enVehicleType);
        else
        {
            std::cout << "Invalid search param: " << strategy << '\n';
            return;
        }

        auto matches = parkingSearch_->matches(floors_);
        parkingSearch_ = nullptr;

        if (strategy == "free_count")
        {
            for (int i = 1; i <= numFloors_; i++)
            {
                int count = matches[i - 1].size();
                std::cout << "No. of free slots for " << vehicleType << " on Floor " << i << ": "
                          << count << '\n';
            }
        }
        else if (strategy == "free_slots")
        {
            for (int i = 1; i <= numFloors_; i++)
            {
                std::stringstream ssSlots{};
                int count{};
                for (const auto& j : matches[i - 1])
                {
                    if (count > 0)
                        ssSlots << ",";

                    ssSlots << j;
                    count++;
                }

                std::cout << "Free slots for " << vehicleType << " on Floor " << i << ": "
                          << ssSlots.str() << '\n';
            }
        }
        else if (strategy == "occupied_slots")
        {
            for (int i = 1; i <= numFloors_; i++)
            {
                std::stringstream ssSlots{};
                int count{};
                for (const auto& j : matches[i - 1])
                {
                    if (count > 0)
                        ssSlots << ",";

                    ssSlots << j;
                    count++;
                }

                std::cout << "Occupied slots for " << vehicleType << " on Floor " << i << ": "
                          << ssSlots.str() << '\n';
            }
        }
    }

private:
    std::pair<int, int> processTicket(const std::string& ticketID)
    {
        std::stringstream ss(ticketID);
        std::string item{};
        std::vector<std::string> res{};

        while (std::getline(ss, item, '_'))
            res.push_back(item);

        return std::make_pair(std::stoi(res[1]), std::stoi(res[2]));
    }

    std::vector<HParkingFloor> floors_{};
    std::unordered_set<std::string> tickets_{};
    std::string parkingLotID_{};
    HParkingSearchStrategy parkingSearch_{nullptr};
    int numFloors_{};
    int numSlots_{};
};

using HParkingLot = std::unique_ptr<ParkingLot>;

int main()
{
    std::cout << "Welcome to ParkingLot!\n";
    HParkingLot parkingLot{nullptr};

    while (true)
    {
        std::cout << '\n';
        std::string cmd{};
        std::getline(std::cin, cmd);

        std::stringstream ss(cmd);

        std::string primaryCmd{};
        if (!(ss >> primaryCmd))
        {
            std::cout << "No command entered!\n";
            continue;
        }

        if (primaryCmd == "exit")
        {
            std::cout << "Goodbye!\n";
            return 0;
        }
        else if (primaryCmd == "create_parking_lot")
        {
            std::string parkingLotID{};
            if (!(ss >> parkingLotID))
            {
                std::cout << "No parking lot ID entered!\n";
                continue;
            }

            std::string floors{};
            if (!(ss >> floors))
            {
                std::cout << "No number of floors entered!\n";
                continue;
            }

            int numFloors{};
            try
            {
                numFloors = std::stoi(floors);
            }
            catch (std::exception& e)
            {
                std::cout << "Invalid input for number of floors\n";
                continue;
            }

            if (numFloors <= 0)
            {
                std::cout << "Invalid input for number of floors\n";
                continue;
            }

            std::string slots{};
            if (!(ss >> slots))
            {
                std::cout << "No number of slots entered!\n";
                continue;
            }

            int numSlots{};
            try
            {
                numSlots = std::stoi(slots);
            }
            catch (std::exception& e)
            {
                std::cout << "Invalid input for number of floors\n";
                continue;
            }

            if (numSlots <= 0)
            {
                std::cout << "Invalid input for number of floors\n";
                continue;
            }

            parkingLot = std::make_unique<ParkingLot>(parkingLotID, numFloors, numSlots);
            std::cout << "Created parking lot with " << numFloors << " floors and " << numSlots
                      << " slots per floor\n";
        }
        else if (primaryCmd == "park_vehicle")
        {
            if (!parkingLot)
            {
                std::cout << "No parking lot created yet\n";
                continue;
            }

            std::string vehicleType{};
            if (!(ss >> vehicleType))
            {
                std::cout << "No vehicle type entered!\n";
                continue;
            }

            std::string registration{};
            if (!(ss >> registration))
            {
                std::cout << "No registration entered!\n";
                continue;
            }

            std::string color{};
            if (!(ss >> color))
            {
                std::cout << "No color entered!\n";
                continue;
            }

            parkingLot->parkVehicle(registration, color, vehicleType);
        }
        else if (primaryCmd == "unpark_vehicle")
        {
            if (!parkingLot)
            {
                std::cout << "No parking lot created yet\n";
                continue;
            }

            std::string ticketID{};
            if (!(ss >> ticketID))
            {
                std::cout << "No ticket ID entered!\n";
                continue;
            }

            parkingLot->unparkVehicle(ticketID);
        }
        else if (primaryCmd == "display")
        {
            if (!parkingLot)
            {
                std::cout << "No parking lot created yet\n";
                continue;
            }

            std::string strategy{};
            if (!(ss >> strategy))
            {
                std::cout << "No search parameter entered!\n";
                continue;
            }

            std::string vehicleType{};
            if (!(ss >> vehicleType))
            {
                std::cout << "No vehicle type entered!\n";
                continue;
            }

            parkingLot->search(strategy, vehicleType);
        }
        else
        {
            std::cout << "Unknown command: " << primaryCmd << '\n';
        }
    }

    return 0;
}
