
#include <iostream>
using namespace std;

int main() {
    double entryPrice = 1000;
    double stopLoss = 950;
    int quantity = 50;

    double riskPerShare = entryPrice - stopLoss;
    double totalRisk = riskPerShare * quantity;

    cout << "=== NiveshSetu ===" << endl;
    cout << "Entry Price: " << entryPrice << endl;
    cout << "Stop Loss: " << stopLoss << endl;
    cout << "Quantity: " << quantity << endl;
    cout << "Total Planned Risk: Rs. " << totalRisk << endl;

    return 0;
}
