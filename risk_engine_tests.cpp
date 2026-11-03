
#include "RiskEngine.h"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace niveshsetu;

constexpr double EPSILON = 1e-6;

void checkNear(double actual, double expected) {
    assert(std::abs(actual - expected) < EPSILON);
}

TradeInput validTrade() {
    return {"DEMO", "Technology", 100.0, 90.0, 10};
}

PortfolioInput validPortfolio() {
    PortfolioInput p;
    p.accountValue = 100000.0;
    p.availableCash = 50000.0;
    p.sectorExposure["Technology"] = 10000.0;
    return p;
}

RiskLimits validLimits() {
    RiskLimits l;
    l.maxPlannedLoss = 1000.0;
    l.maxSectorExposureFraction = 0.30;
    l.maxStressLossFraction = 0.05;
    l.sectorStressShock["Technology"] = 0.10;
    return l;
}

void testValidTrade() {
    RiskEngine engine;
    auto result = engine.evaluate(
        validTrade(), validPortfolio(), validLimits());

    assert(result.decision == RiskDecision::PASS);
    checkNear(result.plannedLoss, 100.0);
    checkNear(result.requiredCash, 1000.0);
}

void testInvalidEntryPrice() {
    auto trade = validTrade();
    trade.entryPrice = 0.0;

    RiskEngine engine;
    auto result = engine.evaluate(
        trade, validPortfolio(), validLimits());

    assert(result.decision == RiskDecision::BLOCK);
    assert(!result.failureReasons.empty());
}

void testInvalidStopLoss() {
    auto trade = validTrade();
    trade.stopLoss = 100.0;

    RiskEngine engine;
    auto result = engine.evaluate(
        trade, validPortfolio(), validLimits());

    assert(result.decision == RiskDecision::BLOCK);
    assert(!result.failureReasons.empty());
}

void testInvalidQuantity() {
    auto trade = validTrade();
    trade.quantity = 0;

    RiskEngine engine;
    auto result = engine.evaluate(
        trade, validPortfolio(), validLimits());

    assert(result.decision == RiskDecision::BLOCK);
    assert(!result.failureReasons.empty());
}

void testInsufficientCash() {
    auto portfolio = validPortfolio();
    portfolio.availableCash = 999.0;

    RiskEngine engine;
    auto result = engine.evaluate(
        validTrade(), portfolio, validLimits());

    assert(result.decision == RiskDecision::BLOCK);
}

void testPlannedLossExactLimit() {
    auto limits = validLimits();
    limits.maxPlannedLoss = 100.0;

    RiskEngine engine;
    auto result = engine.evaluate(
        validTrade(), validPortfolio(), limits);

    assert(result.decision == RiskDecision::PASS);
    checkNear(result.plannedLoss, 100.0);
}

void testPlannedLossAboveLimit() {
    auto limits = validLimits();
    limits.maxPlannedLoss = 99.0;

    RiskEngine engine;
    auto result = engine.evaluate(
        validTrade(), validPortfolio(), limits);

    assert(result.decision == RiskDecision::BLOCK);
}

void testSectorConcentrationExactLimit() {
    auto portfolio = validPortfolio();

    
    portfolio.sectorExposure["Technology"] = 29000.0;

    auto limits = validLimits();
    limits.maxSectorExposureFraction = 0.30;

    RiskEngine engine;
    auto result = engine.evaluate(
        validTrade(), portfolio, limits);

    assert(result.decision == RiskDecision::PASS);
    checkNear(
        result.postTradeSectorConcentration.at("Technology"),
        0.30
    );
}

void testSectorConcentrationAboveLimit() {
    auto portfolio = validPortfolio();
    portfolio.sectorExposure["Technology"] = 30000.0;

    auto limits = validLimits();
    limits.maxSectorExposureFraction = 0.30;

    RiskEngine engine;
    auto result = engine.evaluate(
        validTrade(), portfolio, limits);

    assert(result.decision == RiskDecision::BLOCK);
}

void testMissingStressShock() {
    auto limits = validLimits();
    limits.sectorStressShock.clear();

    RiskEngine engine;
    auto result = engine.evaluate(
        validTrade(), validPortfolio(), limits);

    assert(result.decision == RiskDecision::BLOCK);
    assert(!result.failureReasons.empty());
}

int main() {
    testValidTrade();
    std::cout << "T01 PASS: Valid trade\n";

    testInvalidEntryPrice();
    std::cout << "T02 PASS: Invalid entry price blocked\n";

    testInvalidStopLoss();
    std::cout << "T03 PASS: Invalid stop-loss blocked\n";

    testInvalidQuantity();
    std::cout << "T04 PASS: Invalid quantity blocked\n";

    testInsufficientCash();
    std::cout << "T05 PASS: Insufficient cash blocked\n";

    testPlannedLossExactLimit();
    std::cout << "T06 PASS: Exact planned-loss limit\n";

    testPlannedLossAboveLimit();
    std::cout << "T07 PASS: Above planned-loss limit blocked\n";

    testSectorConcentrationExactLimit();
    std::cout << "T08 PASS: Exact sector concentration limit\n";

    testSectorConcentrationAboveLimit();
    std::cout << "T09 PASS: Above sector concentration blocked\n";

    testMissingStressShock();
    std::cout << "T10 PASS: Missing stress shock blocked\n";

    std::cout << "\nAll 10 tests passed!\n";
    return 0;
}
