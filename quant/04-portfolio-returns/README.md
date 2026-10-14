# Portfolio Returns in C++17

Calculate the return of a portfolio from the fractions invested in each asset and each asset's return.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for a full introduction starting from high-school mathematics.

## Build and run

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o portfolio
./portfolio
g++ -std=c++17 -Wall -Wextra -Wpedantic tests.cpp -o portfolio_tests
./portfolio_tests
```

Expected program output: `Portfolio return: 4.00%`.

The implementation assumes long-only weights that sum to 1, a single measurement period, and finite numeric inputs. It excludes fees, taxes, cash flows during the period, leverage and changing weights.
