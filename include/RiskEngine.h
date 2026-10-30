
#ifndef NIVESHSETU_RISK_ENGINE_H
#define NIVESHSETU_RISK_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>

namespace niveshsetu {

// Final decision for a proposed trade
enum class RiskDecision {
    PASS,
    BLOCK
};

// Details of the proposed long BUY trade
struct TradeInput {
    std::string symbol;
    std::string sector;
    double entryPrice = 0.0;
    double stopLoss = 0.0;
    int quantity = 0;
};

// Existing portfolio information
struct PortfolioInput {
    double accountValue = 0.0;
    double availableCash = 0.0;

    // Existing market value invested in each sector
    std::unordered_map<std::string, double> sectorExposure;
};

// Risk thresholds and stress configuration
struct RiskLimits {
    double maxPlannedLoss = 10000.0;
    double maxSectorExposureFraction = 0.30;
    double maxStressLossFraction = 0.05;

    // Example: {"Banking", 0.10} means a 10% stress shock
    std::unordered_map<std::string, double> sectorStressShock;
};

// Result of an individual risk rule
struct RuleResult {
    std::string ruleName;
    bool passed = false;
    std::string explanation;
};

// Complete evaluation result
struct RiskResult {
    RiskDecision decision = RiskDecision::BLOCK;

    double plannedLoss = 0.0;
    double requiredCash = 0.0;
    double stressLossFraction = 0.0;

    std::unordered_map<std::string, double>
        postTradeSectorConcentration;

    std::vector<RuleResult> ruleResults;
    std::vector<std::string> failureReasons;
};

// Main risk evaluation interface
class RiskEngine {
public:
    RiskResult evaluate(
        const TradeInput& trade,
        const PortfolioInput& portfolio,
        const RiskLimits& limits
    ) const;
};

} // namespace niveshsetu

#endif
