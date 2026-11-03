
#include "RiskEngine.h"
#include "Member2.h"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace niveshsetu;

// Display risk report
void printResult(const RiskResult& result) {

    cout << "\n========== RISK REPORT ==========\n";

    cout << "Decision: ";

    if (result.decision == RiskDecision::PASS)
        cout << "PASS\n";
    else
        cout << "BLOCK\n";

    cout << fixed << setprecision(2);

    cout << "Planned Loss: Rs. "
         << result.plannedLoss << "\n";

    cout << "Required Cash: Rs. "
         << result.requiredCash << "\n";

    cout << "Stress Loss: "
         << result.stressLossFraction * 100 << "%\n";

    cout << "\nIndividual Rules:\n";

    for (const auto& rule : result.ruleResults) {

        cout << rule.ruleName << " : "
             << (rule.passed ? "PASS" : "FAIL") << "\n";

        cout << "Reason: "
             << rule.explanation << "\n";
    }

    cout << "\nFailure Reasons:\n";

    if (result.failureReasons.empty()) {
        cout << "No failures found.\n";
    }
    else {
        for (const auto& reason : result.failureReasons) {
            cout << "- " << reason << "\n";
        }
    }

    cout << "=================================\n";
}


// Main function
int main() {

    cout << "\n========== NIVESHSETU ==========\n";
    cout << "Pre-Trade Risk Management Engine\n";

    // 1. Create Risk Engine object
    RiskEngine engine;

    // 2. Load portfolio and risk limits from CSV
    PortfolioInput portfolio;
    RiskLimits limits;

    if (!member2::loadPortfolioCSV(
            "data/portfolio.csv", portfolio)) {
        return 1;
    }

    if (!member2::loadRiskLimitsCSV(
            "data/risk_limits.csv", limits)) {
        return 1;
    }

    cout << "\nPortfolio and risk limits loaded successfully!\n";


    // 3. TEST CASE 1 - Valid Trade
    cout << "\n\n******** TEST CASE 1 ********\n";

    TradeInput trade1;

    trade1.symbol = "TCS";
    trade1.sector = "IT";
    trade1.entryPrice = 100;
    trade1.stopLoss = 90;
    trade1.quantity = 10;

    RiskResult result1 =
        engine.evaluate(trade1, portfolio, limits);

    printResult(result1);

    member2::saveDecisionRecord(
        "data/decision_log.csv", trade1, result1
    );


    // 4. TEST CASE 2 - Insufficient Cash and High Loss
    cout << "\n\n******** TEST CASE 2 ********\n";

    TradeInput trade2;

    trade2.symbol = "INFY";
    trade2.sector = "IT";
    trade2.entryPrice = 100;
    trade2.stopLoss = 90;
    trade2.quantity = 60;

    RiskResult result2 =
        engine.evaluate(trade2, portfolio, limits);

    printResult(result2);

    member2::saveDecisionRecord(
        "data/decision_log.csv", trade2, result2
    );


    // 5. TEST CASE 3 - Invalid Stop Loss
    cout << "\n\n******** TEST CASE 3 ********\n";

    TradeInput trade3;

    trade3.symbol = "WIPRO";
    trade3.sector = "IT";
    trade3.entryPrice = 100;
    trade3.stopLoss = 110;
    trade3.quantity = 10;

    RiskResult result3 =
        engine.evaluate(trade3, portfolio, limits);

    printResult(result3);

    member2::saveDecisionRecord(
        "data/decision_log.csv", trade3, result3
    );


    // 6. TEST CASE 4 - Excessive Sector Concentration
    cout << "\n\n******** TEST CASE 4 ********\n";

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

    member2::saveDecisionRecord(
        "data/decision_log.csv", trade4, result4
    );


    // 7. Replay saved decision records
    cout << "\n\n******** DECISION REPLAY ********\n";

    member2::replayDecisionRecords(
        "data/decision_log.csv"
    );


    cout << "\nNiveshSetu execution completed successfully!\n";

    return 0;
}
