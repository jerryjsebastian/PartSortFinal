#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rs_advanced_mode.hpp>
#include <librealsense2/rsutil.h>

#include <conio.h>
#include <time.h>
#include <chrono>
#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>;
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace rs2;
using namespace rs400;
using namespace cv;

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
    auto image = frame.as<video_frame>();
    if (image)
    {
        /*std::ofstream csv(filename);

        auto profile = image.get_profile();
        csv << "Frame Info: " << std::endl << "Type," << profile.stream_name() << std::endl;
        csv << "Format," << rs2_format_to_string(profile.format()) << std::endl;
        csv << "Frame Number," << image.get_frame_number() << std::endl;
        csv << "Timestamp (ms)," << std::fixed << std::setprecision(2) << image.get_timestamp() << std::endl;
        csv << "Resolution x," << (int)image.get_width() << std::endl;
        csv << "Resolution y," << (int)image.get_height() << std::endl;
        csv << "Bytes per pixel," << (int)image.get_bytes_per_pixel() << std::endl;*/

        /* if (auto vsp = profile.as<video_stream_profile>())
        {
            intrinsics = vsp.get_intrinsics();
            csv << std::endl << "Intrinsic:," << std::fixed << std::setprecision(6) << std::endl;
            csv << "Fx," << intrinsics.fx << std::endl;
            csv << "Fy," << intrinsics.fy << std::endl;
            csv << "PPx," << intrinsics.ppx << std::endl;
            csv << "PPy," << intrinsics.ppy << std::endl;
            csv << "Distortion," << rs2_distortion_to_string(intrinsics.model) << std::endl;
        } */

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

//void readRAW()
//{
//    auto start = std::chrono::high_resolution_clock::now();
//    int IMAGE_WIDTH = 1280, IMAGE_HEIGHT = 720;
//    FILE* fp; char* imagedata; int framesize = IMAGE_WIDTH * IMAGE_HEIGHT;
//    fp = fopen("FinalStack1.raw", "rb");
//    imagedata = (char*)malloc(sizeof(short) * framesize);
//    fread(imagedata, sizeof(short), framesize, fp);
//    cv::Mat depthMat(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_16U, (void*)imagedata, cv::Mat::AUTO_STEP);
//    std::vector<short> AllDepths;
//    double sum = 0;
//    for(int i = 0; i < IMAGE_WIDTH; i++)
//    {
//        for (int j = 0; j < IMAGE_HEIGHT; j++)
//        {
//            float height = (depthMat.at<short>(j, i));
//            // sum += height;
//            AllDepths.push_back(height);
//        }
//    }
//    std::sort(AllDepths.begin(), AllDepths.end(), std::greater<>());
//    for (int i = 0; i < 9216; i++)
//    {
//        std::cout << AllDepths.at(i) << std::endl;
//        sum += AllDepths.at(i);
//    }
//    std::cout << (sum / 9216) << " " << AllDepths.at(0) << std::endl;
//    auto stop = std::chrono::high_resolution_clock::now();
//    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
//    std::cout << "Elapsed time is " << (duration.count()) / 1000 << "s" << std::endl;
//    fclose(fp);
//    free(imagedata);
//}

float height;

void readRAW()
{
    auto start = std::chrono::high_resolution_clock::now();
    int IMAGE_WIDTH = 1280, IMAGE_HEIGHT = 720;
    FILE* fp; char* imagedata; int framesize = IMAGE_WIDTH * IMAGE_HEIGHT;
    fp = fopen("FinalStack1.raw", "rb");
    imagedata = (char*)malloc(sizeof(short) * framesize);
    fread(imagedata, sizeof(short), framesize, fp);
    cv::Mat depthMat(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_16U, (void*)imagedata, cv::Mat::AUTO_STEP);

    height = (depthMat.at<short>(631, 389));

    fclose(fp);
    free(imagedata);
}

int main()
{

    float depth_scale;

    rs2::pipeline pipe;
    //create context
    rs2::context ctx;
    //get device
    auto devices = ctx.query_devices();
    rs2::device dev = devices[0];

    config cfg;
    cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_RGB8, 30);

    string serial = dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
    string json_file_name = "DefaultPresetD435.json";

    cout << "Configuring camera : " << serial << endl;

    auto advanced_mode_dev = dev.as<rs400::advanced_mode>();

    // Check if advanced-mode is enabled to pass the custom config
    if (!advanced_mode_dev.is_enabled())
    {
        // If not, enable advanced-mode
        advanced_mode_dev.toggle_advanced_mode(true);
        cout << "Advanced mode enabled. " << endl;
    }

    std::ifstream t(json_file_name);
    std::string preset_json((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    advanced_mode_dev.load_json(preset_json);
    cfg.enable_device(serial);
    pipe.start(cfg);

    rs2::align align_to(RS2_STREAM_COLOR);

    for (rs2::sensor& sensor : dev.query_sensors())
    {
        if (rs2::depth_sensor dpt = sensor.as<rs2::depth_sensor>())
        {
            // Depth scale is needed for the kinfu set-up
            depth_scale = dpt.get_depth_scale();
            break;
        }
    }
    cout << "Depth Scale: " << depth_scale << endl;

    // post processing filters
    rs2::spatial_filter spat_filter;    // Spatial    - edge-preserving spatial smoothing
    rs2::temporal_filter temp_filter;   // Temporal   - reduces temporal noise
    rs2::disparity_transform depth_to_disparity(true);  // object which converts from depth image to points
    rs2::disparity_transform disparity_to_depth(false);  // object which converts from points to depth image

    // Skips some frames to allow for auto-exposure stabilization
    for (int i = 0; i < 10; i++) pipe.wait_for_frames();

    // Wait for the next set of frames from the camera
    auto frames = pipe.wait_for_frames();
    frames = align_to.process(frames);
    auto depth = frames.get_depth_frame();
    auto color_frame = frames.get_color_frame();

    // set filter parameters and options
    spat_filter.set_option(RS2_OPTION_FILTER_MAGNITUDE, 2.0f);
    spat_filter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.5f);
    spat_filter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 20.0f);
    temp_filter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.4f);
    temp_filter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 20.0f);

    // auto filtered = dec_filter.process(depth);
    auto filtered = depth_to_disparity.process(depth);
    filtered = spat_filter.process(filtered);
    filtered = temp_filter.process(filtered);
    filtered = disparity_to_depth.process(filtered);

    rs2_intrinsics intrinsics;
    cv::Mat dMat_colored = cv::Mat(cv::Size(1280, 720), CV_8UC3, (void*)color_frame.get_data());
    cv::cvtColor(dMat_colored, dMat_colored, cv::COLOR_BGR2RGB);
    cv::imwrite("FinalStack1.png", dMat_colored);
    save_frame_raw_data("FinalStack1.raw", filtered);
    frame_metadata_to_csv("FinalStack1-Metadata.csv", filtered, intrinsics);

    readRAW();

    float point[3];
    float loc[2];
    loc[0] = 631;
    loc[1] = 389;
    rs2_deproject_pixel_to_point(point, &intrinsics, loc, height * depth_scale);

    cout << "The point coordinates in camera coordinate system is: X = " << point[0] << ", Y = " << point[1] << ", Z = " << point[2] << endl;

    return 0;
}