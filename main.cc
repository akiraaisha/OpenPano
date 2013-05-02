// File: main.cc
// Date: Fri May 03 05:47:19 2013 +0800
// Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "render/filerender.hh"
#include "config.hh"
#include "planedrawer.hh"
#include "keypoint.hh"
#include "matcher.hh"
#include "gallery.hh"
#include "panorama.hh"
#include "transformer.hh"
#include <ctime>
#include <cassert>

using namespace std;
using namespace Magick;

bool TEMPDEBUG = false;

#define LABEL_LEN 7

inline real_t gen_rand()
{ return (real_t)rand() / RAND_MAX; }

vector<Feature> get_feature(imgptr& ptr)
{ return Panorama::get_feature(ptr); }

void test_feature(const char* fname) {
	shared_ptr<Img> test(new Img(fname));
	vector<Feature> ans = get_feature(test);
	RenderBase* r = new FileRender(test, "out.png");
	cout << r->get_geo().w << r->get_geo().h << endl;
	PlaneDrawer pld(r);

	cout << ans.size() << endl;
	for (auto i : ans)
		pld.arrow(toCoor(i.real_coor), i.dir, LABEL_LEN);
	r->finish();
	delete r;
}

void test_extrema(const char* fname) {
	imgptr test(new Img(fname)) ;
	RenderBase* r = new FileRender(test, "out.png");
	cout << r->get_geo().w << r->get_geo().h << endl;
	PlaneDrawer pld(r);

	ScaleSpace ss(test, NUM_OCTAVE, NUM_SCALE);
	DOGSpace sp(ss);
	KeyPoint ex(sp, ss);
	ex.work();
	for (auto &i : ex.keyp)
		pld.cross(i, LABEL_LEN / 2);
	r->finish();
	delete r;
}


void gallery(const char* f1, const char* f2) {
	list<Image> imagelist;
	Image pic1(f1);
	Image pic2(f2);
	imagelist.push_back(pic1);
	imagelist.push_back(pic2);
	Gallery ga(imagelist);

	shared_ptr<Img> test(new Img(ga.get()));
	RenderBase* r = new FileRender(test, "out.png");
	PlaneDrawer pld(r);

	vector<Feature> ans = get_feature(test);
	for (auto i : ans) pld.arrow(toCoor(i.real_coor), i.dir, LABEL_LEN);

	shared_ptr<Img> ptr1(new Img(pic1));
	shared_ptr<Img> ptr2(new Img(pic2));
	vector<Feature> feat1 = get_feature(ptr1);
	vector<Feature> feat2 = get_feature(ptr2);

	Matcher match(feat1, feat2);
	auto ret = match.match();
	for (auto &x : ret.data) {
		pld.set_color(::Color(gen_rand(), gen_rand(), gen_rand()));
		pld.circle(toCoor(feat1[x.x].real_coor), LABEL_LEN);
		pld.circle(toCoor(feat2[x.y].real_coor) + Coor(ptr1->w, 0), LABEL_LEN);
		pld.line(toCoor(feat1[x.x].real_coor), toCoor(feat2[x.y].real_coor) + Coor(ptr1->w, 0));
	}

	r->finish();
	delete r;
}

void test_memory(const char* fname) {
	shared_ptr<Img> test(new Img(fname));
	get_feature(test);
	get_feature(test);
	get_feature(test);
	get_feature(test);
	int a;
	cin >> a;
	return;
}

void test_transform(const char* f1, const char* f2) {
	imgptr ptr1(new Img(f1));
	imgptr ptr2(new Img(f2));

	vector<imgptr> imgs = {ptr1, ptr2};
	Panorama p(imgs);
	shared_ptr<Img> res = p.get();

	RenderBase* r = new FileRender(res, "out.png");
	r->finish();
	delete r;
}

void final(int argc, char* argv[]) {
	vector<imgptr> imgs;
	REPL(i, 1, argc) {
		imgptr ptr(new Img(argv[i]));
		imgs.push_back(ptr);
	}
	Panorama p(imgs);
	imgptr res = p.get();

	if (CROP)
		res->crop();
	RenderBase* r = new FileRender(res, "out.png");
	r->finish();
	delete r;
}

void init_config() {
	ConfigParser Config("config.cfg");
	PANO = Config.get("PANO");
	TRANS = Config.get("TRANS");
	CROP = Config.get("CROP");
	if (PANO && TRANS)
		assert(false);

	NUM_OCTAVE = Config.get("NUM_OCTAVE");
	NUM_SCALE = Config.get("NUM_SCALE");
	SCALE_FACTOR = Config.get("SCALE_FACTOR");
	GAUSS_SIGMA = Config.get("GAUSS_SIGMA");
	GAUSS_WINDOW_FACTOR = Config.get("GAUSS_WINDOW_FACTOR");
	JUDGE_EXTREMA_DIFF_THRES = Config.get("JUDGE_EXTREMA_DIFF_THRES");
	CONTRAST_THRES = Config.get("CONTRAST_THRES");
	PRE_COLOR_THRES = Config.get("PRE_COLOR_THRES");
	EDGE_RATIO = Config.get("EDGE_RATIO");
	CALC_OFFSET_DEPTH = Config.get("CALC_OFFSET_DEPTH");
	OFFSET_THRES = Config.get("OFFSET_THRES");
	ORI_WINDOW_FACTOR = Config.get("ORI_WINDOW_FACTOR");
	ORI_RADIUS = Config.get("ORI_RADIUS");
	ORT_HIST_SMOOTH_COUNT = Config.get("ORT_HIST_SMOOTH_COUNT");
	ORT_HIST_PEAK_RATIO = Config.get("ORT_HIST_PEAK_RATIO");
	DESC_HIST_REAL_WIDTH = Config.get("DESC_HIST_REAL_WIDTH");
	DESC_NORM_THRESH = Config.get("DESC_NORM_THRESH");
	DESC_INT_FACTOR = Config.get("DESC_INT_FACTOR");
	MATCH_REJECT_NEXT_RATIO = Config.get("MATCH_REJECT_NEXT_RATIO");
	MATCH_MIN_SIZE = Config.get("MATCH_MIN_SIZE");
	CONNECTED_THRES = Config.get("CONNECTED_THRES");
	RANSAC_ITERATIONS = Config.get("RANSAC_ITERATIONS");
	RANSAC_INLIER_THRES = Config.get("RANSAC_INLIER_THRES");
	SLOPE_PLAIN = Config.get("SLOPE_PLAIN");
	OUTPUT_SIZE_FACTOR = Config.get("OUTPUT_SIZE_FACTOR");
}

int main(int argc, char* argv[]) {
	srand(time(NULL));
	init_config();
	test_extrema(argv[1]);

	// test_memory(argv[1]);
	// test_feature(argv[1]);
	// test_transform(argv[1], argv[2]);
	/*
	 *final(argc, argv);
	 */
}
