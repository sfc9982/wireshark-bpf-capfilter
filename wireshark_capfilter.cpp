#include <cstddef>
#include <iostream>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <vector>


auto cmdline_args_to_span(int argc, const char *argv[]) -> std::span<std::string_view> {

    static std::vector<std::string_view> args(argv, argv + argc);

    return args;
}

auto utf8Convert(std::string str) -> std::vector<unsigned char> {

    std::vector<unsigned char> utf8bytes;

    for (size_t n = 0; n < str.length(); n++) {
        unsigned char c = str[n];
        utf8bytes.push_back(c);
        // TODO: CJK std::wstring support
    }

    return utf8bytes;
}

auto stringToBpf(std::string str, int offset) -> std::string {
    //    int                        offset  = std::stoi(/* Get value of #searchOff input */);
    std::vector<unsigned char> bytes   = utf8Convert(str);
    std::string                snippet = "";
    int                        slicelen;
    std::string                hexbytes;
    std::string                conj = "";
    std::string                offstr;


    while (bytes.size() > 0) {
        if (bytes.size() >= 4) {
            slicelen = 4;
        }
        else if (bytes.size() >= 2) {
            slicelen = 2;
        }
        else {
            slicelen = 1;
        }

        auto slicebytes = std::vector<unsigned char>(bytes.begin(), bytes.begin() + slicelen);
        bytes.erase(bytes.begin(), bytes.begin() + slicelen);

        hexbytes = "";
        for (auto i: slicebytes) {
            char buf[3];
            sprintf(buf, "%02x", i);
            hexbytes += buf;
        }

        if (offset) {
            offstr = " + " + std::to_string(offset);
        }
        else {
            offstr = "";
        }

        snippet += conj + "tcp[((tcp[12:1] & 0xf0) >> 2)" + offstr + ":" + std::to_string(slicelen) + "] = 0x" + hexbytes;

        conj = " && ";
        offset += slicelen;
    }

    return snippet;
}

auto main(int argc, const char *argv[]) -> int {

    bool        debug  = false;
    std::string offset = "0";
    std::string str    = "";

    // TODO: Interactive mode
    if (argc == 1) {
        std::cout << "String-Matching Capture Filter Generator\n"
                  << "Usage:\n"
                  << "\t--str    / -s <string you want to match>\n"
                  << "\t--offset / -o <offset from the start of the TCP data>\n"
                  << "\n";
        return 0;
    }

    auto args = cmdline_args_to_span(argc, argv);

    for (auto it = std::next(args.begin()); it != args.end(); ++it) {
        const auto &arg = *it;
        if (arg == "--debug") {
            debug = true;
        }
        else if (arg == "-o" || arg == "--offset") {
            if (it != args.end() - 1) {
                offset = *(++it);
            }
            else {
                std::cerr << "Error: missing value for --offset parameter\n";
                return 1;
            }
        }
        else if (arg == "-s" || arg == "--str") {
            if (it != args.end() - 1) {
                str = *(++it);
            }
            else {
                std::cerr << "Error: missing value for --str parameter\n";
                return 1;
            }
        }
        else {
            std::cerr << "Error: unknown parameter " << arg << "\n";
            return 1;
        }
    }

    if (debug == true) {
        std::cout << "String: " << str << "\n";
        std::cout << "Offset: " << offset << "\n";
        std::cout << "\n";
    }

    std::cout << stringToBpf(str, std::stoi(offset)) << std::endl;
    return 0;
}
