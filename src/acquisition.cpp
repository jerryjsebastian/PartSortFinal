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
        // std::ofstream csv(filename);

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

        //Intrinsics for color (1280x720) for d435
        intrinsics.fx = 916.61767578125;
        intrinsics.fy = 916.450134277344;
        intrinsics.height = 720;
        intrinsics.ppx = 647.564331054688;
        intrinsics.ppy = 375.717926025391;
        intrinsics.width = 1280;
        intrinsics.model = RS2_DISTORTION_INVERSE_BROWN_CONRADY;
        intrinsics.coeffs[0] = 0;
        intrinsics.coeffs[1] = 0;
        intrinsics.coeffs[2] = 0;
        intrinsics.coeffs[3] = 0;
        intrinsics.coeffs[4] = 0;

        ret = true;
    }

    return ret;
}

void readRAW(rs2::frame FRAME, float depth_scale, std::vector<int> coordinatesX, const rs2_intrinsics& depth_intrinsics, 
             std::vector<std::vector<int32_t>>& points3D, int cam)
{
    std::ofstream myFile("X-Y-Z.csv", std::ios_base::trunc);

    int IMAGE_WIDTH = 1280, IMAGE_HEIGHT = 720;
    cv::Mat depthMat(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_16U, (void*)FRAME.get_data());
    
    std::vector<short> AllDepths; // to calculate the average of top 1% of heights in the depth image
    double sum_AllDepths = 0;
    for (int k = 0; k < IMAGE_WIDTH; k++)
    {
        for (int j = 0; j < IMAGE_HEIGHT; j++)
        {
            float height = (depthMat.at<short>(j, k));
            AllDepths.push_back(height);
        }
    }
    std::sort(AllDepths.begin(), AllDepths.end(), std::greater<>()); //sorting in descending order
    for (int l = 0; l < (0.01 * IMAGE_WIDTH * IMAGE_HEIGHT); l++)
    {
        sum_AllDepths += AllDepths.at(l);
    }
    double avg_AllDepths = sum_AllDepths / (0.01 * IMAGE_WIDTH * IMAGE_HEIGHT);

    int i = 0;
    while (i < coordinatesX.size())
    {
        float sum_depth = 0, average_depth = 0; // to calculate average depth over each barcode sticker area
        int pixel_count = 0;
        for (int k = coordinatesX[i]; k <= coordinatesX[i+2]; k++)
        {
            for (int j = coordinatesX[i + 1]; j <= coordinatesX[i + 3]; j++)
            {
                sum_depth += depthMat.at<short>(j, k);
                pixel_count++;
            }
        }
        average_depth = sum_depth / pixel_count;

        // Midpoint of bounding box
        int BB_mid_height = (coordinatesX[i + 1] + coordinatesX[i + 3]) / 2;
        int BB_mid_width = (coordinatesX[i] + coordinatesX[i + 2]) / 2;
        // Width and height of bounding box
        float BB_height = abs(coordinatesX[i + 1] - coordinatesX[i + 3]);
        float BB_width = abs(coordinatesX[i] - coordinatesX[i + 2]);
        float height = (depthMat.at<short>(BB_mid_height, BB_mid_width));

        std::vector<int32_t> Point3D; // vector to store each point

        //if ((avg_AllDepths - height) >= 18) // extracting positions only if the highest plane
        //{
        //    i += 4;
        //    continue;
        //}
        if (average_depth > 1.2 * height) // eliminating some of the half-stickers
        {
           i += 4;
           continue;
        } 
        else
        {
            float BB[2]; // center point of bounding box
            BB[0] = BB_mid_width;
            BB[1] = BB_mid_height;
            float point[5]; // 3d position of center point of bounding box, in camera coordinate system (X,Y,Z)
                            // 4th element is the bounding box number
                            // 5th element is the orientation 0 for horizontal, 1 for vertical

            rs2_deproject_pixel_to_point(point, &depth_intrinsics, BB, height * depth_scale);

            if (BB_width > BB_height)
                point[4] = 0;
            else
                point[4] = 1;

            // Saving in camera coordinate system
            myFile << point[0] << "\n" << point[1] << "\n" << point[2] << "\n" << point[4] << "\n";
            point[3] = i/4;

            //// transformation from camera coordinate system to world coordinate system
            //if (cam == 0)
            //{
            //    // Camera at site 31
            //    point[0] += 0; 
            //    point[0] *= -1; // X coordinate
            //    point[1] += 0; // Y coordinate
            //    point[2] -= 0;
            //    point[2] *= -1; // Z coordinate
            //}
            //else if  (cam == 1)
            //{
            //    // Camera at site 32
            //    point[0] += 0;
            //    point[1] += 0;
            //    point[1] *= -1;
            //    point[2] -= 0;
            //    point[2] *= -1;
            //}

            if (point[0] > 0)

            {
                point[0] += 0.036;
                point[0] *= -1;
            }
            else
            {
                point[0] *= -1;
                point[0] -= 0.036;
            }
            point[1] -= 0.030; // 10 to 15 cm only
            point[2] -= 2.155;
            point[2] *= -1;

            Point3D.push_back(point[0]*1000);
            Point3D.push_back(point[1]*1000);
            Point3D.push_back(point[2]*1000); // pushing back each coordinate, *1000 is for converting from m to mm
            Point3D.push_back(point[3]); // to track the sticker number
            Point3D.push_back(point[4]); // to track the orientation of sticker
            points3D.push_back(Point3D); // pushing back the whole point to a vector of points
         
            if ((i + 4) > coordinatesX.size())
                break;
            else
                i += 4;
        }
    }
    myFile.close();
}