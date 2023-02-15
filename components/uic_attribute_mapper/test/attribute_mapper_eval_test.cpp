/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "unity.h"
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"
#include "datastore.h"
#include "workaround.hpp"
#include "sl_log.h"
using namespace attribute_store;

attribute n1234;
attribute n1235;
attribute n0;
attribute n1;

ast::eval<result_type_t>::result_type eval_simple(std::string expr)
{
  ast::ast_tree ast;

  TEST_ASSERT_TRUE(ast::Parse("scope 0 { d'6 = " + expr + "}", ast));

  ast::eval<result_type_t> eval(attribute::root());
  ast::scope scope = boost::get<ast::scope>(ast[0]);
  ast::reducer reducer;

  ast::print printer(std::cout);
  ast::complexity complexity;

  int complexity_before = complexity(scope.assignments[0].rhs);
  std::cout << "Before reduction: " << complexity_before << std::endl;
  printer(scope.assignments[0].rhs);
  std::cout << std::endl;
  ast::expression reduced = reducer.reduce_expression(scope.assignments[0].rhs);
  int complexity_after    = complexity(reduced);
  std::cout << "After reduction: " << complexity_after << std::endl;
  printer(reduced);

  std::cout << std::endl;

  TEST_ASSERT_LESS_OR_EQUAL(complexity_before, complexity_after);

  return eval(reduced);
}

extern "C" {

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  attribute_store_register_type(42, "Index type", 0, U8_STORAGE_TYPE);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  /*
  *       /   \
  *    1234  1235
  *    /  \
  *  42:0 42:1
  *  /      \
  * 3333    4444
  */
  attribute::root().delete_node();

  n1234 = attribute::root().add_node(1234);
  n1235 = attribute::root().add_node(1235);

  n0 = n1234.add_node(42);
  n1 = n1234.add_node(42);
  n0.set_reported<uint8_t>(0);
  n1.set_reported<uint8_t>(1);

  n0.add_node(3333);
  n1.add_node(4444);

  n1235.add_node(5555);
}

void test_simple_reference()
{
  ast::ast_tree ast;
  ast::eval<result_type_t> ass_evaluator(attribute::root());
  ast::attribute_path_eval path_evaluator(attribute::root());

  TEST_ASSERT_TRUE(ast::Parse("scope 0 { d'1234 = 1 }", ast));
  ast::scope scope    = boost::get<ast::scope>(ast[0]);
  ast::assignment ass = scope.assignments[0];
  TEST_ASSERT_EQUAL(1, ass_evaluator(ass.rhs).value());
  TEST_ASSERT_EQUAL(1234, path_evaluator(ass.lhs.attribute_path).type());
}

void test_decendant_path()
{
  ast::ast_tree ast;
  ast::attribute_path_eval path_evaluator(attribute::root());
  TEST_ASSERT_TRUE(ast::Parse("scope 0 { d'1235.5555 = d'6666 }", ast));
  ast::scope scope    = boost::get<ast::scope>(ast[0]);
  ast::assignment ass = scope.assignments[0];
  TEST_ASSERT_EQUAL(5555, path_evaluator(ass.lhs.attribute_path).type());
}

void test_child_decendant_path()
{
  ast::ast_tree ast;
  ast::attribute_path_eval path_evaluator(attribute::root());
  TEST_ASSERT_TRUE(ast::Parse("scope 0 { d'1234.42[0].3333 = 1}", ast));
  ast::scope scope    = boost::get<ast::scope>(ast[0]);
  ast::assignment ass = scope.assignments[0];

  TEST_ASSERT_EQUAL(3333, path_evaluator(ass.lhs.attribute_path).type());
}

void test_dep_eval()
{
  ast::ast_tree ast;
  ast::dep_eval dep_path_evaluator;
  TEST_ASSERT_TRUE(ast::Parse("scope 0 { r'5555 = r'1234.42[0].3333 }", ast));
  ast::scope scope    = boost::get<ast::scope>(ast[0]);
  ast::assignment ass = scope.assignments[0];

  ast::dep_eval eval;
  eval(ass.rhs);

  std::set<ast::attribute_dependency_t> dependencies(
    eval.get_dependencies().begin(),
    eval.get_dependencies().end());

  TEST_ASSERT_EQUAL(0, dependencies.count(std::make_pair(1234, 'r')));
  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(42, 'r')));
  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(3333, 'r')));
  TEST_ASSERT_EQUAL(2, dependencies.size());
}

void test_dep_eval_complicated_expression()
{
  ast::ast_tree ast;
  ast::dep_eval dep_path_evaluator;
  TEST_ASSERT_TRUE(ast::Parse(
    "scope 0 { r'5555 = if (e'1234) (3+r'34.5) ((d'4.6.7 * r'1) or e'12) }",
    ast));
  ast::scope scope    = boost::get<ast::scope>(ast[0]);
  ast::assignment ass = scope.assignments[0];

  ast::dep_eval eval;
  eval(ass.rhs);

  std::set<ast::attribute_dependency_t> dependencies(
    eval.get_dependencies().begin(),
    eval.get_dependencies().end());

  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(1234, 'e')));
  TEST_ASSERT_EQUAL(0, dependencies.count(std::make_pair(34, 'r')));
  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(5, 'r')));
  TEST_ASSERT_EQUAL(0, dependencies.count(std::make_pair(4, 'd')));
  TEST_ASSERT_EQUAL(0, dependencies.count(std::make_pair(6, 'd')));
  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(7, 'd')));
  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(1, 'r')));
  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(12, 'e')));
  TEST_ASSERT_EQUAL(5, dependencies.size());
}

void test_dep_eval_with_built_in_function()
{
  ast::ast_tree ast;
  ast::dep_eval dep_path_evaluator;
  TEST_ASSERT_TRUE(ast::Parse("scope 0 { r'5555 = if (e'1234) "
                              "fn_min_value(3+r'34.5,0) fn_max_value(e'12) }",
                              ast));
  ast::scope scope    = boost::get<ast::scope>(ast[0]);
  ast::assignment ass = scope.assignments[0];

  ast::dep_eval eval;
  eval(ass.rhs);

  std::set<ast::attribute_dependency_t> dependencies(
    eval.get_dependencies().begin(),
    eval.get_dependencies().end());

  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(1234, 'e')));
  TEST_ASSERT_EQUAL(0, dependencies.count(std::make_pair(34, 'r')));
  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(5, 'r')));
  TEST_ASSERT_EQUAL(0, dependencies.count(std::make_pair(4, 'd')));
  TEST_ASSERT_EQUAL(0, dependencies.count(std::make_pair(3, 'r')));
  TEST_ASSERT_EQUAL(0, dependencies.count(std::make_pair(1, 'r')));
  TEST_ASSERT_EQUAL(1, dependencies.count(std::make_pair(12, 'e')));
  TEST_ASSERT_EQUAL(3, dependencies.size());
}

void test_def()
{
  ast::ast_tree ast;
  ast::eval<result_type_t> ass_evaluator(attribute::root());
  ast::attribute_path_eval path_evaluator(attribute::root());

  TEST_ASSERT_TRUE(ast::Parse("def hello 1 + 1\n"
                              "scope 0 { d'1234 = hello }\n",
                              ast));
  ast::scope scope    = boost::get<ast::scope>(ast[0]);
  ast::assignment ass = scope.assignments[0];
  TEST_ASSERT_EQUAL(2, ass_evaluator(ass.rhs).value());
  TEST_ASSERT_EQUAL(1234, path_evaluator(ass.lhs.attribute_path).type());
}

void test_simple_assignment()
{
  ast::ast_tree ast;

  ast::attribute_path_eval path_evaluator(attribute::root());
  TEST_ASSERT_TRUE(ast::Parse("scope 0 { d'6 = 1 }", ast));

  ast::attribute attr;
  attr.value_type = 'd';
  attr.attribute_path.push_back(ast::operand(6U));  // <-- extra layer

  ast::expression first;  // <-- redundant, expression in expression
  first.first = 1U;

  ast::expression expr;
  expr.first = first;

  ast::assignment ass;
  ass.lhs = attr;
  ass.rhs = expr;

  ast::scope scope = boost::get<ast::scope>(ast[0]);
  TEST_ASSERT_TRUE(ass == scope.assignments[0]);
}

void test_simple_undefined()
{
  TEST_ASSERT_FALSE(eval_simple("undefined"));
}

void test_exists()
{
  auto val = eval_simple("e'1234");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("e'7778888");
  TEST_ASSERT_EQUAL(0, val.value());
}

void test_if()
{
  auto val = eval_simple("if (1) 2 3");
  TEST_ASSERT_EQUAL(2, val.value());

  val = eval_simple("if (0) 2 3");
  TEST_ASSERT_EQUAL(3, val.value());

  val = eval_simple("if (undefined) 2 3");
  TEST_ASSERT_EQUAL(3, val.value());

  val = eval_simple("if ((r'1 or 1000) == 1000) 2 3");
  TEST_ASSERT_EQUAL(2, val.value());
}

void test_min_value_built_in_function()
{
  auto val = eval_simple("fn_min_value( 3 , 4 )");
  TEST_ASSERT_EQUAL(3, val.value());

  val = eval_simple("fn_min_value(3,4,6,7,8,-3.6f)");
  TEST_ASSERT_EQUAL(-3.6, val.value());

  val = eval_simple("fn_min_value(3,4,undefined)");
  TEST_ASSERT_FALSE(val);
}

void test_max_value_built_in_function()
{
  auto val = eval_simple("fn_max_value( 3 , 4 )");
  TEST_ASSERT_EQUAL(4, val.value());

  val = eval_simple("fn_max_value( 0 ,4,6,7,8,-3.6f)");
  TEST_ASSERT_EQUAL(8, val.value());

  val = eval_simple("fn_max_value(3,4,undefined)");
  TEST_ASSERT_FALSE(val);
}

void test_average_value_built_in_function()
{
  auto val = eval_simple("fn_average_value( 3 , 4 )");
  TEST_ASSERT_EQUAL(3.5, val.value());

  val = eval_simple("fn_average_value( 0 ,4,6+2,7,8,-3.6f)");
  TEST_ASSERT_EQUAL_FLOAT(3.9, val.value());

  val = eval_simple("fn_average_value(3,4,undefined)");
  TEST_ASSERT_FALSE(val);
}

void test_is_all_defined_built_in_function()
{
  auto val = eval_simple("fn_are_all_defined( 3 , 4, undefined )");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("fn_are_all_defined( 3 , 4, 5 )");
  TEST_ASSERT_EQUAL(1, val.value());
}

void test_are_all_undefined_built_in_function()
{
  auto val = eval_simple(
    "fn_are_all_undefined( 3 * undefined, undefined, undefined )");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("fn_are_all_undefined( undefined, 4, 5 )");
  TEST_ASSERT_EQUAL(0, val.value());
}

void test_is_any_defined_built_in_function()
{
  auto val
    = eval_simple("fn_is_any_defined( 3 * undefined, undefined, undefined )");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("fn_is_any_defined( undefined, 4, 5 )");
  TEST_ASSERT_EQUAL(1, val.value());
}

void test_are_all_defined_built_in_function()
{
  auto val = eval_simple("fn_are_all_defined( 3 * 4, 5, 6)");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("fn_are_all_defined( undefined, 4, 5 )");
  TEST_ASSERT_EQUAL(0, val.value());
}

void test_is_any_undefined_built_in_function()
{
  auto val
    = eval_simple("fn_is_any_undefined( 3 * undefined, undefined, undefined )");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("fn_is_any_undefined( undefined, 4, 5 )");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("fn_is_any_undefined( 4, 4, 5 )");
  TEST_ASSERT_EQUAL(0, val.value());
}

void test_absolute_value_built_in_function()
{
  auto val = eval_simple("fn_absolute_value( 3 * undefined, undefined, -6 )");
  TEST_ASSERT_EQUAL(6, val.value());

  val = eval_simple("fn_absolute_value( 0, 4, 5 )");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("fn_absolute_value( 4.9f )");
  TEST_ASSERT_EQUAL(4.9, val.value());
}

void test_encapsulated_built_in_function()
{
  auto val
    = eval_simple("fn_min_value(fn_absolute_value(-5),fn_max_value(-2, -1))");
  TEST_ASSERT_EQUAL(-1, val.value());

  val = eval_simple("fn_absolute_value(fn_are_all_undefined(undefined, 0))");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple(
    "fn_is_any_undefined(fn_is_any_undefined(undefined, 0),undefined)");
  TEST_ASSERT_EQUAL(1, val.value());
}

void test_execute_unknown_function()
{
  // The mapper engine normally filters out unknown functions, but let's
  // say one slipped in the AST anyway.
  ast::ast_tree ast;
  TEST_ASSERT_TRUE(
    ast::Parse("scope 0 { d'6 = fn_unknown_function(r'1)}", ast));

  ast::scope scope = boost::get<ast::scope>(ast[0]);
  ast::eval<result_type_t> evaluator(attribute::root());
  auto value = evaluator(scope.assignments[0].rhs);
  TEST_ASSERT_FALSE(value);
}

void test_option()
{
  auto val = eval_simple(" undefined or 42");
  TEST_ASSERT_EQUAL(42, val.value());

  val = eval_simple(" 55 or 42");
  TEST_ASSERT_EQUAL(55, val.value());

  val = eval_simple(" 55 or undefined");
  TEST_ASSERT_EQUAL(55, val.value());

  val = eval_simple("r'1 or d'1");
  TEST_ASSERT_FALSE(val);
}

void test_operators()
{
  auto val = eval_simple(" 1 == 1");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple(" (2/3) == 1");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple(" (3/4) == 0.75f");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple(" 2 == 1");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple(" (1/3) == 1");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple(" 1 + 1");
  TEST_ASSERT_EQUAL(2, val.value());

  val = eval_simple(" 10 - 3");
  TEST_ASSERT_EQUAL(7, val.value());

  val = eval_simple("3*2");
  TEST_ASSERT_EQUAL(6, val.value());

  val = eval_simple("10 / 2");
  TEST_ASSERT_EQUAL(5, val.value());

  val = eval_simple("1 / 3 * 3");
  TEST_ASSERT_EQUAL_FLOAT(1, val.value());

  val = eval_simple("9/2");
  TEST_ASSERT_EQUAL_FLOAT(4.5, val.value());

  val = eval_simple("(((1 / 30) + (3 * 234)) - 4) * 3");
  TEST_ASSERT_EQUAL_FLOAT(2094.1, val.value());

  val = eval_simple("1/3*5+2/3*6");
  TEST_ASSERT_EQUAL_FLOAT(17.0 / 3, val.value());

  val = eval_simple("10 % 100");
  TEST_ASSERT_EQUAL(10, val.value());

  val = eval_simple("36 % 5");
  TEST_ASSERT_EQUAL(1, val.value());

  // Modulo between float numbers
  val = eval_simple("9.7f % 2.3f");
  TEST_ASSERT_EQUAL(0.5, val.value());

  val = eval_simple("(36 + 2/5) % (2+1/10)");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("(10 ** 5)");
  TEST_ASSERT_EQUAL(100000, val.value());

  val = eval_simple("(10 ** 0)");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("(25 ** (1/2))");  // Square root my friend!
  TEST_ASSERT_EQUAL(5, val.value());

  val = eval_simple("10 < 100");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("10 > 100");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("10 != 100");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("100 != 100");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("0.4f != (2/5)");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("2 <= 2");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("2 <= 3");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("3 <= 2");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("2 >= 3");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple("2 >= 2");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple("4 >= 3");
  TEST_ASSERT_EQUAL(1, val.value());
}

void test_comparison_operator()
{
  auto val = eval_simple(" 1 == 2");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple(" 1 == (1 + 1)");
  TEST_ASSERT_EQUAL(0, val.value());

  val = eval_simple(" 5 == (2 + 3)");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple(" 2 + 3 == 5 ");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple(" 2 | 3 == 3 ");
  TEST_ASSERT_EQUAL(1, val.value());

  val = eval_simple(" 5 ^ 2 == 7 ");
  TEST_ASSERT_EQUAL(1, val.value());

  // The beauty of floating points :-)
  val = eval_simple(" (1.1f + 2.2f) == 3.30000019f");
  TEST_ASSERT_EQUAL(1, val.value());
}
}
