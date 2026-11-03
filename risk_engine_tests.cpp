#include "RiskEngine.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace niveshsetu;

const double EPSILON = 1e-6;

// --------------------------------------------------
// Helper functions
// --------------------------------------------------

TradeInput validTrade() {
    TradeInput trade;
    trade.symbol = "DEMO";
    trade.sector = "Technology";
    trade.entryPrice = 100.0;
    trade.stopLoss = 90.0;
    trade.quantity = 10;
    return trade;
}

PortfolioInput validPortfolio() {
    PortfolioInput portfolio;
    portfolio.accountValue = 100000.0;
    portfolio.availableCash = 50000.0;
    portfolio.sectorExposure["Technology"] = 10000.0;
    return portfolio;
}

RiskLimits validLimits() {
    RiskLimits limits;
    limits.maxPlannedLoss = 1000.0;
    limits.maxSectorExposureFraction = 0.30;
    limits.maxStressLossFraction = 0.05;
    limits.sectorStressShock["Technology"] = 0.10;
    return limits;
}

// --------------------------------------------------
// T01: Valid trade should PASS
// --------------------------------------------------

void testValidTrade() {
    RiskEngine engine;

    RiskResult result = engine.evaluate(
        validTrade(), validPortfolio(), validLimits()
    );

    assert(result.decision == RiskDecision::PASS);
    assert(std::fabs(result.plannedLoss - 100.0) < EPSILON);
    assert(std::fabs(result.requiredCash - 1000.0) < EPSILON);

    std::cout << "T01 PASS: Valid trade accepted\n";
}

// --------------------------------------------------
// T02: Zero entry price should BLOCK
// --------------------------------------------------

void testZeroEntryPrice() {
    TradeInput trade = validTrade();
    trade.entryPrice = 0.0;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        trade, validPortfolio(), validLimits()
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T02 PASS: Zero entry price blocked\n";
}

// --------------------------------------------------
// T03: Stop loss equal to entry price should BLOCK
// --------------------------------------------------

void testStopLossEqualToEntry() {
    TradeInput trade = validTrade();
    trade.stopLoss = trade.entryPrice;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        trade, validPortfolio(), validLimits()
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T03 PASS: Invalid stop loss blocked\n";
}

// --------------------------------------------------
// T04: Zero quantity should BLOCK
// --------------------------------------------------

void testZeroQuantity() {
    TradeInput trade = validTrade();
    trade.quantity = 0;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        trade, validPortfolio(), validLimits()
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T04 PASS: Zero quantity blocked\n";
}

// --------------------------------------------------
// T05: Insufficient cash should BLOCK
// --------------------------------------------------

void testInsufficientCash() {
    PortfolioInput portfolio = validPortfolio();
    portfolio.availableCash = 999.0;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        validTrade(), portfolio, validLimits()
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T05 PASS: Insufficient cash blocked\n";
}

// --------------------------------------------------
// T06: Exact planned-loss limit should PASS
// --------------------------------------------------

void testExactPlannedLossLimit() {
    RiskLimits limits = validLimits();
    limits.maxPlannedLoss = 100.0;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        validTrade(), validPortfolio(), limits
    );

    assert(result.decision == RiskDecision::PASS);
    assert(std::fabs(result.plannedLoss - 100.0) < EPSILON);

    std::cout << "T06 PASS: Exact planned-loss limit accepted\n";
}

// --------------------------------------------------
// T07: Exceeding planned-loss limit should BLOCK
// --------------------------------------------------

void testExceededPlannedLossLimit() {
    RiskLimits limits = validLimits();
    limits.maxPlannedLoss = 99.0;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        validTrade(), validPortfolio(), limits
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T07 PASS: Exceeded planned-loss limit blocked\n";
}

// --------------------------------------------------
// T08: Exact sector concentration limit should PASS
// --------------------------------------------------

void testExactSectorConcentrationLimit() {
    PortfolioInput portfolio = validPortfolio();
    portfolio.sectorExposure["Technology"] = 29000.0;

    RiskLimits limits = validLimits();
    limits.maxSectorExposureFraction = 0.30;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        validTrade(), portfolio, limits
    );

    assert(result.decision == RiskDecision::PASS);
    assert(std::fabs(
        result.postTradeSectorConcentration["Technology"] - 0.30
    ) < EPSILON);

    std::cout << "T08 PASS: Exact sector concentration limit accepted\n";
}

// --------------------------------------------------
// T09: Exceeding sector concentration should BLOCK
// --------------------------------------------------

void testExceededSectorConcentrationLimit() {
    PortfolioInput portfolio = validPortfolio();
    portfolio.sectorExposure["Technology"] = 30000.0;

    RiskLimits limits = validLimits();
    limits.maxSectorExposureFraction = 0.30;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        validTrade(), portfolio, limits
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T09 PASS: Excess sector concentration blocked\n";
}

// --------------------------------------------------
// T10: Missing sector stress shock should BLOCK
// --------------------------------------------------

void testMissingStressShock() {
    RiskLimits limits = validLimits();
    limits.sectorStressShock.clear();

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        validTrade(), validPortfolio(), limits
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T10 PASS: Missing stress shock blocked\n";
}

// --------------------------------------------------
// T11: Negative entry price should BLOCK
// --------------------------------------------------

void testNegativeEntryPrice() {
    TradeInput trade = validTrade();
    trade.entryPrice = -100.0;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        trade, validPortfolio(), validLimits()
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T11 PASS: Negative entry price blocked\n";
}

// --------------------------------------------------
// T12: Negative quantity should BLOCK
// --------------------------------------------------

void testNegativeQuantity() {
    TradeInput trade = validTrade();
    trade.quantity = -5;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        trade, validPortfolio(), validLimits()
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T12 PASS: Negative quantity blocked\n";
}

// --------------------------------------------------
// T13: NaN entry price should BLOCK
// --------------------------------------------------

void testNaNEntryPrice() {
    TradeInput trade = validTrade();
    trade.entryPrice = std::nan("");

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        trade, validPortfolio(), validLimits()
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T13 PASS: NaN entry price blocked\n";
}

// --------------------------------------------------
// T14: Invalid stress shock above 100% should BLOCK
// --------------------------------------------------

void testInvalidStressShock() {
    RiskLimits limits = validLimits();
    limits.sectorStressShock["Technology"] = 1.1;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        validTrade(), validPortfolio(), limits
    );

    assert(result.decision == RiskDecision::BLOCK);

    std::cout << "T14 PASS: Invalid stress shock blocked\n";
}

// --------------------------------------------------
// T15: Exact stress-loss limit should PASS
// --------------------------------------------------

void testExactStressLossLimit() {
    PortfolioInput portfolio = validPortfolio();
    portfolio.sectorExposure["Technology"] = 49000.0;

    RiskLimits limits = validLimits();
    limits.maxSectorExposureFraction = 0.60;
    limits.maxStressLossFraction = 0.05;

    RiskEngine engine;
    RiskResult result = engine.evaluate(
        validTrade(), portfolio, limits
    );

    assert(result.decision == RiskDecision::PASS);
    assert(std::fabs(result.stressLossFraction - 0.05) < EPSILON);

    std::cout << "T15 PASS: Exact stress-loss limit accepted\n";
}

// --------------------------------------------------
// Main test runner
// --------------------------------------------------

int main() {
    std::cout << "====================================\n";
    std::cout << "       NIVESHSETU RISK ENGINE TESTS\n";
    std::cout << "====================================\n\n";

    testValidTrade();
    testZeroEntryPrice();
    testStopLossEqualToEntry();
    testZeroQuantity();
    testInsufficientCash();
    testExactPlannedLossLimit();
    testExceededPlannedLossLimit();
    testExactSectorConcentrationLimit();
    testExceededSectorConcentrationLimit();
    testMissingStressShock();
    testNegativeEntryPrice();
    testNegativeQuantity();
    testNaNEntryPrice();
    testInvalidStressShock();
    testExactStressLossLimit();

    std::cout << "\n====================================\n";
    std::cout << "       ALL 15 TESTS PASSED!\n";
    std::cout << "====================================\n";

    return 0;
}