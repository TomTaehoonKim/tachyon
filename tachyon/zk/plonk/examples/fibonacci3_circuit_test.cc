#include "tachyon/zk/plonk/examples/fibonacci3_circuit.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "tachyon/zk/plonk/examples/circuit_test.h"
#include "tachyon/zk/plonk/halo2/pinned_verifying_key.h"
#include "tachyon/zk/plonk/keys/proving_key.h"
#include "tachyon/zk/plonk/layout/floor_planner/simple_floor_planner.h"

namespace tachyon::zk::plonk::halo2 {

namespace {

constexpr uint8_t kExpectedProof[] = {
    33,  231, 248, 121, 132, 226, 31,  179, 86,  67,  213, 109, 201, 53,  147,
    93,  41,  30,  16,  172, 115, 107, 0,   172, 225, 38,  5,   143, 136, 68,
    3,   47,  222, 110, 120, 246, 167, 202, 30,  64,  192, 113, 187, 20,  209,
    5,   19,  35,  161, 55,  99,  167, 153, 242, 153, 84,  169, 135, 224, 148,
    140, 22,  18,  148, 5,   145, 196, 201, 20,  216, 6,   183, 82,  73,  62,
    87,  36,  44,  76,  29,  120, 103, 124, 253, 87,  100, 218, 122, 245, 84,
    26,  140, 240, 97,  130, 134, 92,  104, 200, 22,  60,  26,  130, 227, 203,
    220, 139, 80,  219, 111, 129, 53,  194, 0,   185, 59,  50,  147, 22,  2,
    25,  58,  255, 70,  194, 103, 39,  5,   143, 199, 14,  63,  41,  37,  111,
    200, 181, 194, 119, 182, 175, 185, 182, 254, 90,  205, 250, 56,  35,  47,
    214, 38,  249, 166, 235, 34,  66,  177, 210, 132, 33,  231, 248, 121, 132,
    226, 31,  179, 86,  67,  213, 109, 201, 53,  147, 93,  41,  30,  16,  172,
    115, 107, 0,   172, 225, 38,  5,   143, 136, 68,  3,   47,  222, 110, 120,
    246, 167, 202, 30,  64,  192, 113, 187, 20,  209, 5,   19,  35,  161, 55,
    99,  167, 153, 242, 153, 84,  169, 135, 224, 148, 140, 22,  18,  148, 5,
    145, 196, 201, 20,  216, 6,   183, 82,  73,  62,  87,  36,  44,  76,  29,
    120, 103, 124, 253, 87,  100, 218, 122, 245, 84,  26,  140, 240, 97,  130,
    134, 92,  104, 200, 22,  60,  26,  130, 227, 203, 220, 139, 80,  219, 111,
    129, 53,  194, 0,   185, 59,  50,  147, 22,  2,   25,  58,  255, 70,  194,
    103, 39,  5,   143, 199, 14,  63,  41,  37,  111, 200, 181, 194, 119, 182,
    175, 185, 182, 254, 90,  205, 250, 56,  35,  47,  214, 38,  249, 166, 235,
    34,  66,  177, 210, 132, 1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   169, 158, 99,  118, 9,   49,  34,  148,
    175, 15,  48,  50,  158, 238, 192, 121, 129, 111, 35,  198, 240, 246, 159,
    196, 103, 149, 207, 12,  223, 180, 245, 157, 130, 0,   103, 220, 171, 217,
    151, 60,  30,  241, 176, 246, 168, 252, 196, 91,  211, 192, 39,  169, 95,
    74,  227, 162, 105, 66,  253, 30,  103, 68,  220, 147, 246, 10,  53,  47,
    157, 64,  10,  241, 80,  81,  142, 177, 3,   100, 79,  137, 34,  210, 16,
    177, 127, 143, 217, 234, 231, 152, 247, 248, 7,   183, 37,  21,  115, 94,
    241, 114, 69,  35,  108, 59,  147, 48,  241, 209, 48,  253, 195, 98,  163,
    86,  2,   242, 247, 40,  255, 139, 223, 248, 71,  107, 143, 125, 199, 43,
    161, 193, 81,  170, 183, 157, 78,  116, 11,  186, 239, 250, 244, 116, 112,
    255, 88,  148, 27,  194, 105, 59,  223, 153, 38,  74,  224, 216, 129, 67,
    35,  8,   189, 85,  55,  24,  84,  79,  78,  95,  78,  100, 246, 214, 36,
    243, 80,  183, 6,   197, 183, 38,  109, 142, 189, 197, 110, 253, 136, 182,
    81,  77,  34,  13,  103, 86,  98,  237, 246, 74,  132, 13,  209, 248, 38,
    178, 227, 16,  167, 146, 70,  201, 194, 251, 202, 156, 255, 230, 58,  100,
    118, 94,  224, 58,  17,  3,   90,  11,  175, 118, 180, 120, 179, 146, 235,
    196, 90,  123, 45,  165, 181, 237, 143, 42,  228, 137, 188, 93,  45,  159,
    100, 240, 189, 115, 162, 81,  171, 15,  115, 94,  241, 114, 69,  35,  108,
    59,  147, 48,  241, 209, 48,  253, 195, 98,  163, 86,  2,   242, 247, 40,
    255, 139, 223, 248, 71,  107, 143, 125, 199, 43,  161, 193, 81,  170, 183,
    157, 78,  116, 11,  186, 239, 250, 244, 116, 112, 255, 88,  148, 27,  194,
    105, 59,  223, 153, 38,  74,  224, 216, 129, 67,  35,  8,   189, 85,  55,
    24,  84,  79,  78,  95,  78,  100, 246, 214, 36,  243, 80,  183, 6,   197,
    183, 38,  109, 142, 189, 197, 110, 253, 136, 182, 81,  77,  34,  13,  103,
    86,  98,  237, 246, 74,  132, 13,  209, 248, 38,  178, 227, 16,  167, 146,
    70,  201, 194, 251, 202, 156, 255, 230, 58,  100, 118, 94,  224, 58,  17,
    3,   90,  11,  175, 118, 180, 120, 179, 146, 235, 196, 90,  123, 45,  165,
    181, 237, 143, 42,  228, 137, 188, 93,  45,  159, 100, 240, 189, 115, 162,
    81,  171, 15,  152, 49,  176, 11,  205, 50,  83,  96,  77,  181, 56,  142,
    42,  36,  138, 246, 55,  247, 141, 105, 239, 78,  64,  63,  205, 200, 253,
    23,  108, 49,  146, 30,  1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   71,  41,  149, 131, 0,   144, 163, 29,
    1,   20,  175, 36,  56,  179, 196, 28,  229, 5,   87,  110, 13,  35,  17,
    220, 159, 123, 169, 98,  107, 125, 182, 172, 71,  41,  149, 131, 0,   144,
    163, 29,  1,   20,  175, 36,  56,  179, 196, 28,  229, 5,   87,  110, 13,
    35,  17,  220, 159, 123, 169, 98,  107, 125, 182, 172};

class Fibonacci3CircuitTest : public CircuitTest {};

}  // namespace

TEST_F(Fibonacci3CircuitTest, Configure) {
  ConstraintSystem<F> constraint_system;
  Fibonacci3Config<F> config =
      Fibonacci3Circuit<F, SimpleFloorPlanner>::Configure(constraint_system);
  EXPECT_EQ(config.selector, Selector::Simple(0));
  EXPECT_EQ(config.a, AdviceColumnKey(0));
  EXPECT_EQ(config.b, AdviceColumnKey(1));
  EXPECT_EQ(config.c, AdviceColumnKey(2));
  EXPECT_EQ(config.a_equals_b.value_inv(), AdviceColumnKey(4));
  EXPECT_EQ(
      *config.a_equals_b.expr(),
      *ExpressionFactory<F>::Sum(
          ExpressionFactory<F>::Constant(F::One()),
          ExpressionFactory<F>::Negated(ExpressionFactory<F>::Product(
              ExpressionFactory<F>::Sum(
                  ExpressionFactory<F>::Advice(
                      AdviceQuery(0, Rotation::Cur(), config.a)),
                  ExpressionFactory<F>::Negated(ExpressionFactory<F>::Advice(
                      AdviceQuery(1, Rotation::Cur(), config.b)))),
              ExpressionFactory<F>::Advice(AdviceQuery(
                  2, Rotation::Cur(), config.a_equals_b.value_inv()))))));
  EXPECT_EQ(config.output, AdviceColumnKey(3));
  EXPECT_EQ(constraint_system.num_fixed_columns(), 0);
  EXPECT_EQ(constraint_system.num_advice_columns(), 5);
  EXPECT_EQ(constraint_system.num_instance_columns(), 0);
  EXPECT_EQ(constraint_system.num_selectors(), 1);
  EXPECT_EQ(constraint_system.num_challenges(), 0);
  std::vector<Phase> expected_advice_column_phases = {
      kFirstPhase, kFirstPhase, kFirstPhase, kFirstPhase, kFirstPhase};
  EXPECT_EQ(constraint_system.advice_column_phases(),
            expected_advice_column_phases);
  EXPECT_TRUE(constraint_system.challenge_phases().empty());
  EXPECT_TRUE(constraint_system.selector_map().empty());
  std::vector<Gate<F>> expected_gates;
  std::vector<std::unique_ptr<Expression<F>>> is_zero_polys;
  {
    std::unique_ptr<Expression<F>> poly = ExpressionFactory<F>::Product(
        ExpressionFactory<F>::Product(
            ExpressionFactory<F>::Selector(Selector::Simple(0)),
            ExpressionFactory<F>::Sum(
                ExpressionFactory<F>::Advice(
                    AdviceQuery(0, Rotation::Cur(), AdviceColumnKey(0))),
                ExpressionFactory<F>::Negated(ExpressionFactory<F>::Advice(
                    AdviceQuery(1, Rotation::Cur(), AdviceColumnKey(1)))))),
        ExpressionFactory<F>::Sum(
            ExpressionFactory<F>::Constant(F::One()),
            ExpressionFactory<F>::Negated(ExpressionFactory<F>::Product(
                ExpressionFactory<F>::Sum(
                    ExpressionFactory<F>::Advice(
                        AdviceQuery(0, Rotation::Cur(), AdviceColumnKey(0))),
                    ExpressionFactory<F>::Negated(ExpressionFactory<F>::Advice(
                        AdviceQuery(1, Rotation::Cur(), AdviceColumnKey(1))))),
                ExpressionFactory<F>::Advice(
                    AdviceQuery(2, Rotation::Cur(), AdviceColumnKey(4)))))));
    is_zero_polys.push_back(std::move(poly));
  }
  expected_gates.push_back(Gate<F>("is_zero", {""}, std::move(is_zero_polys),
                                   {Selector::Simple(0)},
                                   {
                                       {AdviceColumnKey(0), Rotation::Cur()},
                                       {AdviceColumnKey(1), Rotation::Cur()},
                                       {AdviceColumnKey(4), Rotation::Cur()},
                                   }));

  std::vector<std::unique_ptr<Expression<F>>> function_polys;
  function_polys.emplace_back(ExpressionFactory<F>::Product(
      ExpressionFactory<F>::Selector(Selector::Simple(0)),
      ExpressionFactory<F>::Product(
          ExpressionFactory<F>::Sum(
              ExpressionFactory<F>::Constant(F::One()),
              ExpressionFactory<F>::Negated(ExpressionFactory<F>::Product(
                  ExpressionFactory<F>::Sum(
                      ExpressionFactory<F>::Advice(
                          AdviceQuery(0, Rotation::Cur(), AdviceColumnKey(0))),
                      ExpressionFactory<F>::Negated(
                          ExpressionFactory<F>::Advice(AdviceQuery(
                              1, Rotation::Cur(), AdviceColumnKey(1))))),
                  ExpressionFactory<F>::Advice(
                      AdviceQuery(2, Rotation::Cur(), AdviceColumnKey(4)))))),
          ExpressionFactory<F>::Sum(
              ExpressionFactory<F>::Advice(
                  AdviceQuery(4, Rotation::Cur(), AdviceColumnKey(3))),
              ExpressionFactory<F>::Negated(ExpressionFactory<F>::Advice(
                  AdviceQuery(3, Rotation::Cur(), AdviceColumnKey(2))))))));
  function_polys.emplace_back(ExpressionFactory<F>::Product(
      ExpressionFactory<F>::Product(
          ExpressionFactory<F>::Selector(Selector::Simple(0)),
          ExpressionFactory<F>::Sum(
              ExpressionFactory<F>::Constant(F::One()),
              ExpressionFactory<F>::Negated(ExpressionFactory<F>::Sum(
                  ExpressionFactory<F>::Constant(F::One()),
                  ExpressionFactory<F>::Negated(ExpressionFactory<F>::Product(
                      ExpressionFactory<F>::Sum(
                          ExpressionFactory<F>::Advice(AdviceQuery(
                              0, Rotation::Cur(), AdviceColumnKey(0))),
                          ExpressionFactory<F>::Negated(
                              ExpressionFactory<F>::Advice(AdviceQuery(
                                  1, Rotation::Cur(), AdviceColumnKey(1))))),
                      ExpressionFactory<F>::Advice(AdviceQuery(
                          2, Rotation::Cur(), AdviceColumnKey(4))))))))),
      ExpressionFactory<F>::Sum(
          ExpressionFactory<F>::Advice(
              AdviceQuery(4, Rotation::Cur(), AdviceColumnKey(3))),
          ExpressionFactory<F>::Negated(ExpressionFactory<F>::Sum(
              ExpressionFactory<F>::Advice(
                  AdviceQuery(0, Rotation::Cur(), AdviceColumnKey(0))),
              ExpressionFactory<F>::Negated(ExpressionFactory<F>::Advice(
                  AdviceQuery(1, Rotation::Cur(), AdviceColumnKey(1)))))))));
  expected_gates.push_back(Gate<F>("f(a, b, c) = if a == b {c} else {a - b}",
                                   {"", ""}, std::move(function_polys),
                                   {Selector::Simple(0)},
                                   {
                                       {AdviceColumnKey(0), Rotation::Cur()},
                                       {AdviceColumnKey(1), Rotation::Cur()},
                                       {AdviceColumnKey(2), Rotation::Cur()},
                                       {AdviceColumnKey(3), Rotation::Cur()},
                                   }));
  EXPECT_EQ(constraint_system.gates(), expected_gates);
  std::vector<AdviceQueryData> expected_advice_queries = {
      AdviceQueryData(Rotation::Cur(), AdviceColumnKey(0)),
      AdviceQueryData(Rotation::Cur(), AdviceColumnKey(1)),
      AdviceQueryData(Rotation::Cur(), AdviceColumnKey(4)),
      AdviceQueryData(Rotation::Cur(), AdviceColumnKey(2)),
      AdviceQueryData(Rotation::Cur(), AdviceColumnKey(3)),
  };
  EXPECT_EQ(constraint_system.advice_queries(), expected_advice_queries);
  std::vector<RowIndex> expected_num_advice_queries = {1, 1, 1, 1, 1};
  EXPECT_EQ(constraint_system.num_advice_queries(),
            expected_num_advice_queries);
  EXPECT_TRUE(constraint_system.instance_queries().empty());
  EXPECT_TRUE(constraint_system.fixed_queries().empty());
  EXPECT_TRUE(constraint_system.permutation().columns().empty());
  EXPECT_TRUE(constraint_system.lookups().empty());
  EXPECT_TRUE(constraint_system.general_column_annotations().empty());
  EXPECT_FALSE(constraint_system.minimum_degree().has_value());
}

TEST_F(Fibonacci3CircuitTest, Synthesize) {
  size_t n = 16;
  CHECK(prover_->pcs().UnsafeSetup(n, F(2)));
  prover_->set_domain(Domain::Create(n));
  const Domain* domain = prover_->domain();

  ConstraintSystem<F> constraint_system;
  Fibonacci3Config config =
      Fibonacci3Circuit<F, SimpleFloorPlanner>::Configure(constraint_system);
  Assembly<RationalEvals> assembly =
      VerifyingKey<F, Commitment>::CreateAssembly<RationalEvals>(
          domain, constraint_system);

  F a(10);
  F b(12);
  F c(15);
  Fibonacci3Circuit<F, SimpleFloorPlanner> circuit(std::move(a), std::move(b),
                                                   std::move(c));
  typename Fibonacci3Circuit<F, SimpleFloorPlanner>::FloorPlanner floor_planner;
  floor_planner.Synthesize(&assembly, circuit, std::move(config),
                           constraint_system.constants());

  EXPECT_TRUE(assembly.fixed_columns().empty());

  EXPECT_TRUE(assembly.permutation().columns().empty());

  const CycleStore& cycle_store = assembly.permutation().cycle_store();
  EXPECT_TRUE(cycle_store.mapping().IsEmpty());
  EXPECT_TRUE(cycle_store.aux().IsEmpty());
  EXPECT_TRUE(cycle_store.sizes().IsEmpty());

  // clang-format off
  std::vector<std::vector<bool>> expected_selectors = {
      { true, false, false, false, false, false, false, false,
       false, false, false, false, false, false, false, false}};
  // clang-format on
  EXPECT_EQ(assembly.selectors(), expected_selectors);
  EXPECT_EQ(assembly.usable_rows(), base::Range<RowIndex>::Until(10));
}

TEST_F(Fibonacci3CircuitTest, LoadVerifyingKey) {
  size_t n = 16;
  CHECK(prover_->pcs().UnsafeSetup(n, F(2)));
  prover_->set_domain(Domain::Create(n));

  F a(10);
  F b(12);
  F c(15);
  Fibonacci3Circuit<F, SimpleFloorPlanner> circuit(std::move(a), std::move(b),
                                                   std::move(c));

  VerifyingKey<F, Commitment> vkey;
  ASSERT_TRUE(vkey.Load(prover_.get(), circuit));

  EXPECT_TRUE(vkey.permutation_verifying_key().commitments().empty());

  std::vector<Commitment> expected_fixed_commitments;
  {
    std::vector<Point> points = {
        {"0x0006d246b1045b5cf7ef706abdca51d7f88992335199a85f594360f81b3435a9",
         "0x0ce5093fe91a56ef9d54ef73d457aa726be43905202132cd98cb1893cfee96a6"},
    };
    expected_fixed_commitments = CreateCommitments(points);
  }
  EXPECT_EQ(vkey.fixed_commitments(), expected_fixed_commitments);

  F expected_transcript_repr = F::FromHexString(
      "0x015bd699cd46cf1250b4b2b2553645628551f6c76bd3e5cc995a355972340be4");
  EXPECT_EQ(vkey.transcript_repr(), expected_transcript_repr);
}

TEST_F(Fibonacci3CircuitTest, LoadProvingKey) {
  size_t n = 16;
  CHECK(prover_->pcs().UnsafeSetup(n, F(2)));
  prover_->set_domain(Domain::Create(n));

  F a(10);
  F b(12);
  F c(15);
  Fibonacci3Circuit<F, SimpleFloorPlanner> circuit(std::move(a), std::move(b),
                                                   std::move(c));

  for (size_t i = 0; i < 2; ++i) {
    ProvingKey<Poly, Evals, Commitment> pkey;
    bool load_verifying_key = i == 0;
    SCOPED_TRACE(
        absl::Substitute("load_verifying_key: $0", load_verifying_key));
    if (load_verifying_key) {
      VerifyingKey<F, Commitment> vkey;
      ASSERT_TRUE(vkey.Load(prover_.get(), circuit));
      ASSERT_TRUE(
          pkey.LoadWithVerifyingKey(prover_.get(), circuit, std::move(vkey)));
    } else {
      ASSERT_TRUE(pkey.Load(prover_.get(), circuit));
    }

    Poly expected_l_first;
    {
      std::vector<std::string_view> poly = {
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
      };
      expected_l_first = CreatePoly(poly);
    }
    EXPECT_EQ(pkey.l_first(), expected_l_first);

    Poly expected_l_last;
    {
      std::vector<std::string_view> poly = {
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2014447de15a99b6df03833e95f96ae1299c9ec6ff990b6e75fa3b3b04846a57",
          "0x0f1f5883e65f820d14d56342dc92fd12a944d4cbbdce5377b7439bd07108fc9d",
          "0x02b337de1c8c14f22ec9b9e2f96afef3652627366f8170a0a948dad4ac1bd5e8",
          "0x030644e72e131a029b85045b68181585d2833e84879b9709143e1f593f000000",
          "0x105009f4ffd70672d94cc277eb87ed7bfe9749817a206522cde7ba58eb7b95aa",
          "0x2144f5eefad21e1ca37ae273a4ee5b4a7eef137cbbeb1d198c9e59c37ef70364",
          "0x2db11694c4a58b3789868bd388165969c30dc1120a37fff09a991abf43e42a19",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2014447de15a99b6df03833e95f96ae1299c9ec6ff990b6e75fa3b3b04846a57",
          "0x0f1f5883e65f820d14d56342dc92fd12a944d4cbbdce5377b7439bd07108fc9d",
          "0x02b337de1c8c14f22ec9b9e2f96afef3652627366f8170a0a948dad4ac1bd5e8",
          "0x030644e72e131a029b85045b68181585d2833e84879b9709143e1f593f000000",
          "0x105009f4ffd70672d94cc277eb87ed7bfe9749817a206522cde7ba58eb7b95aa",
          "0x2144f5eefad21e1ca37ae273a4ee5b4a7eef137cbbeb1d198c9e59c37ef70364",
          "0x2db11694c4a58b3789868bd388165969c30dc1120a37fff09a991abf43e42a19",
      };
      expected_l_last = CreatePoly(poly);
    }
    EXPECT_EQ(pkey.l_last(), expected_l_last);

    Poly expected_l_active_row;
    {
      std::vector<std::string_view> poly = {
          "0x12259d6b14729c0fa51e1a2470908122ef13771b2da58a367974bc177a000001",
          "0x1a8133201ba2fe22aff30c7fd04fadcb17ceab0715a13371b06d35acd9695b3d",
          "0x0d49c50dd1c3ec703dc7be1c836fd7f62c140afcf284ce19b9a99affac7b95aa",
          "0x117dae38a05bcb8c7c290ee16cec493d73ef8dafa5c61fa4a2efd9a39e63abf4",
          "0x0c19139cb84c680a79505ee7747ae78cd6c196473632bc6ea3057c773208fc9d",
          "0x136156e428a2662bc2fddfd3b39f6475dafecb8699a611f3da6edd22c3479af1",
          "0x2aaad1ad96927134ee0187781ffe43e3f08a828d829c68e7865afb6604e42a19",
          "0x28d7d254c17b7ea40ebc4659996adacebd0d8f52d021284040d407c2f33b896f",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x1ef7221577b1e8f8cebdcb2fcd10296b6d9d43267b395c820757c0fc87681d81",
          "0x1d0dff96b3477fb4437e7ee32de1555b5719604277fd746561bc1be1c5846a57",
          "0x1be193603aacb384678281793fa8f20b949cf5976d9493017ba8a357ee49da57",
          "0x1e3eb107ccbf041a07f5de183cd645c4ac6bd4f8344f861078603a6a3ff70364",
          "0x20080468beb85b16c9f71b3ea2ce10fb6cdc81c346721c888ebc9109900adec6",
          "0x30114169cfaa9b194b94fb3e12d441cabad6d0fa619f4a28d8ecb10f5d1bd5e9",
          "0x2edcc3ce4ec47abd9b83b31a4db9571236223b590c30997fd30ce24f7bf2fdd6",
      };
      expected_l_active_row = CreatePoly(poly);
    }
    EXPECT_EQ(pkey.l_active_row(), expected_l_active_row);

    std::vector<Evals> expected_fixed_columns;
    {
      // clang-format off
      std::vector<std::vector<std::string_view>> evals = {{
          "0x0000000000000000000000000000000000000000000000000000000000000001",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
          "0x0000000000000000000000000000000000000000000000000000000000000000",
      }};
      // clang-format on
      expected_fixed_columns = CreateColumns(evals);
    }
    EXPECT_EQ(pkey.fixed_columns(), expected_fixed_columns);

    std::vector<Poly> expected_fixed_polys;
    {
      // clang-format off
      std::vector<std::vector<std::string_view>> polys = {{
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
          "0x2d5e098bb31e86271ccb415b196942d755b0a9c3f21dd9882fa3d63ab1000001",
      }};
      // clang-format on
      expected_fixed_polys = CreatePolys(polys);
    }
    EXPECT_EQ(pkey.fixed_polys(), expected_fixed_polys);

    EXPECT_TRUE(pkey.permutation_proving_key().permutations().empty());

    EXPECT_TRUE(pkey.permutation_proving_key().polys().empty());
  }
}

TEST_F(Fibonacci3CircuitTest, CreateProof) {
  size_t n = 16;
  CHECK(prover_->pcs().UnsafeSetup(n, F(2)));
  prover_->set_domain(Domain::Create(n));

  F a(10);
  F b(12);
  F c(15);
  Fibonacci3Circuit<F, SimpleFloorPlanner> circuit(std::move(a), std::move(b),
                                                   std::move(c));
  std::vector<Fibonacci3Circuit<F, SimpleFloorPlanner>> circuits = {
      circuit, std::move(circuit)};

  std::vector<Evals> instance_columns;
  std::vector<std::vector<Evals>> instance_columns_vec = {
      instance_columns, std::move(instance_columns)};

  ProvingKey<Poly, Evals, Commitment> pkey;
  ASSERT_TRUE(pkey.Load(prover_.get(), circuit));
  prover_->CreateProof(pkey, std::move(instance_columns_vec), circuits);

  std::vector<uint8_t> proof = prover_->GetWriter()->buffer().owned_buffer();
  std::vector<uint8_t> expected_proof(std::begin(kExpectedProof),
                                      std::end(kExpectedProof));
  EXPECT_THAT(proof, testing::ContainerEq(expected_proof));
}

TEST_F(Fibonacci3CircuitTest, VerifyProof) {
  size_t n = 16;
  CHECK(prover_->pcs().UnsafeSetup(n, F(2)));
  prover_->set_domain(Domain::Create(n));

  F a(10);
  F b(12);
  F c(15);
  Fibonacci3Circuit<F, SimpleFloorPlanner> circuit(std::move(a), std::move(b),
                                                   std::move(c));

  VerifyingKey<F, Commitment> vkey;
  ASSERT_TRUE(vkey.Load(prover_.get(), circuit));

  std::vector<uint8_t> owned_proof(std::begin(kExpectedProof),
                                   std::end(kExpectedProof));
  Verifier<PCS> verifier =
      CreateVerifier(CreateBufferWithProof(absl::MakeSpan(owned_proof)));

  std::vector<Evals> instance_columns;
  std::vector<std::vector<Evals>> instance_columns_vec = {
      instance_columns, std::move(instance_columns)};

  size_t num_circuits = instance_columns_vec.size();
  Proof<F, Commitment> proof;
  F h_eval;
  ASSERT_TRUE(verifier.VerifyProofForTesting(vkey, instance_columns_vec, &proof,
                                             &h_eval));

  std::vector<std::vector<Commitment>> expected_advice_commitments_vec;
  {
    std::vector<Point> points = {
        {"0x2f0344888f0526e1ac006b73ac101e295d9335c96dd54356b31fe28479f8e721",
         "0x03d9c8b82e3fdc536e0dc94e102b13f6f02b68a87483ee77ff9d315b2c7d60de"},
        {"0x1412168c94e087a95499f299a76337a1231305d114bb71c0401ecaa7f6786ede",
         "0x13c23c6582942c390a5a788602da1db2a55a2424d0da56357809ac5b2e441f65"},
        {"0x068261f08c1a54f57ada6457fd7c67781d4c2c24573e4952b706d814c9c49105",
         "0x145e88698efe8890d0ac0148f7323474c9a32b7ee6eca0ad95915643c1bd689d"},
        {"0x052767c246ff3a19021693323bb900c235816fdb508bdccbe3821a3c16c8685c",
         "0x1993a757677c0957fb51ca2cccfcef84c924a13fdf4d91786ce1a1c6f733199c"},
        {"0x04d2b14222eba6f926d62f2338facd5afeb6b9afb677c2b5c86f25293f0ec78f",
         "0x22c64d91d4b0dc41516f1f8c3828a535c7918b49936ac04e5b57e466d0face1f"},
    };
    expected_advice_commitments_vec.push_back(CreateCommitments(points));

    points = {
        {"0x2f0344888f0526e1ac006b73ac101e295d9335c96dd54356b31fe28479f8e721",
         "0x03d9c8b82e3fdc536e0dc94e102b13f6f02b68a87483ee77ff9d315b2c7d60de"},
        {"0x1412168c94e087a95499f299a76337a1231305d114bb71c0401ecaa7f6786ede",
         "0x13c23c6582942c390a5a788602da1db2a55a2424d0da56357809ac5b2e441f65"},
        {"0x068261f08c1a54f57ada6457fd7c67781d4c2c24573e4952b706d814c9c49105",
         "0x145e88698efe8890d0ac0148f7323474c9a32b7ee6eca0ad95915643c1bd689d"},
        {"0x052767c246ff3a19021693323bb900c235816fdb508bdccbe3821a3c16c8685c",
         "0x1993a757677c0957fb51ca2cccfcef84c924a13fdf4d91786ce1a1c6f733199c"},
        {"0x04d2b14222eba6f926d62f2338facd5afeb6b9afb677c2b5c86f25293f0ec78f",
         "0x22c64d91d4b0dc41516f1f8c3828a535c7918b49936ac04e5b57e466d0face1f"},
    };
    expected_advice_commitments_vec.push_back(CreateCommitments(points));
  }
  EXPECT_EQ(proof.advices_commitments_vec, expected_advice_commitments_vec);

  EXPECT_TRUE(proof.challenges.empty());

  F expected_theta = F::FromHexString(
      "0x21041410ba638ce7c204e6cc158153c1d0c6d5e38dbd1a7d2d1c5e8dec7e7a41");
  EXPECT_EQ(proof.theta, expected_theta);

  ASSERT_EQ(proof.lookup_permuted_commitments_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.lookup_permuted_commitments_vec[i].empty());
  }

  F expected_beta = F::FromHexString(
      "0x2050b307c388ef3282c426b2240a553a27185eb64eaaa3de9f526485c465d3f5");
  EXPECT_EQ(proof.beta, expected_beta);

  F expected_gamma = F::FromHexString(
      "0x0e282c5331a800dd692bf42af12eea40e4c67f1cb051cd39ad62b569226e518c");
  EXPECT_EQ(proof.gamma, expected_gamma);

  ASSERT_EQ(proof.permutation_product_commitments_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.permutation_product_commitments_vec[i].empty());
  }

  ASSERT_EQ(proof.lookup_product_commitments_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.lookup_product_commitments_vec[i].empty());
  }

  Commitment expected_vanishing_random_poly_commitment;
  {
    expected_vanishing_random_poly_commitment = CreateCommitment(
        {"0x0000000000000000000000000000000000000000000000000000000000000001",
         "0x0000000000000000000000000000000000000000000000000000000000000002"});
  }
  EXPECT_EQ(proof.vanishing_random_poly_commitment,
            expected_vanishing_random_poly_commitment);

  F expected_y = F::FromHexString(
      "0x2cee61253b2de156f58ed09a43a84ea23caf6425865e16cbbb102d3c598bdaf8");
  EXPECT_EQ(proof.y, expected_y);

  std::vector<Commitment> expected_vanishing_h_poly_commitments;
  {
    std::vector<Point> points = {
        {"0x1df5b4df0ccf9567c49ff6f0c6236f8179c0ee9e32300faf9422310976639ea9",
         "0x129a725f88657cfb30028a65613cae4f585bc89766aedf1c4c1d0993a58eb001"},
        {"0x13dc44671efd4269a2e34a5fa927c0d35bc4fca8f6b0f11e3c97d9abdc670082",
         "0x2bda490d302dbdce632ca03b09fa7aa1a2e7d695e18d944239db8b7c6595e775"},
        {"0x1525b707f8f798e7ead98f7fb110d222894f6403b18e5150f10a409d2f350af6",
         "0x08f27a95b21e1614f7e249bca88f92292c81da46435dad8417cc132c12c5619e"},
    };
    expected_vanishing_h_poly_commitments = CreateCommitments(points);
  }
  EXPECT_EQ(proof.vanishing_h_poly_commitments,
            expected_vanishing_h_poly_commitments);

  F expected_x = F::FromHexString(
      "0x1e84d9712614ed3fb13d41f77541a62860d7badfded3ba24f2bda7ecc12a1434");
  EXPECT_EQ(proof.x, expected_x);

  std::vector<std::vector<F>> expected_advice_evals_vec;
  {
    std::vector<std::string_view> evals = {
        "0x2bc77d8f6b47f8df8bff28f7f20256a362c3fd30d1f130933b6c234572f15e73",
        "0x08234381d8e04a2699df3b69c21b9458ff7074f4faefba0b744e9db7aa51c1a1",
        "0x0d224d51b688fd6ec5bd8e6d26b7c506b750f324d6f6644e5f4e4f54183755bd",
        "0x03113ae05e76643ae6ff9ccafbc2c94692a710e3b226f8d10d844af6ed625667",
        "0x0fab51a273bdf0649f2d5dbc89e42a8fedb5a52d7b5ac4eb92b378b476af0b5a",
    };
    expected_advice_evals_vec.push_back(CreateEvals(evals));

    evals = {
        "0x2bc77d8f6b47f8df8bff28f7f20256a362c3fd30d1f130933b6c234572f15e73",
        "0x08234381d8e04a2699df3b69c21b9458ff7074f4faefba0b744e9db7aa51c1a1",
        "0x0d224d51b688fd6ec5bd8e6d26b7c506b750f324d6f6644e5f4e4f54183755bd",
        "0x03113ae05e76643ae6ff9ccafbc2c94692a710e3b226f8d10d844af6ed625667",
        "0x0fab51a273bdf0649f2d5dbc89e42a8fedb5a52d7b5ac4eb92b378b476af0b5a",
    };
    expected_advice_evals_vec.push_back(CreateEvals(evals));
  }
  EXPECT_EQ(proof.advice_evals_vec, expected_advice_evals_vec);

  std::vector<F> expected_fixed_evals;
  {
    std::vector<std::string_view> evals = {
        "0x1e92316c17fdc8cd3f404eef698df737f68a242a8e38b54d605332cd0bb03198",
    };
    expected_fixed_evals = CreateEvals(evals);
  }
  EXPECT_EQ(proof.fixed_evals, expected_fixed_evals);

  F expected_vanishing_random_eval = F::FromHexString(
      "0x0000000000000000000000000000000000000000000000000000000000000001");
  EXPECT_EQ(proof.vanishing_random_eval, expected_vanishing_random_eval);

  EXPECT_TRUE(proof.common_permutation_evals.empty());

  ASSERT_EQ(proof.permutation_product_evals_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.permutation_product_evals_vec[i].empty());
  }

  ASSERT_EQ(proof.permutation_product_next_evals_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.permutation_product_next_evals_vec[i].empty());
  }

  ASSERT_EQ(proof.permutation_product_last_evals_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.permutation_product_last_evals_vec[i].empty());
  }

  ASSERT_EQ(proof.lookup_product_evals_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.lookup_product_evals_vec[i].empty());
  }

  ASSERT_EQ(proof.lookup_product_next_evals_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.lookup_product_next_evals_vec[i].empty());
  }

  ASSERT_EQ(proof.lookup_permuted_input_evals_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.lookup_permuted_input_evals_vec[i].empty());
  }

  ASSERT_EQ(proof.lookup_permuted_input_inv_evals_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.lookup_permuted_input_inv_evals_vec[i].empty());
  }

  ASSERT_EQ(proof.lookup_permuted_table_evals_vec.size(), num_circuits);
  for (size_t i = 0; i < num_circuits; ++i) {
    EXPECT_TRUE(proof.lookup_permuted_table_evals_vec[i].empty());
  }
}

}  // namespace tachyon::zk::plonk::halo2
