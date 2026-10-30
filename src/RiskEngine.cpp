#include "RiskEngine.h"

namespace niveshsetu {

RiskResult RiskEngine::evaluate(
    const TradeInput& trade,
    const PortfolioInput& portfolio,
    const RiskLimits& limits
) const {

    RiskResult result;

    // Helper function to record rule results
    auto addRule = [&](std::string name, bool passed,
                       std::string explanation) {
        result.ruleResults.push_back({
            name,
            passed,
            explanation
        });

        if (!passed) {
            result.failureReasons.push_back(explanation);
        }
    };

    // 1. Validate trade inputs
    bool validTrade =
        !trade.symbol.empty() &&
        !trade.sector.empty() &&
        trade.entryPrice > 0 &&
        trade.stopLoss > 0 &&
        trade.stopLoss < trade.entryPrice &&
        trade.quantity > 0;

    addRule(
        "Trade Validation",
        validTrade,
        validTrade ? "Trade inputs are valid."
                   : "Invalid trade details."
    );

    if (!validTrade) return result;

    // 2. Validate portfolio
    bool validPortfolio =
        portfolio.accountValue > 0 &&
        portfolio.availableCash >= 0;

    addRule(
        "Portfolio Validation",
        validPortfolio,
        validPortfolio ? "Portfolio details are valid."
                       : "Invalid account value or available cash."
    );

    if (!validPortfolio) return result;

    // 3. Validate risk limits
    bool validLimits =
        limits.maxPlannedLoss >= 0 &&
        limits.maxSectorExposureFraction >= 0 &&
        limits.maxStressLossFraction >= 0;

    addRule(
        "Risk Limit Validation",
        validLimits,
        validLimits ? "Risk limits are valid."
                    : "Invalid risk limit configuration."
    );

    if (!validLimits) return result;

    // 4. Calculate planned loss
    result.plannedLoss =
        (trade.entryPrice - trade.stopLoss) * trade.quantity;

    addRule(
        "Maximum Planned Loss",
        result.plannedLoss <= limits.maxPlannedLoss,
        "Planned loss: " + std::to_string(result.plannedLoss)
    );

    // 5. Calculate required cash
    result.requiredCash =
        trade.entryPrice * trade.quantity;

    addRule(
        "Cash Availability",
        result.requiredCash <= portfolio.availableCash,
        "Required cash: " + std::to_string(result.requiredCash)
    );

    // 6. Calculate sector exposure
    auto exposures = portfolio.sectorExposure;

    for (const auto& item : exposures) {
        if (item.first.empty() || item.second < 0) {
            addRule(
                "Portfolio Exposure Validation",
                false,
                "Invalid existing sector exposure."
            );
            return result;
        }
    }

    double existingExposure = 0.0;

    if (exposures.count(trade.sector)) {
        existingExposure = exposures[trade.sector];
    }

    double tradeValue = result.requiredCash;

    double postTradeExposure = existingExposure + tradeValue;

    double concentration =
        postTradeExposure / portfolio.accountValue;

    result.postTradeSectorConcentration[trade.sector] =
        concentration;

    addRule(
        "Sector Concentration",
        concentration <= limits.maxSectorExposureFraction,
        "Post-trade sector concentration: " +
        std::to_string(concentration * 100) + "%"
    );

    // 7. Calculate portfolio stress loss
    exposures[trade.sector] = postTradeExposure;

    double totalStressLoss = 0.0;

    for (const auto& item : exposures) {

        const std::string& sector = item.first;
        double exposure = item.second;

        auto shock = limits.sectorStressShock.find(sector);

        if (shock == limits.sectorStressShock.end()) {
            addRule(
                "Stress Configuration",
                false,
                "Missing stress shock for sector: " + sector
            );
            return result;
        }

        if (shock->second < 0 || shock->second > 1) {
            addRule(
                "Stress Configuration",
                false,
                "Invalid stress shock for sector: " + sector
            );
            return result;
        }

        totalStressLoss += exposure * shock->second;

        result.postTradeSectorConcentration[sector] =
            exposure / portfolio.accountValue;
    }

    result.stressLossFraction =
        totalStressLoss / portfolio.accountValue;

    addRule(
        "Portfolio Stress Test",
        result.stressLossFraction <= limits.maxStressLossFraction,
        "Stress loss: " +
        std::to_string(result.stressLossFraction * 100) + "%"
    );

    // 8. Final decision
    result.decision = result.failureReasons.empty()
        ? RiskDecision::PASS
        : RiskDecision::BLOCK;

    return result;
}

} // namespace niveshsetu
