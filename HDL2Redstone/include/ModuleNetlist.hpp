#pragma once

#include <memory>
#include <string>
#include <vector>

#include <blifparse.hpp>

#include <Component.hpp>
#include <Connection.hpp>
#include <ModulePort.hpp>

namespace HDL2Redstone {
class ModuleNetlist {
  public:
    ModuleNetlist(const std::string& File);

  private:
    class ExtractNetlist : public blifparse::Callback {
      public:
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
        void parse_error(const int curr_lineno, const std::string& near_text, const std::string& msg) override {
            fprintf(stderr, "Custom Error at line %d near '%s': %s\n", curr_lineno, near_text.c_str(), msg.c_str());
            had_error_ = true;
        }

        bool had_error() { return had_error_ = 1; }

        bool had_error_ = false;
        std::vector<std::unique_ptr<ModulePort>> ModulePorts;
        std::vector<std::unique_ptr<Component>> Components;
        std::vector<std::unique_ptr<Connection>> Connections;
    };

    // Input and output ports of the top module
    std::vector<std::unique_ptr<ModulePort>> ModulePorts;

    std::vector<std::unique_ptr<Component>> Components;
    std::vector<std::unique_ptr<Connection>> Connections;

    friend class ExtractNetlist;
};
} // namespace HDL2Redstone
