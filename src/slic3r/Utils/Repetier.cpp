///|/ Copyright (c) Prusa Research 2020 - 2023 Oleksandra Iushchenko @YuSanka, David Kocík @kocikdav, Lukáš Matěna @lukasmatena, Vojtěch Bubník @bubnikv
///|/
///|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
///|/
#include "Repetier.hpp"

#include <algorithm>
#include <sstream>
#include <exception>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <wx/progdlg.h>


#include "libslic3r/PrintConfig.hpp"
#include "slic3r/GUI/I18N.hpp"
#include "slic3r/GUI/GUI.hpp"
#include "slic3r/GUI/format.hpp"
#include "Http.hpp"
#include "nlohmann/json.hpp"


namespace fs = boost::filesystem;
namespace pt = boost::property_tree;
using json = nlohmann::json;

namespace Slic3r {

Repetier::Repetier(DynamicPrintConfig *config) :
    host(config->opt_string("print_host")),
    apikey(config->opt_string("printhost_apikey")),
    cafile(config->opt_string("printhost_cafile")),
    port(config->opt_string("printhost_port"))
{}

const char* Repetier::get_name() const { return "Repetier"; }



static wxString validate_repetier(const boost::optional<std::string>& name,
                              const boost::optional<std::string>& soft,
                              const boost::optional<std::string>& printers)
{
    if (soft) {
        // See https://github.com/prusa3d/PrusaSlicer/issues/7807:
        // Repetier allows "rebranding", so the "name" value is not reliable when detecting
        // server type. Newer Repetier versions send "software", which should be invariant.
        if ((*soft) == "Repetier-Server")
            return GUI::format_wxstr(_L("Mismatched type of print host: %s"), (soft ? *soft : "Repetier"));
    } else {
        // If there is no "software" value, validate there is at lest a printer field
        if (!name.has_value())
            return GUI::format_wxstr(_L("Can't process the repetier return message: missing field '%s'"), "name");
        else if (!printers.has_value()) {
            return GUI::format_wxstr(_L("Can't process the repetier return message: missing field '%s'"), "printers");
        }
    }
    return "";
}



bool Repetier::test(wxString &msg) const
{
    // Since the request is performed synchronously here,
    // it is ok to refer to `msg` from within the closure

    const char *name = get_name();

    bool res = true;
    auto url = make_url("printer/info");

    BOOST_LOG_TRIVIAL(info) << boost::format("%1%: List version at: %2%") % name % url;

    auto http = Http::get(std::move(url));
    set_auth(http);
    
    http.on_error([&](std::string body, std::string error, unsigned status) {
            BOOST_LOG_TRIVIAL(error) << boost::format("%1%: Error getting version: %2%, HTTP %3%, body: `%4%`") % name % error % status % body;
            res = false;
            msg = format_error(body, error, status);
        })
        .on_complete([&](std::string body, unsigned) {
            BOOST_LOG_TRIVIAL(debug) << boost::format("%1%: Got version: %2%") % name % body;

            try {
                std::stringstream ss(body);
                pt::ptree ptree;
                pt::read_json(ss, ptree);

                const auto text = ptree.get_optional<std::string>("name");
                const auto soft = ptree.get_optional<std::string>("software");
                const auto printers = ptree.get_optional<std::string>("printers");
                wxString error_msg = validate_repetier(text, soft, printers);
                if (! error_msg.empty()) {
                    msg = error_msg;
                }
            }
            catch (const std::exception &) {
                res = false;
                msg = "Could not parse server response";
            }
        })
        .perform_sync();

    return res;
}

wxString Repetier::get_test_failed_msg (wxString &msg) const
{
    return GUI::format_wxstr(_L("Could not connect to %s: %s\n\n%s"), get_name(), msg,
                             _L("Note: Repetier version at least 0.90.0 is required."));
}

bool Repetier::upload(PrintHostUpload upload_data, ProgressFn prorgess_fn, ErrorFn error_fn, InfoFn info_fn) const
{
    const char *name = get_name();

    const auto upload_filename = upload_data.upload_path.filename();
    const auto upload_parent_path = upload_data.upload_path.parent_path();

    wxString test_msg;
    if (! test(test_msg)) {
        error_fn(std::move(test_msg));
        return false;
    }

    bool res = true;

    auto url = upload_data.post_action == PrintHostPostUploadAction::StartPrint
        ? make_url((boost::format("printer/job/%1%") % port).str())
        : make_url((boost::format("printer/model/%1%") % port).str());

    BOOST_LOG_TRIVIAL(info) << boost::format("%1%: Uploading file %2% at %3%, filename: %4%, path: %5%, print: %6%, group: %7%")
        % name
        % upload_data.source_path
        % url
        % upload_filename.string()
        % upload_parent_path.string()
        % (upload_data.post_action == PrintHostPostUploadAction::StartPrint ? "true" : "false")
        % upload_data.group;

    auto http = Http::post(std::move(url));
    set_auth(http);

    if (! upload_data.group.empty() && upload_data.group != _u8L("Default")) {
        http.form_add("group", upload_data.group);
    }

    if(upload_data.post_action == PrintHostPostUploadAction::StartPrint) {
        http.form_add("name", upload_filename.string());
        http.form_add("autostart", "true"); // See https://github.com/prusa3d/PrusaSlicer/issues/7807#issuecomment-1235519371
    }

    http.form_add("a", "upload")
        .form_add_file("filename", upload_data.source_path.string(), upload_filename.string())
        .on_complete([&](std::string body, unsigned status) {
            BOOST_LOG_TRIVIAL(debug) << boost::format("%1%: File uploaded: HTTP %2%: %3%") % name % status % body;
        })
        .on_error([&](std::string body, std::string error, unsigned status) {
            BOOST_LOG_TRIVIAL(error) << boost::format("%1%: Error uploading file: %2%, HTTP %3%, body: `%4%`") % name % error % status % body;
            error_fn(format_error(body, error, status));
            res = false;
        })
        .on_progress([&](Http::Progress progress, bool &cancel) {
            prorgess_fn(std::move(progress), cancel);
            if (cancel) {
                // Upload was canceled
                BOOST_LOG_TRIVIAL(info) << "Repetier: Upload canceled";
                res = false;
            }
        })
        .perform_sync();

    return res;
}

std::string removeHttpPrefix(const std::string& host) {
    std::string cleanedHost = host;

    const std::string httpPrefix = "http://";
    const std::string httpsPrefix = "https://";

    if (cleanedHost.compare(0, httpPrefix.size(), httpPrefix) == 0) {
        cleanedHost.erase(0, httpPrefix.size());
    } else if (cleanedHost.compare(0, httpsPrefix.size(), httpsPrefix) == 0) {
        cleanedHost.erase(0, httpsPrefix.size());
    }

    return cleanedHost;
}

bool Repetier::cooldown_printer() const {
    
    std::string endpoint = "/printer/api/" + port + "?a=cooldown";
    std::string jsonData = R"({"extruder":1, "bed":1, "chamber":1})";

    std::string encoded_json = Http::url_encode(jsonData);
    std::string cleanedHost = removeHttpPrefix(host);

    std::string url = "http://" + cleanedHost + endpoint + "&data=" + encoded_json;
    bool res = true;
    
    auto http = Http::post(std::move(url));
    set_auth(http);
    
    http.form_add("a", "cooldown")
        .on_complete([&](std::string body, unsigned status) {
            std::cout << "Cooldown was successful" << std::endl;
            res = true;
        })
        .on_error([&](std::string body, std::string error, unsigned status) {
            std::cout << "Error preheating" << error << std::endl;
            res = false;
        })
        .perform_sync();
    
    
    return res;
}

bool Repetier::preheat_printer(DynamicPrintConfig config) const {
    
    bool res = true;
    size_t first_layer_temp_count = config.option<ConfigOptionInts>("first_layer_temperature")->size();
    
    for (int i = 0; i < first_layer_temp_count; i++) {
        
        int first_layer_temp = config.option<ConfigOptionInts>("first_layer_temperature")->get_at(i);
        
        std::string endpoint = "/printer/api/" + port + "?a=setExtruderTemperature";
        
        //{"temperature":215,"extruder":0}
        std::string jsonData = R"({"temperature": )" + std::to_string(first_layer_temp) + R"(,"extruder": )" + std::to_string(i) + "}";
        
        std::string cleanedHost = removeHttpPrefix(host);
        std::string encoded_json = Http::url_encode(jsonData);
        
        std::string url = "http://" + cleanedHost + endpoint + "&data=" + encoded_json;
        
        auto http = Http::post(std::move(url));
        set_auth(http);
        
        http.form_add("a", "setExtruderTemperature")
            .on_complete([&](std::string body, unsigned status) {
                std::cout << "Preheat was successful" << std::endl;
            })
            .on_error([&](std::string body, std::string error, unsigned status) {
                std::cout << "Error preheating" << error << std::endl;
                res = false;
            })
            .perform_sync();
        
    }
    
    return res;
}

void Repetier::get_printer_config(const CompletionHandler& handler) const {
    std::string endpoint = "/printer/api/" + port + "?a=getPrinterConfig";
    std::string url      = make_url((boost::format("printer/api/%1%") % port).str());
    json json_response;

    auto http = Http::get(std::move(url));
    set_auth(http);

    http.form_add("a", "getPrinterConfig")
        .on_complete([&](std::string body, unsigned status) {
            std::cout << "Getting printer config was successful: " << body << std::endl;
            json_response = json::parse(body);
            handler(json_response, true, "");  // Call handler with success
        })
        .on_error([&](std::string body, std::string error, unsigned status) {
            std::cout << "Error getting printer config: " << error << std::endl;
            handler(json(), false, error);  // Call handler with error
        })
        .perform_sync();
}


void Repetier::collect_json_values(const json &j, const std::string &key, std::vector<json> &results)
{
    if (j.is_object()) {
        if (j.contains(key)) {
            results.push_back(j.at(key));
        }
        for (const auto &item : j.items()) { collect_json_values(item.value(), key, results); }
    } else if (j.is_array()) {
        for (const auto &item : j) { collect_json_values(item, key, results); }
    }
}

std::vector<json> Repetier::get_all_json_values(const json &j, const std::string &key)
{
    std::vector<json> results;
    collect_json_values(j, key, results);
    return results;
}

void Repetier::set_auth(Http &http) const
{
    http.header("X-Api-Key", apikey);

    if (! cafile.empty()) {
        http.ca_file(cafile);
    }
}

std::string Repetier::make_url(const std::string &path) const
{
    if (host.find("http://") == 0 || host.find("https://") == 0) {
        if (host.back() == '/') {
            return (boost::format("%1%%2%") % host % path).str();
        } else {
            return (boost::format("%1%/%2%") % host % path).str();
        }
    } else {
        return (boost::format("http://%1%/%2%") % host % path).str();
    }
}

bool Repetier::get_groups(wxArrayString& groups) const
{
    bool res = true;
    
    const char *name = get_name();
    auto url = make_url((boost::format("printer/api/%1%") % port).str());

    BOOST_LOG_TRIVIAL(info) << boost::format("%1%: Get groups at: %2%") % name % url;

    auto http = Http::get(std::move(url));
    set_auth(http);
    http.form_add("a", "listModelGroups");
    http.on_error([&](std::string body, std::string error, unsigned status) {
            BOOST_LOG_TRIVIAL(error) << boost::format("%1%: Error getting version: %2%, HTTP %3%, body: `%4%`") % name % error % status % body;
        })
        .on_complete([&](std::string body, unsigned) {
            BOOST_LOG_TRIVIAL(debug) << boost::format("%1%: Got groups: %2%") % name % body;

            try {
                std::stringstream ss(body);
                pt::ptree ptree;
                pt::read_json(ss, ptree);

                BOOST_FOREACH(boost::property_tree::ptree::value_type &v, ptree.get_child("groupNames.")) {
                    if (v.second.data() == "#") {
                        groups.push_back(_L("Default"));
                    } else {
                        // Is it safe to assume that the data are utf-8 encoded?
                        groups.push_back(GUI::from_u8(v.second.data()));
                    }
                }
            }
            catch (const std::exception &) {
                //msg = "Could not parse server response";
                res = false;
            }
        })
        .perform_sync();

    return res;
}

bool Repetier::get_printers(wxArrayString& printers) const
{
    const char *name = get_name();

    bool res = true;
    auto url = make_url("printer/list");

    BOOST_LOG_TRIVIAL(info) << boost::format("%1%: List printers at: %2%") % name % url;

    auto http = Http::get(std::move(url));
    set_auth(http);
    
    http.on_error([&](std::string body, std::string error, unsigned status) {
            BOOST_LOG_TRIVIAL(error) << boost::format("%1%: Error listing printers: %2%, HTTP %3%, body: `%4%`") % name % error % status % body;
            res = false;
        })
        .on_complete([&](std::string body, unsigned http_status) {
            BOOST_LOG_TRIVIAL(debug) << boost::format("%1%: Got printers: %2%, HTTP status: %3%") % name % body % http_status;
            
            if (http_status != 200)
                throw HostNetworkError(GUI::format(_L("HTTP status: %1%\nMessage body: \"%2%\""), http_status, body));

            std::stringstream ss(body);
            pt::ptree ptree;
            try {
                pt::read_json(ss, ptree);
            } catch (const pt::ptree_error &err) {
                throw HostNetworkError(GUI::format(_L("Parsing of host response failed.\nMessage body: \"%1%\"\nError: \"%2%\""), body, err.what()));
            }
            
            const auto error = ptree.get_optional<std::string>("error");
            if (error)
                throw HostNetworkError(*error);

            try {
                BOOST_FOREACH(boost::property_tree::ptree::value_type &v, ptree.get_child("data.")) {
                    const auto port = v.second.get<std::string>("slug");
                    printers.push_back(Slic3r::GUI::from_u8(port));
                }
            } catch (const std::exception &err) {
                throw HostNetworkError(GUI::format(_L("Enumeration of host printers failed.\nMessage body: \"%1%\"\nError: \"%2%\""), body, err.what()));
            }
        })
        .perform_sync();

    return res;
}

}
