/*
Copyright (C) 2002 Ryan Nunn

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <assert.h>
#include <windows.h>
#include <malloc.h>
#include "../ref_gl/gl_local.h"
#include "glw_win.h"
#include "winquake.h"

/*
** WGL_ARB_extensions_string
*/
const char * ( WINAPI * qwglGetExtensionsStringARB ) (HDC);


/*
** WGL_ARB_pixel_format
*/
BOOL ( APIENTRY * qwglGetPixelFormatAttribivARB ) (HDC hdc,
				GLint iPixelFormat, 
				GLint iLayerPlane,
				GLuint nAttributes, 
				const GLint *piAttributes,
				GLint *pValues);
BOOL ( APIENTRY * qwglGetPixelFormatAttribfvARB ) (HDC hdc,
				GLint iPixelFormat, 
				GLint iLayerPlane,
				GLuint nAttributes, 
				const GLint *piAttributes,
				GLfloat *pValues);
BOOL ( APIENTRY * qwglChoosePixelFormatARB ) (HDC hdc,
				const GLint *piAttribIList,
				const GLfloat *pfAttribFList,
				GLuint nMaxFormats,
				GLint *piFormats,
				GLuint *nNumFormats);

/*
** WGL_3DFX_multisample
*/
qboolean use_WGL_3DFX_multisample;

/*
** WGL_ARB_multisample
*/
qboolean use_WGL_ARB_multisample;

static void WGL_GetExtensions(void);

qboolean GLimp_InitGL (void);

/*
** WGL_InitGL
*/
#define	WINDOW_CLASS_NAME	"Quake 2 WGL 'Trick'"
qboolean once = false;
void WGL_InitGL( void )
{
	WNDCLASS		wc;
	RECT			r;
	cvar_t			*vid_xpos, *vid_ypos;
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;

	//if (once) return;
	once = true;

	/* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)DefWindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = glw_state.hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass (&wc) )
		ri.Sys_Error (ERR_FATAL, "Couldn't register window class");

	exstyle = 0;
	stylebits = WINDOW_STYLE;
	__try
	{

		glw_state.hWnd = CreateWindowEx(
			exstyle,
			WINDOW_CLASS_NAME,
			"Quake 2",
			stylebits,
			1, 1, 1, 1,
			NULL,
			NULL,
			glw_state.hInstance,
			NULL);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}

	// We don't really care if we couldn't create a window
	if (!glw_state.hWnd) {
		UnregisterClass (WINDOW_CLASS_NAME, glw_state.hInstance);
		return;
	}
	
	// Kill all the previous extensions
	qwglGetExtensionsStringARB = 0;
	use_WGL_3DFX_multisample = false;
	use_WGL_ARB_multisample = false;
	gl_config.wglextensions_string = " ";
	
	if (GLimp_InitGL()) {
		ri.Con_Printf( PRINT_ALL, "WGL: Initalized OpenGL\n" );

		WGL_GetExtensions();

		// Uninit's opengl
		if ( qwglMakeCurrent && !qwglMakeCurrent( NULL, NULL ) )
			ri.Con_Printf( PRINT_ALL, "WGL_InitGL() - wglMakeCurrent failed\n");
		if ( glw_state.hDC )
		{
			if ( !ReleaseDC( glw_state.hWnd, glw_state.hDC ) ) {
				ri.Con_Printf( PRINT_ALL, "WGL_InitGL() - ReleaseDC failed:\n" );
			}
			glw_state.hDC   = NULL;
		}
		if ( glw_state.hGLRC )
		{
			if (  qwglDeleteContext && !qwglDeleteContext( glw_state.hGLRC ) )
				ri.Con_Printf( PRINT_ALL, "WGL_InitGL() - wglDeleteContext failed\n");
			glw_state.hGLRC = NULL;
		}
	}
	else {
		ri.Con_Printf( PRINT_ALL, "WGL: Unable to initalize OpenGL\n" );
	}

	//
	// Kill the window
	//
	if (glw_state.hWnd)
	{
		DestroyWindow (	glw_state.hWnd );
		glw_state.hWnd = NULL;
	}

	UnregisterClass (WINDOW_CLASS_NAME, glw_state.hInstance);
}

/*
** WGL_GetExtensions
*/
static void WGL_GetExtensions(void)
{
	qwglGetExtensionsStringARB = (decltype(qwglGetExtensionsStringARB)) qwglGetProcAddress("wglGetExtensionsStringARB");

	if (qwglGetExtensionsStringARB) {
		gl_config.wglextensions_string = qwglGetExtensionsStringARB(glw_state.hDC);
		ri.Con_Printf (PRINT_ALL, "WGL_EXTENSIONS: %s\n", gl_config.wglextensions_string);
		ri.Con_Printf( PRINT_ALL, "...enabling WGL_ARB_extension_string\n" );
	}
	else {
		gl_config.wglextensions_string = " ";
		ri.Con_Printf( PRINT_ALL, "...WGL_ARB_extension_string not found\n" );
	}

	if ( strstr( gl_config.wglextensions_string, "WGL_ARB_pixel_format" ) )
	{
		qwglGetPixelFormatAttribivARB = (decltype(qwglGetPixelFormatAttribivARB)) qwglGetProcAddress( "wglGetPixelFormatAttribivARB" );
		qwglGetPixelFormatAttribfvARB = (decltype(qwglGetPixelFormatAttribfvARB)) qwglGetProcAddress( "wglGetPixelFormatAttribfvARB" );
		qwglChoosePixelFormatARB = (decltype(qwglChoosePixelFormatARB)) qwglGetProcAddress( "wglChoosePixelFormatARB" );
		ri.Con_Printf( PRINT_ALL, "...enabling WGL_ARB_pixel_format\n" );

		if (!qwglGetPixelFormatAttribivARB) 
			ri.Con_Printf( PRINT_ALL, "...didn't get wglGetPixelFormatAttribivARB\n" );
		else if (!qwglGetPixelFormatAttribfvARB) 
			ri.Con_Printf( PRINT_ALL, "...didn't get wglGetPixelFormatAttribfvARB\n" );
		else if (!qwglChoosePixelFormatARB) 
			ri.Con_Printf( PRINT_ALL, "...didn't get wglChoosePixelFormatARB\n" );

	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...WGL_ARB_pixel_format not found\n" );
	}

	if ( strstr( gl_config.wglextensions_string, "WGL_ARB_multisample" ) )
	{
		if ( gl_ext_3dfx_multisample->value > 1)
		{
			ri.Con_Printf( PRINT_ALL, "...using WGL_ARB_multisample\n" );
			use_WGL_ARB_multisample = true;
		}
		else
		{
			ri.Con_Printf( PRINT_ALL, "...ignoring WGL_ARB_multisample\n" );
			use_WGL_ARB_multisample = false;
		}
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...WGL_ARB_multisample not found\n" );
		use_WGL_ARB_multisample = false;
	}

	if ( strstr( gl_config.wglextensions_string, "WGL_3DFX_multisample" ) )
	{
		if ( use_WGL_ARB_multisample ) 
		{
			ri.Con_Printf( PRINT_ALL, "...WGL_3DFX_multisample deprecated in favor of WGL_ARB_multisample\n" );
		}
		else if ( gl_ext_3dfx_multisample->value > 1)
		{
			ri.Con_Printf( PRINT_ALL, "...using WGL_3DFX_multisample\n" );
			use_WGL_3DFX_multisample = true;
		}
		else
		{
			ri.Con_Printf( PRINT_ALL, "...ignoring WGL_3DFX_multisample\n" );
			use_WGL_3DFX_multisample = false;
		}
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...WGL_3DFX_multisample not found\n" );
		use_WGL_3DFX_multisample = false;
	}

}

