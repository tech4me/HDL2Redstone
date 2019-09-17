#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <blifparse.hpp>

#include <CellLibrary.hpp>
#include <Component.hpp>
#include <Connection.hpp>

namespace HDL2Redstone {
class ModuleNetlist {
  public:
    ModuleNetlist(const std::string& File_, const CellLibrary& CellLib_);
    std::vector<std::unique_ptr<Component>>& getComponents();
    std::vector<std::unique_ptr<Connection>>& getConnections();

  private:
    class ExtractNetlist : public blifparse::Callback {
        friend class ModuleNetlist;

      public:
        ExtractNetlist(const CellLibrary& CellLib_);
        void start_parse() override {}
        void filename(std::string /*fname*/) override {}
        void lineno(int /*line_num*/) override {}
        void begin_model(std::string /*model_name*/) override {}
        void inputs(std::vector<std::string> /*inputs*/) override;
        void outputs(std::vector<std::string> /*outputs*/) override;
        void names(std::vector<std::string> /*nets*/,
                   std::vector<std::vector<blifparse::LogicValue>> /*so_cover*/) override {}
        void latch(std::string /*input*/, std::string /*output*/, blifparse::LatchType /*type*/,
                   std::string /*control*/, blifparse::LogicValue /*init*/) override {}
        void subckt(std::string /*model*/, std::vector<std::string> /*ports*/,
                    std::vector<std::string> /*nets*/) override;
        void blackbox() override {}
        void end_model() override {}
        void finish_parse() override {}
        void parse_error(const int /*curr_lineno*/, const std::string& /*near_text*/,
                         const std::string& /*msg*/) override;

        bool had_error() { return had_error_; }

      private:
        bool had_error_ = false;
        const CellLibrary& CellLib;
        std::vector<std::unique_ptr<Component>> Components;
        std::vector<std::unique_ptr<Connection>> Connections;
    };

    // Netlist data
    std::vector<std::unique_ptr<Component>> Components;
    std::vector<std::unique_ptr<Connection>> Connections;
    friend std::ostream& operator<<(std::ostream& out, const ModuleNetlist& ModuleNetlist_);
};
} // namespace HDL2Redstone
