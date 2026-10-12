// =====================================================================
//  herpvs_campaign.cpp -- campaign version of tutorial-viewer15.cpp
//  (Hermite photometric visual servoing with robust weighting, Chapter 4).
//
//  The servoing computation is that of tutorial-viewer15.cpp, line for
//  line: the same Hermite kernels, features, interaction maps, gain and
//  damping law, floor on the damped Hessian and stopping rule. What
//  changes is around it:
//    * no display, no plot, no click: runs unattended;
//    * options select the weighting (V1-V5), the structure measure
//      (summed or first-order kernels), the interaction matrix (current
//      or desired image), the textures, the pose, the pixel convention
//      and the floor;
//    * the feature vector and the interaction matrix are assembled here
//      exactly as the modified vpFeatureLuminance does, so the program
//      builds against an unmodified ViSP (>= 3.3);
//    * the double-image filter missing from ViSP is defined locally, as a
//      correlation like ViSP's unsigned-char filter;
//    * every pass is logged by campaign::Logger.
//
//  Reproduction check (README): with -w 3 -G 0 or -G 1 -x 0, the output
//  equals that of tutorial-viewer15.cpp built against the same ViSP.
// =====================================================================
#include <visp3/core/vpCameraParameters.h>
#include <visp3/core/vpHomogeneousMatrix.h>
#include <visp3/core/vpImage.h>
#include <visp3/core/vpImageFilter.h>
#include <visp3/core/vpMath.h>
#include <visp3/core/vpMatrix.h>
#include <visp3/io/vpImageIo.h>
#include <visp3/io/vpParseArgv.h>
#include <visp3/robot/vpImageSimulator.h>
#include <visp3/robot/vpSimulatorCamera.h>

#include <cmath>
#include <cstdio>
#include <iostream>

#include "campaign.h"

#ifndef M_PI // not defined by <cmath> on MSVC without _USE_MATH_DEFINES
#define M_PI 3.14159265358979323846
#endif

#define GETOPTARGS "cdn:w:L:r:t:P:p:o:m:x:G:F:h"

namespace
{
const double Z = 1.3;
const unsigned int H = 240, W = 320, bord = 10;
const unsigned int hIn = H - 2 * bord, wIn = W - 2 * bord, nbIn = hIn * wIn; // 220 x 300 = 66000
const unsigned int nbCh = 4;                                                    // P = 4 * 66000

vpCameraParameters cam(870, 870, 160, 120);
const double px = cam.get_px(), py = cam.get_py();
const int n_max = 4, m_max = 4, kersize = 9;
const double sigma1 = 0.5, sigma2 = 1.1, sigma3 = 1.8, sigma4 = 2.8;

campaign::Options g_opts;
int g_structMeasure = 1; // 0: summed kernels w_h, w_v, w_d; 1: first-order kernels (Chapter 4)
int g_floor = 1;         // floor on the damped Hessian, eq. (4.5)

vpImage<unsigned char> I(H, W, 0), Id(H, W, 0);
vpMatrix Dnm1(kersize, kersize), Dnm2(kersize, kersize), Dnm3(kersize, kersize), Dnm4(kersize, kersize);
vpMatrix G10s2(kersize, kersize), G01s2(kersize, kersize), G10s3(kersize, kersize), G01s3(kersize, kersize),
    G10s4(kersize, kersize), G01s4(kersize, kersize);

// Response maps of one image: 4 feature maps and 6 x 4 interaction maps,
// in the order (a, h, v, d) = (sigma1, sigma2, sigma3, sigma4).
struct HermiteMaps {
  vpImage<double> w[4];
  vpImage<double> L[6][4]; // vx, vy, vz, wx, wy, wz
};
HermiteMaps cur, des;
vpImage<double> g10_2, g01_2, g10_3, g01_3, g10_4, g01_4;

double hermitePolynomial(int n, double x)
{
  if (n == 0) return 1.0;
  if (n == 1) return 2 * x;
  return 2 * x * hermitePolynomial(n - 1, x) - 2 * (n - 1) * hermitePolynomial(n - 2, x);
}

double dnFunction(int n, double x, double sigma)
{
  double normalizedX = x / sigma;
  double prefactor = (std::pow(-1, n)) / (std::sqrt(std::pow(2, n) * std::tgamma(n + 1) * std::sqrt(M_PI) * sigma));
  return prefactor * hermitePolynomial(n, normalizedX) * std::exp(-(normalizedX * normalizedX) / 2.0);
}

void Hermite()
{
  Dnm1 = 0; Dnm2 = 0; Dnm3 = 0; Dnm4 = 0;
  for (int i = 0; i < kersize; i++)
    for (int j = 0; j < kersize; j++) {
      double x = j - ((kersize - 1) / 2.0);
      double y = ((kersize - 1) / 2.0) - i;
      for (int n = 0; n <= n_max; ++n)
        for (int m = 0; m <= m_max; ++m) {
          Dnm1[i][j] += dnFunction(n, x, sigma1) * dnFunction(m, y, sigma1);
          Dnm2[i][j] += dnFunction(n, x, sigma2) * dnFunction(m, y, sigma2);
          Dnm3[i][j] += dnFunction(n, x, sigma3) * dnFunction(m, y, sigma3);
          Dnm4[i][j] += dnFunction(n, x, sigma4) * dnFunction(m, y, sigma4);
        }
    }
}

void HermiteDerivKernels()
{
  for (int i = 0; i < kersize; i++)
    for (int j = 0; j < kersize; j++) {
      double x = j - ((kersize - 1) / 2.0);
      double y = ((kersize - 1) / 2.0) - i;
      G10s2[i][j] = dnFunction(1, x, sigma2) * dnFunction(0, y, sigma2);
      G01s2[i][j] = dnFunction(0, x, sigma2) * dnFunction(1, y, sigma2);
      G10s3[i][j] = dnFunction(1, x, sigma3) * dnFunction(0, y, sigma3);
      G01s3[i][j] = dnFunction(0, x, sigma3) * dnFunction(1, y, sigma3);
      G10s4[i][j] = dnFunction(1, x, sigma4) * dnFunction(0, y, sigma4);
      G01s4[i][j] = dnFunction(0, x, sigma4) * dnFunction(1, y, sigma4);
    }
}

// Correlation of a double image with M, zero where the kernel does not fit:
// the operation ViSP's filter(unsigned char) performs, for double input.
void filterD(const vpImage<double> &In, vpImage<double> &Out, const vpMatrix &M)
{
  const unsigned int sy = M.getRows(), sx = M.getCols(), hy = sy / 2, hx = sx / 2;
  Out.resize(In.getHeight(), In.getWidth(), 0.0);
  for (unsigned int i = hy; i < In.getHeight() - hy; i++)
    for (unsigned int j = hx; j < In.getWidth() - hx; j++) {
      double acc = 0;
      for (unsigned int a = 0; a < sy; a++)
        for (unsigned int b = 0; b < sx; b++) acc += M[a][b] * In[i - hy + a][j - hx + b];
      Out[i][j] = acc;
    }
}

// Ihermite() / Idhermite() of viewer15, for either image.
void hermiteMaps(const vpImage<unsigned char> &Im, HermiteMaps &M)
{
  vpImage<double> dIx, dIy;
  vpImageFilter::getGradX(Im, dIx);
  vpImageFilter::getGradY(Im, dIy);
  for (unsigned int i = bord; i < Im.getHeight(); i++)
    for (unsigned int j = bord; j < Im.getWidth(); j++) {
      dIx[i][j] = px * dIx[i][j];
      dIy[i][j] = py * dIy[i][j];
    }
  vpImage<double> v[6];
  for (int k = 0; k < 6; k++) v[k].resize(H, W, 0.0);
  for (unsigned int i = bord; i < Im.getHeight() - bord; i++)
    for (unsigned int j = bord; j < Im.getWidth() - bord; j++) {
      double x = 0, y = 0;
      campaign::pixelToMeter(cam, i, j, g_opts.pixelOrder, x, y);
      double gx = dIx[i][j], gy = dIy[i][j];
      v[0][i][j] = gx * (1 / Z);
      v[1][i][j] = gy * (1 / Z);
      v[2][i][j] = (-x * gx - y * gy) * (1 / Z);
      v[3][i][j] = -x * y * gx - (1 + y * y) * gy;
      v[4][i][j] = x * y * gy + (1 + x * x) * gx;
      v[5][i][j] = x * gy - y * gx;
    }
  const vpMatrix *K[4] = {&Dnm1, &Dnm2, &Dnm3, &Dnm4};
  for (int s = 0; s < 4; s++) {
    vpImageFilter::filter(Im, M.w[s], *K[s]);
    for (int k = 0; k < 6; k++) filterD(v[k], M.L[k][s], *K[s]);
  }
}

void firstOrderMaps(const vpImage<unsigned char> &Im)
{
  vpImageFilter::filter(Im, g10_2, G10s2); vpImageFilter::filter(Im, g01_2, G01s2);
  vpImageFilter::filter(Im, g10_3, G10s3); vpImageFilter::filter(Im, g01_3, G01s3);
  vpImageFilter::filter(Im, g10_4, G10s4); vpImageFilter::filter(Im, g01_4, G01s4);
}

// Feature vector and interaction matrix, channel-blocked as the modified
// vpFeatureLuminance builds them: entry s * nbIn + k, k row-major interior.
void feature(const HermiteMaps &M, vpColVector &s)
{
  s.resize(nbCh * nbIn, false);
  unsigned int l = 0;
  for (unsigned int c = 0; c < nbCh; c++)
    for (unsigned int i = bord; i < H - bord; i++)
      for (unsigned int j = bord; j < W - bord; j++) s[l++] = M.w[c][i][j];
}

void interaction(const HermiteMaps &M, vpMatrix &L)
{
  L.resize(nbCh * nbIn, 6, false);
  unsigned int l = 0;
  for (unsigned int c = 0; c < nbCh; c++)
    for (unsigned int i = bord; i < H - bord; i++)
      for (unsigned int j = bord; j < W - bord; j++, l++)
        for (int k = 0; k < 6; k++) L[l][k] = M.L[k][c][i][j];
}

// The structure measure G over the interior positions, eq. (4.11), or its
// summed-kernel predecessor (-G 0).
void structureMeasure(vpColVector &G)
{
  G.resize(nbIn, false);
  for (unsigned int k = 0; k < nbIn; k++) {
    const unsigned int i = bord + k / wIn, j = bord + k % wIn;
    if (g_structMeasure == 0) {
      const double a = cur.w[1][i][j], b = cur.w[2][i][j], c = cur.w[3][i][j];
      G[k] = std::sqrt(a * a + b * b + c * c);
    }
    else {
      G[k] = std::sqrt(g10_2[i][j] * g10_2[i][j] + g01_2[i][j] * g01_2[i][j] + g10_3[i][j] * g10_3[i][j] +
                       g01_3[i][j] * g01_3[i][j] + g10_4[i][j] * g10_4[i][j] + g01_4[i][j] * g01_4[i][j]);
    }
  }
}

void usage(const char *name)
{
  std::printf("Usage: %s [options]\n"
              "  -n N    passes (default 600)\n"
              "  -w 1-5  weighting: none, tukey, hermite, penalised, adaptive (default 3)\n"
              "  -G 0|1  structure measure: summed kernels | first-order kernels (default 1)\n"
              "  -L 0|1  interaction matrix from the current | desired image (default 0)\n"
              "  -x 0|1  pixel convention: thesis (swapped) | ViSP (default 1)\n"
              "  -F 0|1  floor on the damped Hessian off | on (default 1)\n"
              "  -r FILE reference texture (default ./peppers.jpg)\n"
              "  -t FILE servoing texture (default ./peppers.jpg)\n"
              "  -P FILE pose file, -p ID pose id (default 0: Chapter 4 pose)\n"
              "  -o FILE per-pass CSV log, -m PREFIX weight and influence maps\n",
              name);
}
} // namespace

int main(int argc, const char **argv)
{
  int opt_niter = 600;
  const char *optarg_;
  int c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg_)) > 1) {
    switch (c) {
    case 'c': case 'd': break; // accepted for compatibility; nothing is displayed
    case 'n': opt_niter = std::atoi(optarg_); break;
    case 'p': g_opts.poseId = std::atoi(optarg_); break;
    case 'G': g_structMeasure = std::atoi(optarg_); break;
    case 'F': g_floor = std::atoi(optarg_); break;
    case 'h': usage(argv[0]); return 0;
    default:
      if (!campaign::parseOption(c, optarg_, g_opts)) { usage(argv[0]); return 2; }
    }
  }
  if (c == 1 || c == -1) { usage(argv[0]); return 2; }

  try {
    vpImage<unsigned char> Iref, Iservo;
    vpImageIo::read(Iref, g_opts.refTexture);
    vpImageIo::read(Iservo, g_opts.servoTexture);

    vpColVector X[4];
    for (int i = 0; i < 4; i++) X[i].resize(3);
    X[0][0] = -0.3; X[1][0] = 0.3;  X[2][0] = 0.3;   X[3][0] = -0.3;
    X[0][1] = -0.215; X[1][1] = -0.215; X[2][1] = 0.215; X[3][1] = 0.215;
    X[0][2] = 0;    X[1][2] = 0;    X[2][2] = 0;     X[3][2] = 0;

    vpImageSimulator sim;
    sim.setInterpolationType(vpImageSimulator::BILINEAR_INTERPOLATION);
    sim.init(Iref, X); // reference I* from the reference texture
    Hermite();
    HermiteDerivKernels();

    vpHomogeneousMatrix cdMo;
    cdMo[2][3] = Z;
    sim.setCameraPosition(cdMo);
    sim.getImage(I, cam);
    Id = I;
    hermiteMaps(Id, des);

    vpHomogeneousMatrix cMo;
    if (!campaign::loadPose(g_opts.poseFile, g_opts.poseId, cMo)) {
      std::cerr << "pose " << g_opts.poseId << " not found" << std::endl;
      return 3;
    }
    vpHomogeneousMatrix wMo, wMc;
    sim.init(Iservo, X); // images acquired during servoing
    sim.setCameraPosition(cMo);
    I = 0;
    sim.getImage(I, cam);

    vpSimulatorCamera robot;
    robot.setSamplingTime(0.04);
    wMc = wMo * cMo.inverse();
    robot.setPosition(wMc);
    robot.setRobotState(vpRobot::STATE_VELOCITY_CONTROL);

    vpColVector sd, s, error, errorI(H * W), w, error_p, structG, e, v;
    feature(des, sd);
    vpMatrix Lsd, Ldes, Hsd, H6, Lp, diagHsd(6, 6);
    interaction(des, Ldes);

    campaign::Weigher weigher(g_opts);
    campaign::Logger logger;
    std::ostringstream extra;
    extra << "# struct_measure=" << (g_structMeasure ? "first_order" : "summed") << "\n"
          << "# floor=" << g_floor << "\n# passes=" << opt_niter << "\n";
    logger.open(g_opts.logPath, g_opts, extra.str());

    double lambda = 0, mu = 0, normeError = 0, normeErrorI = 0, Ne = 0;
    vpPoseVector errorpose;
    int iter = 1;
    do {
      campaign::Timer ti;
      const int pass = iter++;

      sim.setCameraPosition(cMo);
      I = 0;
      sim.getImage(I, cam);
      hermiteMaps(I, cur);
      if (g_opts.weighting >= campaign::W_HERMITE && g_structMeasure == 1) firstOrderMaps(I);

      feature(cur, s);
      error = s - sd;
      unsigned int k = 0;
      for (unsigned int i = 0; i < H; i++)
        for (unsigned int j = 0; j < W; j++) errorI[k++] = Id[i][j] - I[i][j];
      normeError = error.sumSquare();
      normeErrorI = errorI.sumSquare();
      if (iter == 2) Ne = normeErrorI;

      if (g_opts.interaction == campaign::L_DESIRED) Lsd = Ldes;
      else interaction(cur, Lsd);

      campaign::Timer tw;
      if (g_opts.weighting >= campaign::W_HERMITE) structureMeasure(structG);
      weigher.compute(error, structG, w);
      const double tWeight = tw.ms();

      Lp.resize(Lsd.getRows(), Lsd.getCols(), false);
      error_p.resize(error.getRows(), false);
      for (unsigned int i = 0; i < error.getRows(); i++) {
        for (unsigned int j = 0; j < Lsd.getCols(); j++) Lp[i][j] = w[i] * Lsd[i][j];
        error_p[i] = w[i] * error[i];
      }
      Hsd = Lp.AtA();
      diagHsd.eye(6);
      for (unsigned int i = 0; i < 6; i++) diagHsd[i][i] = Hsd[i][i];

      lambda = std::pow(10, (std::log10(normeErrorI) - 0.8 * std::log10(Ne)));
      mu = std::pow(10, (5 * std::log10(normeErrorI) - 5.2 * std::log10(Ne)));

      if (g_floor) {
        double meanDiag = 0.0;
        for (unsigned int i = 0; i < 6; i++) meanDiag += diagHsd[i][i];
        meanDiag /= 6;
        double muFloor = 1e-6 * meanDiag;
        if (muFloor < 1e-12) muFloor = 1e-12;
        vpMatrix Ifloor(6, 6);
        Ifloor.eye(6);
        H6 = ((mu * diagHsd) + Hsd + muFloor * Ifloor).inverseByLU();
      }
      else {
        H6 = ((mu * diagHsd) + Hsd).inverseByLU();
      }
      e = H6 * Lp.t() * error_p;
      v = -lambda * e;

      robot.setVelocity(vpRobot::CAMERA_FRAME, v);
      wMc = robot.getPosition();
      cMo = wMc.inverse() * wMo;

      // thesis convention: component-wise difference of the pose vectors
      vpPoseVector currentpose(cMo), desiredpose(cdMo);
      for (unsigned int i = 0; i < 6; i++) errorpose[i] = currentpose[i] - desiredpose[i];

      campaign::IterationRecord rec;
      rec.iter = pass;
      rec.poseErr = campaign::poseError(cdMo, cMo);
      for (unsigned int i = 0; i < 3; i++) rec.tTarget[i] = errorpose[i];
      rec.v = v;
      rec.e2 = normeError;
      rec.eI2 = normeErrorI;
      rec.lambda = lambda;
      rec.mu = mu;
      rec.scale = weigher.scale();
      rec.gamma = weigher.gamma();
      logger.weightStats(w, Hsd, nbCh, rec);
      rec.tWeightMs = tWeight;
      rec.tIterMs = ti.ms();
      logger.write(rec);

      if (!g_opts.mapPrefix.empty() && (pass == 1 || pass == opt_niter))
        campaign::saveMaps(g_opts.mapPrefix, pass, w, Lp, error_p, hIn, wIn);
    } while (normeError > 10000 && iter <= opt_niter);

    std::printf("### final pass %d  errorpose(m, rad) %.6g %.6g %.6g %.6g %.6g %.6g  |e|^2 %.6g  lambda %.6g  mu %.6g  "
                "|Tc| %.6g\n",
                iter - 1, errorpose[0], errorpose[1], errorpose[2], errorpose[3], errorpose[4], errorpose[5],
                normeError, lambda, mu, std::sqrt(v.sumSquare()));
    return 0;
  }
  catch (const vpException &ex) {
    std::cerr << "Catch an exception: " << ex << std::endl;
    return 1;
  }
}
