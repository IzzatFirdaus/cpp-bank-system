// Lightweight HTTP/REST API server for the cpp-bank-system engine.
// Uses cpp-httplib (single-header) to expose account operations as JSON endpoints.
// Serves the static web dashboard from public/.
#include <httplib.h>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "bank/SavingsAccount.h"

// ---------------------------------------------------------------------------
// Minimal JSON helpers (no external dependency).
// These build small, well-formed JSON strings for our known payloads.
// ---------------------------------------------------------------------------

namespace json {

inline std::string escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

// Returns a fixed-precision string for a double (avoids trailing zeros noise).
inline std::string fmt(double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.2f", v);
    return buf;
}

// Extract a numeric value for "key": <number> from a flat JSON object string.
// Returns true on success. Handles integers and decimals (negative optional).
inline bool get_number(const std::string& body, const std::string& key,
                       double& out) {
    const std::string needle = "\"" + key + "\"";
    auto pos = body.find(needle);
    if (pos == std::string::npos) return false;

    // Move past the key, skip whitespace, colon, whitespace.
    pos = body.find(':', pos + needle.size());
    if (pos == std::string::npos) return false;
    ++pos;

    // Skip whitespace.
    while (pos < body.size() && (body[pos] == ' ' || body[pos] == '\t'))
        ++pos;
    if (pos >= body.size()) return false;

    // Parse the number.
    char* end = nullptr;
    double val = std::strtod(body.c_str() + pos, &end);
    if (end == body.c_str() + pos) return false;  // no digits consumed
    if (!std::isfinite(val)) return false;

    out = val;
    return true;
}

}  // namespace json

// ---------------------------------------------------------------------------
// CORS helper — sets permissive headers for local development.
// ---------------------------------------------------------------------------

static void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

// ---------------------------------------------------------------------------
// Read the last N lines of log.txt (most recent first).
// Falls back to an empty vector when the file does not exist.
// ---------------------------------------------------------------------------

static std::vector<std::string> tail_log(std::size_t n) {
    std::vector<std::string> lines;
    const std::filesystem::path logPath("log.txt");
    if (!std::filesystem::exists(logPath)) return lines;

    std::ifstream ifs(logPath);
    if (!ifs.is_open()) return lines;

    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty()) lines.push_back(line);
    }

    // Return the last n entries, most recent first.
    if (lines.size() > n) {
        lines.erase(lines.begin(), lines.end() - static_cast<std::ptrdiff_t>(n));
    }
    std::reverse(lines.begin(), lines.end());
    return lines;
}

// ---------------------------------------------------------------------------
// main — configure routes and start listening on port 8080.
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    // Allow overriding the port via CLI argument.
    int port = 8080;
    if (argc > 1) {
        char* end = nullptr;
        long p = std::strtol(argv[1], &end, 10);
        if (end != argv[1] && p > 0 && p <= 65535) port = static_cast<int>(p);
    }

    // Create the domain engine — a single SavingsAccount for demo purposes.
    auto account =
        std::make_unique<bank::SavingsAccount>("ACC-1001", 500.00, 2.5);

    httplib::Server svr;

    // --- Pre-flight CORS for all routes ---------------------------------
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 204;
    });

    // --- GET /api/account ------------------------------------------------
    svr.Get("/api/account",
            [&account](const httplib::Request&, httplib::Response& res) {
                set_cors(res);
                res.set_content(
                    "{"
                    "\"accountNumber\":\"" +
                    json::escape(account->getAccountNumber()) +
                    "\","
                    "\"balance\":" +
                    json::fmt(account->getBalance()) +
                    ","
                    "\"interestRate\":" +
                    json::fmt(static_cast<const bank::SavingsAccount&>(*account)
                                  .getInterestRate()) +
                    "}",
                    "application/json");
            });

    // --- POST /api/deposit ----------------------------------------------
    svr.Post("/api/deposit",
             [&account](const httplib::Request& req, httplib::Response& res) {
                 set_cors(res);
                 double amount = 0.0;
                 if (!json::get_number(req.body, "amount", amount) ||
                     amount <= 0.0) {
                     res.status = 400;
                     res.set_content(
                         "{\"success\":false,"
                         "\"error\":\"Invalid amount. Must be a positive "
                         "number.\"}",
                         "application/json");
                     return;
                 }
                 account->deposit(amount);
                 res.set_content(
                     "{\"success\":true,"
                     "\"balance\":" +
                     json::fmt(account->getBalance()) + "}",
                     "application/json");
             });

    // --- POST /api/withdraw ---------------------------------------------
    svr.Post("/api/withdraw",
             [&account](const httplib::Request& req, httplib::Response& res) {
                 set_cors(res);
                 double amount = 0.0;
                 if (!json::get_number(req.body, "amount", amount) ||
                     amount <= 0.0) {
                     res.status = 400;
                     res.set_content(
                         "{\"success\":false,"
                         "\"error\":\"Invalid amount. Must be a positive "
                         "number.\"}",
                         "application/json");
                     return;
                 }
                 bool ok = account->withdraw(amount);
                 if (!ok) {
                     res.status = 422;
                     res.set_content(
                         "{\"success\":false,"
                         "\"error\":\"Insufficient funds.\","
                         "\"balance\":" +
                         json::fmt(account->getBalance()) + "}",
                         "application/json");
                     return;
                 }
                 res.set_content(
                     "{\"success\":true,"
                     "\"balance\":" +
                     json::fmt(account->getBalance()) + "}",
                     "application/json");
             });

    // --- POST /api/apply-interest ---------------------------------------
    svr.Post("/api/apply-interest",
             [&account](const httplib::Request&, httplib::Response& res) {
                 set_cors(res);
                 double before = account->getBalance();
                 account->applyInterest();
                 double after = account->getBalance();
                 double credited = after - before;
                 res.set_content(
                     "{\"success\":true,"
                     "\"balance\":" +
                     json::fmt(after) +
                     ","
                     "\"credited\":" +
                     json::fmt(credited) + "}",
                     "application/json");
             });

    // --- GET /api/logs ---------------------------------------------------
    svr.Get("/api/logs",
            [](const httplib::Request&, httplib::Response& res) {
                set_cors(res);
                auto lines = tail_log(50);
                std::ostringstream oss;
                oss << "[";
                for (std::size_t i = 0; i < lines.size(); ++i) {
                    if (i > 0) oss << ",";
                    oss << "\"" << json::escape(lines[i]) << "\"";
                }
                oss << "]";
                res.set_content(oss.str(), "application/json");
            });

    // --- Static file serving (public/) -----------------------------------
    if (std::filesystem::is_directory("public")) {
        svr.set_mount_point("/", "public");
    }

    std::cout << "Bank API server listening on http://localhost:" << port
              << "\n";
    std::cout << "Dashboard: http://localhost:" << port << "/index.html\n";
    std::cout << "Press Ctrl+C to stop.\n";

    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server on port " << port << "\n";
        return 1;
    }
    return 0;
}
