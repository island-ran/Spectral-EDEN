#include <gtest/gtest.h>

#include <mr_dtg_plus/spectral_router.h>

#include <cmath>
#include <cstdint>

namespace DTGPlus {
namespace {

SpectralGraphSnapshot MakeLargeIrregularChain(std::uint32_t node_count) {
  SpectralGraphSnapshot snapshot;
  snapshot.nodes.reserve(node_count);
  snapshot.edges.reserve(node_count - 1U);
  for (std::uint32_t id = 1U; id <= node_count; ++id) {
    snapshot.nodes.emplace_back(1000U + id, id % 7U == 0U);
    if (id > 1U) {
      const double length = 0.7 + 0.03 * static_cast<double>(id % 11U);
      snapshot.edges.emplace_back(999U + id, 1000U + id, length);
    }
  }
  return snapshot;
}

TEST(SpectralIterativeSolverTest,
     AboveDenseRefreshLimitUsesSparseSolverAndMatchesDenseSpectrum) {
  const SpectralGraphSnapshot snapshot = MakeLargeIrregularChain(49U);

  SpectralConfig dense_config;
  dense_config.max_spectral_nodes = 80U;
  dense_config.dense_solver_max_nodes = 80U;
  const SpectralResult dense = SpectralRouter(dense_config).Solve(snapshot);
  ASSERT_TRUE(dense.success()) << dense.diagnostics.reason;
  ASSERT_EQ(dense.diagnostics.solver_type,
            SpectralSolverType::DENSE_SELF_ADJOINT);

  SpectralConfig iterative_config = dense_config;
  iterative_config.dense_solver_max_nodes = 40U;
  iterative_config.iterative_max_iterations = 150U;
  iterative_config.iterative_tolerance = 1.0e-11;
  const SpectralResult iterative =
      SpectralRouter(iterative_config).Solve(snapshot);

  ASSERT_TRUE(iterative.success()) << iterative.diagnostics.reason;
  EXPECT_EQ(iterative.diagnostics.solver_type,
            SpectralSolverType::SPARSE_BLOCK_INVERSE_ITERATION);
  EXPECT_GT(iterative.diagnostics.solver_iterations, 0U);
  EXPECT_LE(iterative.diagnostics.solver_iterations,
            iterative_config.iterative_max_iterations);
  EXPECT_NEAR(iterative.diagnostics.lambda1, dense.diagnostics.lambda1,
              1.0e-10);
  EXPECT_NEAR(iterative.diagnostics.lambda2, dense.diagnostics.lambda2,
              1.0e-9);
  EXPECT_NEAR(iterative.diagnostics.lambda3, dense.diagnostics.lambda3,
              1.0e-9);
  EXPECT_LT(iterative.diagnostics.residual_norm,
            iterative_config.max_residual_norm);
}

TEST(SpectralIterativeSolverTest,
     SolverSelectionHonorsV41DefaultFortyEightAndSixtyFourLimits) {
  const SpectralRouter router;

  const SpectralResult forty_eight =
      router.Solve(MakeLargeIrregularChain(48U));
  ASSERT_TRUE(forty_eight.success())
      << forty_eight.diagnostics.reason;
  EXPECT_EQ(forty_eight.diagnostics.solver_type,
            SpectralSolverType::DENSE_SELF_ADJOINT);

  const SpectralResult forty_nine =
      router.Solve(MakeLargeIrregularChain(49U));
  ASSERT_TRUE(forty_nine.success())
      << forty_nine.diagnostics.reason;
  EXPECT_EQ(forty_nine.diagnostics.solver_type,
            SpectralSolverType::SPARSE_BLOCK_INVERSE_ITERATION);

  const SpectralResult sixty_four =
      router.Solve(MakeLargeIrregularChain(64U));
  ASSERT_TRUE(sixty_four.success())
      << sixty_four.diagnostics.reason;
  EXPECT_EQ(sixty_four.diagnostics.solver_type,
            SpectralSolverType::SPARSE_BLOCK_INVERSE_ITERATION);

  const SpectralResult sixty_five =
      router.Solve(MakeLargeIrregularChain(65U));
  EXPECT_EQ(sixty_five.status, SpectralSolveStatus::TOO_MANY_NODES);
  EXPECT_EQ(sixty_five.diagnostics.solver_type,
            SpectralSolverType::NONE);
}

TEST(SpectralIterativeSolverTest, PreviousFiedlerSeedsIterativeSubspace) {
  SpectralConfig config;
  config.dense_solver_max_nodes = 40U;
  config.max_spectral_nodes = 80U;
  config.iterative_max_iterations = 150U;
  config.iterative_tolerance = 1.0e-11;
  config.max_residual_norm = 1.0e-8;
  const SpectralRouter router(config);
  const SpectralGraphSnapshot snapshot = MakeLargeIrregularChain(48U);

  const SpectralResult first = router.Solve(snapshot);
  ASSERT_TRUE(first.success()) << first.diagnostics.reason;
  EXPECT_FALSE(first.diagnostics.iterative_warm_start_used);

  const SpectralResult warmed = router.Solve(snapshot, first.fiedler_by_h_id);
  ASSERT_TRUE(warmed.success()) << warmed.diagnostics.reason;
  EXPECT_TRUE(warmed.diagnostics.iterative_warm_start_used);
  EXPECT_TRUE(warmed.diagnostics.history_alignment_used);
  EXPECT_NEAR(warmed.diagnostics.lambda2, first.diagnostics.lambda2, 1.0e-10);
  for (const auto& entry : first.fiedler_by_h_id) {
    ASSERT_NE(warmed.fiedler_by_h_id.find(entry.first),
              warmed.fiedler_by_h_id.end());
    EXPECT_NEAR(warmed.fiedler_by_h_id.at(entry.first), entry.second, 1.0e-7);
  }
}

TEST(SpectralIterativeSolverTest, IterationLimitFailsClosed) {
  SpectralConfig config;
  config.dense_solver_max_nodes = 40U;
  config.max_spectral_nodes = 80U;
  config.iterative_max_iterations = 1U;
  config.iterative_tolerance = 1.0e-14;

  const SpectralResult result =
      SpectralRouter(config).Solve(MakeLargeIrregularChain(48U));

  EXPECT_EQ(result.status, SpectralSolveStatus::EIGEN_SOLVER_FAILURE);
  EXPECT_EQ(result.diagnostics.solver_type,
            SpectralSolverType::SPARSE_BLOCK_INVERSE_ITERATION);
  EXPECT_EQ(result.diagnostics.solver_iterations, 1U);
  EXPECT_NE(result.diagnostics.reason.find("did not converge"),
            std::string::npos);
}

}  // namespace
}  // namespace DTGPlus
