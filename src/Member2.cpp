
#include "Member2.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace niveshsetu {
namespace member2 {

namespace {

// ==================================================
// HELPER FUNCTIONS
// ==================================================

std::string trim(const std::string& str) {
    const std::string whitespace = " \t\r\n";
    const std::size_t first = str.find_first_not_of(whitespace);

    if (first == std::string::npos)
        return "";

    const std::size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, last - first + 1);
}

std::string toLower(std::string str) {
    std::transform(
        str.begin(), str.end(), str.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return str;
}

std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool insideQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];

        if (c == '"') {
            if (insideQuotes && i + 1 < line.size()
                && line[i + 1] == '"') {
                field += '"';
                ++i;
            } else {
                insideQuotes = !insideQuotes;
            }
        }
        else if (c == ',' && !insideQuotes) {
            fields.push_back(trim(field));
            field.clear();
        }
        else {
            field += c;
        }
    }

    fields.push_back(trim(field));
    return fields;
}

std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"\n\r") == std::string::npos)
        return value;

    std::string result = "\"";

    for (char c : value) {
        if (c == '"')
            result += "\"\"";
        else
            result += c;
    }

    result += '"';
    return result;
}

bool parseDouble(const std::string& value, double& result) {
    try {
        std::string cleaned = trim(value);
        std::size_t position = 0;

        result = std::stod(cleaned, &position);

        return position == cleaned.size()
            && std::isfinite(result);
    }
    catch (...) {
        return false;
    }
}

std::string formatDouble(double value) {
    std::ostringstream out;
    out << std::setprecision(17) << value;
    return out.str();
}

// FNV-1a checksum for accidental data-change detection.
// This is not a cryptographic authentication mechanism.
std::string generateHash(const std::string& data) {
    std::uint64_t hash = 14695981039346656037ULL;

    for (unsigned char c : data) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }

    std::ostringstream out;
    out << std::hex << std::setfill('0')
        << std::setw(16) << hash;

    return out.str();
}

// Create exactly the same nine-field record for saving
// and hash verification.
std::string makeRecordData(
    const TradeInput& trade,
    const RiskResult& result
) {
    std::ostringstream out;

    out << csvEscape(trade.symbol) << ","
        << csvEscape(trade.sector) << ","
        << formatDouble(trade.entryPrice) << ","
        << formatDouble(trade.stopLoss) << ","
        << trade.quantity << ","
        << static_cast<int>(result.decision) << ","
        << formatDouble(result.plannedLoss) << ","
        << formatDouble(result.requiredCash) << ","
        << formatDouble(result.stressLossFraction);

    return out.str();
}

} // anonymous namespace


// ==================================================
// LOAD PORTFOLIO CSV
//
// Expected format:
//
// type,key,value
// account_value,,100000
// available_cash,,5000
// sector,IT,20000
// sector,Banking,20000
// ==================================================

bool loadPortfolioCSV(
    const std::string& filename,
    PortfolioInput& portfolio
) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Could not open portfolio CSV: "
                  << filename << "\n";
        return false;
    }

    bool accountFound = false;
    bool cashFound = false;

    std::string line;

    while (std::getline(file, line)) {
        if (trim(line).empty())
            continue;

        std::vector<std::string> fields = parseCSVLine(line);

        if (fields.size() < 3) {
            std::cerr << "Invalid portfolio CSV row: "
                      << line << "\n";
            return false;
        }

        std::string type = toLower(trim(fields[0]));
        std::string key = trim(fields[1]);
        std::string valueText = trim(fields[2]);

        if (type == "type")
            continue;

        double value = 0.0;

        if (!parseDouble(valueText, value)) {
            std::cerr << "Invalid portfolio value: "
                      << line << "\n";
            return false;
        }

        if (type == "account_value") {
            if (accountFound || value <= 0.0) {
                std::cerr << "Invalid or duplicate account value.\n";
                return false;
            }

            portfolio.accountValue = value;
            accountFound = true;
        }
        else if (type == "available_cash") {
            if (cashFound || value < 0.0) {
                std::cerr << "Invalid or duplicate available cash.\n";
                return false;
            }

            portfolio.availableCash = value;
            cashFound = true;
        }
        else if (type == "sector") {
            if (key.empty() || value < 0.0) {
                std::cerr << "Invalid sector exposure.\n";
                return false;
            }

            portfolio.sectorExposure[key] = value;
        }
        else {
            std::cerr << "Unknown portfolio CSV type: "
                      << type << "\n";
            return false;
        }
    }

    if (!accountFound || !cashFound) {
        std::cerr << "Portfolio CSV is missing account value "
                  << "or available cash.\n";
        return false;
    }

    return true;
}


// ==================================================
// LOAD RISK LIMITS CSV
//
// Expected format:
//
// type,key,value
// max_planned_loss,,500
// max_sector_exposure,,0.30
// max_stress_loss,,0.05
// sector_shock,IT,0.10
// sector_shock,Banking,0.05
// ==================================================

bool loadRiskLimitsCSV(
    const std::string& filename,
    RiskLimits& limits
) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Could not open risk limits CSV: "
                  << filename << "\n";
        return false;
    }

    bool plannedLossFound = false;
    bool sectorLimitFound = false;
    bool stressLimitFound = false;

    std::string line;

    while (std::getline(file, line)) {
        if (trim(line).empty())
            continue;

        std::vector<std::string> fields = parseCSVLine(line);

        if (fields.size() < 3) {
            std::cerr << "Invalid risk limits CSV row: "
                      << line << "\n";
            return false;
        }

        std::string type = toLower(trim(fields[0]));
        std::string key = trim(fields[1]);
        std::string valueText = trim(fields[2]);

        if (type == "type")
            continue;

        double value = 0.0;

        if (!parseDouble(valueText, value)) {
            std::cerr << "Invalid risk limit value: "
                      << line << "\n";
            return false;
        }

        if (type == "max_planned_loss") {
            if (plannedLossFound || value < 0.0)
                return false;

            limits.maxPlannedLoss = value;
            plannedLossFound = true;
        }
        else if (type == "max_sector_exposure") {
            if (sectorLimitFound || value < 0.0 || value > 1.0)
                return false;

            limits.maxSectorExposureFraction = value;
            sectorLimitFound = true;
        }
        else if (type == "max_stress_loss") {
            if (stressLimitFound || value < 0.0 || value > 1.0)
                return false;

            limits.maxStressLossFraction = value;
            stressLimitFound = true;
        }
        else if (type == "sector_shock") {
            if (key.empty() || value < 0.0 || value > 1.0)
                return false;

            limits.sectorStressShock[key] = value;
        }
        else {
            std::cerr << "Unknown risk limit type: "
                      << type << "\n";
            return false;
        }
    }

    if (!plannedLossFound ||
        !sectorLimitFound ||
        !stressLimitFound) {
        std::cerr << "Risk limits CSV is missing a required limit.\n";
        return false;
    }

    return true;
}


// ==================================================
// SAVE DECISION RECORD
// ==================================================

bool saveDecisionRecord(
    const std::string& filename,
    const TradeInput& trade,
    const RiskResult& result
) {
    bool needsHeader = false;

    {
        std::ifstream check(filename, std::ios::binary | std::ios::ate);

        needsHeader =
            !check.is_open() || check.tellg() == 0;
    }

    std::ofstream file(filename, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Could not open decision log: "
                  << filename << "\n";
        return false;
    }

    const std::string record = makeRecordData(trade, result);
    const std::string hash = generateHash(record);

    if (needsHeader) {
        file << "symbol,sector,entry_price,stop_loss,quantity,"
             << "decision,planned_loss,required_cash,"
             << "stress_loss_fraction,hash\n";
    }

    file << record << "," << hash << "\n";

    if (!file.good()) {
        std::cerr << "Failed to write decision record.\n";
        return false;
    }

    std::cout << "Decision record saved. Hash: "
              << hash << "\n";

    return true;
}


// ==================================================
// REPLAY DECISION RECORDS
// Skips repeated headers and verifies each hash.
// ==================================================

void replayDecisionRecords(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Could not open decision log for replay: "
                  << filename << "\n";
        return;
    }

    std::cout << "\n========== DECISION REPLAY ==========\n";

    std::string line;
    int recordNumber = 0;
    int verifiedCount = 0;
    int failedCount = 0;

    while (std::getline(file, line)) {
        if (trim(line).empty())
            continue;

        std::vector<std::string> fields = parseCSVLine(line);

        if (fields.empty())
            continue;

        // Skip every repeated header.
        if (toLower(trim(fields[0])) == "symbol")
            continue;

        ++recordNumber;

        if (fields.size() != 10) {
            ++failedCount;

            std::cout << "Record " << recordNumber
                      << ": Integrity: FAILED "
                      << "(invalid field count)\n";
            continue;
        }

        std::ostringstream data;

        for (int i = 0; i < 9; ++i) {
            if (i > 0)
                data << ",";

            data << fields[i];
        }

        const std::string calculatedHash =
            generateHash(data.str());

        const std::string storedHash = fields[9];

        const bool verified =
            (calculatedHash == storedHash);

        if (verified)
            ++verifiedCount;
        else
            ++failedCount;

        std::cout << "Record " << recordNumber
                  << ": Integrity: "
                  << (verified ? "VERIFIED" : "FAILED")
                  << "\n";

        std::cout << "  Symbol: " << fields[0] << "\n"
                  << "  Sector: " << fields[1] << "\n"
                  << "  Decision: "
                  << (fields[5] == "0" ? "PASS" : "BLOCK")
                  << "\n";
    }

    std::cout << "\nReplay Summary:\n"
              << "Total records: " << recordNumber << "\n"
              << "Verified: " << verifiedCount << "\n"
              << "Failed: " << failedCount << "\n";

    std::cout << "=====================================\n";
}

} // namespace member2
} // namespace niveshsetu
