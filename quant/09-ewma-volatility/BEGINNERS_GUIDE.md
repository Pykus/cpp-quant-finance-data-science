# A First-Year Student's Guide: EWMA Volatility

This tutorial starts with percentages and builds to volatility forecasting, normal-distribution risk estimates, and robust C++20 implementation. Work each arithmetic example on paper first. No prior finance, statistics, or trading knowledge is assumed.

## 1. Returns and percentages

A daily simple return is (new price - old price)/old price. Going from 100 to 102 is (102-100)/100 = 0.02, which is +2%. Going from 100 to 97 is -0.03, or -3%. In the C++ program, always provide decimal fractions, never whole percentages. Passing 2 instead of 0.02 would mean +200%.

## 2. What does volatility measure?

Volatility describes how large returns tend to be, not whether tomorrow's return is positive or negative. A +3% day and a -3% day have equal magnitude. Risk management often studies variability because large movements can produce large gains or losses.

## 3. Squaring returns

Squaring removes the sign: (+0.03)^2 = 0.0009 and (-0.03)^2 = 0.0009. A larger move contributes disproportionately: a 6% return squared is 0.0036, four times the square of 3%. The model treats large shocks as evidence for larger future volatility.

## 4. Variance versus volatility

Variance is in squared return units. Volatility is its square root. For example variance 0.0001 corresponds to sqrt(0.0001)=0.01=1% volatility. Variance 0.0004 corresponds to 2% volatility. This is easy to confuse: 0.0001 is not a volatility of 0.01%.

## 5. Why not an ordinary average?

An equally weighted average of old squared returns treats distant history like yesterday. EWMA reduces the weight of older observations to respond to changes in market variability. It requires a single saved variance rather than retaining the entire series.

## 6. The EWMA update

The formula is next_variance = lambda*old_variance + (1-lambda)*today_return^2. Lambda is a decimal strictly between zero and one. If lambda = 0.94, the update keeps 94% of the previous variance estimate and adds 6% of today's squared return. The weights add to one.

## 7. Hand calculation

Set old variance to 0.0001 (1% volatility), lambda to 0.94, and observed return to 0.02 (2%). Squared return: 0.0004. Old contribution: 0.94*0.0001=0.000094. New contribution: 0.06*0.0004=0.000024. Add: 0.000118. Square root: approximately 0.0108628, or 1.0863%.

## 8. Negative and quiet days

A -2% return has squared value 0.0004 too. After variance 0.000118, another +/-2% return gives 0.94*0.000118+0.06*0.0004=0.00013492. A subsequent zero return gives 0.94*0.00013492=0.0001268248. Big moves raise the forecast; quiet days let it drift lower.

## 9. Time ordering: no future data

Before observing the return for day t, the model's v[t] is a forecast for day t. After that return becomes known, observe(r[t]) calculates v[t+1]. Never update with today's return and then claim the result was today's earlier forecast. That mistake is called look-ahead bias, a common form of data leakage in backtesting.

## 10. How lambda controls reaction

When lambda is low, the new squared return receives high weight; when it is high, the old forecast receives high weight. For old variance 0.0001 and a +10% shock, lambda 0.50 gives 0.00505 while lambda 0.99 gives 0.000199. The best value depends on the data and how out-of-sample forecasts are evaluated.

## 11. Understanding half-life

Old observations have relative weight proportional to lambda^n after n updates. Setting this equal to 0.5 yields n=log(0.5)/log(lambda). With lambda 0.94 the half-life is approximately 11.2 observations. With lambda 0.5 it is exactly one observation. Half-life is not a hard cutoff.

## 12. Initializing variance

The recurrence requires a variance before seeing the first return. The demonstration chooses 0.0001 explicitly. For a real evaluation, calculate a seed using only earlier training returns. Do not use data from a later evaluation interval to initialize the model.

## 13. Daily to annual risk

Under a simple independent-period scaling approximation, variance grows in proportion to time, and volatility by sqrt(time). So daily volatility 2% annualizes to 0.02*sqrt(252), approximately 31.75%. Multiplying volatility directly by 252 is wrong. The assumption can be inaccurate when volatility changes persistently.

## 14. Normal-model 95% VaR

The previous historical VaR lesson sorted past realized losses. In contrast, this lesson assumes tomorrow's return is normally distributed with mean zero and standard deviation equal to EWMA sigma. The one-sided 95% normal multiplier is about 1.64485. With sigma 0.02 the VaR fraction is 0.032897, around 3.29%, or 329 units on a 10,000-unit portfolio.

## 15. What VaR does not promise

A normal-model 95% VaR of 329 does not mean losses cannot exceed 329. In that model, 5% of outcomes have larger losses. Actual market returns need not be normally distributed. Heavy tails and market shocks can make extreme events much more common than predicted.

## 16. Expected shortfall

Expected shortfall asks for the average of the losses within the worst 5% tail. Under this normal model it is sigma*phi(z95)/0.05, or about 2.06271*sigma. If sigma is 2%, ES is roughly 4.13%, or 413 units on 10,000. It exceeds VaR because it averages outcomes *beyond* the threshold.

## 17. Reading the code

EwmaVolatility stores a decay parameter, a variance forecast, and an observation count. Accessors marked const cannot change the state. [[nodiscard]] alerts readers that these return values should not be casually ignored. std::size_t counts observations. The observe method validates the return and intermediate calculations before committing changes.

## 18. Safe behavior on errors

Floating-point numbers can contain NaN and infinity, or produce infinity when a huge finite value is squared. Input validation detects these conditions. An exception is raised before modifying the variance or count. Tests confirm that an invalid update does not leave a half-updated forecasting model.

## 19. Verifying a numerical model

The tests reproduce hand calculations, check volatility conversion, normal tail-risk constants, shock response, edge cases, and forecast timing. Tolerances accommodate small binary floating-point rounding differences. They also verify that bad input produces an exception and leaves prior state intact.

## 20. Further experiments

Change lambda to 0.70 and compare the model path with lambda 0.94. Try a long sequence of zeros after a large shock to observe decay. Then compare model forecasts against unseen future returns in chronological windows. Advanced work can calibrate lambda, investigate fat-tailed distributions, or compare EWMA with GARCH.

## Exercises

1. How is -1.5% written as a decimal return?

2. What does a return of -3% contribute after squaring?

3. With lambda 0.9, variance 0.0001 and return +2%, calculate next variance.

4. What daily volatility corresponds to variance 0.00013?

5. What is next variance after a zero return with v=0.0004 and lambda=0.8?

6. Which reacts faster to a sudden 10% move: lambda 0.6 or 0.98?

7. If you learn Tuesday's return after market close, which forecast can it update?

8. For sigma=1% and portfolio value 20,000, what is normal-model 95% VaR?

9. What is normal-model 95% expected shortfall with sigma=1%?

10. Why is expected shortfall larger than VaR?

11. What happens if observe(infinity) is called?

12. How would you select lambda without using future test data?

## Worked answers

1. -0.015; divide by 100.

2. (-0.03)^2 = 0.0009, identical to a +3% squared return.

3. 0.9*0.0001+0.1*0.0004=0.00013.

4. sqrt(0.00013) approximately 0.011402, or 1.14%.

5. 0.8*0.0004=0.00032.

6. 0.6, because the new observation receives 40% weight instead of 2%.

7. Wednesday's forecast, not the Tuesday forecast made earlier.

8. About 1.64485% or 328.97 monetary units.

9. About 2.06271% or 412.54 units on the same portfolio.

10. ES averages the worst-tail losses while VaR identifies the start of that tail.

11. The method throws; the saved forecast and observation counter stay unchanged.

12. Choose it with earlier training/validation observations; keep a later test interval untouched.

## Final recap

A valid forecast must use only past information. EWMA estimates conditional variance by weighting the previous forecast and the latest squared return. The square root gives volatility. A normal-model VaR or ES estimate adds a strong tail-shape assumption. Treat data units, chronology, numerical validation, and held-out evaluation as central parts of the model.
