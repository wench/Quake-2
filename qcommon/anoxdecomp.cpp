#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include "zlib.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
extern "C"{
#include "qcommon.h"
}
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif
#define CHUNK 16384
#include "zip/unzip.h"
struct ThreadData {
	FILE*source;
	FILE *dest;
	int size_src;
	~ThreadData() {
		fclose(source);
		fclose(dest);
	}

}; struct ThreadDataZ {
	FILE*source;
	FILE *dest;
	int size_read;
	unzFile unzfile;
	~ThreadDataZ() {
		fclose(source);
		fclose(dest);
		unzClose(unzfile);
	}

};
INT decompthreadproc(ThreadData*data)
{
	__try {
		int ret;
		unsigned remaining = data->size_src;
		z_stream strm;
		unsigned char in[CHUNK];
		unsigned char out[CHUNK];

		/* allocate inflate state */
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		ret = inflateInit(&strm);
		if (ret != Z_OK)
			return ret;


		/* decompress until deflate stream ends or end of file */
		do {
			int toget = min(remaining, CHUNK);
			strm.avail_in = fread(in, 1, CHUNK, data->source);
			if (ferror(data->source)) {
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
			remaining -= toget;
			if (strm.avail_in == 0 || remaining < 0)
				break;
			strm.next_in = in;
			/* run inflate() on input until output buffer not full */
			do {
				strm.avail_out = CHUNK;
				strm.next_out = out;
				ret = inflate(&strm, Z_NO_FLUSH);
				assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
				switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;     /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
				}
				uInt have = CHUNK - strm.avail_out;
				if (fwrite(out, 1, have, data->dest) != have || ferror(data->dest)) {
					(void)inflateEnd(&strm);
					return Z_ERRNO;
				}
			} while (strm.avail_out == 0);

			/* done when inflate() says it's done */
		} while (ret != Z_STREAM_END);
	}
	__finally {
		delete data;
	}
	return Z_OK;
}INT decompthreadprocZ(ThreadDataZ*data)
{
	__try {
		int ret;	

		unsigned char in[CHUNK];
		unsigned char out[CHUNK];




		/* decompress until deflate stream ends or end of file */
		do {
			int toget = CHUNK;
			int have= unzReadCurrentFile(data->unzfile, in, CHUNK );
			if (have == 0)break;


			/* run inflate() on input until output buffer not full */
			
			if (fwrite(in, 1, have, data->dest) != have || ferror(data->dest)) {
				break;
			}
			

			/* done when inflate() says it's done */
		} while (1);
	}
	__finally {
		delete data;
	}
	return Z_OK;
}
extern "C" FILE *DecompressZIP(FILE *source, unzFile unzipfile, int insize, int outsize)
{
	int handles[2];
	_pipe(handles, outsize, _O_BINARY);
	ThreadDataZ *data = new ThreadDataZ;
	data->dest = fdopen(handles[1], "wb");
	data->unzfile = unzipfile;
	data->source = source;
	data->size_read = outsize;
	HANDLE thread = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)&decompthreadprocZ, data, 0, NULL);

	if (!thread) {
		delete data;
		return 0;
	}
	CloseHandle(thread);
	return fdopen(handles[0], "rb");
}
extern "C" FILE *DecompressANOXDATA(FILE *source, int insize, int outsize)
{
	int handles[2];
	_pipe(handles,outsize, _O_BINARY);
	ThreadData *data = new ThreadData;
	data->dest = fdopen(handles[1], "wb");
	data->size_src = insize;
	data->source = source;
	HANDLE thread = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)&decompthreadproc, data, 0, NULL);

	if(!thread) {
		delete data;
		return 0;
	}
	CloseHandle(thread);
	return fdopen(handles[0], "rb");
}
/* report a zlib or i/o error */
void zerr(int ret)
{
	fputs("zpipe: ", stderr);
	switch (ret) {
	case Z_ERRNO:
		if (ferror(stdin))
			fputs("error reading stdin\n", stderr);
		if (ferror(stdout))
			fputs("error writing stdout\n", stderr);
		break;
	case Z_STREAM_ERROR:
		fputs("invalid compression level\n", stderr);
		break;
	case Z_DATA_ERROR:
		fputs("invalid or incomplete deflate data\n", stderr);
		break;
	case Z_MEM_ERROR:
		fputs("out of memory\n", stderr);
		break;
	case Z_VERSION_ERROR:
		fputs("zlib version mismatch!\n", stderr);
	}
}
