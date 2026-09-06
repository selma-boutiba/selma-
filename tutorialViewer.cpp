#include <iostream>
#include <visp3/core/vpDebug.h>
#include <visp3/core/vpConfig.h>
#include <visp3/core/vpImage.h>
#include <visp3/io/vpImageIo.h>
#include <visp3/core/vpImageTools.h>
#include <visp3/core/vpImageFilter.h>

#include <visp3/core/vpCameraParameters.h>
#include <visp3/core/vpTime.h>
#include <visp3/robot/vpSimulatorCamera.h>

#include <visp3/core/vpMath.h>
#include <visp3/core/vpHomogeneousMatrix.h>
#include <visp3/gui/vpDisplayGTK.h>
#include <visp3/gui/vpDisplayGDI.h>
#include <visp3/gui/vpDisplayOpenCV.h>
#include <visp3/gui/vpDisplayD3D.h>
#include <visp3/gui/vpDisplayX.h>


#include <visp3/visual_features/vpFeatureLuminance.h>
#include <visp3/io/vpParseArgv.h>
#include <visp3/core/vpPixelMeterConversion.h>
#include <visp3/core/vpRobust.h>

#include <visp3/robot/vpImageSimulator.h>
#include <stdlib.h>
#define  Z             1.3

#include <visp3/io/vpParseArgv.h>
#include <visp3/core/vpIoTools.h>
#include <visp3/gui/vpPlot.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <visp3/core/vpMatrix.h>


// List of allowed command line options
#define GETOPTARGS  "cdi:n:w:h"

// Ablation mode: which weighting matrix D the control law uses.
// W_NONE   -> D = I            (Chapter 3 / HER-PVS baseline, exactly)
// W_TUKEY  -> D = D^T          (plain residual-based Tukey M-estimator)
// W_HERMITE-> D = D^H          (Hermite-informed structure-aware weighting)
enum WeightMode { W_NONE = 1, W_TUKEY = 2, W_HERMITE = 3 };

void usage(const char *name, const char *badparam, std::string ipath, int niter);
bool getOptions(int argc, const char **argv, std::string &ipath,
	bool &click_allowed, bool &display, int &niter, int &wmode);

void usage(const char *name, const char *badparam, std::string ipath, int niter)
{
	fprintf(stdout, "\n\
    Tracking of Surf key-points.\n\
    \n\
    SYNOPSIS\n\
      %s [-i <input image path>] [-c] [-d] [-n <number of iterations>] [-h]\n", name);

	fprintf(stdout, "\n\
   97 OPTIONS:                                               Default\n\
   98   -i <input image path>                                %s\n\
   99      Set image input path.\n\
  100      From this path read \"ViSP-images/doisneau/doisneau.jpg\"\n\
  101      images. \n\
  102      Setting the VISP_INPUT_IMAGE_PATH environment\n\
  103      variable produces the same behaviour than using\n\
  104      this option.\n\
  105 \n\
  106   -c\n\
  107      Disable the mouse click. Useful to automaze the \n\
  108      execution of this program without humain intervention.\n\
  109 \n\
  110   -d \n\
  111      Turn off the display.\n\
  112 \n\
  113   -n %%d                                               %d\n\
  114      Number of iterations.\n\
  115 \n\
  116   -w <1|2|3>                                            3\n\
  117      Ablation weighting mode: 1 = D=I (no weighting, Ch.3 baseline),\n\
  118      2 = plain Tukey, 3 = Hermite-informed Tukey.\n\
  119 \n\
  120   -h\n\
  121      Print the help.\n",
		ipath.c_str(), niter);

	if (badparam)
		fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);
}
bool getOptions(int argc, const char **argv, std::string &ipath,
	bool &click_allowed, bool &display, int &niter, int &wmode)
{
	const char *optarg_;
	int   c;
	while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg_)) > 1) {

		switch (c) {
		case 'c': click_allowed = false; break;
		case 'd': display = false; break;
		case 'i': ipath = optarg_; break;
		case 'n': niter = atoi(optarg_); break;
		case 'w': wmode = atoi(optarg_); break;
		case 'h': usage(argv[0], NULL, ipath, niter); return false; break;

		default:
			usage(argv[0], optarg_, ipath, niter);
			return false; break;
		}
	}

	if ((c == 1) || (c == -1)) {
		// standalone param or error
		usage(argv[0], NULL, ipath, niter);
		std::cerr << "ERROR: " << std::endl;
		std::cerr << "  Bad argument " << optarg_ << std::endl << std::endl;
		return false;
	}

	return true;
}

void init(WeightMode wmode);
void Hermite();
void Ihermite();
void Idhermite();
void computeHermiteNormalizedResidual(const vpColVector &error, vpColVector &structG, vpColVector &error_n);
void init_visp_plot(vpPlot&);

int main(int argc, const char ** argv)
{
	try {
		std::string env_ipath;
		std::string opt_ipath;
		std::string ipath;
		std::string filename;
		bool opt_click_allowed = true;
		bool opt_display = true;
		int opt_niter = 400;
		int opt_wmode = W_HERMITE; // default: reproduces the previously shipped behavior

		// Get the visp-images-data package path or VISP_INPUT_IMAGE_PATH environment variable value
		env_ipath = vpIoTools::getViSPImagesDataPath();

		// Set the default input path
		if (!env_ipath.empty())
			ipath = env_ipath;

		// Read the command line options
		if (getOptions(argc, argv, opt_ipath, opt_click_allowed,
			opt_display, opt_niter, opt_wmode) == false) {
			return (-1);
		}
		if (opt_wmode != W_NONE && opt_wmode != W_TUKEY && opt_wmode != W_HERMITE) {
			std::cerr << "ERROR: -w must be 1 (none), 2 (Tukey) or 3 (Hermite)" << std::endl;
			return (-1);
		}

		// Get the option values
		if (!opt_ipath.empty())
			ipath = opt_ipath;

		// Compare ipath and env_ipath. If they differ, we take into account
		// the input path comming from the command line option
		if (!opt_ipath.empty() && !env_ipath.empty()) {
			if (ipath != env_ipath) {
				std::cout << std::endl
					<< "WARNING: " << std::endl;
				std::cout << "  Since -i <visp image path=" << ipath << "> "
					<< "  is different from VISP_IMAGE_PATH=" << env_ipath << std::endl
					<< "  we skip the environment variable." << std::endl;
			}
		}

		// Test if an input path is set
		if (opt_ipath.empty() && env_ipath.empty()) {
			usage(argv[0], NULL, ipath, opt_niter);
			std::cerr << std::endl
				<< "ERROR:" << std::endl;
			std::cerr << "  Use -i <visp image path> option or set VISP_INPUT_IMAGE_PATH "
				<< std::endl
				<< "  environment variable to specify the location of the " << std::endl
				<< "  image path where test images are located." << std::endl << std::endl;
			exit(-1);
		}

		init((WeightMode)opt_wmode);

		return 0;
	}
	catch (vpException e) {
		std::cout << "Catch an exception: " << e << std::endl;
		return 1;
	}
}

vpImage<unsigned char> I(240, 320, 0);
vpImage<unsigned char> Id(240, 320, 0);
vpImage<double> dIx;
vpImage<double> dIy;
vpImage<double> vvx(240, 320, 0); vpImage<double> vvy(240, 320, 0); vpImage<double> vvz(240, 320, 0); vpImage<double> vwx(240, 320, 0); vpImage<double> vwy(240, 320, 0); vpImage<double> vwz(240, 320, 0);
vpImage<double> w_a(240, 320, 0); vpImage<double> w_h(240, 320, 0); vpImage<double> w_v(240, 320, 0); vpImage<double> w_d(240, 320, 0);
vpImage<double> wvx_a(240, 320, 0); vpImage<double> wvx_h(240, 320, 0); vpImage<double> wvx_v(240, 320, 0); vpImage<double> wvx_d(240, 320, 0);
vpImage<double> wvy_a(240, 320, 0); vpImage<double> wvy_h(240, 320, 0); vpImage<double> wvy_v(240, 320, 0); vpImage<double> wvy_d(240, 320, 0);
vpImage<double> wvz_a(240, 320, 0); vpImage<double> wvz_h(240, 320, 0); vpImage<double> wvz_v(240, 320, 0); vpImage<double> wvz_d(240, 320, 0);
vpImage<double> wwx_a(240, 320, 0); vpImage<double> wwx_h(240, 320, 0); vpImage<double> wwx_v(240, 320, 0); vpImage<double> wwx_d(240, 320, 0);
vpImage<double> wwy_a(240, 320, 0); vpImage<double> wwy_h(240, 320, 0); vpImage<double> wwy_v(240, 320, 0); vpImage<double> wwy_d(240, 320, 0);
vpImage<double> wwz_a(240, 320, 0); vpImage<double> wwz_h(240, 320, 0); vpImage<double> wwz_v(240, 320, 0); vpImage<double> wwz_d(240, 320, 0);
vpImage<double> ddIx;
vpImage<double> ddIy;
vpImage<double> dvvx(240, 320, 0); vpImage<double> dvvy(240, 320, 0); vpImage<double> dvvz(240, 320, 0); vpImage<double> dvwx(240, 320, 0); vpImage<double> dvwy(240, 320, 0); vpImage<double> dvwz(240, 320, 0);
vpImage<double> dw_a(240, 320, 0); vpImage<double> dw_h(240, 320, 0); vpImage<double> dw_v(240, 320, 0); vpImage<double> dw_d(240, 320, 0);
vpImage<double> dwvx_a(240, 320, 0); vpImage<double> dwvx_h(240, 320, 0); vpImage<double> dwvx_v(240, 320, 0); vpImage<double> dwvx_d(240, 320, 0);
vpImage<double> dwvy_a(240, 320, 0); vpImage<double> dwvy_h(240, 320, 0); vpImage<double> dwvy_v(240, 320, 0); vpImage<double> dwvy_d(240, 320, 0);
vpImage<double> dwvz_a(240, 320, 0); vpImage<double> dwvz_h(240, 320, 0); vpImage<double> dwvz_v(240, 320, 0); vpImage<double> dwvz_d(240, 320, 0);
vpImage<double> dwwx_a(240, 320, 0); vpImage<double> dwwx_h(240, 320, 0); vpImage<double> dwwx_v(240, 320, 0); vpImage<double> dwwx_d(240, 320, 0);
vpImage<double> dwwy_a(240, 320, 0); vpImage<double> dwwy_h(240, 320, 0); vpImage<double> dwwy_v(240, 320, 0); vpImage<double> dwwy_d(240, 320, 0);
vpImage<double> dwwz_a(240, 320, 0); vpImage<double> dwwz_h(240, 320, 0); vpImage<double> dwwz_v(240, 320, 0); vpImage<double> dwwz_d(240, 320, 0);

unsigned int bord = 10;
vpCameraParameters cam(870, 870, 160, 120);
double px = cam.get_px();
double py = cam.get_py();
int n_max=4; 
int m_max=4;
int kersize = 9;
double sigma1 = 0.5;
double sigma2 = 1.1;
double sigma3 = 1.8;
double sigma4 = 2.8;

vpMatrix Dnm1(kersize, kersize); vpMatrix Dnm2(kersize, kersize); vpMatrix Dnm3(kersize, kersize); vpMatrix Dnm4(kersize, kersize);

double gaussianFactor, normalization;


using namespace std;

void init(WeightMode wmode)
{
	std::cout << "Weighting mode: " << (int)wmode
		<< (wmode == W_NONE ? " (D=I, no weighting)" :
		    wmode == W_TUKEY ? " (plain Tukey)" : " (Hermite-informed Tukey)")
		<< std::endl;
	bool opt_click_allowed = true;
	bool opt_display = true;
	int opt_niter = 1000;
	vpImage<unsigned char> Itexture;
	vpImage<unsigned char> Itextured;
	vpImageIo::read(Itexture, "./peppers.jpg");
	vpImageIo::read(Itextured, "./peppers_o_b.jpg");
	
	vpColVector X[4];
	for (int i = 0; i < 4; i++) X[i].resize(3);
	// Top left corner     // Top right corner  // Bottom right corner  //Bottom left corner
	X[0][0] = -0.3;        X[1][0] = 0.3;         X[2][0] = 0.3;        X[3][0] = -0.3;
	X[0][1] = -0.215;      X[1][1] = -0.215;      X[2][1] = 0.215;      X[3][1] = 0.215;
	X[0][2] = 0;           X[1][2] = 0;           X[2][2] = 0;          X[3][2] = 0;

	vpImageSimulator sim;

	sim.setInterpolationType(vpImageSimulator::BILINEAR_INTERPOLATION);
	sim.init(Itextured, X);

	vpPlot ViSP_plot;
	init_visp_plot(ViSP_plot);

	Hermite();

	//std::cin.get();
	// ----------------------------------------------------------
	// Create the framegraber (here a simulated image)

	//camera desired position
	vpHomogeneousMatrix cdMo;
	
	//cdMo[2][3] = 1;
	cdMo[2][3] = 1.3;
	
	//set the robot at the desired position
	sim.setCameraPosition(cdMo);
	sim.getImage(I, cam);  // and aquire the image Id
	Id = I;

	Idhermite();

	// display the image
	#if defined VISP_HAVE_X11
	vpDisplayX d;
	#elif defined VISP_HAVE_GDI
	vpDisplayGDI d;
	#elif defined VISP_HAVE_GTK
	vpDisplayGTK d;
	#elif defined VISP_HAVE_OPENCV
	vpDisplayOpenCV d;
	#endif

	#if defined(VISP_HAVE_X11) || defined(VISP_HAVE_GDI) || defined(VISP_HAVE_GTK) || defined(VISP_HAVE_OPENCV) 
	if (opt_display) {
		d.init(I, 20, 10, "Photometric visual servoing : s");
		vpDisplay::display(I);
		vpDisplay::flush(I);
		d.init(Id, 60, 30, "Photometric visual servoing : s*");
		vpDisplay::display(Id);
		vpDisplay::flush(Id);
		
	}

	if (opt_display && opt_click_allowed) {
		std::cout << "Click in the image to continue..." << std::endl;
		vpDisplay::getClick(I);
	}
	#endif

	// display the image

	// ----------------------------------------------------------
	// position the robot at the initial position
	// ----------------------------------------------------------


	vpHomogeneousMatrix cMo;

	//cMo.buildFrom(0.38, -0.33, 3.7, vpMath::rad(27), vpMath::rad(-36), vpMath::rad(14));
     //peppe//
	//cMo.buildFrom(-0.32, -0.28, 3.5, vpMath::rad(19), vpMath::rad(-22), vpMath::rad(14));
	// Fixed: tx/ty had a stray extra leading digit (-3.32/-2.28), which put the
	// target completely outside the camera's field of view (0% visible). The
	// intended translation was the same order of magnitude as the line above,
	// just paired with the farther Z=6.5 / larger rotation test case.
	cMo.buildFrom(-0.32, -0.28, 6.5, vpMath::rad(29), vpMath::rad(-32), vpMath::rad(24));

	//partial occulation

	//cMo.buildFrom(0.27, -0.25, 3.20, vpMath::rad(31), vpMath::rad(-28), vpMath::rad(6));
	//cMo.buildFrom(-0.24, 0.26, 3.18, vpMath::rad(-29), vpMath::rad(30), vpMath::rad(-7));
	//cMo.buildFrom(0.25, 0.28, 3.22, vpMath::rad(34), vpMath::rad(-30), vpMath::rad(5));
	//cMo.buildFrom(-0.26, -0.22, 3.15, vpMath::rad(36), vpMath::rad(27), vpMath::rad(-6));
	//cMo.buildFrom(0.29, -0.23, 3.19, vpMath::rad(-33), vpMath::rad(-29), vpMath::rad(9));
	//cMo.buildFrom(-0.28, 0.25, 3.20, vpMath::rad(32), vpMath::rad(30), vpMath::rad(-4));
	//cMo.buildFrom(0.30, 0.22, 3.21, vpMath::rad(35), vpMath::rad(-32), vpMath::rad(8));
	//cMo.buildFrom(-0.25, -0.28, 3.16, vpMath::rad(-34), vpMath::rad(31), vpMath::rad(-9));
	//cMo.buildFrom(0.23, -0.30, 3.17, vpMath::rad(33), vpMath::rad(-31), vpMath::rad(6));
	///cMo.buildFrom(-0.29, 0.27, 3.18, vpMath::rad(36), vpMath::rad(30), vpMath::rad(-7));






	vpHomogeneousMatrix wMo; // Set to identity
	vpHomogeneousMatrix wMc; // Camera position in the world frame
	
	//set the robot at the desired position
	sim.init(Itexture, X);
	sim.setCameraPosition(cMo);
	I = 0;
	sim.getImage(I, cam);  // and aquire the image Id

	Ihermite();

	#if defined(VISP_HAVE_X11) || defined(VISP_HAVE_GDI) || defined(VISP_HAVE_GTK) 
	if (opt_display) {
		vpDisplay::display(I);
		vpDisplay::flush(I);
	}
	if (opt_display && opt_click_allowed) {
		std::cout << "Click in the image to continue..." << std::endl;
		vpDisplay::getClick(I);
	}
	#endif  

	vpImage<unsigned char> Idiff;
	Idiff = I;

	vpImageTools::imageDifference(I, Id, Idiff);

	// Affiche de l'image de difference
	#if defined VISP_HAVE_X11
	vpDisplayX d1;
	#elif defined VISP_HAVE_GDI
	vpDisplayGDI d1;
	#elif defined VISP_HAVE_GTK
	vpDisplayGTK d1;
	#endif
	#if defined(VISP_HAVE_X11) || defined(VISP_HAVE_GDI) || defined(VISP_HAVE_GTK) 
	if (opt_display) {
		d1.init(Idiff, 40 + (int)I.getWidth(), 10, "photometric visual servoing : s-s* ");
		vpDisplay::display(Idiff);
		vpDisplay::flush(Idiff);
	}
	#endif
	// create the robot (here a simulated free flying camera)
	vpSimulatorCamera robot;
	robot.setSamplingTime(0.04);
	wMc = wMo * cMo.inverse();
	robot.setPosition(wMc);

	// ------------------------------------------------------
	// Visual feature, interaction matrix, error
	// s, Ls, Lsd, Lt, Lp, etc
	// ------------------------------------------------------

	// current visual feature built from the image
	// (actually, this is the image...)
	vpFeatureLuminance sI;
	sI.init(I.getHeight(), I.getWidth(), Z);
	sI.setCameraParameters(cam);
	sI.buildFrom(I,
		w_a, w_h, w_v, w_d,
		wvx_a, wvx_h, wvx_v, wvx_d,
		wvy_a, wvy_h, wvy_v, wvy_d,
		wvz_a, wvz_h, wvz_v, wvz_d,
		wwx_a, wwx_h, wwx_v, wwx_d,
		wwy_a, wwy_h, wwy_v, wwy_d,
		wwz_a, wwz_h, wwz_v, wwz_d);

	// desired visual feature built from the image
	vpFeatureLuminance sId;
	sId.init(I.getHeight(), I.getWidth(), Z);
	sId.setCameraParameters(cam);
	sId.buildFrom(Id,
		dw_a, dw_h, dw_v, dw_d,
		dwvx_a, dwvx_h, dwvx_v, dwvx_d,
		dwvy_a, dwvy_h, dwvy_v, dwvy_d,
		dwvz_a, dwvz_h, dwvz_v, dwvz_d,
		dwwx_a, dwwx_h, dwwx_v, dwwx_d,
		dwwy_a, dwwy_h, dwwy_v, dwwy_d,
		dwwz_a, dwwz_h, dwwz_v, dwwz_d);

	// Matrice d'interaction, Hessien, erreur,...
	vpMatrix Lsd;   // matrice d'interaction a la position desiree
	vpMatrix Hsd;  // hessien a la position desiree
	vpMatrix H; // Hessien utilise pour le levenberg-Marquartd
	vpColVector error; // Erreur I-I*
	vpColVector errorI;
	errorI.resize(76800);

	// Robust M-estimation (Tukey) to down-weight occluded/outlier pixels
	vpRobust robust;
	vpColVector w;      // per-pixel robust weight, 1 = inlier, ~0 = outlier
	vpMatrix Lp;        // weighted interaction matrix: Lp = diag(w) * Lsd
	vpColVector error_p; // weighted error: error_p = diag(w) * error

	// Hermite-informed heteroscedastic normalization applied before the
	// M-estimator (see computeHermiteNormalizedResidual() below): the raw
	// residual is rescaled by local multi-scale structure energy so the
	// robust weighting is structure-aware instead of using one global scale.
	vpColVector error_n; // Hermite-normalized residual, fed to MEstimator() only
	vpColVector structG; // per-pixel local structure energy from w_h/w_v/w_d

	// Compute the interaction matrix
	// link the variation of image intensity to camera motion

	// here it is computed at the desired position
	sI.interaction(Lsd);

	// Compute the Hessian H = L^TL
	Hsd = Lsd.AtA();

	// Compute the Hessian diagonal for the Levenberg-Marquartd
	// optimization process
	unsigned int n = 6;
	vpMatrix diagHsd(n, n);
	diagHsd.eye(n);
	for (unsigned int i = 0; i < n; i++) diagHsd[i][i] = Hsd[i][i];

	// Fixed-floor identity term added to the damped Hessian below. Because
	// the LM damping here scales with diag(Hsd) rather than the identity,
	// it vanishes on any direction the (possibly robustly-weighted) Hsd
	// itself leaves unconstrained - e.g. if enough pixels get a zero Tukey
	// weight that an entire degree of freedom loses support - which would
	// otherwise make (mu*diagHsd + Hsd) singular and inverseByLU() throw.
	// muFloor guarantees invertibility regardless of how Hsd degenerates.
	vpMatrix Ifloor(n, n);
	Ifloor.eye(n);

	// ------------------------------------------------------
	// Control law
	double lambda; //gain
	vpColVector e;
	vpColVector v; // camera velocity send to the robot

				   // ----------------------------------------------------------
				   // Minimisation

	double mu;  // mu = 0 : Gauss Newton ; mu != 0  : LM
	double lambdaGN;

	mu = 0.01;
	lambda = 30;
	lambdaGN = 30;

	// set a velocity control mode
	robot.setRobotState(vpRobot::STATE_VELOCITY_CONTROL);

	// ----------------------------------------------------------
	int iter = 1;
	int iterGN = 80; // swicth to Gauss Newton after iterGN iterations

	double normeError = 0;
	double normeErrorI = 0;
	double Ne = 0;

	vpPoseVector currentpose;
	vpPoseVector desiredpose;
	vpPoseVector errorpose;
	vpPoseVector err;
	double total_iteration_time = 0;

	do {
		double t_start_iter = vpTime::measureTimeMs();
		  std::cout << "--------------------------------------------" << iter++ << std::endl;

		//  Acquire the new image
		sim.setCameraPosition(cMo);
		I = 0;
		sim.getImage(I, cam);

		Ihermite();

		#if defined(VISP_HAVE_X11) || defined(VISP_HAVE_GDI) || defined(VISP_HAVE_GTK) 
		if (opt_display) {
			vpDisplay::display(I);
			vpDisplay::flush(I);
		}
		#endif
		vpImageTools::imageDifference(I, Id, Idiff);
		#if defined(VISP_HAVE_X11) || defined(VISP_HAVE_GDI) || defined(VISP_HAVE_GTK) 
		if (opt_display) {
			vpDisplay::display(Idiff);
			vpDisplay::flush(Idiff);
		}
		#endif


		// Compute current visual feature
		sI.buildFrom(I,
			w_a, w_h, w_v, w_d,
			wvx_a, wvx_h, wvx_v, wvx_d,
			wvy_a, wvy_h, wvy_v, wvy_d,
			wvz_a, wvz_h, wvz_v, wvz_d,
			wwx_a, wwx_h, wwx_v, wwx_d,
			wwy_a, wwy_h, wwy_v, wwy_d,
			wwz_a, wwz_h, wwz_v, wwz_d);

		// compute current error
		sI.error(sId, error);
		int k = 0;
		for (unsigned int i = 0; i<240; i++)
		{
			for (unsigned int j = 0; j<320; j++)
			{
				errorI[k] = Id[i][j] - I[i][j];
				k++;
			}
		}

		normeError = (error.sumSquare());
		normeErrorI = (errorI.sumSquare());
		std::cout << "|e| " << normeError << std::endl;

		// double t = vpTime::measureTimeMs() ;

		// ---------- Levenberg Marquardt method --------------

		if (iter == 2)
		{
			Ne = normeErrorI;
		}

		sI.interaction(Lsd);

		// Ablation weighting matrix D = diag(w). This is the ONLY block that
		// differs between the three variants; everything before and after it
		// (feature extraction, interaction matrix, LM update, gains, stopping
		// rule) is identical code shared by all three, so a run's variant is
		// determined solely by -w and nothing else needs to be kept in sync
		// by hand.
		w.resize(error.getRows());
		w = 1; // W_NONE: D = I, falls through unchanged (Chapter 3 / HER-PVS exactly)

		if (wmode == W_TUKEY) {
			// Plain, structure-blind Tukey M-estimator on the raw residual.
			robust.MEstimator(vpRobust::TUKEY, error, w);
		}
		else if (wmode == W_HERMITE) {
			// Hermite-informed: normalize the residual by local multi-scale
			// structure energy (see computeHermiteNormalizedResidual()) before
			// the M-estimator sees it, so the rejection threshold is
			// structure-aware instead of using one global image-wide scale.
			computeHermiteNormalizedResidual(error, structG, error_n);
			robust.MEstimator(vpRobust::TUKEY, error_n, w);
		}

		Lp.resize(Lsd.getRows(), Lsd.getCols());
		error_p.resize(error.getRows());
		for (unsigned int i = 0; i < error.getRows(); i++) {
			for (unsigned int j = 0; j < Lsd.getCols(); j++)
				Lp[i][j] = w[i] * Lsd[i][j];
			error_p[i] = w[i] * error[i];
		}

		Hsd = Lp.AtA();
		diagHsd.eye(n);
		for (unsigned int i = 0; i < n; i++) diagHsd[i][i] = Hsd[i][i];

		lambda = pow(10, (log10(normeErrorI) - 0.8*log10(Ne)));
		mu = pow(10, (5 * log10(normeErrorI) - 5.2*log10(Ne)));
		//mu = pow(10, (5 * log10(normeErrorI) - 48));

		{
			/*if (iter > iterGN)
			{
			mu = 0.0001;
			lambda = lambdaGN;
			}*/

			std::cout << "lambda   =    " << lambda << std::endl;
			std::cout << "mu       =    " << mu << std::endl;
			std::cout << "Ne       =    " << (5.23*log10(Ne)) << std::endl;
			//std::cin.get();

			// Compute the levenberg Marquartd term. muFloor*Ifloor guarantees
			// (mu*diagHsd + Hsd + muFloor*Ifloor) stays invertible even if
			// robust rejection has driven some diagonal entry of Hsd to zero.
			{
				double meanDiag = 0.0;
				for (unsigned int i = 0; i < n; i++) meanDiag += diagHsd[i][i];
				meanDiag /= n;
				double muFloor = 1e-6 * meanDiag;
				if (muFloor < 1e-12) muFloor = 1e-12;

				H = ((mu * diagHsd) + Hsd + muFloor * Ifloor).inverseByLU();
			}
			//  compute the control law
			e = H * Lp.t() * error_p;

			v = -lambda*e;
		}

		std::cout << "lambda = " << lambda << "  mu = " << mu;
		std::cout << " |Tc| = " << sqrt(v.sumSquare()) << std::endl;

		// send the robot velocity
		robot.setVelocity(vpRobot::CAMERA_FRAME, v);
		wMc = robot.getPosition();
		cMo = wMc.inverse() * wMo;
		currentpose.buildFrom(cMo);
		desiredpose.buildFrom(cdMo);
		err[0] = normeError; err[1] = normeError; err[2] = normeError; err[3] = normeError; err[4] = normeError; err[5] = normeError;

		for (unsigned int i = 0; i < 6; i++)
		{
			errorpose[i] = currentpose[i] - desiredpose[i];
			std::cout << errorpose[i];
			std::cout << std::endl;

		}
		//std::cin.get();
		ViSP_plot.plot(0, (iter), v);
		ViSP_plot.plot(1, (iter) , errorpose);
		ViSP_plot.plot(2, (iter) , err);
		//if (iter == 200 || normeError < 10000) { std::cin.get(); }
		
		double t_iter = vpTime::measureTimeMs()- t_start_iter;
		double average_time_ms = total_iteration_time / (iter - 1);
		total_iteration_time += t_iter;
		std::cout << "cumulative time at iteration " << (iter - 1) <<": "<< total_iteration_time<< "ms"<< std::endl;
		std::cout << "average time per iterations: " << average_time_ms <<"ms"<< std::endl;
	} while (normeError > 10000 && iter < opt_niter);

	v = 0;
	robot.setVelocity(vpRobot::CAMERA_FRAME, v);

	



}


double hermitePolynomial(int n, double x) {
	if (n == 0) return 1.0;  // Base case: H_0(x) = 1
	if (n == 1) return 2 * x;  // Base case: H_1(x) = 2x
					   // Recursive formula: H_n(x) = 2x * H_(n-1)(x) - 2(n-1) * H_(n-2)(x)
	return 2 * x * hermitePolynomial(n - 1, x) - 2 * (n - 1) * hermitePolynomial(n - 2, x);
}
double dnFunction(int n, double x, double sigma) {
	double normalizedX = x / sigma;
	double prefactor = (std::pow(-1, n)) / (std::sqrt(std::pow(2, n) * std::tgamma(n + 1) * std::sqrt(M_PI) * sigma));
	double hermiteValue = hermitePolynomial(n, normalizedX);
	double exponential = std::exp( -(normalizedX * normalizedX) / 2.0);
	return prefactor * hermiteValue * exponential;
}
void Hermite(){

// Function to compute the 2D Hermite filter
	Dnm1 = 0; Dnm2 = 0; Dnm3 = 0; Dnm4 = 0;

	for (int i = 0; i < kersize; i++) {
		for (int j = 0; j < kersize; j++) {
			// Compute x and y based on kernel size
			double x = j - ((kersize - 1) / 2.0);
			double y = ((kersize - 1) / 2.0) - i;

			// Sum the contributions from all Hermite polynomial combinations
			for (int n = 0; n <= n_max; ++n) {
				for (int m = 0; m <= m_max; ++m) {
					Dnm1[i][j] += dnFunction(n, x, sigma1) * dnFunction(m, y, sigma1);
					Dnm2[i][j] += dnFunction(n, x, sigma2) * dnFunction(m, y, sigma2);
					Dnm3[i][j] += dnFunction(n, x, sigma3) * dnFunction(m, y, sigma3);
					Dnm4[i][j] += dnFunction(n, x, sigma4) * dnFunction(m, y, sigma4);
				}
			}
		}
	}
}

void Ihermite()
{
	dIx = 0;
	dIy = 0;

	vpImageFilter::getGradX(I, dIx);
	vpImageFilter::getGradY(I, dIy);

	for (unsigned int i = bord; i < I.getHeight(); i++) {
		for (unsigned int j = bord; j < I.getWidth(); j++)
		{
			dIx[i][j] = px * dIx[i][j];
			dIy[i][j] = py * dIy[i][j];
		}
	}

	for (unsigned int i = bord; i < I.getHeight() - bord; i++)
	{
		for (unsigned int j = bord; j < I.getWidth() - bord; j++)
		{
			double x = 0, y = 0;
			vpPixelMeterConversion::convertPoint(cam, i, j, y, x);

			vvx[i][j] = (dIx.getValue(i, j)) * (1 / Z);
			vvy[i][j] = (dIy.getValue(i, j)) * (1 / Z);
			vvz[i][j] = (-x * (dIx.getValue(i, j)) - y * (dIy.getValue(i, j))) * (1 / Z);
			vwx[i][j] = -x * y * (dIx.getValue(i, j)) - (1 + y * y) * (dIy.getValue(i, j));
			vwy[i][j] = x * y * (dIy.getValue(i, j)) + (1 + x * x) * (dIx.getValue(i, j));
			vwz[i][j] = x * (dIy.getValue(i, j)) - y * (dIx.getValue(i, j));
		}
	}

	vpImageFilter::filter(I, w_a, Dnm1); vpImageFilter::filter(I, w_h, Dnm2); vpImageFilter::filter(I, w_v, Dnm3); vpImageFilter::filter(I, w_d, Dnm4);
	vpImageFilter::filter(vvx, wvx_a, Dnm1); vpImageFilter::filter(vvx, wvx_h, Dnm2); vpImageFilter::filter(vvx, wvx_v, Dnm3); vpImageFilter::filter(vvx, wvx_d, Dnm4);
	vpImageFilter::filter(vvy, wvy_a, Dnm1); vpImageFilter::filter(vvy, wvy_h, Dnm2); vpImageFilter::filter(vvy, wvy_v, Dnm3); vpImageFilter::filter(vvy, wvy_d, Dnm4);
	vpImageFilter::filter(vvz, wvz_a, Dnm1); vpImageFilter::filter(vvz, wvz_h, Dnm2); vpImageFilter::filter(vvz, wvz_v, Dnm3); vpImageFilter::filter(vvz, wvz_d, Dnm4);
	vpImageFilter::filter(vwx, wwx_a, Dnm1); vpImageFilter::filter(vwx, wwx_h, Dnm2); vpImageFilter::filter(vwx, wwx_v, Dnm3); vpImageFilter::filter(vwx, wwx_d, Dnm4);
	vpImageFilter::filter(vwy, wwy_a, Dnm1); vpImageFilter::filter(vwy, wwy_h, Dnm2); vpImageFilter::filter(vwy, wwy_v, Dnm3); vpImageFilter::filter(vwy, wwy_d, Dnm4);
	vpImageFilter::filter(vwz, wwz_a, Dnm1); vpImageFilter::filter(vwz, wwz_h, Dnm2); vpImageFilter::filter(vwz, wwz_v, Dnm3); vpImageFilter::filter(vwz, wwz_d, Dnm4);
}

void Idhermite() {
	ddIx = 0;
	ddIy = 0;

	vpImageFilter::getGradX(Id, ddIx);
	vpImageFilter::getGradY(Id, ddIy);

	for (unsigned int i = bord; i < I.getHeight(); i++) {
		for (unsigned int j = bord; j < I.getWidth(); j++)
		{
			ddIx[i][j] = px * ddIx[i][j];
			ddIy[i][j] = py * ddIy[i][j];
		}
	}

	for (unsigned int i = bord; i < I.getHeight() - bord; i++)
	{
		for (unsigned int j = bord; j < I.getWidth() - bord; j++)
		{
			double x = 0, y = 0;
			vpPixelMeterConversion::convertPoint(cam, i, j, y, x);

			dvvx[i][j] = (ddIx.getValue(i, j)) * (1 / Z);
			dvvy[i][j] = (ddIy.getValue(i, j)) * (1 / Z);
			dvvz[i][j] = (-x * (ddIx.getValue(i, j)) - y * (ddIy.getValue(i, j))) * (1 / Z);
			dvwx[i][j] = -x * y * (ddIx.getValue(i, j)) - (1 + y * y) * (ddIy.getValue(i, j));
			dvwy[i][j] = x * y * (ddIy.getValue(i, j)) + (1 + x * x) * (ddIx.getValue(i, j));
			dvwz[i][j] = x * (ddIy.getValue(i, j)) - y * (ddIx.getValue(i, j));
		}
	}

	vpImageFilter::filter(Id, dw_a, Dnm1); vpImageFilter::filter(Id, dw_h, Dnm2); vpImageFilter::filter(Id, dw_v, Dnm3); vpImageFilter::filter(Id, dw_d, Dnm4);
	vpImageFilter::filter(dvvx, dwvx_a, Dnm1); vpImageFilter::filter(dvvx, dwvx_h, Dnm2); vpImageFilter::filter(dvvx, dwvx_v, Dnm3); vpImageFilter::filter(dvvx, dwvx_d, Dnm4);
	vpImageFilter::filter(dvvy, dwvy_a, Dnm1); vpImageFilter::filter(dvvy, dwvy_h, Dnm2); vpImageFilter::filter(dvvy, dwvy_v, Dnm3); vpImageFilter::filter(dvvy, dwvy_d, Dnm4);
	vpImageFilter::filter(dvvz, dwvz_a, Dnm1); vpImageFilter::filter(dvvz, dwvz_h, Dnm2); vpImageFilter::filter(dvvz, dwvz_v, Dnm3); vpImageFilter::filter(dvvz, dwvz_d, Dnm4);
	vpImageFilter::filter(dvwx, dwwx_a, Dnm1); vpImageFilter::filter(dvwx, dwwx_h, Dnm2); vpImageFilter::filter(dvwx, dwwx_v, Dnm3); vpImageFilter::filter(dvwx, dwwx_d, Dnm4);
	vpImageFilter::filter(dvwy, dwwy_a, Dnm1); vpImageFilter::filter(dvwy, dwwy_h, Dnm2); vpImageFilter::filter(dvwy, dwwy_v, Dnm3); vpImageFilter::filter(dvwy, dwwy_d, Dnm4);
	vpImageFilter::filter(dvwz, dwwz_a, Dnm1); vpImageFilter::filter(dvwz, dwwz_h, Dnm2); vpImageFilter::filter(dvwz, dwwz_v, Dnm3); vpImageFilter::filter(dvwz, dwwz_d, Dnm4);
}

// Hermite-informed heteroscedastic residual normalization.
//
// A plain M-estimator (Tukey) judges every pixel's residual against one
// global scale (its MAD over the whole image). That is blind to the fact
// that the *expected* size of a photometric residual is not uniform: on a
// strongly textured/edge pixel, a sub-pixel misalignment naturally produces
// a large intensity swing even with no occlusion, while the same raw
// residual on a flat, low-texture pixel is much more surprising and much
// more likely to be a genuine outlier (occlusion, specularity).
//
// w_h/w_v/w_d are the multi-scale Hermite-filtered responses of the current
// image I already computed by Ihermite() at sigma2/sigma3/sigma4 (w_a, at
// the finest scale sigma1, is left out as it stays close to raw intensity
// and adds little discriminative structure). Their combined magnitude is
// used as a per-pixel local structure-energy estimate structG, normalized
// to be ~1 on average. Each residual is then rescaled by structG before
// being handed to the M-estimator, so residuals on high-structure pixels
// are shrunk (judged less extreme) and residuals on flat pixels are
// amplified (judged more extreme) relative to the plain, unnormalized
// approach used previously.
//
// Assumes the feature vector `error` is one scalar per image pixel, in
// row-major (i*width+j) order, matching vpFeatureLuminance's standard
// layout. If the custom vpFeatureLuminance used here stacks additional
// channels into `error`, this falls back to the unnormalized residual.
void computeHermiteNormalizedResidual(const vpColVector &error, vpColVector &structG, vpColVector &error_n)
{
	const unsigned int height = I.getHeight();
	const unsigned int width = I.getWidth();
	const unsigned int nbPixels = height * width;

	error_n = error;
	if (error.getRows() != nbPixels) {
		// Feature layout doesn't match one-scalar-per-pixel: skip normalization.
		structG.resize(0);
		return;
	}

	structG.resize(nbPixels);
	double sumG = 0.0;
	unsigned int k = 0;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			double gh = w_h[i][j], gv = w_v[i][j], gd = w_d[i][j];
			structG[k] = std::sqrt(gh * gh + gv * gv + gd * gd);
			sumG += structG[k];
			k++;
		}
	}

	double Gmean = sumG / nbPixels;
	if (Gmean < 1e-12) Gmean = 1e-12;

	for (unsigned int idx = 0; idx < nbPixels; idx++) {
		double Gbar = structG[idx] / Gmean; // relative local structure energy, ~1 on average
		if (Gbar < 0.1) Gbar = 0.1;          // cap amplification in near-flat regions
		error_n[idx] = error[idx] / Gbar;
	}
}

void
init_visp_plot(vpPlot& ViSP_plot) {
	/* -------------------------------------
	* Initialize ViSP Plotting
	* -------------------------------------
	*/
	const unsigned int NbGraphs = 4;                                // No. of graphs
	const unsigned int NbCurves_in_graph[NbGraphs] = { 6,6,6,6 };       // Curves in each graph
	ViSP_plot.init(NbGraphs, 700, 800, 10, 10, "Visual Servoing results...");
	vpColor Colors[6] = { \
		// Colour for s1, s2, s3,  in 1st plot
		vpColor::purple, vpColor::green, vpColor::red, \
		vpColor::orange, vpColor::cyan,vpColor::blue
	};
	for (unsigned int p = 0; p<NbGraphs; p++) {
		ViSP_plot.initGraph(p, NbCurves_in_graph[p]);
		for (unsigned int c = 0; c<NbCurves_in_graph[p]; c++)
			ViSP_plot.setColor(p, c, Colors[c]);
		
	}
	
	
	ViSP_plot.setTitle(0, "camera velocities");
	ViSP_plot.setUnitY(0," v(m/s),w(rad/s)"),
      ViSP_plot.setUnitX(0,"iteration"),
	ViSP_plot.setGraphThickness(0, 2);
	ViSP_plot.setGridThickness(0, 1);
	ViSP_plot.setLegend(0, 0, "");
	ViSP_plot.setLegend(0, 1, "");
	ViSP_plot.setLegend(0, 2, "");
	ViSP_plot.setLegend(0, 3, "");
	ViSP_plot.setLegend(0, 4, "");
	ViSP_plot.setLegend(0, 5, "");
	ViSP_plot.setTitle(1, "errorpose");
	ViSP_plot.setUnitY(1, "d(m),dr(rad)"),
	ViSP_plot.setUnitX(1, "iteration"),
	ViSP_plot.setGraphThickness(1, 2);
	ViSP_plot.setGridThickness(1, 1);
	ViSP_plot.setLegend(1, 0, "");
	ViSP_plot.setLegend(1, 1, "");
	ViSP_plot.setLegend(1, 2, "");
	ViSP_plot.setLegend(1, 3, "");
	ViSP_plot.setLegend(1, 4, "");
	ViSP_plot.setLegend(1, 5, "");
	ViSP_plot.setTitle(2, "err ");
	ViSP_plot.setUnitX(2, "iteration"),
	ViSP_plot.setGraphThickness(2, 2);
	ViSP_plot.setGridThickness(2, 1);
	ViSP_plot.setLegend(2, 5, "||e|| _____");
	
	
	
}
