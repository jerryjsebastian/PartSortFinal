#include <iostream>
#include <fstream>
#include <iomanip>

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>;
#include <opencv2/imgproc.hpp>

using namespace std;

bool save_frame_raw_data(const std::string& filename, rs2::frame frame)
{
    bool ret = false;
    auto image = frame.as<rs2::video_frame>();
    if (image)
    {
        std::ofstream outfile(filename.data(), std::ofstream::binary);
        outfile.write(static_cast<const char*>(image.get_data()), image.get_height() * image.get_stride_in_bytes());

        outfile.close();
        ret = true;
    }

    return ret;
}

bool frame_metadata_to_csv(const std::string& filename, rs2::frame frame, rs2_intrinsics& intrinsics)
{
    bool ret = false;
    auto image = frame.as<rs2::video_frame>();
    if (image)
    {
        std::ofstream csv(filename);

        /* auto profile = image.get_profile();
        csv << "Frame Info: " << std::endl << "Type," << profile.stream_name() << std::endl;
        csv << "Format," << rs2_format_to_string(profile.format()) << std::endl;
        csv << "Frame Number," << image.get_frame_number() << std::endl;
        csv << "Timestamp (ms)," << std::fixed << std::setprecision(2) << image.get_timestamp() << std::endl;
        csv << "Resolution x," << (int)image.get_width() << std::endl;
        csv << "Resolution y," << (int)image.get_height() << std::endl;
        csv << "Bytes per pixel," << (int)image.get_bytes_per_pixel() << std::endl;

        // Intrinsics for color (1280x720)
        if (auto vsp = profile.as<rs2::video_stream_profile>())
        {
            intrinsics = vsp.get_intrinsics();
            csv << std::endl << "Intrinsic:," << std::fixed << std::setprecision(6) << std::endl;
            csv << "Fx," << intrinsics.fx << std::endl;
            csv << "Fy," << intrinsics.fy << std::endl;
            csv << "PPx," << intrinsics.ppx << std::endl;
            csv << "PPy," << intrinsics.ppy << std::endl;
            csv << "Distortion," << rs2_distortion_to_string(intrinsics.model) << std::endl;
        } */

        //// Intrisics for depth (1280x720) for d415
        //intrinsics.fx = 890.644348144531;
        //intrinsics.fy = 890.644348144531;
        //intrinsics.height = 720;
        //intrinsics.ppx = 634.755920410156;
        //intrinsics.ppy = 342.459014892578;
        //intrinsics.width = 1280;
        //intrinsics.model = RS2_DISTORTION_BROWN_CONRADY;
        //intrinsics.coeffs[0] = 0;
        //intrinsics.coeffs[1] = 0;
        //intrinsics.coeffs[2] = 0;
        //intrinsics.coeffs[3] = 0;
        //intrinsics.coeffs[4] = 0; 

        //Intrinsics for depth (1280x720) for d435
        intrinsics.fx = 640.887817382813;
        intrinsics.fy = 640.887817382813;
        intrinsics.height = 720;
        intrinsics.ppx = 632.649536132813;
        intrinsics.ppy = 366.060363769531;
        intrinsics.width = 1280;
        intrinsics.model = RS2_DISTORTION_BROWN_CONRADY;
        intrinsics.coeffs[0] = 0;
        intrinsics.coeffs[1] = 0;
        intrinsics.coeffs[2] = 0;
        intrinsics.coeffs[3] = 0;
        intrinsics.coeffs[4] = 0;

        ret = true;
    }

    return ret;
}

void readRAW(rs2::frame FRAME, float depth_scale, std::vector<int> coordinatesX, const rs2_intrinsics& depth_intrinsics, std::vector<std::vector<float>>& points3D)
{
    std::ofstream myFile("X-Y-Z.csv", std::ios_base::trunc);

    int IMAGE_WIDTH = 1280, IMAGE_HEIGHT = 720;
    cv::Mat depthMat(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_16U, (void*)FRAME.get_data());
    int i = 0;
    // cout << "coordinatesX - size: " << coordinatesX.size() << endl;

    while (i < coordinatesX.size())
    {
        float sum_depth = 0, average_depth = 0;
        int pixel_count = 0;
        for (int k = coordinatesX[i]; k <= coordinatesX[i+2]; k++)
        {
            for (int j = coordinatesX[i + 1]; j <= coordinatesX[i + 3]; j++)
            {
                sum_depth += depthMat.at<short>(j, k);
                pixel_count++;
            }
        }
        // cout << "Heights of barcodes (in m): " << sum_depth / pixel_count << endl;
        average_depth = sum_depth / pixel_count;

        int BB_mid_height = (coordinatesX[i + 1] + coordinatesX[i + 3]) / 2;
        int BB_mid_width = (coordinatesX[i] + coordinatesX[i + 2]) / 2;
        float height = (depthMat.at<short>(BB_mid_height, BB_mid_width));

        std::vector<float> Point3D; // vector to store each point

        if (average_depth > 1.2 * height)
        {
           i += 4;
           continue;
        } 
        else
        {
            float BB[2]; // center point of bounding box
            BB[0] = BB_mid_width;
            BB[1] = BB_mid_height;
            float point[4]; // 3d position of center point of bounding box, in camera coordinate system

            rs2_deproject_pixel_to_point(point, &depth_intrinsics, BB, height * depth_scale);
            // BB_heights.push_back(height);
            myFile << point[0] << "\n" << point[1] << "\n" << point[2] << "\n";
            point[3] = i/4;

            Point3D.push_back(point[0]);
            Point3D.push_back(point[1]);
            Point3D.push_back(point[2]); // pushing back each coordinate
            Point3D.push_back(point[3]); // to track the sticker number
            points3D.push_back(Point3D); // pushing back the whole point to a vector of points
         
            if ((i + 4) > coordinatesX.size())
                break;
            else
                i += 4;
        }
    }
    myFile.close();
}