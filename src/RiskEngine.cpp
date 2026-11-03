
#include "RiskEngine.h"

#include <cmath>
#include <string>

namespace niveshsetu {

RiskResult RiskEngine::evaluate(
    const TradeInput& trade,
    const PortfolioInput& portfolio,
    const RiskLimits& limits
) const {

    RiskResult result;

    // Function to record every rule
    auto addRule = [&](std::string name,
                       bool passed,
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

    // DAY 1: Validate trade details
    bool validTrade =
        !trade.symbol.empty() &&
        !trade.sector.empty() &&
        std::isfinite(trade.entryPrice) &&
        std::isfinite(trade.stopLoss) &&
        trade.entryPrice > 0 &&
        trade.stopLoss > 0 &&
        trade.stopLoss < trade.entryPrice &&
        trade.quantity > 0;

    addRule(
        "Trade Validation",
        validTrade,
        validTrade
            ? "Trade details are valid."
            : "Invalid trade details."
    );

    if (!validTrade) return result;

    // Validate portfolio
    bool validPortfolio =
        std::isfinite(portfolio.accountValue) &&
        std::isfinite(portfolio.availableCash) &&
        portfolio.accountValue > 0 &&
        portfolio.availableCash >= 0;

    addRule(
        "Portfolio Validation",
        validPortfolio,
        validPortfolio
            ? "Portfolio details are valid."
            : "Invalid portfolio details."
    );

    if (!validPortfolio) return result;

    // DAY 2: Validate existing sector exposures
    bool validExposure = true;

    for (const auto& item : portfolio.sectorExposure) {

        if (item.first.empty() ||
            !std::isfinite(item.second) ||
            item.second < 0) {

            validExposure = false;
            break;
        }
    }

    addRule(
        "Exposure Validation",
        validExposure,
        validExposure
            ? "Existing sector exposures are valid."
            : "Invalid sector exposure found."
    );

    if (!validExposure) return result;

    // DAY 3: Validate risk configuration
    bool validLimits =
        std::isfinite(limits.maxPlannedLoss) &&
        std::isfinite(limits.maxSectorExposureFraction) &&
        std::isfinite(limits.maxStressLossFraction) &&
        limits.maxPlannedLoss >= 0 &&
        limits.maxSectorExposureFraction >= 0 &&
        limits.maxSectorExposureFraction <= 1 &&
        limits.maxStressLossFraction >= 0 &&
        limits.maxStressLossFraction <= 1;

    addRule(
        "Risk Limit Validation",
        validLimits,
        validLimits
            ? "Risk limits are valid."
            : "Invalid risk limit configuration."
    );

    if (!validLimits) return result;

    // DAY 4: Calculate planned loss
    result.plannedLoss =
        (trade.entryPrice - trade.stopLoss) *
        trade.quantity;

    addRule(
        "Maximum Planned Loss",
        result.plannedLoss <= limits.maxPlannedLoss,
        "Calculated planned loss: " +
        std::to_string(result.plannedLoss)
    );

    // Calculate required cash
    result.requiredCash =
        trade.entryPrice * trade.quantity;

    addRule(
        "Cash Availability",
        result.requiredCash <= portfolio.availableCash,
        "Required cash: " +
        std::to_string(result.requiredCash)
    );

    // DAY 5: Calculate post-trade sector exposure
    auto exposures = portfolio.sectorExposure;

    exposures[trade.sector] += result.requiredCash;

    bool concentrationPassed = true;

    for (const auto& item : exposures) {

        double concentration =
            item.second / portfolio.accountValue;

        result.postTradeSectorConcentration[item.first] =
            concentration;

        if (concentration >
            limits.maxSectorExposureFraction) {

            concentrationPassed = false;
        }
    }

    addRule(
        "Sector Concentration",
        concentrationPassed,
        concentrationPassed
            ? "All sector exposure limits are satisfied."
            : "One or more sectors exceed the allowed concentration."
    );

    // DAY 5: Calculate portfolio stress loss
    double totalStressLoss = 0.0;
    bool stressConfigurationValid = true;

    for (const auto& item : exposures) {

        const std::string& sector = item.first;
        double exposure = item.second;

        auto shock =
            limits.sectorStressShock.find(sector);

        if (shock == limits.sectorStressShock.end()) {

            stressConfigurationValid = false;
            break;
        }

        if (!std::isfinite(shock->second) ||
            shock->second < 0 ||
            shock->second > 1) {

            stressConfigurationValid = false;
            break;
        }

        totalStressLoss += exposure * shock->second;
    }

    addRule(
        "Stress Configuration",
        stressConfigurationValid,
        stressConfigurationValid
            ? "Stress configuration is valid."
            : "Missing or invalid sector stress configuration."
    );

    if (!stressConfigurationValid) return result;

    result.stressLossFraction =
        totalStressLoss / portfolio.accountValue;

    addRule(
        "Portfolio Stress Test",
        result.stressLossFraction <=
            limits.maxStressLossFraction,
        "Calculated stress loss: " +
        std::to_string(result.stressLossFraction * 100) +
        "%"
    );

    // Final decision
    result.decision =
        result.failureReasons.empty()
        ? RiskDecision::PASS
        : RiskDecision::BLOCK;

    return result;
}

} // namespace niveshsetu
