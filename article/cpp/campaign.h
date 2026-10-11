// =====================================================================
//  campaign.h -- instrumentation and weighting variants for the
//  simulation campaign of the article.
//
//  Header-only, depends on ViSP core only. Include it in the servoing
//  program (tutorial-viewer15.cpp); INTEGRATION.md gives the insertion
//  points. Nothing here changes the control law: the program keeps its
//  own computation of the gain, the damping, the floor and the velocity.
//
//  Measurement layout assumed throughout, as in the Hermite feature:
//  the feature vector has P = C * N entries, C = 4 scale channels stacked
//  as blocks of N = 66000 interior positions, entry c * N + k.
// =====================================================================
#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include <visp3/core/vpCameraParameters.h>
#include <visp3/core/vpColVector.h>
#include <visp3/core/vpHomogeneousMatrix.h>
#include <visp3/core/vpMath.h>
#include <visp3/core/vpMatrix.h>
#include <visp3/core/vpPoseVector.h>
#include <visp3/core/vpRobust.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace campaign
{

// ---------------------------------------------------------------------
//  Weighting variants. Numbering follows the chapter (1-3) and extends it.
// ---------------------------------------------------------------------
enum Weighting {
  W_NONE = 1,      // V1: W = I
  W_TUKEY = 2,     // V2: Tukey on the raw residual
  W_HERMITE = 3,   // V3: Tukey on e / max(Gbar, Gmin)            (Chapter 4)
  W_PENALISED = 4, // V4: Tukey times a structure-penalising design weight
                   //     1 / (1 + Gbar^2), applied so that the normal
                   //     equations carry w_T^2 / (1 + Gbar^2)  (DSO-style)
  W_ADAPTIVE = 5   // V5: Tukey on e / sqrt(Gbar^2 + gamma_k^2),
                   //     gamma_k = Gmin * s_0 / s_k, which tends to V2 as
                   //     the residual scale contracts (heteroscedastic model)
};

enum Interaction {
  L_CURRENT = 0, // interaction matrix from the current image, every iteration
  L_DESIRED = 1  // interaction matrix from the desired image, computed once
};

inline const char *weightingName(int w)
{
  switch (w) {
  case W_NONE: return "none";
  case W_TUKEY: return "tukey";
  case W_HERMITE: return "hermite";
  case W_PENALISED: return "penalised";
  case W_ADAPTIVE: return "adaptive";
  default: return "unknown";
  }
}

struct Options {
  int weighting = W_HERMITE;
  int interaction = L_CURRENT;
  std::string refTexture = "./peppers.jpg";   // reference image I*
  std::string servoTexture = "./peppers.jpg"; // images acquired during servoing
  std::string poseFile;                       // CSV from make_poses.py
  int poseId = 0;                             // 0 = Chapter 4 pose
  std::string logPath;                        // per-iteration CSV
  std::string mapPrefix;                      // weight/influence maps; empty = none
  double gMin = 0.1;                          // floor on Gbar (V3, V5)
  int pixelOrder = 1;                         // 1: x from the column, y from the row (ViSP);
                                              // 0: the thesis programs' swapped call
};

// Normalised coordinates of pixel (row i, column j). ViSP's convertPoint
// takes (u, v) = (column, row). The thesis programs call
// convertPoint(cam, i, j, y, x), which evaluates x = (j - v0)/py and
// y = (i - u0)/px: the principal point (160, 120) is used as (120, 160).
// pixelOrder = 0 reproduces that call, for the reproduction check only.
inline void pixelToMeter(const vpCameraParameters &cam, unsigned int i, unsigned int j, int pixelOrder,
                         double &x, double &y)
{
  if (pixelOrder == 1) {
    x = (j - cam.get_u0()) / cam.get_px();
    y = (i - cam.get_v0()) / cam.get_py();
  }
  else {
    y = (i - cam.get_u0()) / cam.get_px();
    x = (j - cam.get_v0()) / cam.get_py();
  }
}

// Handles the campaign flags inside the getopt switch of the program.
// Returns true if the flag was one of ours. Flags: w L r t P o m x
inline bool parseOption(int c, const char *arg, Options &o)
{
  switch (c) {
  case 'w': o.weighting = std::atoi(arg); return true;
  case 'L': o.interaction = std::atoi(arg); return true;
  case 'r': o.refTexture = arg; return true;
  case 't': o.servoTexture = arg; return true;
  case 'P': o.poseFile = arg; return true;
  case 'o': o.logPath = arg; return true;
  case 'm': o.mapPrefix = arg; return true;
  case 'x': o.pixelOrder = std::atoi(arg); return true;
  default: return false;
  }
}

// ---------------------------------------------------------------------
//  Poses
// ---------------------------------------------------------------------
// Chapter 4 pose: t in metres, theta*u in degrees.
static const double CHAPTER4_POSE[6] = {0.04, -0.04, 1.10, 12.0, -12.0, 6.0};

// Reads pose `id` from the CSV written by make_poses.py
// (pose_id,bin,tx_m,ty_m,tz_m,tux_deg,tuy_deg,tuz_deg,...).
// Without a file only id 0, the Chapter 4 pose, is defined; any other id,
// or an id absent from the file, is an error rather than a silent default.
inline bool loadPose(const std::string &file, int id, vpHomogeneousMatrix &cMo)
{
  double p[6];
  std::copy(CHAPTER4_POSE, CHAPTER4_POSE + 6, p);
  if (file.empty() && id != 0) return false;
  if (!file.empty()) {
    std::ifstream in(file.c_str());
    if (!in) return false;
    std::string line;
    std::getline(in, line); // header
    bool found = false;
    while (std::getline(in, line)) {
      std::stringstream ss(line);
      std::string cell;
      std::vector<double> v;
      while (std::getline(ss, cell, ',')) v.push_back(std::atof(cell.c_str()));
      if (v.size() >= 8 && static_cast<int>(v[0]) == id) {
        for (int i = 0; i < 6; i++) p[i] = v[2 + i];
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  cMo.buildFrom(p[0], p[1], p[2], vpMath::rad(p[3]), vpMath::rad(p[4]), vpMath::rad(p[5]));
  return true;
}

// Pose error as the relative transformation cdMc, which is the quantity
// a pose error should be (the component-wise difference of two pose
// vectors is not a pose). Translation in metres, theta*u in radians.
inline vpPoseVector poseError(const vpHomogeneousMatrix &cdMo, const vpHomogeneousMatrix &cMo)
{
  return vpPoseVector(cdMo * cMo.inverse());
}

// ---------------------------------------------------------------------
//  Small numerics
// ---------------------------------------------------------------------
inline double median(std::vector<double> v)
{
  if (v.empty()) return 0.0;
  size_t n = v.size(), m = (n - 1) / 2;
  std::nth_element(v.begin(), v.begin() + m, v.end());
  return v[m];
}

// 1.4826 * MAD about the median, as vpRobust computes it.
inline double robustScale(const vpColVector &r)
{
  std::vector<double> v(r.data, r.data + r.getRows());
  double med = median(v);
  for (double &x : v) x = std::fabs(x - med);
  return 1.4826 * median(v);
}

// Eigenvalues of a symmetric matrix by the cyclic Jacobi method; used for
// lambda_min(H_W) of the 6 x 6 Hessian, so no third-party solver is needed.
inline std::vector<double> symmetricEigenvalues(const vpMatrix &Ain)
{
  const unsigned int n = Ain.getRows();
  std::vector<double> a(n * n);
  for (unsigned int i = 0; i < n; i++)
    for (unsigned int j = 0; j < n; j++) a[i * n + j] = 0.5 * (Ain[i][j] + Ain[j][i]);
  for (int sweep = 0; sweep < 100; sweep++) {
    double off = 0.0;
    for (unsigned int i = 0; i < n; i++)
      for (unsigned int j = i + 1; j < n; j++) off += a[i * n + j] * a[i * n + j];
    if (off < 1e-30) break;
    for (unsigned int p = 0; p < n; p++)
      for (unsigned int q = p + 1; q < n; q++) {
        double apq = a[p * n + q];
        if (std::fabs(apq) < 1e-300) continue;
        double theta = 0.5 * (a[q * n + q] - a[p * n + p]) / apq;
        double t = (theta >= 0 ? 1.0 : -1.0) / (std::fabs(theta) + std::sqrt(theta * theta + 1.0));
        double c = 1.0 / std::sqrt(t * t + 1.0), s = t * c;
        for (unsigned int k = 0; k < n; k++) { // columns p, q
          double akp = a[k * n + p], akq = a[k * n + q];
          a[k * n + p] = c * akp - s * akq;
          a[k * n + q] = s * akp + c * akq;
        }
        for (unsigned int k = 0; k < n; k++) { // rows p, q
          double apk = a[p * n + k], aqk = a[q * n + k];
          a[p * n + k] = c * apk - s * aqk;
          a[q * n + k] = s * apk + c * aqk;
        }
      }
  }
  std::vector<double> ev(n);
  for (unsigned int i = 0; i < n; i++) ev[i] = a[i * n + i];
  std::sort(ev.begin(), ev.end());
  return ev;
}

// ---------------------------------------------------------------------
//  Weights
// ---------------------------------------------------------------------
class Weigher
{
public:
  explicit Weigher(const Options &o) : opt_(o), s0_(-1.0), gamma_(o.gMin), scale_(0.0) {}

  // e      : feature error, P = C * N entries, channel-blocked
  // structG: structure measure G on the N interior positions (V3-V5);
  //          may be empty for V1 and V2
  // w      : output weights, P entries
  void compute(const vpColVector &e, const vpColVector &structG, vpColVector &w)
  {
    const unsigned int P = e.getRows();
    w.resize(P, false);
    if (opt_.weighting == W_NONE) {
      w = 1.0;
      scale_ = robustScale(e);
      return;
    }
    const bool needG = opt_.weighting >= W_HERMITE;
    const unsigned int N = structG.getRows();
    if (needG && (N == 0 || P % N != 0)) {
      throw vpException(vpException::dimensionError,
                        "campaign::Weigher: structG has %u entries for %u measurements", N, P);
    }
    vpRobust robust;
    if (opt_.weighting == W_TUKEY || opt_.weighting == W_PENALISED) {
      robust.MEstimator(vpRobust::TUKEY, e, w);
      scale_ = robust.getMedianAbsoluteDeviation();
      if (opt_.weighting == W_PENALISED) {
        const double gmean = mean(structG);
        for (unsigned int j = 0; j < P; j++) {
          double gb = structG[j % N] / gmean;
          w[j] *= 1.0 / std::sqrt(1.0 + gb * gb); // normal equations: w_T^2/(1+Gbar^2)
        }
      }
      return;
    }
    // V3 and V5: normalise, then Tukey with the scale of the normalised residual
    const double gmean = mean(structG);
    if (opt_.weighting == W_ADAPTIVE) {
      double s = robustScale(e);
      if (s0_ < 0) s0_ = s;
      gamma_ = opt_.gMin * (s > 0 ? s0_ / s : 1e6);
      gamma_ = std::min(gamma_, 1e6);
    }
    vpColVector en(P);
    for (unsigned int j = 0; j < P; j++) {
      double gb = structG[j % N] / gmean;
      // V5: the constant factor sqrt(1 + gamma^2) is inert (a uniform
      // rescaling, Section 4.4.3) but keeps en on the scale of e, so that
      // vpRobust's absolute floor on the MAD (0.0017) never comes into play.
      double d = (opt_.weighting == W_HERMITE)
                     ? std::max(gb, opt_.gMin)
                     : std::sqrt(gb * gb + gamma_ * gamma_) / std::sqrt(1.0 + gamma_ * gamma_);
      en[j] = e[j] / d;
    }
    robust.MEstimator(vpRobust::TUKEY, en, w);
    scale_ = robust.getMedianAbsoluteDeviation();
  }

  double scale() const { return scale_; }  // MAD-based scale used by the estimator
  double gamma() const { return gamma_; }  // current floor (V5); Gmin otherwise

private:
  static double mean(const vpColVector &v)
  {
    double s = 0.0;
    for (unsigned int i = 0; i < v.getRows(); i++) s += v[i];
    double m = s / std::max(1u, v.getRows());
    return m > 1e-12 ? m : 1e-12;
  }
  Options opt_;
  double s0_, gamma_, scale_;
};

// ---------------------------------------------------------------------
//  Per-iteration log
// ---------------------------------------------------------------------
struct IterationRecord {
  int iter = 0;
  vpPoseVector poseErr;   // cdMc: t (m), theta*u (rad)
  vpColVector v;          // commanded velocity (6)
  double e2 = 0, eI2 = 0; // ||e||^2, ||e_I||^2
  double lambda = 0, mu = 0;
  double hMin = 0, hTrace = 0; // lambda_min and trace of H_W
  double scale = 0, gamma = 0;
  double meanW = 0, fracW01 = 0, fracW25 = 0;
  double fracW25ch[4] = {0, 0, 0, 0};
  double dW = 0;          // ||w_k - w_{k-1}|| / ||w_k||
  double tWeightMs = 0, tIterMs = 0;
};

class Logger
{
public:
  bool open(const std::string &path, const Options &o, const std::string &extraHeader = "")
  {
    if (path.empty()) return false;
    out_.open(path.c_str());
    if (!out_) return false;
    out_ << "# weighting=" << weightingName(o.weighting) << "\n"
         << "# interaction=" << (o.interaction == L_DESIRED ? "desired" : "current") << "\n"
         << "# ref_texture=" << o.refTexture << "\n"
         << "# servo_texture=" << o.servoTexture << "\n"
         << "# pose_file=" << o.poseFile << "\n"
         << "# pose_id=" << o.poseId << "\n"
         << "# gmin=" << o.gMin << "\n"
         << "# pixel_order=" << (o.pixelOrder == 1 ? "corrected" : "thesis") << "\n";
    if (!extraHeader.empty()) out_ << extraHeader;
    out_ << "iter,tx,ty,tz,tux,tuy,tuz,vx,vy,vz,wx,wy,wz,e2,eI2,lambda,mu,h_min,h_trace,"
            "scale,gamma,mean_w,frac_w01,frac_w25,frac_w25_c0,frac_w25_c1,frac_w25_c2,"
            "frac_w25_c3,dW,t_weight_ms,t_iter_ms\n";
    out_.precision(10);
    return true;
  }

  // Fills the weight and Hessian statistics of `r` from w and H_W.
  void weightStats(const vpColVector &w, const vpMatrix &Hw, unsigned int channels, IterationRecord &r)
  {
    const unsigned int P = w.getRows();
    double s = 0, n01 = 0, n25 = 0, d2 = 0, w2 = 0;
    std::vector<double> c25(channels, 0.0);
    const unsigned int N = channels ? P / channels : P;
    for (unsigned int j = 0; j < P; j++) {
      s += w[j];
      if (w[j] <= 0.01) n01++;
      if (w[j] < 0.25) { n25++; if (channels) c25[std::min(j / N, channels - 1)]++; }
      w2 += w[j] * w[j];
      if (prevW_.getRows() == P) d2 += (w[j] - prevW_[j]) * (w[j] - prevW_[j]);
    }
    r.meanW = s / P;
    r.fracW01 = n01 / P;
    r.fracW25 = n25 / P;
    for (unsigned int c = 0; c < channels && c < 4; c++) r.fracW25ch[c] = c25[c] / N;
    r.dW = (prevW_.getRows() == P && w2 > 0) ? std::sqrt(d2 / w2) : 0.0;
    prevW_ = w;
    std::vector<double> ev = symmetricEigenvalues(Hw);
    r.hMin = ev.front();
    r.hTrace = 0;
    for (double x : ev) r.hTrace += x;
  }

  void write(const IterationRecord &r)
  {
    if (!out_) return;
    out_ << r.iter;
    for (unsigned int i = 0; i < 6; i++) out_ << ',' << r.poseErr[i];
    for (unsigned int i = 0; i < 6; i++) out_ << ',' << (r.v.getRows() == 6 ? r.v[i] : 0.0);
    out_ << ',' << r.e2 << ',' << r.eI2 << ',' << r.lambda << ',' << r.mu << ',' << r.hMin << ',' << r.hTrace
         << ',' << r.scale << ',' << r.gamma << ',' << r.meanW << ',' << r.fracW01 << ',' << r.fracW25;
    for (int c = 0; c < 4; c++) out_ << ',' << r.fracW25ch[c];
    out_ << ',' << r.dW << ',' << r.tWeightMs << ',' << r.tIterMs << '\n';
    out_.flush(); // a run that aborts keeps every completed iteration
  }

private:
  std::ofstream out_;
  vpColVector prevW_;
};

// ---------------------------------------------------------------------
//  Maps: mean weight per position, and influence ||w^2 e l|| summed over
//  the channels, written as 8-bit PGM for figures and float32 for analysis.
// ---------------------------------------------------------------------
inline void writePgm(const std::string &path, const std::vector<double> &v, unsigned int h, unsigned int w,
                     double lo, double hi)
{
  std::ofstream f(path.c_str(), std::ios::binary);
  f << "P5\n" << w << ' ' << h << "\n255\n";
  for (double x : v) {
    double g = 255.0 * (x - lo) / (hi - lo);
    unsigned char c = static_cast<unsigned char>(std::max(0.0, std::min(255.0, g)) + 0.5);
    f.put(static_cast<char>(c));
  }
}

inline void writeFloat(const std::string &path, const std::vector<double> &v)
{
  std::ofstream f(path.c_str(), std::ios::binary);
  for (double x : v) {
    float y = static_cast<float>(x);
    f.write(reinterpret_cast<const char *>(&y), sizeof(float));
  }
}

// Lw = W L (rows scaled by w), ew = W e: the program's Lp and error_p.
// hIn x wIn is the interior grid (220 x 300), positions in row-major order.
inline void saveMaps(const std::string &prefix, int iter, const vpColVector &w, const vpMatrix &Lw,
                     const vpColVector &ew, unsigned int hIn, unsigned int wIn)
{
  if (prefix.empty()) return;
  const unsigned int N = hIn * wIn, P = w.getRows(), C = P / N;
  std::vector<double> wm(N, 0.0), infl(N, 0.0);
  for (unsigned int c = 0; c < C; c++)
    for (unsigned int k = 0; k < N; k++) {
      unsigned int j = c * N + k;
      wm[k] += w[j] / C;
      double s = 0.0;
      for (unsigned int d = 0; d < Lw.getCols(); d++) s += Lw[j][d] * Lw[j][d];
      infl[k] += std::sqrt(s) * std::fabs(ew[j]); // ||w^2 e l|| = ||W l|| |W e|
    }
  std::string it = std::to_string(iter);
  writePgm(prefix + "_w_" + it + ".pgm", wm, hIn, wIn, 0.0, 1.0);
  writeFloat(prefix + "_w_" + it + ".f32", wm);
  std::vector<double> li(N);
  double mx = 0.0;
  for (unsigned int k = 0; k < N; k++) mx = std::max(mx, infl[k]);
  for (unsigned int k = 0; k < N; k++) li[k] = std::log10(infl[k] / (mx > 0 ? mx : 1.0) + 1e-6);
  writePgm(prefix + "_infl_" + it + ".pgm", li, hIn, wIn, -6.0, 0.0);
  writeFloat(prefix + "_infl_" + it + ".f32", infl);
}

// Scoped timer in milliseconds.
class Timer
{
public:
  Timer() : t0_(std::chrono::steady_clock::now()) {}
  double ms() const
  {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0_).count();
  }

private:
  std::chrono::steady_clock::time_point t0_;
};

} // namespace campaign

#endif // CAMPAIGN_H
