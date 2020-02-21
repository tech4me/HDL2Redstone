#include <Design.hpp>
#include <Placer.hpp>
#include <Router.hpp>
using namespace HDL2Redstone;

Design::Design(const std::string& File_, const CellLibrary& CellLib_, const DesignConstraint& DC_)
    : Width(std::get<0>(DC_.getDimension())), Height(std::get<1>(DC_.getDimension())),
      Length(std::get<2>(DC_.getDimension())), CellLib(CellLib_), DC(DC_), MN(File_, CellLib_, DC) {}

const std::tuple<uint16_t, uint16_t, uint16_t> Design::getSpace() const {
    return std::make_tuple(Width, Height, Length);
}

const ModuleNetlist& Design::getModuleNetlist() const { return MN; }

bool Design::doPlaceAndRoute() {
    std::cout << "Now Placing..." << std::endl;
    Placer P(*this);
    if (P.place()) {
        return true;
    }
    std::cout << "Now Routing..." << std::endl;
    Router R(*this);
    R.route(*this);
    return false;
}

Schematic Design::exportDesign() const {
    Schematic Schem(Width, Height, Length);
    for (const auto& Component : MN.getComponents()) {
        if (Component->getPlaced()) {
            // auto names = Component->getPinNames();
            // for (auto n : names) std::cout<<n<<std::endl;
            if (Component->getType() == "INPUT")
                std::cout << "input:" << (int)Component->getPinFacing("Y") << std::endl;
            else if (Component->getType() == "OUTPUT")
                std::cout << "output" << (int)Component->getPinFacing("A") << std::endl;
            Schem.insertSubSchematic(Component->getPlacement(), Component->getSchematic(), Component->getType(), 0,
                                     Component->getName()); // type and 0 for debug, name argument for port names
        }
    }
    for (const auto& Connection : MN.getConnections()) {
        if (Connection->getRouted()) {
            // TODO: Check this once we have something other than wire
            for (const auto& R : Connection->Result) {
                // if(std::get<0>(R.coord)==35&&std::get<1>(R.coord)==11&& std::get<2>(R.coord)==1){
                //     std::cout<<"FOUND wire name: "<<Connection->getName()<<std::endl;
                // }
                // if(std::get<0>(R.coord)==35&&std::get<1>(R.coord)==12&& std::get<2>(R.coord)==1){
                //     std::cout<<"FOUND wire1 name: "<<Connection->getName()<<std::endl;
                // }
                Schem.insertSubSchematic({std::get<0>(R.coord), std::get<1>(R.coord), std::get<2>(R.coord), R.Ori},
                                         R.CellPtr->getSchematic(), R.CellPtr->getType(),
                                         1); // last 2 arguments for debug
            }
            /*
            const auto& Ports = Connection->getPortConnection();
            for (const auto& Port : Ports) {
                for (const auto& Segment : std::get<2>(Port).getParameters()) {
                    Schem.insertSubSchematic(
                        {std::get<0>(Segment), std::get<1>(Segment), std::get<2>(Segment), Orientation::ZeroCW},
                        CellLib.getCellPtr("redstone_dust")->getSchematic());
                }
            }
            */
        }
    }
    return Schem;
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Design& Design_) {
    out << "Design Layout:"
        << "\n"
        << Design_.MN;
    return out;
}
} // namespace HDL2Redstone
