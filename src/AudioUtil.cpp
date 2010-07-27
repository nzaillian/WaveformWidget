#include "AudioUtil.h"

/*!
\file AudioUtil.cpp
\brief AudioUtil implementation file.
*/

/**
 * \brief Default constructor.
 *
 *  Constructs instance of AudioUtil.  An AudioUtil instance created using this constructor is effectively 
 *  useless until its setFile() function has been invoked with a valid path to a WAV file.  For this reason, you 
 *  may elect to use the alternate AudioUtil constructor, which takes such a path as a parameter. 
 */
AudioUtil::AudioUtil()
{
        this->sfinfo = new SF_INFO;
        this->fileHandlingMode = DISK_MODE;
        sndFileNotEmpty = false;
}

/**
 *  \brief Alternate, single-parameter constructor.
 *
 * Alternate constructor.  An instance of AudioUtil created using this constructor does not need to have 
 * its setFile() function explicitly invoked to be useful.
 * 
 * @param filePath path to a WAV file
*/
AudioUtil::AudioUtil(string filePath)
{
        this->sfinfo = new SF_INFO;
        this->fileHandlingMode = DISK_MODE;
        sndFileNotEmpty = false;
	this->setFile(filePath);
}


/*
 * Clean up heap-allocated objects.
 */
AudioUtil::~AudioUtil()
{
    if(sndFileNotEmpty == true)
    {
        sf_close(this->sndFile);
    }
    delete sfinfo;
}

/**
 *\brief The mutator for the file-handling mode of an instance of AudioUtil.
 *
 *  AudioUtil objects can function in one of two modes: \link AudioUtil::DISK_MODE \endlink and \link AudioUtil::FULL_CACHE 
 *  \endlink mode.  The default mode for AudioUtil objects is DISK_MODE.  In DISK_MODE, an instance of 
 *  AudioUtil will dynamically load a region of the audio file it wraps from disk 
 *  into memory when asked to analyze or return this region (when the peakForRegion, getAllFrames and 
 *  grabFrame function are invoked, for example).  This keeps memory use minimal, but has an immense
 *  consequence with respect to performance.  Upon being set to FULL_CACHE mode, an AudioUtil instance will
 *  load the entire audio file that it wraps into memory (storing it as a vector of double-precision floating
 *  point values), and use this cached data to perform the operations that, in DISK_MODE, require that 
 *  data be loaded dynamically from disk for processing.  In FULL_CACHE mode, you get drastically increased performance, but 
 *  pay a penalty in increased memory consumption.
 * 
 *  @param mode  file-handling scheme for the AudioUtil instance.  Valid options: \link AudioUtil::DISK_MODE \endlink, \link 
 *  AudioUtil::FULL_CACHE \endlink
 */
void AudioUtil::setFileHandlingMode(FileHandlingMode mode)
{
    this->fileHandlingMode = mode;
    if(mode == FULL_CACHE)
    {
        this->fileCache.clear();
        this->populateCache();
    }
}

/**
 * \brief Accessor for an instance of AudioUtil's file-handling mode
 * @return the mode of this AudioUtil instance
 */
AudioUtil::FileHandlingMode AudioUtil::getFileHandlingMode()
{
    return this->fileHandlingMode;
}


/**
  * \brief Sets the file to be wrapped by an instance of AudioUtil.
  *
  *  Set the audio file to be wrapped by an instance of AudioUtil.  This function must be passed a string
  *  representing a valid path to a WAV file.  In the case that it is not given such a string, it will 
  *  print an error message and return false.  Otherwise, it will return true.
  *
  *  @param filePath a string representing a valid path to a WAV file.
  *  @return true if file was successfully set, false otherwise.  
  */
bool AudioUtil::setFile(string filePath)
{

    if(sndFileNotEmpty == true)
    {
        sf_close(this->sndFile);
    }
    this->sfinfo->format=0;
        if (! (this->sndFile = sf_open (filePath.c_str(), SFM_READ, this->sfinfo)))
        {
                /* Open failed so print an error message. */
                fprintf (stderr, "failed to open input file \"%s\".\n", filePath.c_str()) ;
                /* Print the error message fron libsndfile. */
                sf_perror (NULL) ;
                return false;
        };

        /* turn on normalization  */
        sf_command (this->sndFile, SFC_SET_NORM_DOUBLE, NULL, SF_TRUE) ;

        /*channel num check! */
        if (this->sfinfo->channels > MAX_CHANNELS)
        {
            fprintf (stderr, "Error.  Input has too many channels.  Maximum channels: %d channels\n", MAX_CHANNELS) ;
            return false;
        };

        if(this->fileHandlingMode == FULL_CACHE)
        {
            this->fileCache.clear();
            this->fileCache = this->getAllFrames();
        }

        this->sndFileNotEmpty = true;
	return true;
}

/**
 * \brief Calculates peak values for the normalized audio data of the audio file wrapped by an instance of AudioUtil.
 *
 *
 * @return a vector containing the peak for each channel of the audio file wrapped by an instance of AudioUtil.
 * in the case of an error, an empty vector is returned.
 */
vector<double> AudioUtil::calculateNormalizedPeaks()
{
        this->peaks.clear();

        double *peaksPtr = (double *) malloc(2*sizeof(double));

        sf_command (sndFile, SFC_CALC_NORM_MAX_ALL_CHANNELS, peaksPtr, sizeof(double)*this->getNumChannels()) ;

        if(this->getNumChannels() == 2)
        {
               this->peaks.push_back(peaksPtr[0]);
               this->peaks.push_back((peaksPtr[1]));
        }
        else if (this->getNumChannels() == 1)
        {
            this->peaks.push_back(peaksPtr[0]);
        } else
	{
		perror("Error in AudioUtil::calculateNormalizedPeaks()\n");
	}

        free(peaksPtr);

        return peaks;
}


/**
 * \brief The number of channels of the wrapped audio file.
 *
 *  @return the number of channels in the audio file wrapped by an instance of AudioUtil 
 */
int AudioUtil::getNumChannels()
{
        if (sfinfo != NULL)
        {
                return this->sfinfo->channels;
        }
        else
        {
                return NULL;
        }
}

/**
 * \brief The sample rate of the wrapped audio file.
 * 
 *  @return the sample rate of the audio file wrapped by an instance of AudioUtil
 */
int AudioUtil::getSampleRate()
{
        return this->sfinfo->samplerate;
}


/*!
\brief The total number of frames of the wrapped audio file.
@return the number of frames of the wrapped audio file.
*/
int AudioUtil::getTotalFrames()
{
    if (sfinfo != NULL)
    {
            return this->sfinfo->frames;
    }
    else
    {
            return NULL;
    }
}


/** 
 * \brief Get a given frame of the wrapped audio file.
 *
 * Function to get the frame at a given index in the audio file wrapped by an instance of AudioUtil. 
 * Will return a double-precision floating point vector of dimension equal to the number_of_channels in the wrapped file 
 * (either 1 or 2) containing the data of the requested frame.  In the case that a frame is requested which is out of 
 * bounds, an error message will be printed and an empty vector returned.
 *
 * @param frameIndex The desired frame. 
 *
 * @return A vector of double-precision floating point values representing the contents of the requested frame.  In 
 * the case that an out-of-bounds frame is requested, an empty vector will be returned. 
 */
vector<double> AudioUtil::grabFrame(int frameIndex)
{
    vector<double> frameData;

    if(this->fileHandlingMode == FULL_CACHE)
    {
        if(this->getNumChannels() == 1)
        {
            if(frameIndex >= (int) this->fileCache.size())
            {
                perror("err in AudioUtil::grabFrame -- caller attempting to access out-of-range frame\n");
                return frameData;
            }

            else
            {
                frameData.push_back(this->fileCache.at(frameIndex));
            }
        }

        if(this->getNumChannels() == 2)
        {
            if(2*frameIndex+1 >= (int) this->fileCache.size())
            {
                perror("err in AudioUtil::grabFrame -- caller attempting to access out-of-range frame\n");
                return frameData;
            }
            else
            {
                frameData.push_back(this->fileCache.at(2*frameIndex));
                frameData.push_back(this->fileCache.at(2*frameIndex+1));
            }

        }

    }

    if(this->fileHandlingMode == DISK_MODE)
    {
        if (sf_seek(sndFile, frameIndex, SEEK_SET) == -1)
        {
            perror("seek error in AudioUtil::grabFrame\n");
            return frameData;
        }
        else
        {
                if(sf_readf_double(sndFile, data, 1) == 0)
                {
                    perror("file read error in AudioUtil::grabFrame\n");
                    return frameData;

                }
                else
                {
                    if(this->getNumChannels() == 2)
                    {
                        frameData.push_back(data[0]);
                        frameData.push_back(data[1]);
                        return frameData;
                    }
                    if(this->getNumChannels() == 1)
                    {
                        frameData.push_back(data[0]);
                        return frameData;
                    }
                }
        }
    }
    //we should never find ourselves here, but as a precaution...
    perror("invalid mode selected for AudioUtil instance\n");
    return frameData;


}

/** 
 *\brief Peak for a given region of the wrapped audio file.
 *
 * Function to get the peak for each channel of a given region of the audio file wrapped by an instance of AudioUtil.
 * Returns a vector of double-precision floating point values representing this data.  In the case that
 * an invalid region is specified by the function's parameters, prints an error message and returns an 
 * empty vector.  This function performs drastically better when the AudioUtil instance it is being 
 * invoked with is operating in FULL_CACHE mode rather than DISK_MODE.  
 * 
 * @param region_start_frame The frame marking the beginning of the region to be analyzed
 * @param region_end_frame The frame marking the end of the region to be analyzed
 * @return A vector of double-precision floating point values representing the peak for each channel of the specified region 
 * of the audio file wrapped by an instance of AudioUtil.  In the case that an invalid region has been specified, return 
 * value is an empty vector.
 */
vector<double> AudioUtil::peakForRegion(int region_start_frame, int region_end_frame)
{
 
    int numChannels = this->getNumChannels();

    if(this->fileHandlingMode == FULL_CACHE)
    {
        if(numChannels == 2)
        {
            this->regionPeak.clear();

            double max0 = 0.0;
            double max1 = 0.0;

             for (int i = 2*region_start_frame; i <  2*region_end_frame; i+=2)
             {
                  if(fabs(fileCache[i]) > fabs(max0))
                {
                  max0 = fileCache[i];
                }
                if(fabs(fileCache[i+1]) > fabs(max1))
                {
                    max1 = fileCache[i+1];
                }

            }

            this->regionPeak.push_back(max0);
            this->regionPeak.push_back(max1);

           return regionPeak;
       }

        if(numChannels == 1)
        {
            this->regionPeak.clear();

            double max0 = 0.0;

             for (int i = region_start_frame; i <  region_end_frame; i++)
             {
                if(fabs(fileCache[i]) > fabs(max0))
                {
                  max0 = fileCache[i];
                }
            }

           this->regionPeak.push_back(max0);

           return this->regionPeak;
       }

    }
    if(this->fileHandlingMode == DISK_MODE)
    {
        if(numChannels == 2)
        {
            this->regionPeak.clear();
            double *chunk = new double[ 2*(region_end_frame - region_start_frame)];
             //seek to region start
            if (sf_seek(sndFile, region_start_frame, SEEK_SET) == -1)
            {
                 perror("seek error in AudioUtil::peakForRegion function\n");
                 return this->regionPeak;
            }
            /*read the region into an array*/
            if(sf_readf_double(sndFile, chunk, (region_end_frame - region_start_frame) ) == 0)
            {
                perror("read error in AudioUtil::peakForRegion function\n");
                return this->regionPeak;
            }

            double max0 = 0.0;
            double max1 = 0.0;

             for (int i = 0; i <  2*(region_end_frame - region_start_frame); i+=2)
             {
                  if(fabs(chunk[i]) > fabs(max0))
                {
                  max0 = chunk[i];
                }
                if(fabs(chunk[i+1]) > fabs(max1))
                {
                    max1 = (chunk[i+1]);
                }

            }

            this->regionPeak.push_back(max0);
            this->regionPeak.push_back(max1);

           delete[] chunk;

           return regionPeak;
       }


        if(numChannels == 1)
        {
            this->regionPeak.clear();
            double *chunk = new double[ region_end_frame - region_start_frame];
             //seek to region start
            if (sf_seek(sndFile, region_start_frame, SEEK_SET) == -1)
            {
                perror("seek error in AudioUtil::peakForRegion function\n");
                return this->regionPeak;
            }
            /*read the region into an array*/
            if(sf_readf_double(sndFile, chunk, (region_end_frame - region_start_frame) ) == 0)
            {
                perror("read error in AudioUtil::peakForRegion function\n");
                return this->regionPeak;
            }

            double max0 = 0.0;

             for (int i = 0; i <  region_end_frame - region_start_frame; i++)
             {
                if(fabs(chunk[i]) > max0)
                {
                  max0 = fabs(chunk[i]);
                }
            }

           this->regionPeak.push_back(max0);

           delete[] chunk;

           return this->regionPeak;
       }
   }
   perror("err in AudioUtil::peakForRegion function.  Max channels: 2\n");
   return this->regionPeak;
}


/**
 * \brief The content of the wrapped audio file.
 *
 * Function to get all frames of the audio file wrapped by an instance of AudioUtil as a vector of double-precision 
 * floating-point values.
 *
 * @return a vector of double-precision floating-point values representing all frames of the audio file wrapped by this
 * instance of AudioUtil.
 */
vector<double> AudioUtil::getAllFrames()
{

   if(this->fileHandlingMode == FULL_CACHE)
   {
      return this->fileCache;
   }
   else
   {
       this->dataVector.clear();
       int readSize = 1024;

       //seek to file start
      if (sf_seek(sndFile, 0, SEEK_SET) == -1)
      {
          fprintf(stderr, "seek failed in AudioUtil::getAllFrames() function\n");
      }
       double *chunk = new double[readSize];
       int itemsRead = sf_read_double(this->sndFile, chunk, readSize);

       while(itemsRead == readSize)
       {

           for(int i = 0; i < readSize; i++)
           {
                this->dataVector.push_back(chunk[i]);
           }

           itemsRead = sf_read_double(this->sndFile, chunk, readSize);
       }

       //add the last items to the vector
       for(int i = 0; i < itemsRead; i++)
       {
            this->dataVector.push_back(chunk[i]);
       }

       delete[] chunk;

       return this->dataVector;
   }
}


/**
 * For internal use only!!!  Function populates the fileCache vector with the contents of the audio file wrapped by this 
 * instance of AudioUtil.
 */
void AudioUtil::populateCache()
{
    this->fileCache.clear();
    int readSize = 1024;

    //seek to file start
   if (sf_seek(sndFile, 0, SEEK_SET) == -1)
   {
       fprintf(stderr, "seek failed in AudioUtil::getAllFrames() function\n");
   }

    double *chunk = new double[readSize];
    int itemsRead = sf_read_double(this->sndFile, chunk, readSize);

    while(itemsRead == readSize)
    {

        for(int i = 0; i < readSize; i++)
        {
             this->fileCache.push_back(chunk[i]);
        }

        itemsRead = sf_read_double(this->sndFile, chunk, readSize);
    }

    //add the last items to the vector
    for(int i = 0; i < itemsRead; i++)
    {
         this->fileCache.push_back(chunk[i]);
    }

    delete[] chunk;

}
