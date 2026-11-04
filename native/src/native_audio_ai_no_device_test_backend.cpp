// Linked only by test-native-audio-ai.ps1 -NoDevice. Keep the real host worker,
// clock and synchronization, but deterministically report no waveOut device.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <cstdlib>
extern "C" MMRESULT WINAPI waveOutOpen(LPHWAVEOUT,UINT,LPCWAVEFORMATEX,DWORD_PTR,DWORD_PTR,DWORD) { return MMSYSERR_NODRIVER; }
extern "C" UINT WINAPI waveOutGetNumDevs() { return 0; }
extern "C" MMRESULT WINAPI waveOutPrepareHeader(HWAVEOUT,LPWAVEHDR,UINT) { std::abort(); }
extern "C" MMRESULT WINAPI waveOutWrite(HWAVEOUT,LPWAVEHDR,UINT) { std::abort(); }
extern "C" MMRESULT WINAPI waveOutUnprepareHeader(HWAVEOUT,LPWAVEHDR,UINT) { std::abort(); }
extern "C" MMRESULT WINAPI waveOutReset(HWAVEOUT) { std::abort(); }
extern "C" MMRESULT WINAPI waveOutClose(HWAVEOUT) { std::abort(); }
