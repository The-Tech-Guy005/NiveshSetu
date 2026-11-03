# NiveshSetu – Pre-Trade Risk Management Engine

A C++ based pre-trade risk management system that evaluates proposed trades against portfolio constraints and configured risk limits before execution.

NiveshSetu helps identify potentially risky trades by validating trade details, calculating planned losses, checking available cash, monitoring sector concentration, and performing portfolio stress testing.

---

## Project Overview

NiveshSetu is designed to demonstrate how a pre-trade risk management engine can help prevent trades that violate predefined financial and portfolio risk constraints.

The system evaluates each proposed trade and generates one of two decisions:

- **PASS:** The trade satisfies all configured risk rules.
- **BLOCK:** The trade violates one or more risk rules.

The system also records trade decisions, generates integrity checksums, and supports replaying previously recorded decisions.

## Features

- **Trade Validation:** Checks trade parameters such as entry price, stop-loss price, and quantity.
- **Planned Loss Calculation:** Calculates the potential loss associated with a proposed trade.
- **Cash Availability Check:** Ensures sufficient available cash exists for the proposed trade.
- **Sector Concentration Monitoring:** Prevents excessive portfolio exposure to a particular sector.
- **Portfolio Stress Testing:** Estimates potential portfolio losses using configured sector stress shocks.
- **CSV Configuration:** Loads portfolio information and risk limits from CSV files.
- **Decision Logging:** Saves evaluated trade decisions in a CSV log.
- **Hash-Based Integrity Verification:** Uses FNV-1a checksums to detect accidental changes to logged records.
- **Decision Replay:** Reads previously recorded decisions and verifies their integrity.
- **Automated Testing:** Includes 15 risk-engine test scenarios.

## Technology Stack

| Technology | Purpose |
|---|---|
| C++ | Core application development |
| C++17 | Programming language standard |
| CMake | Build configuration |
| MinGW GCC | C++ compiler |
| CSV | Portfolio and risk configuration |
| Git & GitHub | Version control |

## Project Structure

```text
NiveshSetu/
│
├── include/
│   ├── RiskEngine.h
│   └── Member2.h
│
├── src/
│   ├── RiskEngine.cpp
│   ├── Member2.cpp
│   └── main.cpp
│
├── data/
│   ├── portfolio.csv
│   ├── risk_limits.csv
│   └── decision_log.csv
│
├── risk_engine_tests.cpp
├── CMakeLists.txt
├── README.md
└── build/
```

## Risk Management Rules

The engine evaluates trades using the following rules:

### 1. Trade Validation

Checks whether the proposed trade has valid parameters, including positive entry price and quantity and a valid stop-loss price.

### 2. Planned Loss

For a valid BUY trade:

```text
Planned Loss = (Entry Price - Stop Loss) × Quantity
```

The calculated loss must not exceed the configured maximum planned loss.

### 3. Cash Availability

```text
Required Cash = Entry Price × Quantity
```

The trade is blocked if required cash exceeds available cash.

### 4. Sector Concentration

The engine calculates the sector's post-trade exposure as a fraction of the account value.

The exposure must not exceed the configured sector concentration limit.

### 5. Portfolio Stress Testing

The engine applies configured sector stress shocks to portfolio exposures and calculates the estimated stress loss as a fraction of account value.

Trades are blocked if the configured maximum stress-loss fraction is exceeded.

### Decision Logic

A trade receives **PASS** only when all required risk checks succeed. If any required check fails, the trade receives **BLOCK**.

## CSV Configuration

The application reads portfolio and risk configuration from CSV files.

### Portfolio Configuration

File: `data/portfolio.csv`

Example:

```csv
type,key,value
account_value,,100000
available_cash,,5000
sector,IT,20000
sector,Banking,20000
```

| Field | Description |
|---|---|
| account_value | Total portfolio account value |
| available_cash | Cash currently available |
| sector | Existing sector exposure |
| key | Sector name |
| value | Corresponding numeric amount |

### Risk Limits Configuration

File: `data/risk_limits.csv`

Example:

```csv
type,key,value
max_planned_loss,,500
max_sector_exposure,,0.30
max_stress_loss,,0.05
sector_shock,IT,0.10
sector_shock,Banking,0.05
```

| Configuration | Description |
|---|---|
| max_planned_loss | Maximum permitted planned loss |
| max_sector_exposure | Maximum sector exposure fraction |
| max_stress_loss | Maximum portfolio stress-loss fraction |
| sector_shock | Stress shock configured for a sector |

**Note:** Fractional values are represented as decimals. For example, `0.30` means 30%.

## Installation and Setup

### Prerequisites

Install the following:

- C++ compiler supporting C++17
- CMake 3.10 or later
- MinGW GCC (for Windows users)
- Git

### Clone the Repository

```bash
git clone https://github.com/The-Tech-Guy005/NiveshSetu.git
```

Navigate to the project directory:

```bash
cd NiveshSetu
```

### Configure the Project

For Windows with MinGW Makefiles:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
```

### Build the Project

```powershell
cmake --build build
```

## Running the Application

After a successful build, run:

```powershell
.\build\tradeguard.exe
```

The application will:

1. Load portfolio configuration.
2. Load risk limits.
3. Evaluate the predefined trade scenarios.
4. Display detailed risk reports.
5. Save decision records.
6. Replay saved records.
7. Verify record integrity.

## Running Automated Tests

Build and execute the test suite:

```powershell
cmake --build build --clean-first
ctest --test-dir build --output-on-failure
```

To run the test executable directly:

```powershell
.\build\tradeguard_tests.exe
```

### Test Coverage

The project includes 15 automated test scenarios:

| Test | Scenario | Expected Result |
|---|---|---|
| T01 | Valid trade | PASS |
| T02 | Zero entry price | BLOCK |
| T03 | Invalid stop-loss | BLOCK |
| T04 | Zero quantity | BLOCK |
| T05 | Insufficient cash | BLOCK |
| T06 | Exact planned-loss limit | PASS |
| T07 | Planned-loss limit exceeded | BLOCK |
| T08 | Exact sector concentration limit | PASS |
| T09 | Sector concentration exceeded | BLOCK |
| T10 | Missing stress shock | BLOCK |
| T11 | Negative entry price | BLOCK |
| T12 | Negative quantity | BLOCK |
| T13 | NaN entry price | BLOCK |
| T14 | Invalid stress shock | BLOCK |
| T15 | Exact stress-loss limit | PASS |

## Decision Logging and Replay

Each evaluated trade is saved in a decision log.

The log stores:

- Stock symbol
- Sector
- Entry price
- Stop-loss price
- Quantity
- Risk decision
- Planned loss
- Required cash
- Stress-loss fraction
- Integrity checksum

The application uses the FNV-1a checksum algorithm to detect accidental modifications to stored decision records.

During replay, the system recalculates the checksum and compares it with the stored value.

**Important:** FNV-1a is a non-cryptographic checksum. It provides basic integrity checking against accidental changes, not protection against deliberate tampering.

## Sample Execution Results

The current implementation demonstrates four trade scenarios:

| Symbol | Decision | Reason |
|---|---|---|
| TCS | PASS | All risk checks satisfied |
| INFY | BLOCK | Planned loss and cash constraints |
| WIPRO | BLOCK | Invalid trade details |
| INFY | BLOCK | Sector concentration exceeded |

### Latest Verification

The project has successfully completed the following checks:

- CMake build: Successful
- CTest: Passed
- Risk-engine tests: 15/15 passed
- Portfolio CSV loading: Successful
- Risk limits CSV loading: Successful
- Trade execution: 4 scenarios completed
- Decision replay: 4/4 records verified, 0 failures

## Contributors

This project was developed collaboratively, with responsibilities distributed across:

- **Member 1:** Core risk engine and risk evaluation logic.
- **Member 2:** CSV configuration loading, decision logging, hashing, and replay.
- **Integration and Testing:** Build configuration, integration, automated testing, and final verification.

## Future Improvements

Potential enhancements include:

- Interactive trade input instead of predefined test scenarios.
- Support for additional trade types.
- More advanced portfolio risk metrics.
- Cryptographic integrity verification using a secure hash algorithm.
- Database-backed decision storage.
- Graphical user interface and reporting dashboard.
- Expanded automated and integration testing.

## Disclaimer

NiveshSetu is an educational and demonstration project for pre-trade risk management. It is not intended to provide investment advice or replace professional financial risk systems.

---

**NiveshSetu – Evaluate Risk Before Executing Trades.**