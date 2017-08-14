#ifndef TEST_UNIT_MATH_PRIM_MAT_VECTORIZE_EXPECT_BINARY_STD_VECTOR_ERROR_HPP
#define TEST_UNIT_MATH_PRIM_MAT_VECTORIZE_EXPECT_BINARY_STD_VECTOR_ERROR_HPP

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

template <typename F, typename result_t, typename vector_t1,
          typename vector_t2>
void expect_binary_std_vector_err_throw(std::vector<vector_t1> input_v1,
                                        std::vector<vector_t2> input_v2) {
  using std::vector;
  EXPECT_THROW(F::template apply<vector<result_t> >(input_v1, input_v2),
               std::domain_error);
}

template <typename F, typename result_t, typename vector_t1,
          typename vector_t2>
void expect_binary_std_vector_std_vector_err_throw(std::vector<vector_t1>
                                                   input_v1,
                                                   std::vector<vector_t2>
                                                   input_v2) {
  using std::vector;
  vector<vector<vector_t1> > a;
  a.push_back(input_v1);
  a.push_back(input_v1);
  vector<vector<vector_t2> > b;
  b.push_back(input_v2);
  b.push_back(input_v2);
  EXPECT_THROW(F::template apply<vector<vector<result_t> > >(a, b),
               std::domain_error);
}

template <typename F, typename T>
void expect_binary_std_vector_size_err_throw() {

  using std::vector;
  vector<T> badsize_tv1(4);
  vector<T> badsize_tv2(9);
  vector<int> badsize_iv(7);
  vector<double> badsize_dv(5);
  EXPECT_THROW(F::template apply<vector<T> >(badsize_tv1, badsize_iv),
               std::invalid_argument);
  EXPECT_THROW(F::template apply<vector<T> >(badsize_iv, badsize_tv2),
               std::invalid_argument);
  EXPECT_THROW(F::template apply<vector<T> >(badsize_tv1, badsize_dv),
               std::invalid_argument);
  EXPECT_THROW(F::template apply<vector<T> >(badsize_dv, badsize_tv2),
               std::invalid_argument);
  EXPECT_THROW(F::template apply<vector<T> >(badsize_tv1, badsize_tv2),
               std::invalid_argument);

}

template <typename F, typename T>
void expect_binary_std_vector_error() {
  using std::vector;

  expect_binary_std_vector_size_err_throw<F, T>();

  vector<double> invalid_inputs1 = F::invalid_inputs1();
  if (invalid_inputs1.size() == 0) return;
  vector<double> invalid_inputs2 = F::invalid_inputs2();
  vector<int> int_invalid_inputs1 = F::int_invalid_inputs1();
  vector<int> int_invalid_inputs2 = F::int_invalid_inputs2();
  vector<T> y1(invalid_inputs1.begin(), invalid_inputs1.end());
  vector<T> y2(invalid_inputs2.begin(), invalid_inputs2.end());
  expect_binary_std_vector_err_throw<F, T>(y1, int_invalid_inputs2);
  expect_binary_std_vector_err_throw<F, T>(int_invalid_inputs1, y2);
  expect_binary_std_vector_err_throw<F, T>(y1, invalid_inputs2);
  expect_binary_std_vector_err_throw<F, T>(invalid_inputs1, y2);
  expect_binary_std_vector_err_throw<F, T>(y1, y2);

  expect_binary_std_vector_std_vector_err_throw<F, T>(
    y1, int_invalid_inputs2);
  expect_binary_std_vector_std_vector_err_throw<F, T>(
    int_invalid_inputs1, y2);
  expect_binary_std_vector_std_vector_err_throw<F, T>(
    y1, invalid_inputs2);
  expect_binary_std_vector_std_vector_err_throw<F, T>(
    invalid_inputs1, y2);
  expect_binary_std_vector_std_vector_err_throw<F, T>(y1, y2);
}

#endif
