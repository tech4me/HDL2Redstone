#include <fstream>

#include <blif_pretty_print.hpp>

#include <ModuleNetlist.hpp>

using namespace blifparse;
using namespace HDL2Redstone;

ModuleNetlist::ModuleNetlist(const std::string& File) {
    BlifPrettyPrinter callback(true);
    blif_parse_filename(File, callback);
    if (callback.had_error()) {
        throw "error";
    }

    ExtractNetlist EN;
    blif_parse_filename(File, EN);
    if (EN.had_error()) {
        throw "error";
    }
}
