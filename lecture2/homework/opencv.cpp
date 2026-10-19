#include "io/camera.hpp"
#include "tasks/yolo.hpp"
#include "opencv2/opencv.hpp"
#include "tools/img_tools.hpp"
#include "tasks/apriltag_detector.hpp"

//附加题代码
int main()
{
    // 初始化相机、yolo类
    Camera camera;
    auto_charge::AprilTagDetector apriltag_detector("configs/apriltag.yaml");
     while (1) {
        // 调用相机读取图像
        cv::Mat img = camera.Camera_read();
        if (img.empty()) break;;

        // 调用yolo识别opencv标志
        auto detections = apriltag_detector.detect(img);

        // 画出识别结果
        for (auto & detection : detections) {
            tools::draw_points(img, detection.corners, {0, 255, 0});
            std::string info = "ID: " + std::to_string(detection.id);
            tools::draw_text(img, info, detection.center, {0, 255, 0});
        }   
        // 显示图像
        cv::resize(img, img, cv::Size(640, 480));
        cv::imshow("img", img);
        if (cv::waitKey(20) == 'q') {
            break;
        }
    }

    return 0;
}