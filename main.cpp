
#include <iostream>
#include <iomanip>
using namespace std;

class TradeTicket {
private:
    double entryPrice;
    double stopLoss;
    int quantity;

public:
    TradeTicket(double entry, double stop, int qty) {
        entryPrice = entry;
        stopLoss = stop;
        quantity = qty;
    }

    double getRiskPerShare() const {
        return entryPrice - stopLoss;
    }

    double getTotalRisk() const {
        return getRiskPerShare() * quantity;
    }

    bool isValid() const {
        return entryPrice > 0 &&
               stopLoss > 0 &&
               stopLoss < entryPrice &&
               quantity > 0;
    }

    void display() const {
        cout << fixed << setprecision(2);
        cout << "\n--- Trade Details ---\n";
        cout << "Entry Price: Rs. " << entryPrice << '\n';
        cout << "Stop Loss: Rs. " << stopLoss << '\n';
        cout << "Quantity: " << quantity << '\n';
        cout << "Planned Risk: Rs. " << getTotalRisk() << '\n';
    }
};

class RiskLimits {
private:
    double maxAllowedRisk;

public:
    RiskLimits(double limit) {
        maxAllowedRisk = limit;
    }

    bool isWithinLimit(double totalRisk) const {
        return totalRisk <= maxAllowedRisk;
    }

    void displayResult(double totalRisk) const {
        cout << "\n--- Risk Assessment ---\n";
        cout << "Maximum Allowed Risk: Rs. "
             << maxAllowedRisk << '\n';

        if (isWithinLimit(totalRisk)) {
            cout << "Result: PASS - Risk is within the limit.\n";
        } else {
            cout << "Result: BLOCK - Risk exceeds the limit.\n";
        }
    }
};


class Portfolio {
private:
    double availableCash;
    double holdingsValue;

public:
    Portfolio(double cash, double holdings) {
        availableCash = cash;
        holdingsValue = holdings;
    }

    double getAvailableCash() const {
        return availableCash;
    }

    double getHoldingsValue() const {
        return holdingsValue;
    }

    double getAccountValue() const {
        return availableCash + holdingsValue;
    }

    void display() const {
        cout << "\n--- Portfolio Summary ---\n";
        cout << "Available Cash: Rs. "
             << availableCash << '\n';
        cout << "Holdings Value: Rs. "
             << holdingsValue << '\n';
        cout << "Account Value: Rs. "
             << getAccountValue() << '\n';
    }
};

int main() {
    double entry, stop;
    int qty;

    cout << "=== NiveshSetu ===\n";

    cout << "Enter entry price: ";
    if (!(cin >> entry)) {
        cout << "Invalid entry price.\n";
        return 1;
    }

    cout << "Enter stop-loss price: ";
    if (!(cin >> stop)) {
        cout << "Invalid stop-loss price.\n";
        return 1;
    }

    cout << "Enter quantity: ";
    if (!(cin >> qty)) {
        cout << "Invalid quantity.\n";
        return 1;
    }

    TradeTicket trade(entry, stop, qty);

    if (!trade.isValid()) {
        cout << "Invalid trade details. Check your inputs.\n";
        return 1;
    }

    RiskLimits limits(3000);

    Portfolio portfolio(600000, 400000);
    portfolio.display();

    trade.display();
    limits.displayResult(trade.getTotalRisk());

    return 0;
}
