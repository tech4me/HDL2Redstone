#pragma once

#include <map>
#include <string>

#include <Placement.hpp>

namespace HDL2Redstone {
class DesignConstraint {
  public:
    bool readDCFile(const std::string& File_);
    void addForcedPlacement(const std::string& Name_, const Placement& P_) { ForcedPlacement.emplace(Name_, P_); }
    const std::map<std::string, Placement>& getForcedPlacement() const { return ForcedPlacement; }

  private:
    std::map<std::string, Placement> ForcedPlacement;
};
} // namespace HDL2Redstone
