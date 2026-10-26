
#include <iostream>

int main() {
    std::cout << "TradeGuard 360 Test Plan\n\n";

    std::cout << "T01: Valid trade should PASS\n";
    std::cout << "T02: Invalid entry price should BLOCK\n";
    std::cout << "T03: Invalid stop-loss should BLOCK\n";
    std::cout << "T04: Invalid quantity should BLOCK\n";
    std::cout << "T05: Insufficient cash should BLOCK\n";
    std::cout << "T06: Planned risk above limit should BLOCK\n";
    std::cout << "T07: Sector concentration breach should BLOCK\n";
    std::cout << "T08: Stress-loss breach should BLOCK\n";
    std::cout << "T09: Boundary values should be checked\n";
    std::cout << "T10: Multiple failures should report all reasons\n";

    std::cout << "\nTest plan created successfully.\n";

    return 0;
}
