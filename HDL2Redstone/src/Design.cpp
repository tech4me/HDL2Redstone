#include <Design.hpp>
#include <Placer.hpp>
#include <Router.hpp>
using namespace HDL2Redstone;

Design::Design(uint16_t Width_, uint16_t Height_, uint16_t Length_, const std::string& File_,
               const CellLibrary& CellLib_, const DesignConstraint& DC_)
    : Width(Width_), Height(Height_), Length(Length_), CellLib(CellLib_), DC(DC_), MN(File_, CellLib_, DC) {}

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
    Router R(*this);
    std::cout << "Now Routing..." << std::endl;
    R.route(*this);
    return false;
}

Schematic Design::exportDesign() const {
    Schematic Schem(Width, Height, Length);
    for (const auto& Component : MN.getComponents()) {
        if (Component->getPlaced()) {
            Schem.insertSubSchematic(Component->getPlacement(), Component->getSchematic(), Component->getType(),
                                     0); // last 2 arguments for debug
        }
    }
    for (const auto& Connection : MN.getConnections()) {
        if (Connection->getRouted()) {
            // TODO: Check this once we have something other than wire
            for (const auto& R : Connection->Result) {
                // if(std::get<0>(R.coord)==33&&std::get<1>(R.coord)==12&& std::get<2>(R.coord)==9){
                //     std::cout<<"FOUND wire name: "<<Connection->getName()<<std::endl;
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
