#include <Design.hpp>
#include <Placer.hpp>
#include <Router.hpp>
#include <Timing.hpp>

using namespace HDL2Redstone;

Design::Design(const std::string& File_, const CellLibrary& CellLib_, const DesignConstraint& DC_, int Seed_)
    : Width(std::get<0>(DC_.getDimension())), Height(std::get<1>(DC_.getDimension())),
      Length(std::get<2>(DC_.getDimension())), CellLib(CellLib_), DC(DC_), MN(File_, CellLib_, DC), Seed(Seed_) {}

std::tuple<uint16_t, uint16_t, uint16_t> Design::getSpace() const { return std::make_tuple(Width, Height, Length); }

const ModuleNetlist& Design::getModuleNetlist() const { return MN; }

bool Design::doPlaceAndRoute() {
    std::cout << "Placing..." << std::endl;
    Placer P(*this);
    if (P.place()) {
        return true;
    }
    std::cout << "Routing..." << std::endl;
    Router R(*this);
    if (R.route()) {
        return true;
    }
    std::cout << "Analyzing Timing Requirements..." << std::endl;
    Timing T(*this);
    // TODO: change sequence of the following
    int PropDelay = T.computePropDelay();
    std::cout << "    Combinational propagation delay = " << PropDelay << std::endl;

    std::cout << "    Hold violations..." << std::endl;
    T.findHoldViolations();
    // find path with hold violations sample usage
    // for (const auto& P : T.HoldViolatedPaths) {
    // for (const auto& C : P.CombPath) {
    //     std::cout << "(Comp,Conn: "<< C.second << ","<< C.first << ")->";
    // }
    //     std::cout << "\n    curr delay:" << P.Delay << ", need to add:" << P.DelayNeeded << std::endl;
    // }
    // std::cout<<T;
    if(R.ReTiming(T)){
        return true;
    }
    double Fmax = T.computeFmax(*this);
    if (Fmax) { // won't print if combinational cct
        std::cout << "    Sequential fmax = " << Fmax << std::endl;
    } else {
        std::cout << "    Combinational circuit, no fmax" << std::endl;
    }

    // find longest/shortest path given src and dest, sample usage; T.src and T.dest just used for testing in mux2to1
    /*std::vector<Component*> Path = T.findShortestDelay(T.src, T.dest);
    for (auto c : Path) {
        std::cout<<c<<"<-";
    }
    std::cout<<std::endl;*/
    return false;
}

Schematic Design::exportDesign() const {
    Schematic Schem(Width, Height, Length);
    for (const auto& Component : MN.getComponents()) {
        if (Component->getPlaced()) {
            // auto names = Component->getPinNames();
            // for (auto n : names) std::cout<<n<<std::endl;
            /*if (Component->getType() == "INPUT")
                std::cout<<"input: "<<(int)Component->getPinFacing("Y")<<std::endl;
            else if (Component->getType() == "OUTPUT")
                std::cout<<"output "<<(int)Component->getPinFacing("A")<<std::endl;	*/ //for checking port facing
            Schem.insertSubSchematic(Component->getPlacement(), Component->getSchematic(), Component->getType(), 0,
                                     Component->getName()); // type and 0 for debug, name argument for port names
        }
    }
    for (const auto& Connection : MN.getConnections()) {
        if (Connection->getRouted()) {
            // TODO: Check this once we have something other than wire
            for (const auto& R : Connection->RouteResult) {
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
