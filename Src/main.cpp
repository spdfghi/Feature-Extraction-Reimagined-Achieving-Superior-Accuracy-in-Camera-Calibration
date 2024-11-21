#include "application.h"

//interface for project https://github.com/puzzlepaint/camera_calibration
void refine(std::string img_path, std::string fet_path)
{
	refineDataset(img_path, fet_path + "/features_10px.bin", fet_path + "/features_10px_refined.bin");
}

int main()
{
	std::string data_path = "../Data/";
	refine(data_path + "star8", data_path + "star16");
	return 0;
}
