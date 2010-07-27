#ifndef AUDIOUTIL_H
#define AUDIOUTIL_H

#include <sndfile.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <vector>

/*!
    \file AudioUtil.h
    \brief AudioUtil header file
 */

#define MAX_CHANNELS 2

using namespace std;

/*!
\brief Provides a number of utilities for pulling useful data from audio files.

This class began as a nice, object-oriented wrapper for certain functions that I found myself frequently using in Erik de Castro Lopo's <a href="http://www.mega-nerd.com/libsndfile/">libsndfile</a>.  It now supports an optional caching scheme (enabled by calling setFileHandlingMode(AudioUtil::FULL_CACHE) on an instance of AudioUtil)  to dramatically speed up the performance of certain functions, like that for accessing arbitrary frames (grabFrame()) of an audio file and that for determining the peak value for a given region of an audio file (peakForRegion()).
*/
class AudioUtil
{

public:
        AudioUtil();
	AudioUtil(string filePath);
        ~AudioUtil();
        bool setFile(string filePath);
        int getNumChannels();
        int getSampleRate();
        int getTotalFrames();
        vector<double> calculateNormalizedPeaks();
        vector<double> grabFrame(int frameIndex);
        vector<double> peakForRegion(int region_start_frame, int region_end_frame);
        vector<double> getAllFrames();
        enum FileHandlingMode {FULL_CACHE, DISK_MODE};
        FileHandlingMode getFileHandlingMode();
        void setFileHandlingMode(FileHandlingMode mode);

private:
        double data [MAX_CHANNELS];
        FileHandlingMode fileHandlingMode;
        string srcFilePath;
        SNDFILE *sndFile;
        SF_INFO *sfinfo;
        bool sndFileNotEmpty;
        vector<double> peaks;
        vector<double> regionPeak;
        vector<double> fileCache;
        int readcount;
        vector<double> dataVector;
        void populateCache();

};

#endif // AUDIOUTIL_H
