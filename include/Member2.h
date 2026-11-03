#ifndef NIVESHSETU_MEMBER2_H
#define NIVESHSETU_MEMBER2_H

#include "RiskEngine.h"
#include <string>

namespace niveshsetu {
namespace member2 {

// Load portfolio details from CSV
bool loadPortfolioCSV(
    const std::string& filename,
    PortfolioInput& portfolio
);

// Load risk limits and sector shocks from CSV
bool loadRiskLimitsCSV(
    const std::string& filename,
    RiskLimits& limits
);

// Save a decision with a deterministic hash
bool saveDecisionRecord(
    const std::string& filename,
    const TradeInput& trade,
    const RiskResult& result
);

// Read and display previously saved decisions
void replayDecisionRecords(
    const std::string& filename
);

} // namespace member2
} // namespace niveshsetu

#endif