/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "attribute_mapper_built_in_functions.hpp"

namespace ast
{
///////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup attribute_mapper_built_in_functions_definitions UAM built-in functions definitions
 * @ingroup unify_attribute_mapper
 * @brief Built-in functions that can be invoked directly from UAM map files.
 *
 * This submodule provides a list of available functions in UAM synthax files.
 * @{
 */

/**
 * @brief Find the min value of a set of expressions.
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @return The minimum value of the list of expressions.
 *         Returns "undefined" value if any of the expression is undefined
 */
template<typename T>
eval_result_type<T> min_value(std::vector<eval_result_type<T>> &results)
{
  eval_result_type<T> min_value = eval_result_type<T>();

  for (eval_result_type<T> &result: results) {
    // Any undefined value will give an undefined min value.
    if (!result) {
      return eval_result_type<T>();
    }

    // Else check if we have a new min.
    if (!min_value) {
      min_value = result.value();
    } else if (min_value.value() > result.value()) {
      min_value = result.value();
    }
  }
  return min_value;
}

/**
 * @brief Find the max value of a set of expressions.
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @return The maximum value of the list of expressions.
 *         Returns "undefined" value if any of the expression is undefined
 */
template<typename T>
eval_result_type<T> max_value(std::vector<eval_result_type<T>> &results)
{
  eval_result_type<T> max_value = eval_result_type<T>();

  for (eval_result_type<T> &result: results) {
    // Any undefined value will give an undefined min value.
    if (!result) {
      return eval_result_type<T>();
    }

    // Else check if we have a new min.
    if (!max_value) {
      max_value = result.value();
    } else if (max_value.value() < result.value()) {
      max_value = result.value();
    }
  }
  return max_value;
}

/**
 * @brief Find the average value of a set of expressions.
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @return The average value of the list of expressions.
 *         Returns "undefined" value if any of the expression is undefined
 */
template<typename T>
eval_result_type<T> average_value(std::vector<eval_result_type<T>> &results)
{
  eval_result_type<T> average_value = eval_result_type<T>(0);
  unsigned int value_count          = 0;

  for (eval_result_type<T> &result: results) {
    if (!result) {
      return eval_result_type<T>();
    }

    value_count += 1;
    average_value = average_value.value() + result.value();
  }

  if (value_count == 0) {
    // This cannot happen with the grammar(at least 1 expression in a function),
    // but just make sure that we cannot divide by 0 here.
    return eval_result_type<T>();
  } else {
    return (average_value.value() / static_cast<T>(value_count));
  }
}

/**
 * @brief Find the absolute value of the first defined expression.
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @return The absolute value of the dirst defined expressions.
 *         Returns "undefined" value if all of the expression are undefined
 */
template<typename T>
eval_result_type<T> absolute_value(std::vector<eval_result_type<T>> &results)
{
  for (eval_result_type<T> &result: results) {
    // Grab the first defined expression and return its absolute value
    if (result) {
      return result.value() >= 0 ? result.value() : -result.value();
    }
  }

  // No luck finding the absolute value.
  return eval_result_type<T>();
}

/**
 * @brief Checks if all the expressions are defined (i.e. have a result)
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @returns true if all expressions are defined (have a value)
 * @returns false if any expressions is undefined (does not have a value)
 */
template<typename T>
eval_result_type<T> are_all_defined(std::vector<eval_result_type<T>> &results)
{
  for (eval_result_type<T> &result: results) {
    if (!result) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Checks if any of the expressions are defined (i.e. have a result)
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @returns true if all expressions are defined (have a value)
 * @returns false if any expressions is undefined (does not have a value)
 */
template<typename T>
eval_result_type<T> is_any_defined(std::vector<eval_result_type<T>> &results)
{
  for (eval_result_type<T> &result: results) {
    if (result) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Checks if all of the expressions are undefined (i.e. do not have a result)
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @returns true if all expressions are defined (have a value)
 * @returns false if any expressions is undefined (does not have a value)
 */
template<typename T>
eval_result_type<T> are_all_undefined(std::vector<eval_result_type<T>> &results)
{
  for (eval_result_type<T> &result: results) {
    if (result) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Checks if any of the expressions are defined (i.e. have a result)
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @returns true if all expressions are defined (have a value)
 * @returns false if any expressions is undefined (does not have a value)
 */
template<typename T>
eval_result_type<T> is_any_undefined(std::vector<eval_result_type<T>> &results)
{
  for (eval_result_type<T> &result: results) {
    if (!result) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Computes the log10 of the first defined argument
 *
 * @tparam T          Type of the result.
 * @param results     Vector of expressions results.
 * @returns undefined value if if all expressions are undefined (do not have a value)
 * @returns log10 value otherwise
 */
template<typename T>
eval_result_type<T> log10(std::vector<eval_result_type<T>> &results)
{
  for (eval_result_type<T> &result: results) {
    if (result) {
      return std::log10(result.value());
    }
  }

  // No luck finding a defined argument
  return eval_result_type<T>();
}
/** @} end attribute_mapper_built_in_functions_definitions */

///////////////////////////////////////////////////////////////////////////////
// Constants (function map)
///////////////////////////////////////////////////////////////////////////////
template<typename T> const mapper_function_map<T> built_in_functions = {
  {std::string("min_value"), &min_value<T>},
  {std::string("max_value"), &max_value<T>},
  {std::string("average_value"), &average_value<T>},
  {std::string("absolute_value"), &absolute_value<T>},
  {std::string("are_all_defined"), &are_all_defined<T>},
  {std::string("is_any_defined"), &is_any_defined<T>},
  {std::string("are_all_undefined"), &are_all_undefined<T>},
  {std::string("is_any_undefined"), &is_any_undefined<T>},
  {std::string("log10"), &log10<T>},
};

template<typename T> const mapper_function_map<T> &get_built_in_functions()
{
  return built_in_functions<T>;
}

// Declare the following template specializations:
template const mapper_function_map<float> &get_built_in_functions();
template const mapper_function_map<uint32_t> &get_built_in_functions();

}  // namespace ast
