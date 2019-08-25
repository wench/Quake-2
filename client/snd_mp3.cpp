extern "C" {
#include "client.h"
#include "snd_loc.h"
#include "mpglib/mpglib.h"
}
#include <Windows.h>
char *mp3data = 0;
int mp3_size = 0;
mpstr mp = { 0 };
qboolean mp3loaded = qfalse;
int samples_size = 0;
char samples[MAX_RAW_SAMPLES*4 + 4608];
HANDLE hExit=0; // tell thread to exit
HANDLE hMore = 0;
int decodethread(LPVOID data);
static const int freqs_mp3[9] = { 44100, 48000, 32000,
				  22050, 24000, 16000 ,
				  11025 , 12000 , 8000 };

qboolean S_PlayMp3Music(char*filename)
{
	__try
	{
		Sys_lockSound();
		S_StopMp3Music();
		mp3loaded = qfalse;
		mp3data = 0;
		mp3_size = FS_LoadFile(filename, (void**)&mp3data);
		if (!mp3data)return qfalse;
		InitMP3(&mp);
		mp3loaded = qtrue;
		if (decodeMP3(&mp, mp3data, mp3_size, samples, sizeof(samples), &samples_size) != MP3_OK)
		{
			S_StopMp3Music();
			return qfalse;
		}


		// switch up to 44 khz sound if necessary
		float old_khz = Cvar_VariableValue("s_khz");
		int freq = freqs_mp3[mp.fr.sampling_frequency];
		if (old_khz !=  freq/ 1000)
		{
			Cvar_SetValue("s_khz",  freq/ 1000);
			CL_Snd_Restart_f();
		}
		HANDLE thread = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)&decodethread, 0, 0, NULL);

		return qtrue;



	}
	__finally
	{
		Sys_UnlockSound();

	}
}
void S_StopMp3Music()
{
	if (hExit) SetEvent(hExit);
	Sys_lockSound();
	{
		if (mp3loaded)  ExitMP3(&mp);
		mp3loaded = qfalse;
		if (mp3data) Z_Free(mp3data);
		mp3data = 0;
		CloseHandle(hMore);
		hMore = 0;

	}
	Sys_UnlockSound();
}

void S_ProduceMp3Samples()
{
	if (hMore) SetEvent(hMore);

}


int decodethread(LPVOID data )
{
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);


	hExit = CreateEvent(NULL, TRUE, FALSE, 0);
	hMore = CreateEvent(NULL, FALSE, FALSE, 0);
	while (1) {
		int signaled = WaitForSingleObject(hExit, 100);
		if (signaled == WAIT_OBJECT_0) {
			CloseHandle(hExit);
			hExit = 0;
			return 0;
		}

		if (!mp3loaded)return 0;
		char*in = 0;
		int in_size = 0;

		int channels = 1 + (mp.fr.stereo != 0);
		while (samples_size < MAX_RAW_SAMPLES*channels * 2)
		{

			char*start;
			size_t remaining = sizeof(samples) - samples_size;;
			int amount = 0;
			start = samples + samples_size;
			switch (decodeMP3(&mp, in, in_size, start, remaining, &amount))
			{
			case MP3_NEED_MORE:
				if (in != mp3data) {
					in = mp3data; in_size = mp3_size;
					samples_size += amount;
					break;
				}

			default:
			case MP3_ERR:
				CloseHandle(hExit);
				hExit = 0;
				S_StopMp3Music();
				return 1;

			case MP3_OK:
				in = 0; in_size = 0;
				samples_size += amount;
				break;
			}

		}
		signaled = WaitForSingleObject(hMore, 40);
		///if (signaled == WAIT_OBJECT_0) 
		{

			QueryPerformanceCounter(&EndingTime);
			ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
			StartingTime.QuadPart = EndingTime.QuadPart;

			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

			int freq = freqs_mp3[mp.fr.sampling_frequency];
			LONGLONG needed = min((ElapsedMicroseconds.QuadPart*freq) / 1000, MAX_RAW_SAMPLES);
			int used = S_RawSamples((int)needed, freq, 2, channels, (byte*)samples);

			// copy remaining to beginning of buffer and adjust amount
			int unused = samples_size - used * channels * 2;
			memmove(samples, samples + used * channels * 2, unused);
			samples_size = unused;
		}


	}

	
	

	return 1;
}