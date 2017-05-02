#include <stdio.h>

class Fraction {

private:
  int numerator;
  int denominator;

public:
  Fraction(int n, int d) {
    numerator = n;
    denominator = d;
  }

  int getDenominator() {
    return denominator;
  }
  int getNumerator() {
    return numerator;
  }

  void setNumerator(int numerator) {
    //this keyword is a pointer of type Fraction. Use member access
    //operator to access members of the pointer type.
    this->numerator = numerator;
  }

  void setDenominator(int d) {
    denominator = d;
  }
};

struct t {
  Fraction f;
};

int main(void) {
  Fraction f = Fraction(2, 5);
  struct t f1 = {f};
  struct t f2 = {f};
  printf("Fraction f is: %d/%d\n", f2.f.getNumerator(), f.getDenominator());
}
