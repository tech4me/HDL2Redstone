#include <fstream>

#include <blif_pretty_print.hpp>
#include <blifparse.hpp>

#include <ComponentNetlist.hpp>

using namespace HDL2Redstone;

ComponentNetlist::ComponentNetlist(const std::string& File) {
    blifparse::BlifPrettyPrinter callback(true);
    blifparse::blif_parse_filename(File, callback);

    if (callback.had_error()) {
        throw "error";
    }
}
