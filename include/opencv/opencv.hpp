#ifndef __OPENCV_CLASS__
#define __OPENCV_CLASS__

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

class AiTask {

  public:
    virtual cv::Mat process_image(cv::Mat &input_image) = 0;
};

class LPR : public AiTask {

  private:
    cv::dnn::Net detection_model_;
    cv::dnn::Net net_recognition_model_;
    cv::dnn::TextRecognitionModel recognition_model_;
    std::string path_to_vocabulary_;

  public:
    LPR(std::string path_to_vocabulary, std::string path_to_rec_model,
        std::string path_to_dec_model);

    cv::Mat process_image(cv::Mat &input_image) override;

    std::size_t replace_all(std::string &inout, std::string_view what,
                            std::string_view with);
};

#endif