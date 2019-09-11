#include <Cell.hpp>

using namespace HDL2Redstone;

Cell::PinInfo::PinInfo(std::map<std::string, std::string> JsonInfo_) {
    std::string direct = JsonInfo_["direction"];
    if (direct == "input") {
        Dir = Direction::Input;
    } else if (direct == "output") {
        Dir = Direction::Output;
    } else {
        Dir = Direction::Inout;
    }
}

Cell::Cell(const std::string& Type_, const std::map<std::string, std::map<std::string, std::string>>& Pins_)
    : Type(Type_) {
    for (const auto& Pin : Pins_) {
        Pins.emplace(Pin.first, PinInfo(Pin.second));
    }
}

const std::string& Cell::getType() const { return Type; }
