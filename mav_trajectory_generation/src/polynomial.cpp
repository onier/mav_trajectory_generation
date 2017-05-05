/*
 * Copyright (c) 2016, Markus Achtelik, ASL, ETH Zurich, Switzerland
 * Copyright (c) 2016, Michael Burri, ASL, ETH Zurich, Switzerland
 * Copyright (c) 2016, Helen Oleynikova, ASL, ETH Zurich, Switzerland
 * Copyright (c) 2016, Rik Bähnemann, ASL, ETH Zurich, Switzerland
 * Copyright (c) 2016, Marija Popovic, ASL, ETH Zurich, Switzerland
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mav_trajectory_generation/polynomial.h"

#include <algorithm>

namespace mav_trajectory_generation {

bool Polynomial::findMinMax(double t_1, double t_2, int derivative,
                       const Eigen::VectorXcd& roots_of_derivative, double* min,
                       double* max) {
  // Make sure user input is correct.
  if (t_1 > t_2) {
    const double temp = t_1;
    t_1 = t_2;
    t_2 = temp;
  }

  // Evaluate polynomial at critical points.
  *min = std::numeric_limits<double>::max();
  *max = std::numeric_limits<double>::lowest();
  // Evaluate roots:
  for (size_t i= 0 ; i < roots_of_derivative.size(); i++) {
    // Only real roots are considered as critical points.
    if (roots_of_derivative(i).imag() != 0.0) {
      continue;
    }
    // Do not evaluate points outside the domain.
    if (roots_of_derivative(i).real() < t_1 || roots_of_derivative(i).real() > t_2) {
      continue;
    }
    const double candidate = evaluate(roots_of_derivative(i).real(), derivative);
    *min = std::min(*min, candidate);
    *max = std::max(*max, candidate);
  }
  // Evaluate interval end points:
  const double candidate_t_1 = evaluate(t_1, derivative);
  const double candidate_t_2 = evaluate(t_2, derivative);
  *min = std::min(*min, candidate_t_1);
  *min = std::min(*min, candidate_t_2);
  *max = std::max(*max, candidate_t_1);
  *max = std::max(*max, candidate_t_2);

  return true;
}

bool Polynomial::findMinMax(double t_1, double t_2, int derivative, double* min,
                double* max) {
  Eigen::VectorXcd roots_of_derivative;
  Eigen::VectorXd coeffs = getCoefficients(derivative + 1);
  if (!findRootsJenkinsTraub(coeffs,
                             &roots_of_derivative)) {
    return false;
  }
  return findMinMax(t_1, t_2, derivative, roots_of_derivative, min, max);
}

Eigen::MatrixXd computeBaseCoefficients(int N) {
  Eigen::MatrixXd base_coefficients(N, N);

  base_coefficients.setZero();
  base_coefficients.row(0).setOnes();

  const int DEG = N - 1;
  int order = DEG;
  for (int n = 1; n < N; n++) {
    for (int i = DEG - order; i < N; i++) {
      base_coefficients(n, i) = (order - DEG + i) * base_coefficients(n - 1, i);
    }
    order--;
  }
  return base_coefficients;
}

Eigen::MatrixXd Polynomial::base_coefficients_ =
    computeBaseCoefficients(Polynomial::kMaxN);

}  // namespace mav_trajectory_generation
