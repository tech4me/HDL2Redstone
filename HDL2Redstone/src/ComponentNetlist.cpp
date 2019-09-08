#include <ComponentNetlist.hpp>
#include <blif_pretty_print.hpp>
#include <blifparse.hpp>
#include <fstream>

ComponentNetlist::ComponentNetlist(const std::string& File) {
    blifparse::BlifPrettyPrinter callback(true);
    blifparse::blif_parse_filename(File, callback);

    if (callback.had_error()) {
        throw "error";
    }
}
