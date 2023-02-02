#ifndef __OPENCV_CLASS__
#define __OPENCV_CLASS__

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>

#include <fstream>
#include <iostream>
#include <string>

/**
 * Base class for AI. Need be inherited from this class for using AI analytics.
 * */
class AiTask {

  public:
    //! Virtual destructor for inheritance.
    virtual ~AiTask() = default;
    //! Virtual function for processing and prediction.
    /*!
        \param input_image the input cv::Mat image from FFmpegInput decoding.
        Bounding box and text will be drawn to the referense image.
    */
    virtual void process_image(cv::Mat &input_image) = 0;
};

/**
 * License plate detection and recognition.
 * The class opens model for detection license plate and
 * opens model for text recognition by using Nvidia cuda.
 * */
class LPR : public AiTask {

  private:
    //! File destructor for vocabulary file.
    std::ifstream vocFile_;
    //! Net object for detection model.
    cv::dnn::Net detection_model_;
    //! Net object for recognition model.
    cv::dnn::Net net_recognition_model_;
    //! Useful class for text recognition.
    cv::dnn::TextRecognitionModel recognition_model_;
    //! Useful function for replacing some symbols from inout string.
    /*!
        \param inout Input/Output string before and after changing.
        \param what input symbol which will be found.
        \param with input symbol on which will be changed.
        \return Count of changed symbols.
    */
    size_t replace_all(std::string &inout, const std::string_view &what,
                       const std::string_view &with);

  public:
    //! Contructor for License plate recognition.
    /*!
        \param path_to_vocabulary path to vocabulary file with 94 letters on
       different lines. \param path_to_det_model path to image detection model
       in .onnx format. \param path_to_rec_model path to text recognition model
       in .onnx format.
    */
    LPR(const std::string &path_to_vocabulary,
        const std::string &path_to_det_model,
        const std::string &path_to_rec_model);

    //! Override function for processing and prediction.
    /*!
        \param input_image the input cv::Mat image from FFmpegInput decoding.
        Bounding box and text will be drawn to the referense image.
    */
    void process_image(cv::Mat &input_image) override;
};

#endif
