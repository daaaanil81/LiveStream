#ifndef __FFMPEG_INPUT_H__
#define __FFMPEG_INPUT_H__

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "ffmpeg/deleters.hpp"

/**
 * Base class for input source.
 */
class FFmpegInput {
  protected:
    //! Pointer on AVFormatContext for controlling input source.
    std::shared_ptr<AVFormatContext> spAVFormatContext_;
    //! Pointer on CodecContext for settings codec parameters.
    std::shared_ptr<AVCodecContext> spCodecContext_;
    //! List of points on packages.
    /*!
     * New packages will be inserted to end. Last
     * packages will be poped from begin.
     */
    std::list<std::shared_ptr<AVPacket>> packet_list_;
    //! Descriptor for thread.
    /*!
     * Read package from format context.
     * Add package to package list in the end.
     */
    std::thread thread_;
    //! Variable for ending package thread.
    std::atomic<bool> active_;
    //! Mutex which controls adding and removing packages from/to package list.
    std::mutex mutex_;
    //! Index of video stream in source stream.
    int video_stream_index_;

    //! Decoge package to cv::Mat.
    /*!
     * Receive AVFrame from AVPacket.
     * Save pts from AVFrame.
     * Convert AVFrame to cv::Mat.
     * \param packet Point on packet.
     * \param pts Origin pts from AVFrame.
     * \return Image after converting
     */
    cv::Mat decode_packet(const std::shared_ptr<AVPacket> &packet,
                          int64_t &pts);

    //! Convert AVFrame to cv::Mat.
    /*!
     * \param pFrame Frame for converting.
     * \return Image after converting
     */
    cv::Mat frame2mat(const std::shared_ptr<AVFrame> &pFrame);

  public:
    //! Return package from begin package list.
    /*!
     * \return First package from package list.
     */
    std::shared_ptr<AVPacket> get();
    //! First ffmpeg frame from package list in cv::Mat.
    /*!
     * Receive first AVPacket from package list.
     * Receive AVFrame from the AVPacket.
     * Convert AVFrame to cv::Mat.
     * Save pts from original AVFrame.
     * \param pts Get from original AVFrame for FFmpegOutput.
     * \return First package from package list.
     */
    cv::Mat get_mat(int64_t &pts);
    //! Check that input stream works.
    /*!
     * \return True, if stream works. False, if stream ended.
     */
    bool stream_status() const;
    //! Thread function for reading packages.
    /*!
     * Read packages from AVFormatContext.
     * Add packages to package list.
     */
    void read_video_stream();
    //! Stop current stream and thread for reading packages.
    void stop_stream();
    //! Get input stream and codec parameters.
    /*!
     * \return Structure with parameters.
     */
    std::shared_ptr<stream_desc_t> get_stream_desc() const;
    //! Virtual destructor for inheritance.
    virtual ~FFmpegInput() = default;
};

/**
 * Class for input stream from file.
 */
class FFmpegInputFile : public FFmpegInput {
  public:
    //! Constructor for streaming from file.
    /*!
     * \param path_to_file Path to video file.
     */
    FFmpegInputFile(const char *path_to_file);
    //! Destructor. Disable thread for saving packet to list.
    ~FFmpegInputFile();
};

/**
 * Class for input stream from webcam device.
 */
class FFmpegInputWebCamera : public FFmpegInput {
  public:
    //! Constructor for streaming from webcam device.
    /*!
     * \param device_name Path to webcam device.
     */
    FFmpegInputWebCamera(const char *device_name);
    //! Destructor. Disable thread for saving packet to list.
    ~FFmpegInputWebCamera();
};

#endif
