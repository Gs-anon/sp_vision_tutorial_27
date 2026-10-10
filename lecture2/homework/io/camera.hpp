#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "hikrobot/include/MvCameraControl.h"
class Camera {
public:
    Camera();
    ~Camera();
    void Camera_read();
private:
    void * handle;
    int ret;
    cv::Mat transfer(MV_FRAME_OUT raw);
    void load_config();
    // 相机参数
    unsigned int balance_white_auto_;
    unsigned int exposure_auto_;
    unsigned int gain_auto_;
    float exposure_time_;
    float gain_;
    float frame_rate_;
};


#endif // CAMERA_HPP