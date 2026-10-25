# Beginner's guide — why option prices are linked

## Start with an idea, not a formula

A **call** grants the right to buy a share at a fixed strike later.
A **put** grants the right to sell at that same strike later.
European options can be exercised only at expiry.

Compare two portfolios at expiry:

1. Own one call and hold cash that grows into the strike.
2. Own one put and one share.

If the stock finishes above the strike, portfolio 1 uses the call to buy
the share; portfolio 2 still owns its share. Both end with a share.

If the stock finishes below the strike, portfolio 1 holds the strike in cash;
portfolio 2 can sell the share for the strike using its put. Both end with
the same cash amount.

The two portfolios deliver identical payoffs in this simplified market.
Thus their starting values must match:

    Call + present_value(Strike) = Put + Stock

Rearranging gives the equation the code checks.

## Worked numbers

Stock = 100, strike = 100, risk-free rate = 0.05/year, expiry = 1 year.

Discounted strike = 100 × exp(-0.05) ≈ 95.12294.

Stock minus discounted strike ≈ 4.87706.
If the call costs 10, the matching put price is 10 - 4.87706 = 5.12294.
The computed error for these exact inputs is close to zero, with small
floating-point rounding.

## Learn the code

The input structure makes it difficult to mix up a call and a put.
The `std::exp` calculation converts the future strike into today's value.
Subtracting put from call gives the **quoted** price difference. Subtracting
discounted strike from the stock gives the **theoretical** difference.
The function compares their absolute distance against a tolerance.

`double` is suitable for this numerical teaching example, but actual
financial settlement amounts should use a defined fixed-point or decimal policy.
Use a quote-convention-aware tolerance; 1e-9 is not a real-world trading threshold.

## Limits

The model assumes matching European options, common strike and expiry,
continuous interest, and no dividends. Transaction costs, borrowing rules,
timing mismatches, and market microstructure change practical comparisons.
Do not present this formula as a guaranteed profit detector.
