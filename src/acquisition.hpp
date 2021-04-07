#ifndef acquisition_hpp
#define acquisition_hpp

#include <iostream>
#include <fstream>
#include <iomanip>

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>;
#include <opencv2/imgproc.hpp>

bool save_frame_raw_data(const std::string& filename, rs2::frame frame);

bool frame_metadata_to_csv(const std::string& filename, rs2::frame frame, rs2_intrinsics& intrinsics);

void readRAW(rs2::frame FRAME, float depth_scale, std::vector<int> coordinatesX, const rs2_intrinsics& depth_intrinsics, 
	         std::vector<std::vector<int32_t>>& points3D, int cam);

#endif