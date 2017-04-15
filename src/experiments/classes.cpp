#include <stdio.h>

class Fraction {

private:
  int numerator;
  int denominator;

public:
  int getDenominator() {
    return denominator;
  }
  int getNumerator() {
    return numerator;
  }

  void setNumerator(int n) {
    numerator = n;
  }

  void setDenominator(int d) {
    denominator = d;
  }
};

int main(void) {
  Fraction f;
  printf("Expermenting with classes in C++.");
}
