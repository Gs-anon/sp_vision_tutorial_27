#include "camera.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "hikrobot/include/MvCameraControl.h"

Camera::Camera() {
    // Constructor implementation
    handle = nullptr;
    MV_CC_DEVICE_INFO_LIST device_list;
    ret = MV_CC_EnumDevices(MV_USB_DEVICE, &device_list);
    if (ret != MV_OK) {
        // Handle error
        std::cout << "Error enumerating devices: " << ret << std::endl;
        return;
    }
    if (device_list.nDeviceNum == 0) {
        // Handle no devices found
        std::cout << "No devices found." << std::endl;
        return;
    }
    ret = MV_CC_CreateHandle(&handle, device_list.pDeviceInfo[0]);
    if (ret != MV_OK) {
        // Handle error creating handle
        std::cout << "Error creating handle: " << ret << std::endl;
        return;
    }
    ret = MV_CC_OpenDevice(handle);
    if (ret != MV_OK) {
        // Handle error opening device
        std::cout << "Error opening device: " << ret << std::endl;
        return;
    }

    load_config();
}

void Camera::load_config() {
    // 从配置文件读取相机参数并写入设备
    auto yaml = YAML::LoadFile("configs/yolo.yaml");
    auto cam = yaml["camera"];

    balance_white_auto_ = cam["balance_white_auto"].as<unsigned int>();
    exposure_auto_ = cam["exposure_auto"].as<unsigned int>();
    gain_auto_ = cam["gain_auto"].as<unsigned int>();
    exposure_time_ = cam["exposure_time"].as<float>();
    gain_ = cam["gain"].as<float>();
    frame_rate_ = cam["frame_rate"].as<float>();

    ret = MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", balance_white_auto_);
    if (ret != MV_OK) std::cout << "Failed to set BalanceWhiteAuto: " << ret << std::endl;
    return;
    ret = MV_CC_SetEnumValue(handle, "ExposureAuto", exposure_auto_);
    if (ret != MV_OK) std::cout << "Failed to set ExposureAuto: " << ret << std::endl;
    return;
    ret = MV_CC_SetEnumValue(handle, "GainAuto", gain_auto_);
    if (ret != MV_OK) std::cout << "Failed to set GainAuto: " << ret << std::endl;
    return;
    ret = MV_CC_SetFloatValue(handle, "ExposureTime", exposure_time_);
    if (ret != MV_OK) std::cout << "Failed to set ExposureTime: " << ret << std::endl;
    return;
    ret = MV_CC_SetFloatValue(handle, "Gain", gain_);
    if (ret != MV_OK) std::cout << "Failed to set Gain: " << ret << std::endl;
    return;
    ret = MV_CC_SetFrameRate(handle, frame_rate_);
    if (ret != MV_OK) std::cout << "Failed to set FrameRate: " << ret << std::endl;
    return;
}

Camera::~Camera() {
    // Destructor implementation
    ret = MV_CC_StopGrabbing(handle);
    if (ret != MV_OK) {
        std::cout << "Error stopping grabbing: " << ret << std::endl;
        return;
    }
    ret= MV_CC_CloseDevice(handle);
    if (ret != MV_OK) {
        std::cout << "Error closing device: " << ret << std::endl;
        return;
    }
    ret = MV_CC_DestroyHandle(handle);
    if (ret != MV_OK) {
        std::cout << "Error destroying handle: " << ret << std::endl;
        return;
    }
    
}
cv::Mat Camera::transfer(MV_FRAME_OUT raw) {
    // Implementation for transferring raw frame to cv::Mat
    MV_CC_PIXEL_CONVERT_PARAM cvt_param;
    cv::Mat img(cv::Size(raw.stFrameInfo.nWidth, raw.stFrameInfo.nHeight), CV_8U, raw.pBufAddr);

    cvt_param.nWidth = raw.stFrameInfo.nWidth;
    cvt_param.nHeight = raw.stFrameInfo.nHeight;

    cvt_param.pSrcData = raw.pBufAddr;
    cvt_param.nSrcDataLen = raw.stFrameInfo.nFrameLen;
    cvt_param.enSrcPixelType = raw.stFrameInfo.enPixelType;

    cvt_param.pDstBuffer = img.data;
    cvt_param.nDstBufferSize = img.total() * img.elemSize();
    cvt_param.enDstPixelType = PixelType_Gvsp_BGR8_Packed;

    auto pixel_type = raw.stFrameInfo.enPixelType;
    const static std::unordered_map<MvGvspPixelType, cv::ColorConversionCodes> type_map = {
      {PixelType_Gvsp_BayerGR8, cv::COLOR_BayerGR2RGB},
      {PixelType_Gvsp_BayerRG8, cv::COLOR_BayerRG2RGB},
      {PixelType_Gvsp_BayerGB8, cv::COLOR_BayerGB2RGB},
      {PixelType_Gvsp_BayerBG8, cv::COLOR_BayerBG2RGB}};
    cv::cvtColor(img, img, type_map.at(pixel_type));
    
    return img;
}

void Camera::Camera_read() {
    // Implementation for reading camera data
    ret = MV_CC_StartGrabbing(handle);
    if (ret != MV_OK) {
        std::cout << "Error starting grabbing: " << ret << std::endl;
        return;
    }
    MV_FRAME_OUT raw;
    unsigned int nMsec = 100;
    ret = MV_CC_GetImageBuffer(handle, &raw, nMsec);
    if (ret != MV_OK) {
        std::cout << "Error getting image buffer: " << ret << std::endl;
        return;
    }

    cv::Mat img = transfer(raw);
    cv::imshow("img", img);
    cv::waitKey(0);

    ret = MV_CC_FreeImageBuffer(handle, &raw);
    if (ret != MV_OK) {
        std::cout << "Error freeing image buffer: " << ret << std::endl;
        return;
    }
}
