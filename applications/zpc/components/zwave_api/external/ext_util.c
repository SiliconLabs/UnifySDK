// CLEANUP: Moved from Serialapi.c

/**
 * Return the minimum of two unsigned numbers.
 * Preferring function to macro to avoid double evaluation. No need to performance optimize
 * to preprocessor macro.
 *
 * Cannot find this in standard library,
 */
unsigned int min(unsigned int x, unsigned int y)
{
  if (y < x)
  {
    return y;
  }
  return x;
}
