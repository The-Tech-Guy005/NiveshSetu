#include "Member2.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdint>

using namespace std;

namespace niveshsetu {
namespace member2 {

// Simple deterministic FNV-1a hash
string generateHash(const string& data) {
    uint64_t hash = 14695981039346656037ULL;

    for (unsigned char c : data) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }

    stringstream ss;
    ss << hex << hash;

    return ss.str();
}


// Load portfolio CSV
bool loadPortfolioCSV(
    const string& filename,
    PortfolioInput& portfolio
) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Cannot open portfolio file.\n";
        return false;
    }

    string line;

    while (getline(file, line)) {
        if (line.empty() || line == "type,key,value")
            continue;

        string type, key, value;
        stringstream ss(line);

        getline(ss, type, ',');
        getline(ss, key, ',');
        getline(ss, value, ',');

        try {
            double amount = stod(value);

            if (type == "account_value") {
                portfolio.accountValue = amount;
            }
            else if (type == "available_cash") {
                portfolio.availableCash = amount;
            }
            else if (type == "sector") {
                portfolio.sectorExposure[key] = amount;
            }
        }
        catch (...) {
            cerr << "Invalid portfolio CSV row: " << line << "\n";
            return false;
        }
    }

    return true;
}


// Load risk limits CSV
bool loadRiskLimitsCSV(
    const string& filename,
    RiskLimits& limits
) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Cannot open risk limits file.\n";
        return false;
    }

    string line;

    while (getline(file, line)) {
        if (line.empty() || line == "type,key,value")
            continue;

        string type, key, value;
        stringstream ss(line);

        getline(ss, type, ',');
        getline(ss, key, ',');
        getline(ss, value, ',');

        try {
            double amount = stod(value);

            if (type == "max_planned_loss") {
                limits.maxPlannedLoss = amount;
            }
            else if (type == "max_sector_exposure") {
                limits.maxSectorExposureFraction = amount;
            }
            else if (type == "max_stress_loss") {
                limits.maxStressLossFraction = amount;
            }
            else if (type == "sector_shock") {
                limits.sectorStressShock[key] = amount;
            }
        }
        catch (...) {
            cerr << "Invalid risk limits CSV row: " << line << "\n";
            return false;
        }
    }

    return true;
}


// Save decision record
bool saveDecisionRecord(
    const string& filename,
    const TradeInput& trade,
    const RiskResult& result
) {
    ofstream file(filename, ios::app);

    if (!file.is_open()) {
        cerr << "Error: Cannot open decision log.\n";
        return false;
    }

    if (file.tellp() == 0) {
        file << "symbol,sector,entry,stop,quantity,decision,"
             << "planned_loss,required_cash,stress_fraction,hash\n";
    }

    stringstream record;

    record << setprecision(17)
           << trade.symbol << ","
           << trade.sector << ","
           << trade.entryPrice << ","
           << trade.stopLoss << ","
           << trade.quantity << ","
           << (result.decision == RiskDecision::PASS ? "PASS" : "BLOCK") << ","
           << result.plannedLoss << ","
           << result.requiredCash << ","
           << result.stressLossFraction;

    string data = record.str();
    string hash = generateHash(data);

    file << data << "," << hash << "\n";

    cout << "Decision saved successfully.\n";
    cout << "Record Hash: " << hash << "\n";

    return true;
}


// Replay saved decision records
void replayDecisionRecords(const string& filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Cannot open decision log.\n";
        return;
    }

    string line;

    cout << "\n========== DECISION REPLAY ==========\n";

    getline(file, line); // Skip CSV header

    while (getline(file, line)) {
        if (line.empty())
            continue;

        size_t pos = line.rfind(',');

        if (pos == string::npos) {
            cout << "Invalid record found.\n";
            continue;
        }

        string data = line.substr(0, pos);
        string savedHash = line.substr(pos + 1);

        string calculatedHash = generateHash(data);

        if (savedHash != calculatedHash) {
            cout << "\nRecord integrity check FAILED!\n";
            continue;
        }

        stringstream ss(data);
        string fields[9];

        for (int i = 0; i < 9; i++) {
            getline(ss, fields[i], ',');
        }

        cout << "\nSymbol: " << fields[0];
        cout << "\nSector: " << fields[1];
        cout << "\nEntry Price: " << fields[2];
        cout << "\nStop Loss: " << fields[3];
        cout << "\nQuantity: " << fields[4];
        cout << "\nDecision: " << fields[5];
        cout << "\nPlanned Loss: " << fields[6];
        cout << "\nRequired Cash: " << fields[7];
        cout << "\nStress Loss Fraction: " << fields[8];
        cout << "\nIntegrity: VERIFIED\n";
    }

    cout << "\n=====================================\n";
}

} // namespace member2
} // namespace niveshsetu