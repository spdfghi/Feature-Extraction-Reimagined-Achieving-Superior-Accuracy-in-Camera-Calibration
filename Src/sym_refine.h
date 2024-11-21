#ifndef SYM_REFINE_H
#define SYM_REFINE_H
#include <opencv2/opencv.hpp>

int WIN_SIZE = 10;
int SAMP_RATIO = 4;

#define MULT 8
#define OFFSET 5 // if you make no offset between your patterns, then let it be zero

void get_samps(std::vector<cv::Point2f>& samps)
{
	int w = 2 * WIN_SIZE + 1;
	for (int i = 0; i < SAMP_RATIO * w * w; ++i)
	{
		float x = 2 * ((double)rand() / RAND_MAX) - 1;
		float y = 2 * ((double)rand() / RAND_MAX) - 1;
		samps.push_back({ x * WIN_SIZE,y * WIN_SIZE });
	}
}

#if MULT
void grad_interp(cv::Mat& img, cv::Point2f pos, std::vector<float>& values, std::vector<cv::Point2f>& grads)
{
	int dim = img.channels();
	values.resize(MULT);
	grads.resize(MULT);

	int xi, yi;
	float fx, fy, U, D, fx_inv, fy_inv;

	xi = pos.x, yi = pos.y;
	fx = pos.x - xi, fy = pos.y - yi;
	fx_inv = 1 - fx, fy_inv = 1 - fy;

	cv::Vec<uchar, MULT> lu, ru, ld, rd;
	lu = img.at<cv::Vec<uchar, MULT>>(yi, xi);
	ru = img.at<cv::Vec<uchar, MULT>>(yi, xi + 1);
	ld = img.at<cv::Vec<uchar, MULT>>(yi + 1, xi);
	rd = img.at<cv::Vec<uchar, MULT>>(yi + 1, xi + 1);

	for (int i = 0; i < MULT; ++i)
	{
		U = lu[i] * fx_inv + ru[i] * fx;
		D = ld[i] * fx_inv + rd[i] * fx;
		values[i] = U * fy_inv + D * fy;

		grads[i].x = (ru[i] - lu[i]) * fy_inv + (rd[i] - ld[i]) * fy;
		grads[i].y = D - U;
	}
}

void calcJ
(
	cv::Mat& mult,
	cv::Point2f q,
	std::vector<cv::Point2f>& samps,
	cv::Mat& J,
	cv::Mat& err
)
{
	cv::Point2f p;
	int dim = mult.channels();
	std::vector<cv::Point2f> grad1s(dim), grad2s(dim);
	std::vector<float> v0s(dim), v1s(dim);
	for (int i = 0; i < samps.size(); ++i)
	{
		const auto& samp = samps[i];
		p.x = q.x + samp.x;
		p.y = q.y + samp.y;
		grad_interp(mult, p, v0s, grad1s);

		p.x = q.x - samp.x;
		p.y = q.y - samp.y;
		grad_interp(mult, p, v1s, grad2s);

		for (int j = 0; j < dim; ++j)
		{
			int k = i * dim + j;
			err.at<double>(k) = (v0s[j] - v1s[j]);
			J.at<double>(k, 0) = grad2s[j].x - grad1s[j].x;
			J.at<double>(k, 1) = grad2s[j].y - grad1s[j].y;
		}
	}
}

#else
void grad_interp(cv::Mat img, cv::Point2f pos, float& value, cv::Point2f& grad)
{
	int xi, yi;
	uint8_t lu, ru, ld, rd;
	float fx, fy, U, D, fx_inv, fy_inv;
	uint8_t* data = img.data;
	int n = img.cols;

	xi = pos.x, yi = pos.y;
	fx = pos.x - xi, fy = pos.y - yi;
	fx_inv = 1 - fx, fy_inv = 1 - fy;
	lu = data[yi * n + xi];
	ru = data[yi * n + xi + 1];
	ld = data[(yi + 1) * n + xi];
	rd = data[(yi + 1) * n + xi + 1];

	U = lu * fx_inv + ru * fx;
	D = ld * fx_inv + rd * fx;
	value = U * fy_inv + D * fy;

	grad.x = (ru - lu) * fy_inv + (rd - ld) * fy;
	grad.y = D - U;
}

void calcJ
(
	cv::Mat img,
	cv::Point2f q,
	std::vector<cv::Point2f>& samps,
	cv::Mat& J,
	cv::Mat& err
)
{
	cv::Point2f p, grad1, grad2;
	float v0, v1;
	for (int i = 0; i < samps.size(); ++i)
	{
		const auto& samp = samps[i];
		p.x = q.x + samp.x;
		p.y = q.y + samp.y;
		grad_interp(img, p, v0, grad1);

		p.x = q.x - samp.x;
		p.y = q.y - samp.y;
		grad_interp(img, p, v1, grad2);

		err.at<double>(i) = (v0 - v1);
		J.at<double>(i, 0) = grad2.x - grad1.x;
		J.at<double>(i, 1) = grad2.y - grad1.y;
	}
}
#endif

double LM(
	cv::Point2f& q,
	cv::Mat mult
)
{
	double lambda = 0;
	const int max_iter = 10;
	cv::Mat J, Jt, JtJ, err, Jt_err, delta;
	cv::Mat I = cv::Mat::eye(2, 2, 6);
	double err_norm, err_norm_pre;

	std::vector<cv::Point2f> samps;
	get_samps(samps);
	int samp_num = samps.size();
#if MULT
	J = cv::Mat::zeros(samp_num * MULT, 2, 6);
	err = cv::Mat::zeros(samp_num * MULT, 1, 6);
#else
	J = cv::Mat::zeros(samp_num, 2, 6);
	err = cv::Mat::zeros(samp_num, 1, 6);
#endif

	calcJ(mult, q, samps, J, err);
	err_norm_pre = norm(err);
	cv::transpose(J, Jt);
	JtJ = Jt * J;
	for (int i = 0; i < 2; ++i) lambda += JtJ.at<double>(i);
	lambda /= 1000;

	for (int i = 0; i < max_iter; ++i)
	{
		calcJ(mult, q, samps, J, err);
		cv::transpose(J, Jt);
		JtJ = Jt * J + lambda * I;
		Jt_err = Jt * err;
		cv::solve(JtJ, Jt_err, delta);

		auto* w = (double*)delta.data;
		q.x += w[0];
		q.y += w[1];

		calcJ(mult, q, samps, J, err);
		err_norm = norm(err);

		if (err_norm < err_norm_pre)
		{
			err_norm_pre = err_norm;
			lambda /= 2;
		}
		else
		{
			q.x -= w[0];
			q.y -= w[1];
			lambda *= 2;
		}
	}

	return err_norm_pre;
}

void comb_refine(std::string channel_path, std::string file, std::vector<cv::Point2f>& corners)
{
	std::vector<cv::Mat> imgs(MULT);
	std::vector<int> idx;

	for (int i = 0; i < MULT; ++i)
	{
		std::string img_path = channel_path + std::to_string(i + 1) + "/images/" + file;
		imgs[i] = cv::imread(img_path, 0);
		if (imgs[i].empty())
		{
			std::cout << "can not find: " + img_path;
			exit(0);
		}
		else
		{
			std::cout << "read image: " + img_path << std::endl;
		}
	}

	cv::Mat mult;
	cv::merge(imgs, mult);

	for (auto& q : corners)
	{
		//todo: make no offset in calibration video
		int win_size_bak = WIN_SIZE;

#if OFFSET
		q.y -= OFFSET;
		int thresh = OFFSET * 4;
		if (q.x > thresh && q.y > thresh && q.x < mult.cols - thresh && q.y < mult.rows - thresh)
		{
		}
		else
		{
			WIN_SIZE = OFFSET + 1;
		}
#endif

		LM(q, mult);
		WIN_SIZE = win_size_bak;
	}
}

void sym_refine(cv::Mat& img, std::vector<cv::Point2f>& fets)
{
	if (img.empty())
	{
		std::cout << "empty image for sym_refine";
		exit(0);
	}
	for (auto& pt : fets)
	{
		LM(pt, img);
	}
}

#endif // SYM_REFINE_H
