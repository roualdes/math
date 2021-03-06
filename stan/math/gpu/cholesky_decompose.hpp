#ifndef STAN_MATH_GPU_CHOLESKY_DECOMPOSE_HPP
#define STAN_MATH_GPU_CHOLESKY_DECOMPOSE_HPP
#ifdef STAN_OPENCL
#include <stan/math/gpu/opencl_context.hpp>
#include <stan/math/gpu/matrix_gpu.hpp>
#include <stan/math/gpu/kernels/cholesky_decompose.hpp>
#include <stan/math/gpu/multiply.hpp>
#include <stan/math/gpu/multiply_transpose.hpp>
#include <stan/math/gpu/lower_tri_inverse.hpp>
#include <stan/math/gpu/transpose.hpp>
#include <stan/math/gpu/subtract.hpp>
#include <stan/math/gpu/err/check_diagonal_zeros.hpp>
#include <stan/math/gpu/err/check_nan.hpp>
#include <CL/cl.hpp>
#include <algorithm>
#include <cmath>

namespace stan {
namespace math {
/**
 * Return the lower-triangular Cholesky factor (i.e., matrix
 * square root) of the specified square, symmetric matrix.
 * The return value \f$L\f$ will be a lower-traingular matrix such that the
 * original matrix \f$A\f$ is given by
 * <p>\f$A = L \times L^T\f$.
 * The Cholesky decomposition is computed on the GPU. This algorithm is
 * recursive. The matrix is subset into a matrix of size
 *  <code>A.rows() / 4</code>, and if the submatrix size is less than
 * 50 or <code>min_block</code> then the cholesky decomposition on the GPU
 *  is computed using that submatrix. If the submatrix is greater than
 * 50 or <code>min_block</code> then <code>cholesky_decompose</code> is run
 * again on a submatrix with size equal to <code>submat.rows() / 4</code>.
 * Once the Cholesky Decomposition is computed, the full matrix cholesky
 * is created by propogating the cholesky forward as given in the reference
 * report below.
 *
 * For a full guide to how this works
 * see the Cholesy decompostion chapter in the  reference report
 * <a href="https://goo.gl/6kWkJ5"> here</a>.
 * @param A Symmetric matrix on the GPU.
 * @return Square root of matrix on the GPU.
 * @throw std::domain_error if m is not
 *  positive definite (if m has more than 0 elements)
 */
inline matrix_gpu cholesky_decompose(matrix_gpu& A) {
  if (A.rows() == 0)
    return A;
  // Repeats the blocked cholesky decomposition until the size of the remaining
  // submatrix is smaller or equal to the minimum blocks size
  // or a heuristic of 100.
  // The Cholesky GPU algorithm only uses one local block so we need the matrix
  // To be less than the max thread block size.
  if (A.rows() <= opencl_context.tuning_opts().cholesky_min_L11_size) {
    matrix_gpu L(A.rows(), A.cols());
    try {
      opencl_kernels::cholesky_decompose(cl::NDRange(A.rows()),
                                         cl::NDRange(A.rows()), A.buffer(),
                                         L.buffer(), A.rows());
    } catch (const cl::Error& e) {
      check_opencl_error("cholesky_decompose", e);
    }
    return L;
  }
  // NOTE: The code in this section follows the naming conventions
  // in the report linked in the docs.
  const int block
      = std::floor(A.rows() / opencl_context.tuning_opts().cholesky_partition);
  // Subset the top left block of the input A into A_11
  matrix_gpu A_11(block, block);
  A_11.sub_block(A, 0, 0, 0, 0, block, block);
  // The following function either calls the
  // blocked cholesky recursively for the submatrix A_11
  // or calls the kernel  directly if the size of the block is small enough
  matrix_gpu L_11 = stan::math::cholesky_decompose(A_11);
  // Copies L_11 back to the input matrix
  A.sub_block(L_11, 0, 0, 0, 0, block, block);

  const int block_subset = A.rows() - block;
  matrix_gpu A_21(block_subset, block);
  A_21.sub_block(A, block, 0, 0, 0, block_subset, block);
  // computes A_21*((L_11^-1)^T)
  // and copies the resulting submatrix to the lower left hand corner of A
  matrix_gpu L_21 = A_21 * transpose(lower_triangular_inverse(L_11));
  A.sub_block(L_21, 0, 0, block, 0, block_subset, block);
  matrix_gpu A_22(block_subset, block_subset);
  A_22.sub_block(A, block, block, 0, 0, block_subset, block_subset);
  // computes A_22 - L_21*(L_21^T)
  matrix_gpu L_22 = A_22 - multiply_transpose(L_21);
  // copy L_22 into A's lower left hand corner
  matrix_gpu L_rem_11 = stan::math::cholesky_decompose(L_22);
  A.sub_block(L_rem_11, 0, 0, block, block, block_subset, block_subset);
  check_nan("cholesky_decompose_gpu", "Matrix m", A);
  check_diagonal_zeros("cholesky_decompose_gpu", "Matrix m", A);
  return A;
}

}  // namespace math
}  // namespace stan

#endif
#endif
