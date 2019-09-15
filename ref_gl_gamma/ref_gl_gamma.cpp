// ref_gl_gamma.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "ref_gl_gamma.h"

#include "Imports.h"
#include "ref.h"
#include <GL/gl.h>
#include <string.h>
#include"../ref_gl/glext.h"
#include "Detours-4.0.1/include/detours.h"
#include <string>

#undef FUNCTION
#define FUNCTION(name) \
__declspec(naked) void* __cdecl new_##name(void*) {   \
using  namespace Imports;   \
__asm  jmp name }

refimport_t ri;
void apply_gamma_to_fb();

void	(*orig_EndFrame) (int a1, int a2, int a3);
void EndFrame(int a1, int a2, int a3)
{
	OutputDebugStringA(__FUNCTION__"\n");
	apply_gamma_to_fb();
	orig_EndFrame(a1,a2,a3);
}

refexport_t* OnGetRefAPI(refexport_t* rexp, refimport_t* rimp) 
{
	OutputDebugStringA(__FUNCTION__"\n");
	ri =*rimp;
	orig_EndFrame = rexp->EndFrame;
	rexp->EndFrame = EndFrame;
	return rexp;
}

extern"C"
{
	namespace Exports
	{// refexport_t GetRefAPI (refimport_t rimp )
		
		void* retaddr;
		extern"C" __declspec(dllimport) refexport_t* __cdecl GetRefAPI(refexport_t*rexp,refimport_t *rimp);
		 __declspec(naked)  void __cdecl new_GetRefAPI(refexport_t* rexp, refimport_t* rimp) {
			// __asm int 3
			 
			__asm {
				pop retaddr
				call GetRefAPI
				push retaddr
				jmp OnGetRefAPI
				ret
			}
	
			
			//return 0;// GetRefAPI(rexp, rimp);;
		}
		
		
		 //FUNCTION(GetRefAPI)

#include "Functions.h"
	}
}

// GL_ARB_fragment_program
static PFNGLPROGRAMSTRINGARBPROC glProgramStringARB=0;
static PFNGLBINDPROGRAMARBPROC glBindProgramARB=0;
static PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB=0;
static PFNGLGENPROGRAMSARBPROC glGenProgramsARB=0;
static PFNGLPROGRAMENVPARAMETER4DARBPROC glProgramEnvParameter4dARB=0;
static PFNGLPROGRAMENVPARAMETER4DVARBPROC glProgramEnvParameter4dvARB=0;
static PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB=0;
static PFNGLPROGRAMENVPARAMETER4FVARBPROC glProgramEnvParameter4fvARB=0;
static PFNGLPROGRAMLOCALPARAMETER4DARBPROC glProgramLocalParameter4dARB=0;
static PFNGLPROGRAMLOCALPARAMETER4DVARBPROC glProgramLocalParameter4dvARB=0;
static PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB=0;
static PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB=0;
static PFNGLGETPROGRAMENVPARAMETERDVARBPROC glGetProgramEnvParameterdvARB=0;
static PFNGLGETPROGRAMENVPARAMETERFVARBPROC glGetProgramEnvParameterfvARB=0;
static PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC glGetProgramLocalParameterdvARB=0;
static PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC glGetProgramLocalParameterfvARB=0;
static PFNGLGETPROGRAMIVARBPROC glGetProgramivARB=0;
static PFNGLGETPROGRAMSTRINGARBPROC glGetProgramStringARB=0;
static PFNGLISPROGRAMARBPROC glIsProgramARB=0;
static void (APIENTRY* glMTexCoord2fSGIS)(GLenum, GLfloat, GLfloat)=0;
static void (APIENTRY* glActiveTextureARB) (GLenum)=0;
static void (APIENTRY* glClientActiveTextureARB) (GLenum)=0;

static BOOL  (WINAPI* trueSwapBuffers)(HDC) = 0;
static BOOL        (WINAPI *RealGetDeviceGammaRamp)(_In_ HDC hdc, _Out_writes_bytes_(3 * 256 * 2) LPVOID lpRamp)= GetDeviceGammaRamp;
static BOOL        (WINAPI *RealSetDeviceGammaRamp)(_In_ HDC hdc, _In_reads_bytes_(3 * 256 * 2)  LPVOID lpRamp) = SetDeviceGammaRamp;
struct GammaTable
{
	WORD	red[256];
	WORD	green[256];
	WORD	blue[256];
};
template<typename T>struct Texel {
	T	red;
		T green;
		T BLUE;
};
template<typename T>
struct GammaTexture {
	Texel<T> red[256];
	Texel<T> green[256];
	Texel<T> blue[256];
	Texel<T> unused[256];
};
static GammaTable current;
bool table_dirty = false;
GLuint texid_gamma = 0;
GLuint texid_fb = 0;
static GLuint gamma_frag_prog = 0;

static BOOL APIENTRY GetGammaRamp(HDC, LPVOID lpRamp)
{
	OutputDebugStringA(__FUNCTION__"\n");
	GammaTable* ramp = (GammaTable*)lpRamp;
	*ramp = current;
	return TRUE;
}
static BOOL APIENTRY SetGammaRamp(HDC, LPVOID lpRamp)
{
	OutputDebugStringA(__FUNCTION__"\n");
	GammaTable* ramp = (GammaTable*)lpRamp;
	current = *ramp;
	table_dirty = true;
	return TRUE;
}

void GL_SelectTexture(GLenum texture)
{
	glActiveTextureARB(texture);
	glClientActiveTextureARB(texture);
}
void GL_MBind(GLenum target, int texnum)
{
	GL_SelectTexture(target);
	glBindTexture(GL_TEXTURE_2D, texnum);
}
void	R_SetGL2D(float viewport[4])
{
	// set 2D virtual screen size
	//glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,  viewport[2], viewport[3], 0, -99999, 99999);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1, 1, 1, 1);
}
void apply_gamma_to_fb()
{
	OutputDebugStringA(__FUNCTION__"\n");
	bool ok = true;
	bool redo = false;

	static float oldviewport[4] = { 0 };//x y w h
	float viewport[4];//x y w h
	glGetFloatv(GL_VIEWPORT, viewport);
	if (memcmp(oldviewport, viewport, sizeof(viewport))) {
		redo = true;
		table_dirty = true;
		texid_gamma = 0;
		texid_fb = 0;
		gamma_frag_prog = 0;
	}
		memcpy(oldviewport, viewport, sizeof(viewport));

	if(ok && (redo || !glMTexCoord2fSGIS)) ok = 0 !=(*(PROC*)&glMTexCoord2fSGIS = wglGetProcAddress("glMultiTexCoord2fARB"));
	if (ok && (redo || !glActiveTextureARB)) ok = 0 !=(*(PROC*)&glActiveTextureARB = wglGetProcAddress("glActiveTextureARB"));
	if (ok && (redo || !glClientActiveTextureARB)) ok = 0 !=(*(PROC*)&glClientActiveTextureARB = wglGetProcAddress("glClientActiveTextureARB"));
	if (ok && (redo || !glProgramStringARB)) ok = 0 !=(glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress("glProgramStringARB"));
	if (ok && (redo || !glBindProgramARB)) ok = 0 !=(glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB"));
	if (ok && (redo || !glDeleteProgramsARB)) ok = 0 !=(glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)wglGetProcAddress("glDeleteProgramsARB"));
	if (ok && (redo || !glGenProgramsARB)) ok = 0 !=(glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)wglGetProcAddress("glGenProgramsARB"));
	if (ok && (redo || !glProgramEnvParameter4dARB)) ok = 0 !=(glProgramEnvParameter4dARB = (PFNGLPROGRAMENVPARAMETER4DARBPROC)wglGetProcAddress("glProgramEnvParameter4dARB"));
	if (ok && (redo || !glProgramEnvParameter4dvARB)) ok = 0 !=(glProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARBPROC)wglGetProcAddress("glProgramEnvParameter4dvARB"));
	if (ok && (redo || !glProgramEnvParameter4fARB)) ok = 0 !=(glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)wglGetProcAddress("glProgramEnvParameter4fARB"));
	if (ok && (redo || !glProgramEnvParameter4fvARB)) ok = 0 !=(glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)wglGetProcAddress("glProgramEnvParameter4fvARB"));
	if (ok && (redo || !glProgramLocalParameter4dARB)) ok = 0 !=(glProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC)wglGetProcAddress("glProgramLocalParameter4dARB"));
	if (ok && (redo || !glProgramLocalParameter4dvARB)) ok = 0 !=(glProgramLocalParameter4dvARB = (PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)wglGetProcAddress("glProgramLocalParameter4dvARB"));
	if (ok && (redo || !glProgramLocalParameter4fARB)) ok = 0 !=(glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)wglGetProcAddress("glProgramLocalParameter4fARB"));
	if (ok && (redo || !glProgramLocalParameter4fvARB)) ok = 0 !=(glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)wglGetProcAddress("glProgramLocalParameter4fvARB"));
	if (ok && (redo || !glGetProgramEnvParameterdvARB)) ok = 0 !=(glGetProgramEnvParameterdvARB = (PFNGLGETPROGRAMENVPARAMETERDVARBPROC)wglGetProcAddress("glGetProgramEnvParameterdvARB"));
	if (ok && (redo || !glGetProgramEnvParameterfvARB)) ok = 0 !=(glGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)wglGetProcAddress("glGetProgramEnvParameterfvARB"));
	if (ok && (redo || !glGetProgramLocalParameterdvARB)) ok = 0 !=(glGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)wglGetProcAddress("glGetProgramLocalParameterdvARB"));
	if (ok && (redo || !glGetProgramLocalParameterfvARB)) ok = 0 !=(glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)wglGetProcAddress("glGetProgramLocalParameterfvARB"));
	if (ok && (redo || !glGetProgramivARB)) ok = 0 !=(glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)wglGetProcAddress("glGetProgramivARB"));
	if (ok && (redo || !glGetProgramStringARB)) ok = 0 !=(glGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARBPROC)wglGetProcAddress("glGetProgramStringARB"));
	if (ok && (redo || !glIsProgramARB)) ok = 0 !=(glIsProgramARB = (PFNGLISPROGRAMARBPROC)wglGetProcAddress("glIsProgramARB"));
	if (ok)
	{
		if (table_dirty)
		{

			table_dirty = false;

			if (!texid_gamma) texid_gamma = (1 << 20) + rand();
			glBindTexture(GL_TEXTURE_2D, texid_gamma);
			GammaTexture<unsigned short> data;
			memset(&data, 0, sizeof(data));
			for (int i = 0; i < 256; i++) {
				int r =  current.red[i];
				int g = current.green[i];
				int b = current.blue[i];
				data.red[i].red =  r;
				data.green[i].green = g;
				data.blue[i].BLUE = b;
			}


			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, 256, 4, 0, GL_RGB, GL_UNSIGNED_SHORT, &data);
		}
		if (texid_gamma)
		{

				

			int err;
			if (!gamma_frag_prog)
			{
				char shader[2048] =
					"!!ARBfp1.0\n"
					"TEMP tmp, acc,tc;\n"
					"TEX acc, fragment.texcoord[0], texture[0], 2D;\n"
					"MOV tc.x, acc.r;"
					"MOV tc.y, 0.0125;"
					"TEX tmp, tc, texture[1], 2D;\n"
					"MOV acc.r, tmp.r;\n"
					"TEX tmp, fragment.texcoord[0], texture[0], 2D;\n"
					"MOV tc.x, tmp.g;"
					"MOV tc.y, 0.25;"
					"TEX tmp, tc, texture[1], 2D;\n"
					"MOV acc.g, tmp.g;\n"
					"TEX tmp, fragment.texcoord[0], texture[0], 2D;\n"
					"MOV tc.x, tmp.b;"
					"MOV tc.y, 0.5;"
					"TEX tmp, tc, texture[1], 2D;\n"
					"MOV acc.b, tmp.b;\n"
					"MOV result.color, acc;\n"
					"END\n";
				err = glGetError();
				glGenProgramsARB(1, &gamma_frag_prog);
				err = glGetError();
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gamma_frag_prog);
				err = glGetError();
				glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (int)strlen(shader), shader);
				err = glGetError();
			}
			if (!texid_fb) {
				texid_fb = texid_gamma + 1;
			}
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gamma_frag_prog);
			glEnable(GL_FRAGMENT_PROGRAM_ARB);
			glEnable(GL_TEXTURE_2D);
			GL_MBind(GL_TEXTURE1, texid_gamma);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			GL_MBind(GL_TEXTURE0, texid_fb);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			//qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4096, 4096, 0, GL_RGB, GL_UNSIGNED_BYTE,NULL);
			glReadBuffer(GL_BACK);
			GLenum error = glGetError();

			glFinish();
			glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport[0], viewport[1], viewport[2], viewport[3], 0);
			//glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, 0, 0, vid.width, vid.height);
			R_SetGL2D(viewport);
			error = glGetError();




			//glEnable(GL_BLEND);
			glEnable(GL_TEXTURE_2D);

			
			glBegin(GL_QUADS);

			glTexCoord2f(0, 1);
			glVertex2f(0, 0);
			glTexCoord2f(1, 1);
			glVertex2f(viewport[2], 0);
			glTexCoord2f(1, 0);
			glVertex2f(viewport[2], viewport[3]);
			glTexCoord2f(0, 0);
			glVertex2f(0, viewport[3]);

			glEnd();
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);

			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
			glDisable(GL_FRAGMENT_PROGRAM_ARB);
			GL_MBind(GL_TEXTURE0, 0);
			error = glGetError();

		}
	}

}

bool doneit = false;
void hook()
{
	if (!doneit)
	{
		doneit = true;

		for (int i = 0; i < 256; i++)
		{
			current.blue[i] = current.green[i] = current.red[i] = i << 8;
		}
		HMODULE refgloriginal = GetModuleHandleA("ref_gl_original");
		HMODULE mod = (HMODULE)0x10000000;
		HMODULE opengl32 = GetModuleHandleA("OPENGL32");
		HMODULE gdi32 = GetModuleHandleA("GDI32");
		//*(PROC*)& trueSwapBuffers = GetProcAddress(opengl32, "wglSwapBuffers");
		if (gdi32){
			*(PROC*)& RealGetDeviceGammaRamp = GetProcAddress(gdi32, "GetDeviceGammaRamp");
			*(PROC*)& RealSetDeviceGammaRamp = GetProcAddress(gdi32, "SetDeviceGammaRamp");
			}
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		//DetourAttach(&(PVOID&)trueSwapBuffers, hookedSwapBuffers);
		DetourAttach(&(PVOID&)RealGetDeviceGammaRamp, GetGammaRamp);
		DetourAttach(&(PVOID&)RealSetDeviceGammaRamp, SetGammaRamp);
		auto error = DetourTransactionCommit();
	}
}
void unhook()
{
	if (doneit)
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		//DetourDetach(&(PVOID&)trueSwapBuffers, hookedSwapBuffers);
		DetourDetach(&(PVOID&)RealGetDeviceGammaRamp, GetGammaRamp);
		DetourDetach(&(PVOID&)RealSetDeviceGammaRamp, SetGammaRamp);
		auto error = DetourTransactionCommit();
	}
}