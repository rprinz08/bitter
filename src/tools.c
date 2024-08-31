#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitter.h"


/**
 * pow_i - Value of x raised to the power of n for integer values only.
 * @param[in] x     base value
 * @param[in] n     to the power of n
 * @return          x^n
 */
int pow_i(int x, int n) {
    int r = 1;
    while(n--)
        r *= x;

    return r;
}


/**
 * rand_in_range - Generate a random value between provided min and max
 * (both including) values.
 * Note: Ensure that 'srand(time(NULL));' is called before using this function.
 * @param[in] min   vinimum value
 * @param[in] max   vaximum value
 * @return          random value between min and max
 */
int rand_in_range(int min, int max) {
    double scale = 1.0 / ((uint64_t)RAND_MAX + 1);
    double range = max - min + 1;
    return min + (int) (rand() * scale * range);
}
