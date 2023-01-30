#ifndef __OPENCV_CLASS__
#define __OPENCV_CLASS__

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>

#include <fstream>
#include <iostream>
#include <string>

class AiTask {

  public:
    virtual ~AiTask() = default;
    virtual cv::Mat process_image(cv::Mat &input_image) = 0;
};

class LPR : public AiTask {

  private:
    std::ifstream vocFile_;
    cv::dnn::Net detection_model_;
    cv::dnn::Net net_recognition_model_;
    cv::dnn::TextRecognitionModel recognition_model_;
    std::string path_to_vocabulary_;

  public:
    LPR(const std::string &path_to_vocabulary,
        const std::string &path_to_rec_model,
        const std::string &path_to_dec_model);

    cv::Mat process_image(cv::Mat &input_image) override;

    std::size_t replace_all(std::string &inout, std::string_view what,
                            std::string_view with);
};

#endif
