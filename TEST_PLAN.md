# TradeGuard 360 - Test Plan

## Individual Risk Tests
- T01: A valid trade below the risk limit should pass.
- T02: A trade above the risk limit should fail.
- T03: A trade exactly at the risk limit should follow the agreed boundary rule.
- T04: Invalid entry and stop-loss prices should be rejected.
- T05: Zero or negative quantity should be rejected.

## Portfolio Risk Tests
- T06: Sector exposure within the limit should pass.
- T07: Sector exposure above the limit should fail.

## Stress Tests
- T08: Stress loss within the configured limit should pass.
- T09: Stress loss above the configured limit should fail.

## Reporting Tests
- T10: The report should include planned risk and applicable limits.
- T11: A blocked trade should include a reason.
- T12: The report should show a clear final decision.

## Integration Tests
- T13: The application should handle valid input.
- T14: The application should handle invalid input without crashing.
- T15: The risk engine, portfolio checks, and report should agree on the final decision.

Note: Portfolio and stress tests require the corresponding modules to be implemented.
