// Unit checks for campaign.h. Build with the CMakeLists.txt beside it:
//   cmake -S . -B build && cmake --build build && ./build/test_campaign <poses.csv>
#include "campaign.h"

#include <visp3/core/vpPixelMeterConversion.h>

#include <cstdio>
#include <random>

static int failures = 0;
#define CHECK(cond, msg)                                                                                              \
  do {                                                                                                                \
    if (!(cond)) {                                                                                                    \
      std::printf("FAIL  %s\n", msg);                                                                                 \
      failures++;                                                                                                     \
    } else {                                                                                                          \
      std::printf("ok    %s\n", msg);                                                                                 \
    }                                                                                                                 \
  } while (0)

static double maxAbsDiff(const vpColVector &a, const vpColVector &b)
{
  double m = 0;
  for (unsigned int i = 0; i < a.getRows(); i++) m = std::max(m, std::fabs(a[i] - b[i]));
  return m;
}

int main(int argc, char **argv)
{
  using namespace campaign;
  std::mt19937 gen(7);
  std::normal_distribution<double> nd(0.0, 1.0);

  // 1. Jacobi eigenvalues of Q diag(1..6) Q^T
  {
    vpMatrix A(6, 6), Q(6, 6);
    for (unsigned int i = 0; i < 6; i++)
      for (unsigned int j = 0; j < 6; j++) A[i][j] = nd(gen);
    vpMatrix S = A + A.t();
    vpColVector evals;
    vpMatrix evecs;
    S.eigenValues(evals, evecs); // orthonormal eigenvectors of a random symmetric matrix
    vpMatrix D(6, 6);
    for (unsigned int i = 0; i < 6; i++) D[i][i] = std::pow(10.0, -static_cast<double>(i)); // cond 1e5
    vpMatrix H = evecs * D * evecs.t();
    std::vector<double> ev = symmetricEigenvalues(H);
    bool ok = true;
    for (unsigned int i = 0; i < 6; i++) ok = ok && std::fabs(ev[i] - std::pow(10.0, -5.0 + i)) < 1e-12;
    CHECK(ok, "Jacobi eigenvalues of a 6x6 symmetric matrix with condition number 1e5");
  }

  // Synthetic measurements: C = 4 channels of N positions
  const unsigned int hIn = 22, wIn = 30, N = hIn * wIn, C = 4, P = C * N;
  vpColVector e(P), G(N);
  for (unsigned int j = 0; j < P; j++) e[j] = 10.0 * nd(gen) + (j % 97 == 0 ? 400.0 : 0.0);
  for (unsigned int k = 0; k < N; k++) G[k] = std::fabs(nd(gen)) + 0.05;

  // 2. robust scale equals vpRobust's
  {
    vpColVector w;
    vpRobust r;
    r.MEstimator(vpRobust::TUKEY, e, w);
    CHECK(std::fabs(robustScale(e) - r.getMedianAbsoluteDeviation()) < 1e-9 * r.getMedianAbsoluteDeviation(),
          "robustScale reproduces vpRobust's 1.4826 MAD");
  }

  // 3. V1 and V2
  vpColVector w1, w2;
  {
    Options o;
    o.weighting = W_NONE;
    Weigher(o).compute(e, vpColVector(), w1);
    CHECK(w1.getRows() == P && maxAbsDiff(w1, vpColVector(P, 1.0)) == 0.0, "V1 gives unit weights");
    o.weighting = W_TUKEY;
    Weigher(o).compute(e, vpColVector(), w2);
    vpColVector wr;
    vpRobust r;
    r.MEstimator(vpRobust::TUKEY, e, wr);
    CHECK(maxAbsDiff(w2, wr) == 0.0, "V2 equals vpRobust TUKEY on the raw residual");
  }

  // 4. V3 with a uniform structure measure is inert: equals V2 exactly
  {
    Options o;
    o.weighting = W_HERMITE;
    vpColVector w3;
    Weigher(o).compute(e, vpColVector(N, 3.7), w3);
    CHECK(maxAbsDiff(w3, w2) < 1e-12, "V3 with uniform G reproduces V2 (uniform rescaling is inert)");
  }

  // 5. V3 with varying G differs, and penalises the flat positions
  {
    Options o;
    o.weighting = W_HERMITE;
    vpColVector w3;
    vpColVector Gf = G;
    for (unsigned int k = 0; k < N / 10; k++) Gf[k] = 0.0; // flat region
    Weigher(o).compute(e, Gf, w3);
    double flat = 0, rest = 0;
    for (unsigned int c = 0; c < C; c++)
      for (unsigned int k = 0; k < N; k++) (k < N / 10 ? flat : rest) += w3[c * N + k];
    flat /= C * (N / 10);
    rest /= C * (N - N / 10);
    CHECK(flat < rest, "V3 down-weights positions where G vanishes");
  }

  // 6. V4 never exceeds V2 and applies 1/sqrt(1+Gbar^2)
  {
    Options o;
    o.weighting = W_PENALISED;
    vpColVector w4;
    Weigher(o).compute(e, G, w4);
    double gm = 0;
    for (unsigned int k = 0; k < N; k++) gm += G[k] / N;
    double m = 0;
    for (unsigned int j = 0; j < P; j++) {
      double gb = G[j % N] / gm;
      m = std::max(m, std::fabs(w4[j] - w2[j] / std::sqrt(1 + gb * gb)));
    }
    CHECK(m < 1e-12, "V4 equals the Tukey weight times 1/sqrt(1+Gbar^2)");
  }

  // 7. V5 starts as V3 and tends to V2 as the residual scale contracts
  {
    Options o;
    o.weighting = W_ADAPTIVE;
    Weigher w5(o);
    vpColVector a, b, ref;
    w5.compute(e, G, a);
    CHECK(std::fabs(w5.gamma() - o.gMin) < 1e-12, "V5 floor equals Gmin at the first iteration");
    vpColVector small = e * 1e-3;
    w5.compute(small, G, b);
    Options o2;
    o2.weighting = W_TUKEY;
    Weigher(o2).compute(small, vpColVector(), ref);
    CHECK(w5.gamma() > 90.0 && maxAbsDiff(b, ref) < 1e-2, "V5 approaches V2 when the scale falls 1e3-fold");
    vpColVector c1, c2;
    Weigher(o).compute(e * 1e-3, G, c1);
    Weigher(o).compute(e * 1e3, G, c2);
    CHECK(maxAbsDiff(c1, c2) < 1e-12, "V5 weights are invariant to a global scaling of the residual");
  }

  // 8. Inertness (item A2): weights on rows with l_j = 0 do not change the step
  {
    vpMatrix L(P, 6);
    for (unsigned int j = 0; j < P; j++)
      for (unsigned int d = 0; d < 6; d++) L[j][d] = (j % N < N / 10) ? 0.0 : nd(gen);
    auto step = [&](const vpColVector &w) {
      vpMatrix Lw(P, 6);
      vpColVector ew(P);
      for (unsigned int j = 0; j < P; j++) {
        for (unsigned int d = 0; d < 6; d++) Lw[j][d] = w[j] * L[j][d];
        ew[j] = w[j] * e[j];
      }
      vpMatrix H = Lw.AtA();
      return vpColVector(H.inverseByLU() * Lw.t() * ew);
    };
    vpColVector wa(P, 1.0), wb(P, 1.0);
    for (unsigned int c = 0; c < C; c++)
      for (unsigned int k = 0; k < N / 10; k++) wb[c * N + k] = 0.0; // reject the flat region
    vpColVector va = step(wa), vb = step(wb);
    CHECK(maxAbsDiff(va, vb) < 1e-10 * std::max(1.0, va.frobeniusNorm()),
          "rejecting measurements with zero interaction rows leaves the velocity unchanged");
  }

  // 9. Pose error and pose loading
  {
    vpHomogeneousMatrix cdMo(0, 0, 1.3, 0, 0, 0), cMo(0.01, -0.02, 1.3, 0, 0, 0);
    vpPoseVector pe = poseError(cdMo, cMo);
    CHECK(std::fabs(pe[0] + 0.01) < 1e-12 && std::fabs(pe[1] - 0.02) < 1e-12 && std::fabs(pe[2]) < 1e-12,
          "poseError returns cdMc");
    vpHomogeneousMatrix m0;
    CHECK(loadPose("", 0, m0) && std::fabs(m0[2][3] - 1.10) < 1e-12, "pose 0 is the Chapter 4 pose");
    CHECK(!loadPose("", 5, m0), "a non-zero id without a pose file is an error");
    if (argc > 1) {
      vpHomogeneousMatrix m1;
      CHECK(loadPose(argv[1], 1, m1), "pose 1 is read from the pose file");
      CHECK(!loadPose(argv[1], 9999, m1), "a missing pose id is reported");
    }
  }

  // 10. Pixel-to-metre conventions
  {
    vpCameraParameters cam(870, 870, 160, 120);
    double x, y;
    pixelToMeter(cam, 120, 160, 1, x, y);
    CHECK(std::fabs(x) < 1e-15 && std::fabs(y) < 1e-15, "corrected order: principal point maps to (0, 0)");
    double xv, yv;
    vpPixelMeterConversion::convertPoint(cam, 37.0, 201.0, xv, yv); // u = column 37, v = row 201
    pixelToMeter(cam, 201, 37, 1, x, y);
    CHECK(std::fabs(x - xv) < 1e-15 && std::fabs(y - yv) < 1e-15, "corrected order matches ViSP convertPoint(u, v)");
    pixelToMeter(cam, 201, 37, 0, x, y);
    double ys, xs;
    vpPixelMeterConversion::convertPoint(cam, 201.0, 37.0, ys, xs); // the thesis call
    CHECK(std::fabs(x - xs) < 1e-15 && std::fabs(y - ys) < 1e-15, "thesis order reproduces convertPoint(cam, i, j, y, x)");
  }

  // 11. Logger and maps
  {
    Options o;
    o.weighting = W_HERMITE;
    Logger lg;
    std::string path = "test_log.csv";
    CHECK(lg.open(path, o), "logger opens");
    IterationRecord r;
    r.iter = 1;
    r.poseErr = vpPoseVector(0.001, 0, 0, 0, 0, 0.002);
    r.v = vpColVector(6, 0.1);
    vpMatrix L(P, 6);
    for (unsigned int j = 0; j < P; j++)
      for (unsigned int d = 0; d < 6; d++) L[j][d] = nd(gen);
    lg.weightStats(w2, L.AtA(), C, r);
    lg.write(r);
    lg.weightStats(w1, L.AtA(), C, r);
    CHECK(r.dW > 0, "weight change between iterations is recorded");
    lg.write(r);
    std::ifstream in(path.c_str());
    std::string line;
    int comments = 0, rows = 0;
    while (std::getline(in, line)) (line[0] == '#' ? comments : rows)++;
    CHECK(comments == 8 && rows == 3, "log has 8 metadata lines, a header and 2 rows");

    vpMatrix Lw(P, 6);
    vpColVector ew(P);
    for (unsigned int j = 0; j < P; j++) {
      for (unsigned int d = 0; d < 6; d++) Lw[j][d] = w2[j] * L[j][d];
      ew[j] = w2[j] * e[j];
    }
    saveMaps("test_map", 1, w2, Lw, ew, hIn, wIn);
    std::ifstream f("test_map_infl_1.f32", std::ios::binary | std::ios::ate);
    CHECK(static_cast<long>(f.tellg()) == static_cast<long>(N * sizeof(float)), "influence map has N floats");
  }

  std::printf("%s (%d failure%s)\n", failures ? "FAILED" : "PASSED", failures, failures == 1 ? "" : "s");
  return failures ? 1 : 0;
}
