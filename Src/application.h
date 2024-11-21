#ifndef APPLICATION_H
#define APPLICATION_H

#include "sym_refine.h"
#include <arpa/inet.h>

// Extract image from video. Classify channels can see folder scripts/
void extract_stable_frame(std::string src_path, std::string dst_path, int diff_thre = 1, int len_thre = 5)
{
	cv::Size shrink_size = { 800,600 };
	cv::Mat cur_img, last_img, org_img;
	std::vector<cv::Mat> imlist_buf;
	int idx = 0;

	for (int i = 0; ; ++i)
	{
		cur_img = cv::imread(src_path + "\\" + std::to_string(i) + ".png", 0);
		if (cur_img.empty()) break;

		org_img = cur_img.clone();
		cv::resize(cur_img, cur_img, shrink_size);
		if (!i) last_img = cur_img;

		double ave_diff = cv::sum(cv::abs(cur_img - last_img))[0]
			/ (double)(shrink_size.width * shrink_size.height);

		if (ave_diff > diff_thre)
		{
			int imlist_size = imlist_buf.size();
			if (imlist_size >= len_thre)
			{
				std::string stable_frame = dst_path + "\\" + std::to_string(idx++) + ".png";
				cv::imwrite(stable_frame, imlist_buf[imlist_size / 2]);
			}
			imlist_buf.clear();
		}
		imlist_buf.push_back(org_img);

		last_img = cur_img;
	}

	int imlist_size = imlist_buf.size();
	if (imlist_size)
	{
		std::string stable_frame = dst_path + "\\" + std::to_string(idx++) + ".png";
		cv::imwrite(stable_frame, imlist_buf[imlist_size / 2]);
	}
}

//==============================================================================================================================================
// interface for project https://github.com/puzzlepaint/camera_calibration
// Copyright 2019 ETH Zürich, Thomas Schöps
typedef uchar u8;
typedef unsigned int u32;
typedef int i32;

inline void read_one(u32* data, FILE* file) {
	u32 temp;
	if (fread(&temp, sizeof(u32), 1, file) != 1) {
		std::cout << "read_one() failed to read the element";
	}
	*data = ntohl(temp);
}

inline void read_one(float* data, FILE* file) {
	// TODO: Does this require a potential endian swap?
	if (fread(data, sizeof(float), 1, file) != 1) {
		std::cout << "read_one() failed to read the element";
	}
}

inline void read_one(i32* data, FILE* file) {
	i32 temp;
	if (fread(&temp, sizeof(i32), 1, file) != 1) {
		std::cout << "read_one() failed to read the element";
	}
	*data = ntohl(temp);
}

inline void write_one(const u32* data, FILE* file) {
	u32 temp = htonl(*data);
	fwrite(&temp, sizeof(u32), 1, file);
}

inline void write_one(const i32* data, FILE* file) {
	i32 temp = htonl(*data);
	fwrite(&temp, sizeof(i32), 1, file);
}

inline void write_one(const float* data, FILE* file) {
	// TODO: Does this require a potential endian swap?
	fwrite(data, sizeof(float), 1, file);
}

bool refineDataset
(
	std::string img_path,
	std::string feature_file,
	std::string output_file
)
{
	FILE* file = fopen(feature_file.c_str(), "rb");
	FILE* file_copy = fopen(output_file.c_str(), "wb");
	if (!file) {
		std::cout << "Cannot read file: " << feature_file;
		return false;
	}

	// File format identifier
	u8 header[10];
	if (fread(header, 1, 10, file) != 10 ||
		header[0] != 'c' ||
		header[1] != 'a' ||
		header[2] != 'l' ||
		header[3] != 'i' ||
		header[4] != 'b' ||
		header[5] != '_' ||
		header[6] != 'd' ||
		header[7] != 'a' ||
		header[8] != 't' ||
		header[9] != 'a') {
		std::cout << "Cannot parse file: " << feature_file;
		std::cout << "Invalid file header.";
		fclose(file);
		fclose(file_copy);
		return false;
	}
	fwrite(header, 1, 10, file_copy);

	// File format version
	u32 version;
	read_one(&version, file);
	write_one(&version, file_copy);
	if (version != 0) {
		std::cout << "Cannot parse file: " << feature_file;
		std::cout << "Unsupported file format version.";
		fclose(file);
		fclose(file_copy);
		return false;
	}

	// Cameras
	u32 num_cameras;
	read_one(&num_cameras, file);
	write_one(&num_cameras, file_copy);
	for (int camera_index = 0; camera_index < num_cameras; ++camera_index) {
		u32 width;
		read_one(&width, file);
		write_one(&width, file_copy);
		u32 height;
		read_one(&height, file);
		write_one(&height, file_copy);
	}

	// Imagesets
	u32 num_imagesets;
	u32 total_num_features = 0;
	read_one(&num_imagesets, file);
	write_one(&num_imagesets, file_copy);
	for (int imageset_index = 0; imageset_index < num_imagesets; ++imageset_index) {
		u32 filename_len;
		read_one(&filename_len, file);
		write_one(&filename_len, file_copy);

		std::string filename;
		filename.resize(filename_len);
		if (fread(&filename[0], 1, filename_len, file) != filename_len) {
			fclose(file);
			std::cout << "Unexpected end of file: " << feature_file;
			return false;
		}
		fwrite(filename.data(), 1, filename_len, file_copy);

		for (int camera_index = 0; camera_index < num_cameras; ++camera_index) {
			u32 num_features;
			read_one(&num_features, file);
			write_one(&num_features, file_copy);

			total_num_features += num_features;
			std::vector<cv::Point2f> fets(num_features);
			std::vector<int> id(num_features);
			for (int i = 0; i < num_features; ++i)
			{
				read_one(&fets[i].x, file);
				read_one(&fets[i].y, file);
				read_one(&id[i], file);
			}


#if !MULT
			//new refinement method
			int p = feature_file.rfind('/');
			std::string img_path = feature_file.substr(0, p + 1) + "images/" + filename;
			cv::Mat img = cv::imread(img_path, 0);
			if (img.empty())
			{
				std::cout << "can't find: " + img_path;
				exit(0);
			}
			sym_refine(img, fets);
			std::cout << "refinement image" << imageset_index << " done" << std::endl;
#else
			//mult_img file format: img_path_1, img_path_2, ..., img_path_MULT
			comb_refine(img_path + "_", filename, fets);
#endif

			for (int i = 0; i < num_features; ++i)
			{
				write_one(&fets[i].x, file_copy);
				write_one(&fets[i].y, file_copy);
				write_one(&id[i], file_copy);
			}
		}
	}

	//remains
	size_t bytesRead;
	char buffer[1024];
	while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
		fwrite(buffer, 1, bytesRead, file_copy);
	}

	fclose(file);
	fclose(file_copy);
	return true;
}

//==============================================================================================================================================

#endif // APPLICATION_H
