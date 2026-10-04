/* Issue phyz#3322: the trace file and JSON helpers (see iqtrace.h). */
#include "iqtrace.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

bool iqtrace_enabled = (getenv("IQTREE_TRACE") != NULL);
IqTraceState iqtrace_st = {};

std::ostream &iqtrace_out() {
    static std::ofstream f;
    static bool tried = false;
    if (!tried) {
        tried = true;
        const char *path = getenv("IQTREE_TRACE");
        f.open(path, std::ios::out | std::ios::trunc);
        if (!f) {
            std::cerr << "IQTREE_TRACE: cannot open '" << path << "' for writing" << std::endl;
            exit(2);
        }
        f << "{\"e\":\"header\",\"format\":\"iqtrace\",\"version\":1}\n";
    }
    return f;
}

std::string iqtrace_num(double x) {
    if (!std::isfinite(x)) return "null";
    char buf[32];
    snprintf(buf, sizeof(buf), "%.17g", x);
    return buf;
}

std::string iqtrace_str(const std::string &s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') { out += '\\'; out += c; }
        else if ((unsigned char) c < 0x20) {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char) c);
            out += buf;
        } else out += c;
    }
    return out + "\"";
}
