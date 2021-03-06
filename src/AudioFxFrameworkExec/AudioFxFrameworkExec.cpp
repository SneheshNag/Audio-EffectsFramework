#include <iostream>
#include <ctime>
#include <AudioEffectDelay.h>

#include "AudioFileIf.h"
#include "AudioEffectBiquad.h"
#include "AudioEffectGain.h"

using std::cout;
using std::endl;

// local function declarations
void showClInfo ();
void testCases();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioInput = 0,
                            **ppfAudioOutput = 0;

    CAudioFileIf            *phAudioInputFile = 0,
                            *phAudioOutputFile = 0;
    
    CAudioEffectBiquad        *phAudioEffect = 0;
//    CAudioEffectDelay       *phAudioEffectDelay = 0;

    std::fstream             hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments (not sure if needed)
    std::string pname = argv[0];

    if (argc < 5)
    {
        sInputFilePath = "sweep.wav";
        sOutputFilePath = "sweep_post.wav";
    }

    //////////////////////////////////////////////////////////////////////////////
    // Creating all instances and opening input file
    CAudioFileIf::create(phAudioInputFile);
    CAudioFileIf::create(phAudioOutputFile);

    phAudioInputFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioInputFile->isOpen())
    {
        cout << "Wave file open error!";
        return -1;
    }
    phAudioInputFile->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // open the output audio file
    phAudioOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phAudioOutputFile->isOpen()) {
        cout << "Wave file open error!";
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioInput = new float*[stFileSpec.iNumChannels];
    ppfAudioOutput = new float*[stFileSpec.iNumChannels];

    for (int i = 0; i < stFileSpec.iNumChannels; i++){
        ppfAudioInput[i] = new float[kBlockSize];
        ppfAudioOutput[i] = new float[kBlockSize];
    }
    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    // Initializing the  effect
    
    int iNumParams = 3;
    CAudioEffect::EffectParam_t param[iNumParams];
    float value[iNumParams];
    param[0] = CAudioEffect::kParamGain;
    value[0] = 0.5f;
    param[1] = CAudioEffect::kParamCenterFrequency;
    value[1] = 1000.0f;
    param[2] = CAudioEffect::kParamQ;
    value[2] = 0.707f;
    
//    phAudioEffect = new CAudioEffectGain();
//    phAudioEffect->init(stFileSpec.fSampleRateInHz,stFileSpec.iNumChannels,param,value,iNumParams);

    phAudioEffect = new CAudioEffectBiquad(stFileSpec.fSampleRateInHz,stFileSpec.iNumChannels,param,value,iNumParams);
    
    phAudioEffect->setFilterType(CAudioEffectBiquad::FilterType_t::kBandpass);

    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file

    cout << "\r" << "reading and writing" << endl;
    
    while (!phAudioInputFile->isEof())
    {
        long long iNumFrames = kBlockSize;
        phAudioInputFile->readData(ppfAudioInput,iNumFrames);

        // Call to the process function; the effect implementation function
        Error_t error = phAudioEffect->process(ppfAudioInput, ppfAudioOutput, iNumFrames);
        if(error != kNoError){
            return -1;
        }

        phAudioOutputFile->writeData(ppfAudioOutput, iNumFrames);
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioInputFile);
    CAudioFileIf::destroy(phAudioOutputFile);
    
    delete phAudioEffect;
    phAudioEffect = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++){
        delete[] ppfAudioInput[i];
        delete[] ppfAudioOutput[i];
    }

    delete[] ppfAudioInput;
    delete[] ppfAudioOutput;
    ppfAudioInput = 0;
    ppfAudioOutput = 0;

    return 0;
}

void     showClInfo()
{
    cout << "GTCMT MUSI6106 Project - Audio Effects Framework" << endl;
    cout << "(c) 2020-2020 " << endl;
    cout  << endl;

    return;
}
