# Quant Lesson 00: Your First Option Calculation in C++17

**Start here.** You do not need finance experience, probability, or advanced mathematics.

## 1. What is an option?

Imagine a share costs **100** today. You buy a **call option** that gives you the right (but not the obligation) to buy one share for **100** on a specified future date.

That agreed purchase price is called the **strike price**.

If the share costs **120** on that date, the right to buy it for 100 is worth **20**. If the share costs **90**, you can simply choose not to buy at 100, so the right is worth **0**.

| Share price at expiry | Strike | Value of the call |
| --- | --- | --- |
| 90 | 100 | 0 |
| 100 | 100 | 0 |
| 120 | 100 | 20 |
| 150 | 100 | 50 |

**Important:** This is the *payoff at expiry*, not your trading profit. A real option costs money (the **premium**). We deliberately leave the premium out of this first calculation.

## 2. One simple rule

The payoff is the larger of:

- the difference between the future share price and the strike, or
- zero.

In everyday language: **If buying at the agreed price would save you money, that saving is the payoff. Otherwise the payoff is zero.**

Mathematically:

`payoff = max(future_share_price - strike, 0)`

Here, `max` means "choose the larger number." No probability is involved yet.

## 3. Translate the rule into C++

Open [main.cpp](main.cpp). The key function is:

```cpp
double call_payoff(double future_price, double strike) {
    return std::max(future_price - strike, 0.0);
}
```

- `double`: a C++ number type that can hold decimals.
- `future_price`: the share price on the expiry date.
- `strike`: the agreed purchase price.
- `return`: gives the calculated value back to the caller.
- `std::max(a, b)`: selects the larger of two numbers.

Try `future_price = 120` and `strike = 100`. The function returns `20`.

## 4. Run it yourself

From this lesson's directory:

```sh
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o option_basics
./option_basics
```

Expected output:

```text
future_price=90 strike=100 payoff=0
future_price=100 strike=100 payoff=0
future_price=120 strike=100 payoff=20
future_price=150 strike=100 payoff=50
```

No third-party libraries are required.

## 5. Check your understanding

1. What is the payoff if the share ends at 105 and the strike is 100? **Answer: 5.**
2. What is the payoff if the share ends at 80? **Answer: 0.**
3. If the option originally cost 7 and its payoff is 20, what is the profit *ignoring fees and financing*? **Answer: 13.** This is why payoff and profit are not the same.
4. Change the strike in `main.cpp` to 110. Predict the results before running it.

## 6. What comes next?

Now that you know how to calculate the payoff for **one possible future price**, the [next lesson](../01-monte-carlo-option-pricing/) explains how a computer can try **many possible future prices**. That is the central idea of Monte Carlo simulation.

We will only then introduce probabilities, averages, the Black–Scholes model, and uncertainty estimates.
