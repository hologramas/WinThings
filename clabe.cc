// Checks CLABE number matches the control digit.
// See https://es.wikipedia.org/wiki/CLABE

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stdout, "%s <clabe> : Validate CLABE number.\n", argv[0]);
    return 0;
  }

  const char* clabe = argv[1];
  const int length = strlen(clabe);
  if (length != 18) {
    fprintf(stderr, "Invalid clabe [%s] of %i digits. Should be 10 digits\n", clabe, length);
    return -1;
  }

  const int weigths[3] = { 3, 7, 1 };
  int sum = 0;
  for (int i = 0; i < length - 1; i++) {
    const int digit = clabe[i] - 0x30;
    if (digit < 0 || digit > 9) {
      fprintf(stderr, "Invalid clabe [%s]. Some digits are not 0-9\n", clabe);
      return -1;
    }

    const int weight_position = i % 3;
    const int weight = weigths[weight_position];
    const int product = (digit * weight) % 10;
    sum += product;
  }

  const int expected_control_digit = (10 - (sum % 10)) % 10;
  const int control_digit = clabe[17] - 0x30;
  if (control_digit != expected_control_digit) {
    fprintf(stderr, "Invalid clabe [%s]. Control digit %i does not match computed %i\n",
                    clabe, control_digit, expected_control_digit);
    return -1;
  }

  fprintf(stdout, "Clabe [%s] is valid.\n", clabe);
  return 0;
}
