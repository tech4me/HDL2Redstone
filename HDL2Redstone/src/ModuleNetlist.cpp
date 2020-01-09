#include <algorithm>
#include <fstream>

#include <blif_pretty_print.hpp>

#include <Component.hpp>
#include <Exception.hpp>
#include <ModuleNetlist.hpp>

using namespace blifparse;
using namespace HDL2Redstone;

ModuleNetlist::ExtractNetlist::ExtractNetlist(const CellLibrary& CellLib_, const DesignConstraint& DC_)
    : CellLib(CellLib_), DC(DC_) {}

void ModuleNetlist::ExtractNetlist::inputs(std::vector<std::string> inputs) {
    for (const auto& input : inputs) {
        const Cell* CellPtr = CellLib.getCellPtr("INPUT");
        auto ComponentPtr = std::make_unique<Component>(CellPtr);
        auto It = DC.getForcedPlacement().find(input);
        if (It != DC.getForcedPlacement().end()) {
            ComponentPtr->setPlacement(It->second.X, It->second.Y, It->second.Z, It->second.Orient);
            ComponentPtr->setPlaced(true);
        }
        Connections.push_back(std::make_unique<Connection>(input, ComponentPtr.get(), "Y"));
        Components.push_back(std::move(ComponentPtr));
    }
}

void ModuleNetlist::ExtractNetlist::outputs(std::vector<std::string> outputs) {
    for (const auto& output : outputs) {
        const Cell* CellPtr = CellLib.getCellPtr("OUTPUT");
        auto ComponentPtr = std::make_unique<Component>(CellPtr);
        auto It = DC.getForcedPlacement().find(output);
        if (It != DC.getForcedPlacement().end()) {
            ComponentPtr->setPlacement(It->second.X, It->second.Y, It->second.Z, It->second.Orient);
            ComponentPtr->setPlaced(true);
        }
        Connections.push_back(std::make_unique<Connection>(output, ComponentPtr.get(), "A", false /* Add as sink*/));
        Components.push_back(std::move(ComponentPtr));
    }
}

void ModuleNetlist::ExtractNetlist::subckt(std::string model, std::vector<std::string> ports,
                                           std::vector<std::string> nets) {
    const Cell* CellPtr = CellLib.getCellPtr(model);
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
            // CHECK: This is assuming source of a connection will always be before sink
            if ((*It)->getSourcePortConnection() == std::pair<Component*, std::string>()) { // Not set
                (*It)->addSource(ComponentPtr.get(), port);
            } else {
                (*It)->addSink(ComponentPtr.get(), port);
            }
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

ModuleNetlist::ModuleNetlist(const std::string& File_, const CellLibrary& CellLib_, const DesignConstraint& DC_) {
    ExtractNetlist EN(CellLib_, DC_);
    blif_parse_filename(File_, EN);
    if (EN.had_error()) {
        throw Exception("BLIF parsing failed.");
    }
    // Move all our internal data structure
    Components = std::move(EN.Components);
    Connections = std::move(EN.Connections);
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const ModuleNetlist& ModuleNetlist_) {
    out << "Modules:" << std::endl;
    int i = 0;
    for (const auto& T : ModuleNetlist_.Components) {
        out << i << " ";
        out << *T;
        i++;
    }
    out << "Connections:" << std::endl;
    i = 0;
    for (const auto& T : ModuleNetlist_.Connections) {
        out << i << " ";
        out << *T;
        i++;
    }
    return out;
}
} // namespace HDL2Redstone
