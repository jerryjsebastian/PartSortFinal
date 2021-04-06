#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rs_advanced_mode.hpp>
#include <librealsense2/rsutil.h>

#include <conio.h>
#include <time.h>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>;
#include <opencv2/imgproc.hpp>

#include "dataStructures.h"
#include "objectDetection2D.hpp"
#include "acquisition.hpp"

#include "uaplatformlayer.h"
#include "sampleclient.h"

using namespace std;
using namespace rs2;
using namespace rs400;
using namespace cv;

#define LOGNAME_FORMAT "%Y-%m-%d_%H:%M:%S"
#define LOGNAME_SIZE 20

std::vector<int> coordinates; // vector holding coordinates of bounding boxes
// std::vector<float> BB_heights; // vector holding heights of bounding boxes

// Helper function for naming/logging of files based on date and time
std::string logfile(void)
{
    static char name[LOGNAME_SIZE];
    time_t now = time(0);
    strftime(name, sizeof(name), LOGNAME_FORMAT, localtime(&now));
    return name;
}

// Helper function for sorting points based on z value, in descending order
bool sortFunc(const vector<float>& p1, const vector<float>& p2)
{
    return p1[2] < p2[2];
}

int main(int argc, char* argv[]) try
{
    UaStatus status;

    // Initialize the UA Stack platform layer
    UaPlatformLayer::init();

    // Create instance of SampleClient
    SampleClient* pMyClient = new SampleClient();

    // Connect to OPC UA Server
    status = pMyClient->connect();

    // Connect succeeded
    if (status.isGood())
    {
        auto start = std::chrono::high_resolution_clock::now();

        /* INIT VARIABLES AND DATA STRUCTURES */
        // data location
        string dataPath = "../";

        // object detection
        string yoloBasePath = dataPath + "dat/yolo/deinSchrank/";
        // string yoloBasePath = dataPath + "dat/yolo/";
        string yoloClassesFile = yoloBasePath + "obj.names";
        string yoloModelConfiguration = yoloBasePath + "yolov3-tiny.cfg";
        string yoloModelWeights = yoloBasePath + "yolov3-tiny_last.weights";

        // misc
        int dataBufferSize = 1;       // no. of images which are held in memory (ring buffer) at the same time
        vector<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time
        bool bVis = true;            // visualize results

        float depth_scale;

        // Declare depth colorizer for pretty visualization of depth data
        // rs2::colorizer color_map;
        // Use black to white color map
        // color_map.set_option(RS2_OPTION_COLOR_SCHEME, 2.f);
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
        string json_file_name = "deinSchrank.json";

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

        // post processing
        // Declare filters
        rs2::decimation_filter dec_filter;  // Decimation - reduces depth frame density
        rs2::spatial_filter spat_filter;    // Spatial    - edge-preserving spatial smoothing
        rs2::temporal_filter temp_filter;   // Temporal   - reduces temporal noise
        rs2::disparity_transform depth_to_disparity(true);  // object which converts from depth image to points
        rs2::disparity_transform disparity_to_depth(false);  // object which converts from points to depth image

        char c;
        while(1)
        {
            std::vector<std::vector<float>> points3D;

            // Skips some frames to allow for auto-exposure stabilization
            for (int i = 0; i < 10; i++) pipe.wait_for_frames();

            // Wait for the next set of frames from the camera
            auto frames = pipe.wait_for_frames();
            frames = align_to.process(frames);
            // auto depth = frames.get_depth_frame().apply_filter(color_map);
            auto depth = frames.get_depth_frame();
            auto color_frame = frames.get_color_frame();

            // set filter parameters and options
            dec_filter.set_option(RS2_OPTION_FILTER_MAGNITUDE, 1.0f);
            spat_filter.set_option(RS2_OPTION_FILTER_MAGNITUDE, 2.0f);
            spat_filter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.5f);
            spat_filter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 20.0f);
            // spat_filter.set_option(RS2_OPTION_HOLES_FILL, 5); // 5 = fill all the zero pixels
            temp_filter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.4f);
            temp_filter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 20.0f);

            // auto filtered = dec_filter.process(depth);
            auto filtered = depth_to_disparity.process(depth);
            filtered = spat_filter.process(filtered);
            filtered = temp_filter.process(filtered);
            filtered = disparity_to_depth.process(filtered);

            rs2_intrinsics intrinsics;
            cv::Mat dMat_colored = cv::Mat(cv::Size(1280, 720), CV_8UC3, (void*)color_frame.get_data());

            /* Histogram equalization test
            //Convert the image from BGR to YCrCb color space
            Mat hist_equalized_image;
            cvtColor(dMat_colored, hist_equalized_image, COLOR_BGR2YCrCb);

            //Split the image into 3 channels; Y, Cr and Cb channels respectively and store it in a std::vector
            vector<Mat> vec_channels;
            split(hist_equalized_image, vec_channels);

            //Equalize the histogram of only the Y channel
            equalizeHist(vec_channels[0], vec_channels[0]);

            //Merge 3 channels in the vector to form the color image in YCrCB color space.
            merge(vec_channels, hist_equalized_image);

            //Convert the histogram equalized image from YCrCb to BGR color space again
            cvtColor(hist_equalized_image, hist_equalized_image, COLOR_YCrCb2BGR);
            cvtColor(hist_equalized_image, hist_equalized_image, COLOR_BGR2RGB);

            cv::imwrite("FinalStack1.png", hist_equalized_image); */

            cv::cvtColor(dMat_colored, dMat_colored, cv::COLOR_BGR2RGB);
            cv::imwrite("FinalStack1.png", dMat_colored);
            save_frame_raw_data("FinalStack1.raw", filtered);
            frame_metadata_to_csv("FinalStack1-Metadata.csv", filtered, intrinsics);

            // push image into data frame buffer
            DataFrame frame;
            frame.cameraImg = dMat_colored;
            // frame.cameraImg = hist_equalized_image;
            dataBuffer.push_back(frame);

            /* DETECT & CLASSIFY OBJECTS */
            float confThreshold = 0.2;
            float nmsThreshold = 0.5;
            detectObjects((dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->boundingBoxes, confThreshold, nmsThreshold,
                yoloBasePath, yoloClassesFile, yoloModelConfiguration, yoloModelWeights, bVis, coordinates);

            cout << "Object Detection done!" << endl;

            /* int X, count = 0;
            while (!myFile.eof())
            {
                myFile >> X;
                coordinates.push_back(X);
                count++;
            } */

            cout << ".." << endl;

            readRAW(filtered, depth_scale, coordinates, intrinsics, points3D);

            cout << ".." << endl;

            // float dist;
            // dist = *min_element(BB_heights.begin(), BB_heights.end());
            // cout << "The highest plane is at a height of: " << dist * depth_scale << " m" << endl;

            sort(points3D.begin(), points3D.end(), sortFunc);
            cout << "The highest plane is at a height of: " << points3D[0][2] << " m" << endl;

            // Print out the post-sorted vector of points
            for (int i = 0; i < points3D.size(); i++)
            {
                cout << "X, Y, Z of Label[" << points3D[i][3] << "] in m: " << points3D[i][0] << " " << points3D[i][1] << " " << points3D[i][2] << " " << endl;
            }

            points3D.clear();

            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            std::cout << "Elapsed time is " << duration.count() << "ms" << endl;
            cout << "Press any key to continue, ESC to exit the program. " << endl;

            BOOL Loop = false;
            while (!Loop)
            {
                status = pMyClient->readLoop(Loop);
            }
        }

    } //End of OPC-UA connection

    delete pMyClient;
    pMyClient = NULL;

    // Cleanup the UA Stack platform layer
    UaPlatformLayer::cleanup();

    return EXIT_SUCCESS;
}
catch (const rs2::error& e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
