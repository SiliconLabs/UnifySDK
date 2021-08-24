/* © 2014 Silicon Laboratories Inc.
 */
/**
 * Check that prng has a random value on initalization.
 * 
 */
void test_random_init() {
  uint128_t r1;
  uint128_t r2;
 
  prng_init();
  r1 = prng_rand();
  
  prng_init();
  r2 = prng_rand();

  assert(r1!=r2);
}

/**
 * Check that prng produces random numbers
 */

void test_random_rand() {
  /*TODO should be use a NIST standard check?*/
}
