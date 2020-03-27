#include <Connection.hpp>

using namespace HDL2Redstone;
class SimpleResult {
  public:
    SimpleResult(std::tuple<uint16_t, uint16_t, uint16_t> coord_, int PartialNetID_)
        : coord(coord_), PartialNetID(PartialNetID_){};
    std::tuple<uint16_t, uint16_t, uint16_t> coord;
    int PartialNetID;
};
struct SimpleResultCmp {
    bool operator()(const SimpleResult& lhs, const SimpleResult& rhs) const {
        if (lhs.coord < rhs.coord)
            return true;
        else
            return false;
    }
};

Connection::Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_, bool IsSource)
    : Name(Name_) {
    Unable_Routing = 0;
    if (IsSource) {
        SourcePortConnection = std::make_pair(ComponentPtr_, PortName_);
    } else {
        addSink(ComponentPtr_, PortName_);
    }
}

void Connection::addSource(Component* ComponentPtr_, const std::string& PortName_) {
    SourcePortConnection = std::make_pair(ComponentPtr_, PortName_);
}

void Connection::addSink(Component* ComponentPtr_, const std::string& PortName_) {
    SinkPortConnections.push_back(std::make_pair(ComponentPtr_, PortName_));
}

std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> Connection::checkRouteResult() {
    std::map<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t> ResultsMap;
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> IllegalPoints;
    for (const auto R : Result) {
        // std::cout<<"in check route result, " <<std::get<0>(R.coord)<<" "<<std::get<1>(R.coord)<<"
        // "<<std::get<2>(R.coord)<<std::endl;
        const auto it = ResultsMap.find(R.coord);
        if (it != ResultsMap.end()) {
            switch (it->second) {
            case 0:
                IllegalPoints.insert({std::get<0>(R.coord), std::get<1>(R.coord) - 1, std::get<2>(R.coord), 1});
                break;
            case 1:
            case 2:
                IllegalPoints.insert({std::get<0>(R.coord), std::get<1>(R.coord), std::get<2>(R.coord), 0});
                break;
            case 3:
                IllegalPoints.insert({std::get<0>(R.coord), std::get<1>(R.coord) + 1, std::get<2>(R.coord), 0});
                break;
            case 4:
                IllegalPoints.insert({std::get<0>(R.coord), std::get<1>(R.coord) + 1, std::get<2>(R.coord), 1});
                break;
            default:
                std::cout << "wrong" << std::endl;
                break;
            }
        } else {
            uint16_t x = std::get<0>(R.coord);
            uint16_t y = std::get<1>(R.coord);
            uint16_t z = std::get<2>(R.coord);
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y + 2, z}, 0));
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y + 1, z}, 1));
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y, z}, 2));
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y - 1, z}, 3));
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y - 2, z}, 4));
        }
    }
    return IllegalPoints;
}

std::set<std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>>>
Connection::checkRouteResult_repeater() { // TODO now it assume single wire congestion is diff from multi
    std::set<std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>>> ret;
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> possible_congestion;
    if (!SubResult.empty()) {
        for (int i = 0; i < SubResult.size(); i++) {
            checkRouteResult_repeater_helper(ret, SubResult[i]);
        }
        for (int i = 0; i < SubResult.size(); i++) {
            if (i == SubResult.size() - 1) {
                break;
            }
            for (int j = i + 1; j < SubResult.size(); j++) {
                possible_congestion.clear();
                int start_search;
                for (start_search = 0; start_search < SubResult[i].size(); start_search++) {
                    if (start_search == SubResult[j].size()) {
                        break;
                    }
                    if ((std::get<0>(SubResult[i][start_search]) == std::get<0>(SubResult[j][start_search])) &&
                        (std::get<1>(SubResult[i][start_search]) == std::get<1>(SubResult[j][start_search])) &&
                        (std::get<2>(SubResult[i][start_search]) == std::get<2>(SubResult[j][start_search]))) {
                        // do nothing
                    } else {
                        break;
                    }
                }
                if (start_search != SubResult[j].size()) {
                    for (int prep = start_search; prep < SubResult[i].size(); prep++) {
                        possible_congestion.push_back(SubResult[i][prep]);
                    }
                    for (int prep = start_search; prep < SubResult[j].size(); prep++) {
                        possible_congestion.push_back(SubResult[j][prep]);
                    }
                    if (!possible_congestion.empty()) {
                        checkRouteResult_repeater_helper(ret, possible_congestion);
                    }
                }
            }
        }
    }
    // if(!ret.empty()){
    // std::cout<<"FIND PAIR: "<<std::endl;
    // for(auto& it:ret){
    //     std::cout<<std::get<0>(it.first)<<" "<<std::get<1>(it.first)<<" "<<std::get<2>(it.first)
    //         <<" neighbor "<<std::get<0>(it.second)<<" "<<std::get<1>(it.second)<<" "<<std::get<2>(it.second)
    //         <<std::endl;
    // }
    // }
    return ret;
}

void Connection::checkRouteResult_repeater_helper(
    std::set<std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>>>& ret,
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& possible_congestion) {
    std::set<SimpleResult, SimpleResultCmp> SR;
    for (const auto it : possible_congestion) {
        // if(it.CellPtr->getType()=="WIRE"){
        SR.insert(SimpleResult(std::make_tuple(std::get<0>(it), std::get<1>(it), std::get<2>(it)), std::get<3>(it)));
        // std::cout<<"result "<<
        // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)<<std::endl;
        //}
    }
    auto tmpptr = SR.end();
    for (const auto& it : SR) {
        tmpptr = SR.find(SimpleResult(
            std::make_tuple(std::get<0>(it.coord) + 1, std::get<1>(it.coord), std::get<2>(it.coord)), it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            // std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(SimpleResult(
            std::make_tuple(std::get<0>(it.coord) - 1, std::get<1>(it.coord), std::get<2>(it.coord)), it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(SimpleResult(
            std::make_tuple(std::get<0>(it.coord), std::get<1>(it.coord), std::get<2>(it.coord) + 1), it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(SimpleResult(
            std::make_tuple(std::get<0>(it.coord), std::get<1>(it.coord), std::get<2>(it.coord) - 1), it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }

        tmpptr = SR.find(
            SimpleResult(std::make_tuple(std::get<0>(it.coord) + 1, std::get<1>(it.coord) + 1, std::get<2>(it.coord)),
                         it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(
            SimpleResult(std::make_tuple(std::get<0>(it.coord) - 1, std::get<1>(it.coord) + 1, std::get<2>(it.coord)),
                         it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(
            SimpleResult(std::make_tuple(std::get<0>(it.coord), std::get<1>(it.coord) + 1, std::get<2>(it.coord) + 1),
                         it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(
            SimpleResult(std::make_tuple(std::get<0>(it.coord), std::get<1>(it.coord) + 1, std::get<2>(it.coord) - 1),
                         it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }

        tmpptr = SR.find(
            SimpleResult(std::make_tuple(std::get<0>(it.coord) + 1, std::get<1>(it.coord) - 1, std::get<2>(it.coord)),
                         it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(
            SimpleResult(std::make_tuple(std::get<0>(it.coord) - 1, std::get<1>(it.coord) - 1, std::get<2>(it.coord)),
                         it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(
            SimpleResult(std::make_tuple(std::get<0>(it.coord), std::get<1>(it.coord) - 1, std::get<2>(it.coord) + 1),
                         it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
        tmpptr = SR.find(
            SimpleResult(std::make_tuple(std::get<0>(it.coord), std::get<1>(it.coord) - 1, std::get<2>(it.coord) - 1),
                         it.PartialNetID));
        if (tmpptr != SR.end() && tmpptr->PartialNetID != it.PartialNetID) {
            //             std::cout<<"find repeater loop "<<
            // std::get<0>(it.coord)<<" "<<std::get<1>(it.coord)<<" "<<std::get<2>(it.coord)
            // <<" neighbor "<<std::get<0>(tmpptr->coord)<<" "<<std::get<1>(tmpptr->coord)<<"
            // "<<std::get<2>(tmpptr->coord)
            // <<" ID: "<<tmpptr->PartialNetID<<" , "<<it.PartialNetID<<std::endl;
            bool flag = 0;
            for (auto& i : ret) {
                if (((it.coord == i.first) && (tmpptr->coord == i.second)) ||
                    ((tmpptr->coord == i.first) && (it.coord == i.second))) {
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                ret.insert(std::pair(it.coord, tmpptr->coord));
            // break;
        }
    }
}

void Connection::wireAlign(uint16_t& x, uint16_t& z, Orientation ori) {
    if (ori == HDL2Redstone::Orientation::OneCW) {
        x++;
    } else if (ori == HDL2Redstone::Orientation::TwoCW) {
        x++;
        z++;
    } else if (ori == HDL2Redstone::Orientation::ThreeCW) {
        z++;
    }
}

// calculate delay caused by repeater for path to each sink
void Connection::calculateDelay() {
    for (int i = 0; i < SinkPortConnections.size(); i++) {
        SinkDelays.push_back(0);
    }

    for (auto Con : Result) {
        if (Con.SinkId != -1) {
            SinkDelays[Con.SinkId]++;
        }
    }
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Connection& Connection_) {
    out << "Connection Name:" << Connection_.Name << std::endl;
    out << "Source:" << *(Connection_.SourcePortConnection.first)
        << "    Port Name:" << Connection_.SourcePortConnection.second << std::endl;
    for (const auto& T : Connection_.SinkPortConnections) {
        out << "    Sink:" << *(T.first) << "    Port Name:" << T.second << std::endl;
    }
    return out;
}
} // namespace HDL2Redstone
