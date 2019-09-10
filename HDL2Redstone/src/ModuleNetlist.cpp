#include <fstream>
#include <iostream>

#include <blif_pretty_print.hpp>

#include <ModuleNetlist.hpp>

using namespace blifparse;
using namespace HDL2Redstone;

void ModuleNetlist::ExtractNetlist::inputs(std::vector<std::string> inputs) {
    for (const auto& input : inputs) {
        // Cell* CellPtr = CellLib.getCellPtr("INPUT");
        Cell* CellPtr = nullptr;
        auto ComponentPtr = std::make_unique<Component>(CellPtr);
        Connections.push_back(std::make_unique<Connection>(input, ComponentPtr.get(), "INPUT"));
        Components.push_back(std::move(ComponentPtr));
    }
}

void ModuleNetlist::ExtractNetlist::outputs(std::vector<std::string> outputs) {
    for (const auto& output : outputs) {
        // Cell* CellPtr = CellLib.getCellPtr("OUTPUT");
        Cell* CellPtr = nullptr;
        auto ComponentPtr = std::make_unique<Component>(CellPtr);
        Connections.push_back(std::make_unique<Connection>(output, ComponentPtr.get(), "OUTPUT"));
        Components.push_back(std::move(ComponentPtr));
    }
}

void ModuleNetlist::ExtractNetlist::subckt(std::string model, std::vector<std::string> ports,
                                           std::vector<std::string> nets) {
    // Cell* CellPtr = CellLib.getCellPtr(model);
    Cell* CellPtr = nullptr;
    auto ComponentPtr = std::make_unique<Component>(CellPtr);
    auto It1 = ports.begin();
    auto It2 = nets.begin();
    for (; It1 != ports.end() && It2 != nets.end(); ++It1, ++It2) {
        auto port = *It1;
        auto net = *It2;
        auto P = [net](const std::unique_ptr<Connection>& ConnectionPtr) -> bool {
            if (ConnectionPtr->getName() == net) {
                return true;
            } else {
                return false;
            }
        };
        auto It = std::find_if(Connections.begin(), Connections.end(), P);
        if (It != Connections.end()) {
            // TODO: This is assuming source of a connection will always be before sink
            (*It)->addSink(ComponentPtr.get(), port);
        } else {
            Connections.push_back(std::make_unique<Connection>(net, ComponentPtr.get(), port));
        }
    }
    Components.push_back(std::move(ComponentPtr));
}

void ModuleNetlist::ExtractNetlist::parse_error(const int curr_lineno, const std::string& near_text,
                                                const std::string& msg) {
    fprintf(stderr, "Custom Error at line %d near '%s': %s\n", curr_lineno, near_text.c_str(), msg.c_str());
    had_error_ = true;
}

ModuleNetlist::ModuleNetlist(const std::string& File) {
    ExtractNetlist EN;
    blif_parse_filename(File, EN);
    if (EN.had_error()) {
        throw "error";
    }
    // Move all our internal data structure
    Components = std::move(EN.Components);
    Connections = std::move(EN.Connections);
}
