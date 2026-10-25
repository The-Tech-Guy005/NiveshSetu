
#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

// STEP 1: Store trade information
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
        cout << "\n===== NiveshSetu =====\n";
        cout << "Entry Price: Rs. " << entryPrice << '\n';
        cout << "Stop Loss: Rs. " << stopLoss << '\n';
        cout << "Quantity: " << quantity << '\n';
        cout << "Risk per Share: Rs. "
             << getRiskPerShare() << '\n';
        cout << "Total Planned Risk: Rs. "
             << getTotalRisk() << '\n';
    }
};

// STEP 2: Run the program
int main() {
    double entry, stop;
    int qty;

    cout << "Enter entry price: ";
    cin >> entry;

    cout << "Enter stop-loss price: ";
    cin >> stop;

    cout << "Enter quantity: ";
    cin >> qty;

    TradeTicket trade(entry, stop, qty);

    if (!trade.isValid()) {
        cout << "Invalid trade details. Please check your input.\n";
        return 1;
    }

    trade.display();

    return 0;
}
