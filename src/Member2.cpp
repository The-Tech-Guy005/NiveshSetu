#include "Member2.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <limits>

namespace niveshsetu {
namespace member2 {

namespace {

// --------------------------------------------------
// String utilities
// --------------------------------------------------

std::string trim(const std::string& value) {
    const std::string whitespace = " \t\r\n";

    std::size_t start = value.find_first_not_of(whitespace);

    if (start == std::string::npos) {
        return "";
    }

    std::size_t end = value.find_last_not_of(whitespace);

    return value.substr(start, end - start + 1);
}

std::string toLower(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

// --------------------------------------------------
// CSV parser supporting quoted fields
// --------------------------------------------------

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
        } else if (c == ',' && !insideQuotes) {
            fields.push_back(trim(field));
            field.clear();
        } else {
            field += c;
        }
    }

    fields.push_back(trim(field));

    return fields;
}

std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) {
        return value;
    }

    std::string escaped = "\"";

    for (char c : value) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }

    escaped += "\"";

    return escaped;
}

// --------------------------------------------------
// Numeric parsing
// --------------------------------------------------

bool parseDouble(const std::string& value, double& result) {
    try {
        std::size_t position = 0;

        result = std::stod(value, &position);

        if (position != value.size() || !std::isfinite(result)) {
            return false;
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool parseInt(const std::string& value, int& result) {
    try {
        std::size_t position = 0;

        result = std::stoi(value, &position);

        return position == value.size();
    } catch (...) {
        return false;
    }
}

std::string formatDouble(double value) {
    std::ostringstream output;

    output << std::setprecision(
        std::numeric_limits<double>::max_digits10
    ) << value;

    return output.str();
}

// --------------------------------------------------
// FNV-1a hash
// --------------------------------------------------

std::string generateHash(const std::string& data) {
    std::uint64_t hash = 14695981039346656037ULL;

    const std::uint64_t prime = 1099511628211ULL;

    for (unsigned char c : data) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= prime;
    }

    std::ostringstream output;

    output << std::hex
           << std::setfill('0')
           << std::setw(16)
           << hash;

    return output.str();
}

// --------------------------------------------------
// Find a column in a CSV header
// --------------------------------------------------

int findColumn(
    const std::vector<std::string>& headers,
    const std::string& columnName
) {
    for (std::size_t i = 0; i < headers.size(); ++i) {
        if (toLower(trim(headers[i])) == toLower(columnName)) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

bool getField(
    const std::vector<std::string>& fields,
    int index,
    std::string& value
) {
    if (index < 0 || index >= static_cast<int>(fields.size())) {
        return false;
    }

    value = trim(fields[index]);

    return true;
}

// --------------------------------------------------
// Build the exact decision-record data before hashing
// --------------------------------------------------

std::string createDecisionRecord(
    const TradeInput& trade,
    const RiskResult& result
) {
    std::ostringstream record;

    record << csvEscape(trade.symbol) << ","
           << csvEscape(trade.sector) << ","
           << formatDouble(trade.entryPrice) << ","
           << formatDouble(trade.stopLoss) << ","
           << trade.quantity << ","
           << (
                result.decision == RiskDecision::PASS
                ? "PASS"
                : "BLOCK"
              ) << ","
           << formatDouble(result.plannedLoss) << ","
           << formatDouble(result.requiredCash) << ","
           << formatDouble(result.stressLossFraction);

    return record.str();
}

} // anonymous namespace

// ==================================================
// LOAD PORTFOLIO CSV
//
// Supported row format:
// accountValue,100000
// availableCash,50000
// sectorExposure,IT,30000
//
// Header-based format is also supported:
// accountValue,availableCash,sector,exposure
// 100000,50000,IT,30000
// ==================================================

bool loadPortfolioCSV(
    const std::string& filename,
    PortfolioInput& portfolio
) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open portfolio CSV: "
                  << filename << "\n";
        return false;
    }

    PortfolioInput loadedPortfolio;

    std::string line;
    std::vector<std::string> headers;

    bool accountValueFound = false;
    bool availableCashFound = false;
    bool firstNonEmptyLine = true;

    while (std::getline(file, line)) {
        if (trim(line).empty()) {
            continue;
        }

        std::vector<std::string> fields = parseCSVLine(line);

        if (fields.empty()) {
            continue;
        }

        if (firstNonEmptyLine) {
            firstNonEmptyLine = false;

            headers = fields;

            bool isHeader =
                findColumn(headers, "accountValue") >= 0 ||
                findColumn(headers, "availableCash") >= 0 ||
                findColumn(headers, "sector") >= 0;

            if (isHeader) {
                continue;
            }
        }

        std::string key = toLower(trim(fields[0]));

        // Key-value format
        if (key == "accountvalue" && fields.size() >= 2) {
            double value;

            if (!parseDouble(fields[1], value) || value <= 0.0) {
                std::cerr << "Invalid account value in portfolio CSV.\n";
                return false;
            }

            loadedPortfolio.accountValue = value;
            accountValueFound = true;
            continue;
        }

        if (key == "availablecash" && fields.size() >= 2) {
            double value;

            if (!parseDouble(fields[1], value) || value < 0.0) {
                std::cerr << "Invalid available cash in portfolio CSV.\n";
                return false;
            }

            loadedPortfolio.availableCash = value;
            availableCashFound = true;
            continue;
        }

        if ((key == "sectorexposure" || key == "sector")
            && fields.size() >= 3) {
            double exposure;

            if (trim(fields[1]).empty() ||
                !parseDouble(fields[2], exposure) ||
                exposure < 0.0) {
                std::cerr << "Invalid sector exposure in portfolio CSV.\n";
                return false;
            }

            loadedPortfolio.sectorExposure[fields[1]] = exposure;
            continue;
        }

        // Header-based format
        int accountIndex = findColumn(headers, "accountValue");
        int cashIndex = findColumn(headers, "availableCash");
        int sectorIndex = findColumn(headers, "sector");
        int exposureIndex = findColumn(headers, "exposure");

        std::string value;

        if (accountIndex >= 0 &&
            getField(fields, accountIndex, value) &&
            !value.empty()) {
            double number;

            if (!parseDouble(value, number) || number <= 0.0) {
                return false;
            }

            loadedPortfolio.accountValue = number;
            accountValueFound = true;
        }

        if (cashIndex >= 0 &&
            getField(fields, cashIndex, value) &&
            !value.empty()) {
            double number;

            if (!parseDouble(value, number) || number < 0.0) {
                return false;
            }

            loadedPortfolio.availableCash = number;
            availableCashFound = true;
        }

        std::string sector;
        std::string exposureText;

        if (getField(fields, sectorIndex, sector) &&
            getField(fields, exposureIndex, exposureText) &&
            !sector.empty() && !exposureText.empty()) {
            double exposure;

            if (!parseDouble(exposureText, exposure) || exposure < 0.0) {
                return false;
            }

            loadedPortfolio.sectorExposure[sector] = exposure;
        }
    }

    if (!file.eof() && file.fail()) {
        std::cerr << "Error reading portfolio CSV.\n";
        return false;
    }

    if (!accountValueFound || !availableCashFound ||
        loadedPortfolio.accountValue <= 0.0) {
        std::cerr << "Portfolio CSV is missing account value or available cash.\n";
        return false;
    }

    portfolio = loadedPortfolio;

    return true;
}

// ==================================================
// LOAD RISK LIMITS CSV
//
// Supported row format:
// maxPlannedLoss,10000
// maxSectorExposureFraction,0.30
// maxStressLossFraction,0.05
// sectorStressShock,IT,0.10
//
// Header-based format is also supported:
// maxPlannedLoss,maxSectorExposureFraction,
// maxStressLossFraction,sector,shock
// 10000,0.30,0.05,IT,0.10
// ==================================================

bool loadRiskLimitsCSV(
    const std::string& filename,
    RiskLimits& limits
) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open risk limits CSV: "
                  << filename << "\n";
        return false;
    }

    RiskLimits loadedLimits;

    std::string line;
    std::vector<std::string> headers;

    bool plannedLossFound = false;
    bool sectorLimitFound = false;
    bool stressLimitFound = false;
    bool firstNonEmptyLine = true;

    while (std::getline(file, line)) {
        if (trim(line).empty()) {
            continue;
        }

        std::vector<std::string> fields = parseCSVLine(line);

        if (fields.empty()) {
            continue;
        }

        if (firstNonEmptyLine) {
            firstNonEmptyLine = false;

            headers = fields;

            bool isHeader =
                findColumn(headers, "maxPlannedLoss") >= 0 ||
                findColumn(headers, "maxSectorExposureFraction") >= 0 ||
                findColumn(headers, "sectorStressShock") >= 0;

            if (isHeader) {
                continue;
            }
        }

        std::string key = toLower(trim(fields[0]));

        // Key-value format
        if ((key == "maxplannedloss" ||
             key == "maxsectorexposurefraction" ||
             key == "maxstresslossfraction") &&
            fields.size() >= 2) {
            double value;

            if (!parseDouble(fields[1], value)) {
                std::cerr << "Invalid risk limit value.\n";
                return false;
            }

            if (key == "maxplannedloss") {
                if (value < 0.0) return false;
                loadedLimits.maxPlannedLoss = value;
                plannedLossFound = true;
            } else if (key == "maxsectorexposurefraction") {
                if (value < 0.0 || value > 1.0) return false;
                loadedLimits.maxSectorExposureFraction = value;
                sectorLimitFound = true;
            } else {
                if (value < 0.0 || value > 1.0) return false;
                loadedLimits.maxStressLossFraction = value;
                stressLimitFound = true;
            }

            continue;
        }

        if ((key == "sectorstressshock" || key == "shock") &&
            fields.size() >= 3) {
            double shock;

            if (trim(fields[1]).empty() ||
                !parseDouble(fields[2], shock) ||
                shock < 0.0 || shock > 1.0) {
                std::cerr << "Invalid sector stress shock.\n";
                return false;
            }

            loadedLimits.sectorStressShock[fields[1]] = shock;
            continue;
        }

        // Header-based format
        std::string value;

        int plannedIndex = findColumn(headers, "maxPlannedLoss");
        int sectorLimitIndex =
            findColumn(headers, "maxSectorExposureFraction");
        int stressLimitIndex =
            findColumn(headers, "maxStressLossFraction");
        int sectorIndex = findColumn(headers, "sector");
        int shockIndex = findColumn(headers, "shock");

        if (getField(fields, plannedIndex, value) && !value.empty()) {
            double number;

            if (!parseDouble(value, number) || number < 0.0) return false;

            loadedLimits.maxPlannedLoss = number;
            plannedLossFound = true;
        }

        if (getField(fields, sectorLimitIndex, value) && !value.empty()) {
            double number;

            if (!parseDouble(value, number) ||
                number < 0.0 || number > 1.0) return false;

            loadedLimits.maxSectorExposureFraction = number;
            sectorLimitFound = true;
        }

        if (getField(fields, stressLimitIndex, value) && !value.empty()) {
            double number;

            if (!parseDouble(value, number) ||
                number < 0.0 || number > 1.0) return false;

            loadedLimits.maxStressLossFraction = number;
            stressLimitFound = true;
        }

        std::string sector;
        std::string shockText;

        if (getField(fields, sectorIndex, sector) &&
            getField(fields, shockIndex, shockText) &&
            !sector.empty() && !shockText.empty()) {
            double shock;

            if (!parseDouble(shockText, shock) ||
                shock < 0.0 || shock > 1.0) return false;

            loadedLimits.sectorStressShock[sector] = shock;
        }
    }

    if (!file.eof() && file.fail()) {
        std::cerr << "Error reading risk limits CSV.\n";
        return false;
    }

    if (!plannedLossFound || !sectorLimitFound || !stressLimitFound) {
        std::cerr << "Risk limits CSV is missing one or more required limits.\n";
        return false;
    }

    limits = loadedLimits;

    return true;
}

// ==================================================
// SAVE DECISION RECORD
//
// CSV columns:
// symbol,sector,entryPrice,stopLoss,quantity,decision,
// plannedLoss,requiredCash,stressLossFraction,hash
// ==================================================

bool saveDecisionRecord(
    const std::string& filename,
    const TradeInput& trade,
    const RiskResult& result
) {
    bool writeHeader = false;

    {
        std::ifstream checkFile(filename, std::ios::binary);

        if (!checkFile.is_open()) {
            writeHeader = true;
        } else {
            checkFile.seekg(0, std::ios::end);
            writeHeader = (checkFile.tellg() == 0);
        }
    }

    std::ofstream file(filename, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open decision log: "
                  << filename << "\n";
        return false;
    }

    if (writeHeader) {
        file << "symbol,sector,entryPrice,stopLoss,quantity,"
             << "decision,plannedLoss,requiredCash,"
             << "stressLossFraction,hash\n";
    }

    // Hash exactly the data portion written to the CSV.
    const std::string record = createDecisionRecord(trade, result);
    const std::string hash = generateHash(record);

    file << record << "," << hash << "\n";

    file.flush();

    if (!file.good()) {
        std::cerr << "Error writing decision record.\n";
        return false;
    }

    std::cout << "Decision saved successfully.\n";
    std::cout << "Record Hash: " << hash << "\n";

    return true;
}

// ==================================================
// REPLAY DECISION RECORDS
// ==================================================

void replayDecisionRecords(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open decision log: "
                  << filename << "\n";
        return;
    }

    std::cout << "\n========== DECISION REPLAY ==========\n\n";

    std::string line;
    bool headerSkipped = false;
    bool foundRecord = false;

    while (std::getline(file, line)) {
        if (trim(line).empty()) {
            continue;
        }

        if (!headerSkipped) {
            headerSkipped = true;

            if (toLower(trim(line)).find("symbol,") == 0) {
                continue;
            }
        }

        // The final comma separates the stored hash from the data.
        const std::size_t hashSeparator = line.rfind(',');

        if (hashSeparator == std::string::npos) {
            std::cout << "Invalid decision record format.\n\n";
            continue;
        }

        const std::string recordData = line.substr(0, hashSeparator);
        const std::string storedHash = trim(
            line.substr(hashSeparator + 1)
        );

        const std::string calculatedHash = generateHash(recordData);

        const bool integrityVerified =
            (storedHash == calculatedHash);

        std::vector<std::string> fields = parseCSVLine(recordData);

        if (fields.size() != 9) {
            std::cout << "Invalid decision record fields.\n";
            std::cout << "Integrity: FAILED\n\n";
            continue;
        }

        foundRecord = true;

        std::cout << "Symbol: " << fields[0] << "\n";
        std::cout << "Sector: " << fields[1] << "\n";
        std::cout << "Entry Price: " << fields[2] << "\n";
        std::cout << "Stop Loss: " << fields[3] << "\n";
        std::cout << "Quantity: " << fields[4] << "\n";
        std::cout << "Decision: " << fields[5] << "\n";
        std::cout << "Planned Loss: " << fields[6] << "\n";
        std::cout << "Required Cash: " << fields[7] << "\n";
        std::cout << "Stress Loss Fraction: " << fields[8] << "\n";

        if (integrityVerified) {
            std::cout << "Integrity: VERIFIED\n";
        } else {
            std::cout << "Integrity: FAILED\n";
        }

        std::cout << "\n";
    }

    if (!foundRecord) {
        std::cout << "No valid decision records found.\n";
    }

    std::cout << "=====================================\n";
}

} // namespace member2
} // namespace niveshsetu