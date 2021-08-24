#include "config.h"
#include <stdio.h>
int main(int argc, char **argv)
{
  config_add_bool("demo_bool", "Demo boolean value", false);
  config_add_double("demo_double", "Demo double value", 3.14);
  config_add_int("demo_int", "Demo int value", 42);
  config_add_string("demo_string", "Demo string value", "Hello World");
  if (CONFIG_STATUS_OK != config_parse(argc, argv, "1337")) {
    return 1;
  }

  bool demo_bool;
  double demo_double;
  int demo_int;
  const char *demo_string;
  config_get_as_bool("demo_bool", &demo_bool);
  config_get_as_double("demo_double", &demo_double);
  config_get_as_int("demo_int", &demo_int);
  config_get_as_string("demo_string", &demo_string);

  printf("Got:\n"
        "  demo_bool: %d\n"
        "  demo_double: %f\n"
        "  demo_int: %d\n"
        "  demo_string: %s\n", demo_bool, demo_double, demo_int, demo_string);
  return 0;
}
