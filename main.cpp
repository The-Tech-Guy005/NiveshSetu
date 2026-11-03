#include "RiskEngine.h"
#include <iostream>

using namespace std;
using namespace niveshsetu;

void printResult(const RiskResult& result) {

    cout << "\n========== RISK REPORT ==========\n";

    cout << "Decision: ";

    if (result.decision == RiskDecision::PASS) {
        cout << "PASS\n";
    } else {
        cout << "BLOCK\n";
    }

    cout << "Planned Loss: Rs. "
         << result.plannedLoss << "\n";

    cout << "Required Cash: Rs. "
         << result.requiredCash << "\n";

    cout << "Stress Loss: "
         << result.stressLossFraction * 100
         << "%\n";

    cout << "\nIndividual Rules:\n";

    for (const auto& rule : result.ruleResults) {

        cout << rule.ruleName << " : "
             << (rule.passed ? "PASS" : "FAIL")
             << "\n";

        cout << "Reason: "
             << rule.explanation << "\n";
    }

    cout << "\nFailure Reasons:\n";

    if (result.failureReasons.empty()) {
        cout << "No failures found.\n";
    } else {
        for (const auto& reason : result.failureReasons) {
            cout << "- " << reason << "\n";
        }
    }

    cout << "=================================\n";
}

int main() {

    RiskEngine engine;

    // Portfolio details
    PortfolioInput portfolio;

    portfolio.accountValue = 100000;
    portfolio.availableCash = 5000;

    portfolio.sectorExposure["IT"] = 20000;
    portfolio.sectorExposure["Banking"] = 20000;

    // Risk limits
    RiskLimits limits;

    limits.maxPlannedLoss = 500;
    limits.maxSectorExposureFraction = 0.30;
    limits.maxStressLossFraction = 0.05;

    limits.sectorStressShock["IT"] = 0.10;
    limits.sectorStressShock["Banking"] = 0.05;

    // TEST 1: Valid trade
    cout << "\nTEST 1: VALID TRADE\n";

    TradeInput trade1;

    trade1.symbol = "TCS";
    trade1.sector = "IT";
    trade1.entryPrice = 100;
    trade1.stopLoss = 90;
    trade1.quantity = 10;

    RiskResult result1 =
        engine.evaluate(trade1, portfolio, limits);

    printResult(result1);

    // TEST 2: Trade exceeding risk and cash limits
    cout << "\nTEST 2: RISKY TRADE\n";

    TradeInput trade2;

    trade2.symbol = "INFY";
    trade2.sector = "IT";
    trade2.entryPrice = 100;
    trade2.stopLoss = 90;
    trade2.quantity = 60;

    RiskResult result2 =
        engine.evaluate(trade2, portfolio, limits);

    printResult(result2);

    // TEST 3: Invalid trade

    cout << "\nTEST 3: INVALID TRADE\n";

    TradeInput trade3;

    trade3.symbol = "TCS";
    trade3.sector = "IT";
    trade3.entryPrice = 100;
    trade3.stopLoss = 110;  // Invalid: stop loss above entry
    trade3.quantity = 10;

    RiskResult result3 =
        engine.evaluate(trade3, portfolio, limits);

    printResult(result3);

    // TEST 4: Excessive sector concentration

    cout << "\nTEST 4: SECTOR LIMIT TEST\n";

    PortfolioInput portfolio4 = portfolio;
    portfolio4.availableCash = 20000;

    RiskLimits limits4 = limits;
    limits4.maxPlannedLoss = 2000;

    TradeInput trade4;

    trade4.symbol = "INFY";
    trade4.sector = "IT";
    trade4.entryPrice = 100;
    trade4.stopLoss = 90;
    trade4.quantity = 120;

    RiskResult result4 =
        engine.evaluate(trade4, portfolio4, limits4);

    printResult(result4);

    return 0;
}
