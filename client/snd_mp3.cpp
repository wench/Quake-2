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
int sample_numbytes = 0;
char samples[MAX_RAW_SAMPLES*8 + 4608];
HANDLE hExit=0; // tell thread to exit
HANDLE hMore = 0;
int decodethread(LPVOID data);
int samplesconsumed = MAX_RAW_SAMPLES;
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
		//char outfn[MAX_QPATH];		
		if (!mp3data)return qfalse;
		InitMP3(&mp);
		mp3loaded = qtrue;
		if (decodeMP3(&mp, mp3data, mp3_size, samples, sizeof(samples), &sample_numbytes) != MP3_OK)
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
		samplesconsumed = MAX_RAW_SAMPLES;
		while (!hMore)Sleep(0);
		SetEvent(hMore);
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
	while (hExit)Sleep(0);
}

void S_ProduceMp3Samples(int consumed)
{
	if (mp3loaded)
	{
		consumed;
		int needed = consumed;
		int freq = freqs_mp3[mp.fr.sampling_frequency];
		int channels = 1 + (mp.fr.stereo != 0);
		int avail = sample_numbytes / (channels * 2);

		if (needed > avail) needed = avail;

		int bytesused = S_RawSamples((int)needed, freq, 2, channels, (byte*)samples) * channels * 2;
		if (bytesused > sample_numbytes) bytesused = sample_numbytes;

		// copy remaining to beginning of buffer and adjust amount
		int bytesunused = sample_numbytes - bytesused;
		if (bytesunused)memmove(samples, samples + bytesused, bytesunused);
		sample_numbytes = bytesunused;
		if (hMore) SetEvent(hMore);
	}
}


int decodethread(LPVOID data )
{
//	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	//LARGE_INTEGER Frequency;
	//QueryPerformanceFrequency(&Frequency);
	//QueryPerformanceCounter(&StartingTime);


	HANDLE myExit = hExit = CreateEvent(NULL, TRUE, FALSE, 0);
	HANDLE mymore = hMore = CreateEvent(NULL, FALSE, FALSE, 0);
	while (1) {
		if (WaitForSingleObject(myExit, 0) == WAIT_OBJECT_0) {
				soundlocker sndlocker;
				if(myExit)CloseHandle(myExit);
			hExit = 0;
			return 0;
		}

		if (!mp3loaded)return 0;
		char*in = 0;
		int in_size = 0;

		int channels = 1 + (mp.fr.stereo != 0);
		while (sample_numbytes < MAX_RAW_SAMPLES*channels * 2)
		{
			if (WaitForSingleObject(myExit, 0) == WAIT_OBJECT_0) {
				soundlocker sndlocker;
				if (myExit)CloseHandle(myExit);
				hExit = 0;
				return 0;
			}
			char*start;
			size_t remaining = sizeof(samples) - sample_numbytes;
			int amount = 0;
			start = samples + sample_numbytes;
			switch (decodeMP3(&mp, in, in_size, start, remaining, &amount))
			{
			case MP3_NEED_MORE:
				if (in != mp3data) {
					in = mp3data; in_size = mp3_size;
					sample_numbytes += amount;
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
				sample_numbytes += amount;
				break;
			}

		}
		HANDLE handles[] = { myExit, hMore };
		DWORD signaled = WaitForMultipleObjects(2, handles,FALSE, INFINITE);
		if (signaled == WAIT_OBJECT_0) {
			soundlocker sndlocker;
			if (myExit)CloseHandle(myExit);
			hExit = 0;
			return 0;
		}
		
		
		{
			/*
			QueryPerformanceCounter(&EndingTime);
			ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
			StartingTime.QuadPart = EndingTime.QuadPart;

			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

			int freq = freqs_mp3[mp.fr.sampling_frequency];
			//int	needed = min((ElapsedMicroseconds.QuadPart*freq) / 1000, MAX_RAW_SAMPLES);
			needed = samplesconsumed;

			int bytesused = needed* channels * 2;

			// copy remaining to beginning of buffer and adjust amount
			int bytesunused = sample_numbytes - bytesused;
			memmove(samples, samples + bytesused, bytesunused);
			sample_numbytes = bytesunused;*/
		}


	}

	
	

	return 1;
}