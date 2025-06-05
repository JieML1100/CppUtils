#pragma once
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>

#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <Codecapi.h>
#include <strmif.h>
#include <vector>
#include <string>
#include <Mmdeviceapi.h>
#include <Audioclient.h>
#include <iostream>
#include <wincodec.h>
#include <mftransform.h>
#include <wmcodecdsp.h>

typedef struct MixedFrame
{
	std::vector<float> Data;
	LONGLONG Duration;
	LONGLONG Time;
} MixedFrame;
class AudioReader
{
public:
	AudioReader(const std::wstring& filePath);
	AudioReader(const std::vector<BYTE>& fileBuffer);
	AudioReader(IMFMediaSource* mediaSource);
	~AudioReader();
	IMFMediaType* GetCurrentMediaType(UINT32 index);
	HRESULT GetSourceInfomation();
	IMFSample* ReadNextSample();
	HRESULT SetPosition(LONGLONG timeStamp);
	double LenghtOfSeconds() const;
	IMFMediaType* GetNativeMediaType(UINT32 index);
	HRESULT SetCurrentMediaType(DWORD dwStreamIndex, DWORD* pdwReserved, IMFMediaType* pMediaType);
	std::vector<MixedFrame> ReadAudioFrames(IMFMediaType* targetMediaType);
	UINT64 FileSize;
	UINT64 Duration;
	UINT32 EncodingBitRate;
	UINT32 ChannelCount;
	UINT32 SamplesPerSecond;
	UINT32 AverageBytesPerSecond;
	UINT32 AudioBlockAlignment;
	UINT32 AudioBitsPerSample;
	GUID MediaType;
	GUID SubType;

private:
	std::wstring audioFilename;
	IMFSourceReader* m_pSourceReader;
	IMFByteStream* pByteStream = NULL;
};

class AudioResampler
{
public:
	AudioResampler(IMFMediaType* pInputType, IMFMediaType* pOutputType);
	~AudioResampler();
	HRESULT Resample(IMFSample* pInputSample, IMFSample** ppOutputSample);

private:
	IMFTransform* m_pResampler;
};
class AudioMix
{
public:
	static std::vector<MixedFrame> MixAudio(AudioReader* first, float firstVolume, AudioReader* second, float secondVolume, bool clipToMainAudio = false, bool loopMode = false);
	static std::vector<MixedFrame> MixAudio(AudioReader* first, float firstVolume, const std::vector<AudioReader*>& secondList, float secondVolume, bool clipToMainAudio = false, bool loopMode = false);
	static std::vector<MixedFrame> MixAudio(double samplesPerSecond, std::vector<MixedFrame> framesFirst, float firstVolume, std::vector<MixedFrame> framesSecond, float secondVolume, bool clipToMainAudio = false, bool loopMode = false);
};
class H264Encoder
{
private:
	IMFSample* pVideoSample = NULL;
	std::wstring m_DestFilename;
	int m_Width = 0;
	int m_Height = 0;
	LONG m_cbWidth = 0;
	DWORD m_cbBuffer = 0;
	IMFMediaBuffer* m_pBuffer = NULL;
	IMFSinkWriter* m_pSinkWriter = NULL;
	UINT32 m_VideoBitrate = 4000000;
	UINT32 m_VideoFPS = 25;
	UINT64 m_FrameDuration = 0;
	UINT32 m_FrameIndex = 0;
	int m_Quality = 100;
public:
	static bool WMFLIB_INITED;
	DWORD AudioStreamIndex = 0;
	DWORD VideoStreamIndex = 0;
	LONGLONG AudioTimeStamp = 0;
	LONGLONG VideoTimeStamp = 0;
public:
	H264Encoder(std::wstring destFilename, UINT32 width, UINT32 height, UINT32 fps = 25, UINT32 videoBitrate = 4000000);
	~H264Encoder();
	HRESULT AddVideoFrame(IWICBitmap* buffer, LONGLONG sampleTime = 0, LONGLONG duration = 0);
	HRESULT AddVideoFrame(BYTE* buffer, LONGLONG sampleTime = 0, LONGLONG duration = 0);
	HRESULT AddAudioFrame(IMFSample* pSample);
	HRESULT AddAudioFrame(PVOID buffer, DWORD bufferSize, LONGLONG rameDuration);
	HRESULT TickAudioFrame(LONGLONG sampleTime);
	HRESULT EndAudio();
	HRESULT End();
	HRESULT ConfigAudioStream();
	HRESULT SetCurrentAudioType(IMFMediaType* mediaType);;
	HRESULT InitializeWriter();
	HRESULT Begin();
	HRESULT SetVideoOutputType(IMFMediaType** pMediaTypeOut);
	HRESULT SetVideoInputType(IMFMediaType** pMediaTypeIn);
	HRESULT InitTranscodeVideoType(CComPtr<IMFMediaType>& pStreamMediaType);
	double CurrentTimeOfSeconds();
	UINT64 GetTimeStamp();
	UINT64 GetTimeStampForIndex(UINT32 index);
};

class AudioCapture
{
public:
	AudioCapture();
	~AudioCapture();
	HRESULT Start();
	HRESULT Stop();
	std::vector<uint8_t> Capture(UINT32* packetLength, UINT32* numFramesAvailable, DWORD* dwFlags);
	HRESULT Capture(BYTE* buffer, UINT32* bufferSize, LONGLONG* duration);
	UINT32 BytesPerFrame();
	DWORD Channels();
	DWORD SamplesPerSecond();
	IMFMediaType* GetMediaType();
private:
	WAVEFORMATEX* pwfx = NULL;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
};