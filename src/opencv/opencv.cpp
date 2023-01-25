#include "opencv/opencv.hpp"
#include <atomic>
#include <fstream>

// Constants
const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.75;
const float NMS_THRESHOLD = 0.75;
const float CONFIDENCE_THRESHOLD = 0.25;

// Text parameters
const float FONT_SCALE = 0.7;
const int FONT_FACE = cv::FONT_HERSHEY_SIMPLEX;
const int THICKNESS = 1;

// Colors
cv::Scalar BLUE = cv::Scalar(255, 178, 50);

// Class list
std::vector<std::string> class_list = {"licence"};

std::atomic_bool exiting{false};

std::size_t LPR::replace_all(std::string &inout, std::string_view what,
                             std::string_view with) {
    std::size_t count{};
    for (std::string::size_type pos{};
         inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length(), ++count) {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
}

LPR::LPR(std::string path_to_vocabulary, std::string path_to_rec_model,
         std::string path_to_dec_model) {

    std::ifstream vocFile;
    vocFile.open(path_to_vocabulary);
    if (!vocFile.is_open()) {
        throw std::logic_error("ERROR! Didn't find vocabulary file.\n");
    }

    detection_model_ = cv::dnn::readNet(path_to_dec_model);
    if (detection_model_.empty()) {
        throw std::logic_error("ERROR! Detection model error!\n");
    }

    detection_model_.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    detection_model_.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    /* detection_model_.setPreferableBackend(cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
     */
    /* detection_model_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU); */

    net_recognition_model_ = cv::dnn::readNet(path_to_rec_model);
    if (net_recognition_model_.empty()) {
        throw std::logic_error("ERROR! Recognition model error!\n");
    }

    net_recognition_model_.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    net_recognition_model_.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

    recognition_model_ = cv::dnn::TextRecognitionModel(net_recognition_model_);

    std::string vocLine;
    std::vector<std::string> vocabulary;
    while (std::getline(vocFile, vocLine)) {
        vocabulary.push_back(vocLine);
    }

    recognition_model_.setVocabulary(vocabulary);
    recognition_model_.setDecodeType("CTC-greedy");

    // Parameters for Recognition
    double recScale = 1.0 / 127.5;
    cv::Scalar recMean = cv::Scalar(127.5, 127.5, 127.5);
    cv::Size recInputSize = cv::Size(100, 32);
    recognition_model_.setInputParams(recScale, recInputSize, recMean);
};

cv::Mat LPR::process_image(cv::Mat &input_image) {

    cv::Size S = cv::Size(input_image.cols, input_image.rows);

    // Convert to blob.
    cv::Mat blob;

    cv::dnn::blobFromImage(input_image, blob, 1. / 255.,
                           cv::Size(INPUT_WIDTH, INPUT_HEIGHT),
                           cv::Scalar(0, 0, 0), false, false);

    detection_model_.setInput(blob);

    // Forward propagate.
    std::vector<cv::Mat> outputs;
    detection_model_.forward(outputs,
                             detection_model_.getUnconnectedOutLayersNames());

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    std::vector<std::string> results;

    // Resizing factor.
    float x_factor = input_image.cols / INPUT_WIDTH;
    float y_factor = input_image.rows / INPUT_HEIGHT;

    const int rows = outputs[0].size().width;
    const int count_data = 6;

    float *data = (float *)outputs[0].data;
    for (size_t i = 0; i < rows; ++i) {
        float *dataPoint = data + i * 6;
        float confidence = dataPoint[4];
        if (confidence > CONFIDENCE_THRESHOLD) {
            // Center.
            float cx = dataPoint[0];
            float cy = dataPoint[1];
            // Box dimension.
            float w = dataPoint[2];
            float h = dataPoint[3];
            // Bounding box coordinates.
            int left = int((cx - 0.5 * w) * x_factor);
            int top = int((cy - 0.5 * h) * y_factor);
            int width = int(w * x_factor);
            int height = int(h * y_factor);
            confidences.push_back(confidence);
            // Store good detections in the boxes vector.
            boxes.push_back(cv::Rect(left, top, width, height));
        }
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD,
                      indices);
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        cv::Rect box = boxes[idx];
        int left = box.x;
        int top = box.y;
        int width = box.width;
        int height = box.height;

        if (left < 0) {
            left = 0;
        }

        if (top < 0) {
            top = 0;
        }

        if (width < 0 || height < 0) {
            continue;
        }

        if (left + width > input_image.cols) {
            width = input_image.cols - left;
        }

        if (top + height > input_image.rows) {
            height = input_image.rows - top;
        }

        cv::Mat cropp = input_image(cv::Rect(left, top, width, height));
        std::string recognitionResult = recognition_model_.recognize(cropp);

        // std::stringstream name_of_image;
        // name_of_image << "./data/"
        //               << "image_" << std::to_string(num_of_frame) << "_"
        //               << std::to_string(i);
        // std::string path_to_image = name_of_image.str() + ".png";
        // std::string path_to_label = name_of_image.str() + ".txt";

        // cv::imwrite(path_to_image, cropp);
        // write_label_to_file(path_to_label, recognitionResult);

        std::cout << "Recognition result: "
                  << "---" << recognitionResult << "---" << std::endl;
        replace_all(recognitionResult, "0", "O");
        replace_all(recognitionResult, "/", "I");
        replace_all(recognitionResult, "\\", "I");

        if (recognitionResult.size() >= 4) {
            results.emplace_back(recognitionResult);
            cv::rectangle(input_image, cv::Point(left, top),
                          cv::Point(left + width, top + height), BLUE,
                          3 * THICKNESS);
            putText(input_image, recognitionResult, cv::Point(box.x, box.y),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        }
    }

    return input_image;
};