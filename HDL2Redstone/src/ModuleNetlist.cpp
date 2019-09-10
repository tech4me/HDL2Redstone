#include <fstream>
#include <iostream>

#include <blif_pretty_print.hpp>

#include <ModuleNetlist.hpp>

using namespace blifparse;
using namespace HDL2Redstone;

void ModuleNetlist::ExtractNetlist::inputs(std::vector<std::string> inputs) {
    for (const auto& input : inputs) {
        ModulePorts.push_back(std::make_unique<ModulePort>(input, PortType::Input));
    }
}

void ModuleNetlist::ExtractNetlist::outputs(std::vector<std::string> outputs) {
    for (const auto& output : outputs) {
        ModulePorts.push_back(std::make_unique<ModulePort>(output, PortType::Output));
    }
}

void ModuleNetlist::ExtractNetlist::subckt(std::string model, std::vector<std::string> ports,
                                           std::vector<std::string> nets) {
    //Cell* CellPtr = CellLib.getCellPtr();
    Cell* CellPtr = nullptr;
    auto ComponentPtr = std::make_unique<Component>(CellPtr);
    if (ports.size() != nets.size()) {
        had_error();
    }
    auto It1 = ports.begin();
    auto It2 = nets.begin();
    for (; It1 != ports.end() && It2 != nets.end(); ++It1, ++It2) {
        auto port = *It1;
        auto net = *It2;
        auto P = [net](const std::unique_ptr<Connection>& ConnectionPtr) -> bool {
            if ("" == net) {
            //if (ConnectionPtr->getName() == net) {
                return true;
            } else {
                return false;
            }
        };
        auto It = std::find_if(Connections.begin(), Connections.end(), P);
        if (It != Connections.end()) {

        } else {
            Connections.push_back(std::make_unique<Connection>(net, ComponentPtr.get(), port));
        }
    }
    Components.push_back(std::move(ComponentPtr));
}

ModuleNetlist::ModuleNetlist(const std::string& File) {
    BlifPrettyPrinter callback(true);
    blif_parse_filename(File, callback);
    if (callback.had_error()) {
        throw "error";
    }

    ExtractNetlist EN;
    blif_parse_filename(File, EN);
    if (EN.had_error()) {
        throw "error";
    }
    ModulePorts = std::move(EN.ModulePorts);
    for (auto& ptr : ModulePorts) {
        std::cout << ptr->getName() << std::endl;
    }
}
