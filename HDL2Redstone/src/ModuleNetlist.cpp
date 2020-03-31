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
            ComponentPtr->setForcePlaced();
            ComponentPtr->setName(input); // set port name to what user specified
        } else {
            throw Exception("Input: " + input + " doesn't have a forced placement!");
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
            ComponentPtr->setForcePlaced();
            ComponentPtr->setName(output);
        } else {
            throw Exception("Output: " + output + " doesn't have a forced placement!");
        }
        Connections.push_back(std::make_unique<Connection>(output, ComponentPtr.get(), "A", false /* Add as sink*/));
        Components.push_back(std::move(ComponentPtr));
    }
}

void ModuleNetlist::ExtractNetlist::names(std::vector<std::string> nets,
                                          std::vector<std::vector<blifparse::LogicValue>> so_cover) {
    // FIXME: This currently will only work for names function as alias
    // Construct nets for $false, $true, and $undef
    if (nets.size() == 1) {
        const Cell* CellPtr;
        if (nets.at(0) == "$false") {
            CellPtr = CellLib.getCellPtr("FALSE");
        } else if (nets.at(0) == "$true") {
            CellPtr = CellLib.getCellPtr("TRUE");
        } else if (nets.at(0) == "$undef") {
            // $undef is just $false???
            CellPtr = CellLib.getCellPtr("FALSE");
        } else {
            throw Exception("Unsupported names usage in BLIF!");
        }
        auto ComponentPtr = std::make_unique<Component>(CellPtr);
        Connections.push_back(std::make_unique<Connection>(nets.at(0), ComponentPtr.get(), "Y"));
        return;
    } else if (nets.size() != 2 || so_cover.size() != 1 || so_cover.at(0).at(0) != LogicValue::TRUE ||
               so_cover.at(0).at(1) != LogicValue::TRUE) {
        throw Exception("Unsupported names usage in BLIF!");
    }
    // See if one of the name is an input or output
    auto CPtr1 = findComponent(nets.at(0));
    auto CPtr2 = findComponent(nets.at(1));
    auto Conn1 = findConnection(nets.at(0));
    auto Conn2 = findConnection(nets.at(1));

    if (!CPtr1 && !CPtr2) {
        // If names have no input or output, we merge
        // Test which connection doesn't have a source
        if (!Conn1->getSourcePortConnection().first) {
            for (const auto& C : Conn1->getSinkPortConnections()) {
                Conn2->addSink(C.first, C.second);
            }
            Connections.erase(std::remove_if(Connections.begin(), Connections.end(),
                                             [Conn1](const auto& C) { return C.get() == Conn1; }),
                              Connections.end());
        } else if (!Conn2->getSourcePortConnection().first) {
            for (const auto& C : Conn2->getSinkPortConnections()) {
                Conn1->addSink(C.first, C.second);
            }
            Connections.erase(std::remove_if(Connections.begin(), Connections.end(),
                                             [Conn2](const auto& C) { return C.get() == Conn2; }),
                              Connections.end());
        }
    } else if (CPtr1 && CPtr2) {
        // If names have both input and output, a buffer need to be injected
        const Cell* CellPtr = CellLib.getCellPtr("BUF");
        auto ComponentPtr = std::make_unique<Component>(CellPtr);
        if (CPtr1->getType() == "INPUT" && CPtr2->getType() == "OUTPUT") {
            Conn1->addSink(ComponentPtr.get(), "A");
            Conn2->addSource(ComponentPtr.get(), "Y");
        } else if (CPtr1->getType() == "OUTPUT" && CPtr2->getType() == "INPUT") {
            Conn1->addSource(ComponentPtr.get(), "Y");
            Conn2->addSink(ComponentPtr.get(), "A");
        } else {
            throw Exception("Unsupported names usage in BLIF!");
        }
        Components.push_back(std::move(ComponentPtr));
    } else if (CPtr1 && !CPtr2) {
        // If names have input or output, the other name(connection) needs to be merged
        if (CPtr1->getType() == "INPUT") {
            for (const auto& C : Conn2->getSinkPortConnections()) {
                Conn1->addSink(C.first, C.second);
            }
        } else {
            const auto& C = Conn2->getSourcePortConnection();
            Conn1->addSource(C.first, C.second);
        }
        Connections.erase(
            std::remove_if(Connections.begin(), Connections.end(), [Conn2](const auto& C) { return C.get() == Conn2; }),
            Connections.end());
    } else if (!CPtr1 && CPtr2) {
        // If names have input or output, the other name(connection) needs to be merged
        if (CPtr2->getType() == "INPUT") {
            for (const auto& C : Conn1->getSinkPortConnections()) {
                Conn2->addSink(C.first, C.second);
            }
        } else {
            const auto& C = Conn1->getSourcePortConnection();
            Conn2->addSource(C.first, C.second);
        }
        Connections.erase(
            std::remove_if(Connections.begin(), Connections.end(), [Conn1](const auto& C) { return C.get() == Conn1; }),
            Connections.end());
    }
}

void ModuleNetlist::ExtractNetlist::latch(std::string input, std::string output, blifparse::LatchType type,
                                          std::string control, blifparse::LogicValue init) {
    if (type == LatchType::RISING_EDGE) {
        if (init == LogicValue::DONT_CARE || init == LogicValue::UNKOWN) {
            const Cell* CellPtr = CellLib.getCellPtr("DFF");
            auto ComponentPtr = std::make_unique<Component>(CellPtr);
            std::string Name = control;
            auto P = [&Name](const std::unique_ptr<Connection>& ConnectionPtr) -> bool {
                return ConnectionPtr->getName() == Name;
            };
            auto It = std::find_if(Connections.begin(), Connections.end(), P);
            if (It != Connections.end()) {
                (*It)->addSink(ComponentPtr.get(), "C");
            } else {
                Connections.push_back(std::make_unique<Connection>(Name, ComponentPtr.get(), "C", false));
            }
            Name = input;
            It = std::find_if(Connections.begin(), Connections.end(), P);
            if (It != Connections.end()) {
                (*It)->addSink(ComponentPtr.get(), "D");
            } else {
                Connections.push_back(std::make_unique<Connection>(Name, ComponentPtr.get(), "D", false));
            }
            Name = output;
            It = std::find_if(Connections.begin(), Connections.end(), P);
            if (It != Connections.end()) {
                (*It)->addSource(ComponentPtr.get(), "Q");
            } else {
                Connections.push_back(std::make_unique<Connection>(Name, ComponentPtr.get(), "Q", true));
            }
            Components.push_back(std::move(ComponentPtr));
        } else {
            throw Exception("Latch cannot have initial value!");
        }
    } else {
        throw Exception("Only positive edge triggered FF is supported!");
    }
}

void ModuleNetlist::ExtractNetlist::subckt(std::string model, std::vector<std::string> ports,
                                           std::vector<std::string> nets) {
    const Cell* CellPtr = CellLib.getCellPtr(model);
    auto ComponentPtr = std::make_unique<Component>(CellPtr);
    auto It1 = ports.begin();
    auto It2 = nets.begin();
    for (; It1 != ports.end() && It2 != nets.end(); ++It1, ++It2) {
        auto Port = *It1;
        auto Net = *It2;
        auto PortDir = ComponentPtr->getPinDir(Port);
        bool isSource = true;
        switch (PortDir) {
        case Direction::Input:
            isSource = false;
            break;
        case Direction::Output:
            isSource = true;
            break;
        case Direction::Inout:
        default:
            throw Exception("Cell library has unsupported pin direction!");
        }
        auto P = [Net](const std::unique_ptr<Connection>& ConnectionPtr) -> bool {
            return ConnectionPtr->getName() == Net;
        };
        auto It = std::find_if(Connections.begin(), Connections.end(), P);
        if (It != Connections.end()) {
            if (isSource) {
                (*It)->addSource(ComponentPtr.get(), Port);
            } else {
                (*It)->addSink(ComponentPtr.get(), Port);
            }
        } else {
            Connections.push_back(std::make_unique<Connection>(Net, ComponentPtr.get(), Port, isSource));
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
    // Cleanup unused $false, $true, and $undef
    if (Connection* Conn = EN.findConnection("$false")) {
        if (!Conn->getSinkPortConnections().size()) {
            EN.Connections.erase(std::remove_if(EN.Connections.begin(), EN.Connections.end(),
                                                [Conn](const auto& C) { return C.get() == Conn; }),
                                 EN.Connections.end());
        }
    }
    if (Connection* Conn = EN.findConnection("$true")) {
        if (!Conn->getSinkPortConnections().size()) {
            EN.Connections.erase(std::remove_if(EN.Connections.begin(), EN.Connections.end(),
                                                [Conn](const auto& C) { return C.get() == Conn; }),
                                 EN.Connections.end());
        }
    }
    if (Connection* Conn = EN.findConnection("$undef")) {
        if (!Conn->getSinkPortConnections().size()) {
            EN.Connections.erase(std::remove_if(EN.Connections.begin(), EN.Connections.end(),
                                                [Conn](const auto& C) { return C.get() == Conn; }),
                                 EN.Connections.end());
        }
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
