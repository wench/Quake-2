/*
Copyright (C) 1997-2001 Id Software, Inc.

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
/*
** QGL.H
*/

#ifndef __QGL_H__
#define __QGL_H__

#ifdef _WIN32
#  include <windows.h>
#endif

#include <GL/gl.h>

#ifdef __linux__
//#include <GL/fxmesa.h>
#include <GL/glx.h>
#endif

#include"glext.h"
qboolean QGL_Init( const char *dllname );
void     QGL_Shutdown( void );

#ifndef APIENTRY
#  define APIENTRY
#endif

extern  void ( APIENTRY * qglAccum )(GLenum op, GLfloat value);
extern  void ( APIENTRY * qglAlphaFunc )(GLenum func, GLclampf ref);
extern  GLboolean ( APIENTRY * qglAreTexturesResident )(GLsizei n, const GLuint *textures, GLboolean *residences);
extern  void ( APIENTRY * qglArrayElement )(GLint i);
extern  void ( APIENTRY * qglBegin )(GLenum mode);
extern  void ( APIENTRY * qglBindTexture )(GLenum target, GLuint texture);
extern  void ( APIENTRY * qglBitmap )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
extern  void ( APIENTRY * qglBlendFunc )(GLenum sfactor, GLenum dfactor);
extern  void ( APIENTRY * qglCallList )(GLuint list);
extern  void ( APIENTRY * qglCallLists )(GLsizei n, GLenum type, const GLvoid *lists);
extern  void ( APIENTRY * qglClear )(GLbitfield mask);
extern  void ( APIENTRY * qglClearAccum )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern  void ( APIENTRY * qglClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern  void ( APIENTRY * qglClearDepth )(GLclampd depth);
extern  void ( APIENTRY * qglClearIndex )(GLfloat c);
extern  void ( APIENTRY * qglClearStencil )(GLint s);
extern  void ( APIENTRY * qglClipPlane )(GLenum plane, const GLdouble *equation);
extern  void ( APIENTRY * qglColor3b )(GLbyte red, GLbyte green, GLbyte blue);
extern  void ( APIENTRY * qglColor3bv )(const GLbyte *v);
extern  void ( APIENTRY * qglColor3d )(GLdouble red, GLdouble green, GLdouble blue);
extern  void ( APIENTRY * qglColor3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglColor3f )(GLfloat red, GLfloat green, GLfloat blue);
extern  void ( APIENTRY * qglColor3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglColor3i )(GLint red, GLint green, GLint blue);
extern  void ( APIENTRY * qglColor3iv )(const GLint *v);
extern  void ( APIENTRY * qglColor3s )(GLshort red, GLshort green, GLshort blue);
extern  void ( APIENTRY * qglColor3sv )(const GLshort *v);
extern  void ( APIENTRY * qglColor3ub )(GLubyte red, GLubyte green, GLubyte blue);
extern  void ( APIENTRY * qglColor3ubv )(const GLubyte *v);
extern  void ( APIENTRY * qglColor3ui )(GLuint red, GLuint green, GLuint blue);
extern  void ( APIENTRY * qglColor3uiv )(const GLuint *v);
extern  void ( APIENTRY * qglColor3us )(GLushort red, GLushort green, GLushort blue);
extern  void ( APIENTRY * qglColor3usv )(const GLushort *v);
extern  void ( APIENTRY * qglColor4b )(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
extern  void ( APIENTRY * qglColor4bv )(const GLbyte *v);
extern  void ( APIENTRY * qglColor4d )(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
extern  void ( APIENTRY * qglColor4dv )(const GLdouble *v);
extern  void ( APIENTRY * qglColor4f )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern  void ( APIENTRY * qglColor4fv )(const GLfloat *v);
extern  void ( APIENTRY * qglColor4i )(GLint red, GLint green, GLint blue, GLint alpha);
extern  void ( APIENTRY * qglColor4iv )(const GLint *v);
extern  void ( APIENTRY * qglColor4s )(GLshort red, GLshort green, GLshort blue, GLshort alpha);
extern  void ( APIENTRY * qglColor4sv )(const GLshort *v);
extern  void ( APIENTRY * qglColor4ub )(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
extern  void ( APIENTRY * qglColor4ubv )(const GLubyte *v);
extern  void ( APIENTRY * qglColor4ui )(GLuint red, GLuint green, GLuint blue, GLuint alpha);
extern  void ( APIENTRY * qglColor4uiv )(const GLuint *v);
extern  void ( APIENTRY * qglColor4us )(GLushort red, GLushort green, GLushort blue, GLushort alpha);
extern  void ( APIENTRY * qglColor4usv )(const GLushort *v);
extern  void ( APIENTRY * qglColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern  void ( APIENTRY * qglColorMaterial )(GLenum face, GLenum mode);
extern  void ( APIENTRY * qglColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglCopyPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern  void ( APIENTRY * qglCopyTexImage1D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
extern  void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern  void ( APIENTRY * qglCopyTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern  void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglCullFace )(GLenum mode);
extern  void ( APIENTRY * qglDeleteLists )(GLuint list, GLsizei range);
extern  void ( APIENTRY * qglDeleteTextures )(GLsizei n, const GLuint *textures);
extern  void ( APIENTRY * qglDepthFunc )(GLenum func);
extern  void ( APIENTRY * qglDepthMask )(GLboolean flag);
extern  void ( APIENTRY * qglDepthRange )(GLclampd zNear, GLclampd zFar);
extern  void ( APIENTRY * qglDisable )(GLenum cap);
extern  void ( APIENTRY * qglDisableClientState )(GLenum array);
extern  void ( APIENTRY * qglDrawArrays )(GLenum mode, GLint first, GLsizei count);
extern  void ( APIENTRY * qglDrawBuffer )(GLenum mode);
extern  void ( APIENTRY * qglDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
extern  void ( APIENTRY * qglDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglEdgeFlag )(GLboolean flag);
extern  void ( APIENTRY * qglEdgeFlagPointer )(GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglEdgeFlagv )(const GLboolean *flag);
extern  void ( APIENTRY * qglEnable )(GLenum cap);
extern  void ( APIENTRY * qglEnableClientState )(GLenum array);
extern  void ( APIENTRY * qglEnd )(void);
extern  void ( APIENTRY * qglEndList )(void);
extern  void ( APIENTRY * qglEvalCoord1d )(GLdouble u);
extern  void ( APIENTRY * qglEvalCoord1dv )(const GLdouble *u);
extern  void ( APIENTRY * qglEvalCoord1f )(GLfloat u);
extern  void ( APIENTRY * qglEvalCoord1fv )(const GLfloat *u);
extern  void ( APIENTRY * qglEvalCoord2d )(GLdouble u, GLdouble v);
extern  void ( APIENTRY * qglEvalCoord2dv )(const GLdouble *u);
extern  void ( APIENTRY * qglEvalCoord2f )(GLfloat u, GLfloat v);
extern  void ( APIENTRY * qglEvalCoord2fv )(const GLfloat *u);
extern  void ( APIENTRY * qglEvalMesh1 )(GLenum mode, GLint i1, GLint i2);
extern  void ( APIENTRY * qglEvalMesh2 )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
extern  void ( APIENTRY * qglEvalPoint1 )(GLint i);
extern  void ( APIENTRY * qglEvalPoint2 )(GLint i, GLint j);
extern  void ( APIENTRY * qglFeedbackBuffer )(GLsizei size, GLenum type, GLfloat *buffer);
extern  void ( APIENTRY * qglFinish )(void);
extern  void ( APIENTRY * qglFlush )(void);
extern  void ( APIENTRY * qglFogf )(GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglFogfv )(GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglFogi )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglFogiv )(GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglFrontFace )(GLenum mode);
extern  void ( APIENTRY * qglFrustum )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern  GLuint ( APIENTRY * qglGenLists )(GLsizei range);
extern  void ( APIENTRY * qglGenTextures )(GLsizei n, GLuint *textures);
extern  void ( APIENTRY * qglGetBooleanv )(GLenum pname, GLboolean *params);
extern  void ( APIENTRY * qglGetClipPlane )(GLenum plane, GLdouble *equation);
extern  void ( APIENTRY * qglGetDoublev )(GLenum pname, GLdouble *params);
extern  GLenum ( APIENTRY * qglGetError )(void);
extern  void ( APIENTRY * qglGetFloatv )(GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetIntegerv )(GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetLightfv )(GLenum light, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetLightiv )(GLenum light, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetMapdv )(GLenum target, GLenum query, GLdouble *v);
extern  void ( APIENTRY * qglGetMapfv )(GLenum target, GLenum query, GLfloat *v);
extern  void ( APIENTRY * qglGetMapiv )(GLenum target, GLenum query, GLint *v);
extern  void ( APIENTRY * qglGetMaterialfv )(GLenum face, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetMaterialiv )(GLenum face, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetPixelMapfv )(GLenum map, GLfloat *values);
extern  void ( APIENTRY * qglGetPixelMapuiv )(GLenum map, GLuint *values);
extern  void ( APIENTRY * qglGetPixelMapusv )(GLenum map, GLushort *values);
extern  void ( APIENTRY * qglGetPointerv )(GLenum pname, GLvoid* *params);
extern  void ( APIENTRY * qglGetPolygonStipple )(GLubyte *mask);
extern  const GLubyte * ( APIENTRY * qglGetString )(GLenum name);
extern  void ( APIENTRY * qglGetTexEnvfv )(GLenum target, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetTexEnviv )(GLenum target, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetTexGendv )(GLenum coord, GLenum pname, GLdouble *params);
extern  void ( APIENTRY * qglGetTexGenfv )(GLenum coord, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetTexGeniv )(GLenum coord, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetTexImage )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
extern  void ( APIENTRY * qglGetTexLevelParameterfv )(GLenum target, GLint level, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetTexLevelParameteriv )(GLenum target, GLint level, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetTexParameterfv )(GLenum target, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetTexParameteriv )(GLenum target, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglHint )(GLenum target, GLenum mode);
extern  void ( APIENTRY * qglIndexMask )(GLuint mask);
extern  void ( APIENTRY * qglIndexPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglIndexd )(GLdouble c);
extern  void ( APIENTRY * qglIndexdv )(const GLdouble *c);
extern  void ( APIENTRY * qglIndexf )(GLfloat c);
extern  void ( APIENTRY * qglIndexfv )(const GLfloat *c);
extern  void ( APIENTRY * qglIndexi )(GLint c);
extern  void ( APIENTRY * qglIndexiv )(const GLint *c);
extern  void ( APIENTRY * qglIndexs )(GLshort c);
extern  void ( APIENTRY * qglIndexsv )(const GLshort *c);
extern  void ( APIENTRY * qglIndexub )(GLubyte c);
extern  void ( APIENTRY * qglIndexubv )(const GLubyte *c);
extern  void ( APIENTRY * qglInitNames )(void);
extern  void ( APIENTRY * qglInterleavedArrays )(GLenum format, GLsizei stride, const GLvoid *pointer);
extern  GLboolean ( APIENTRY * qglIsEnabled )(GLenum cap);
extern  GLboolean ( APIENTRY * qglIsList )(GLuint list);
extern  GLboolean ( APIENTRY * qglIsTexture )(GLuint texture);
extern  void ( APIENTRY * qglLightModelf )(GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglLightModelfv )(GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglLightModeli )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglLightModeliv )(GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglLightf )(GLenum light, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglLightfv )(GLenum light, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglLighti )(GLenum light, GLenum pname, GLint param);
extern  void ( APIENTRY * qglLightiv )(GLenum light, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglLineStipple )(GLint factor, GLushort pattern);
extern  void ( APIENTRY * qglLineWidth )(GLfloat width);
extern  void ( APIENTRY * qglListBase )(GLuint base);
extern  void ( APIENTRY * qglLoadIdentity )(void);
extern  void ( APIENTRY * qglLoadMatrixd )(const GLdouble *m);
extern  void ( APIENTRY * qglLoadMatrixf )(const GLfloat *m);
extern  void ( APIENTRY * qglLoadName )(GLuint name);
extern  void ( APIENTRY * qglLogicOp )(GLenum opcode);
extern  void ( APIENTRY * qglMap1d )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
extern  void ( APIENTRY * qglMap1f )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
extern  void ( APIENTRY * qglMap2d )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
extern  void ( APIENTRY * qglMap2f )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
extern  void ( APIENTRY * qglMapGrid1d )(GLint un, GLdouble u1, GLdouble u2);
extern  void ( APIENTRY * qglMapGrid1f )(GLint un, GLfloat u1, GLfloat u2);
extern  void ( APIENTRY * qglMapGrid2d )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
extern  void ( APIENTRY * qglMapGrid2f )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
extern  void ( APIENTRY * qglMaterialf )(GLenum face, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglMaterialfv )(GLenum face, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglMateriali )(GLenum face, GLenum pname, GLint param);
extern  void ( APIENTRY * qglMaterialiv )(GLenum face, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglMatrixMode )(GLenum mode);
extern  void ( APIENTRY * qglMultMatrixd )(const GLdouble *m);
extern  void ( APIENTRY * qglMultMatrixf )(const GLfloat *m);
extern  void ( APIENTRY * qglNewList )(GLuint list, GLenum mode);
extern  void ( APIENTRY * qglNormal3b )(GLbyte nx, GLbyte ny, GLbyte nz);
extern  void ( APIENTRY * qglNormal3bv )(const GLbyte *v);
extern  void ( APIENTRY * qglNormal3d )(GLdouble nx, GLdouble ny, GLdouble nz);
extern  void ( APIENTRY * qglNormal3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglNormal3f )(GLfloat nx, GLfloat ny, GLfloat nz);
extern  void ( APIENTRY * qglNormal3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglNormal3i )(GLint nx, GLint ny, GLint nz);
extern  void ( APIENTRY * qglNormal3iv )(const GLint *v);
extern  void ( APIENTRY * qglNormal3s )(GLshort nx, GLshort ny, GLshort nz);
extern  void ( APIENTRY * qglNormal3sv )(const GLshort *v);
extern  void ( APIENTRY * qglNormalPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglOrtho )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern  void ( APIENTRY * qglPassThrough )(GLfloat token);
extern  void ( APIENTRY * qglPixelMapfv )(GLenum map, GLsizei mapsize, const GLfloat *values);
extern  void ( APIENTRY * qglPixelMapuiv )(GLenum map, GLsizei mapsize, const GLuint *values);
extern  void ( APIENTRY * qglPixelMapusv )(GLenum map, GLsizei mapsize, const GLushort *values);
extern  void ( APIENTRY * qglPixelStoref )(GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglPixelStorei )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglPixelTransferf )(GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglPixelTransferi )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglPixelZoom )(GLfloat xfactor, GLfloat yfactor);
extern  void ( APIENTRY * qglPointSize )(GLfloat size);
extern  void ( APIENTRY * qglPolygonMode )(GLenum face, GLenum mode);
extern  void ( APIENTRY * qglPolygonOffset )(GLfloat factor, GLfloat units);
extern  void ( APIENTRY * qglPolygonStipple )(const GLubyte *mask);
extern  void ( APIENTRY * qglPopAttrib )(void);
extern  void ( APIENTRY * qglPopClientAttrib )(void);
extern  void ( APIENTRY * qglPopMatrix )(void);
extern  void ( APIENTRY * qglPopName )(void);
extern  void ( APIENTRY * qglPrioritizeTextures )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
extern  void ( APIENTRY * qglPushAttrib )(GLbitfield mask);
extern  void ( APIENTRY * qglPushClientAttrib )(GLbitfield mask);
extern  void ( APIENTRY * qglPushMatrix )(void);
extern  void ( APIENTRY * qglPushName )(GLuint name);
extern  void ( APIENTRY * qglRasterPos2d )(GLdouble x, GLdouble y);
extern  void ( APIENTRY * qglRasterPos2dv )(const GLdouble *v);
extern  void ( APIENTRY * qglRasterPos2f )(GLfloat x, GLfloat y);
extern  void ( APIENTRY * qglRasterPos2fv )(const GLfloat *v);
extern  void ( APIENTRY * qglRasterPos2i )(GLint x, GLint y);
extern  void ( APIENTRY * qglRasterPos2iv )(const GLint *v);
extern  void ( APIENTRY * qglRasterPos2s )(GLshort x, GLshort y);
extern  void ( APIENTRY * qglRasterPos2sv )(const GLshort *v);
extern  void ( APIENTRY * qglRasterPos3d )(GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglRasterPos3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglRasterPos3f )(GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglRasterPos3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglRasterPos3i )(GLint x, GLint y, GLint z);
extern  void ( APIENTRY * qglRasterPos3iv )(const GLint *v);
extern  void ( APIENTRY * qglRasterPos3s )(GLshort x, GLshort y, GLshort z);
extern  void ( APIENTRY * qglRasterPos3sv )(const GLshort *v);
extern  void ( APIENTRY * qglRasterPos4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern  void ( APIENTRY * qglRasterPos4dv )(const GLdouble *v);
extern  void ( APIENTRY * qglRasterPos4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern  void ( APIENTRY * qglRasterPos4fv )(const GLfloat *v);
extern  void ( APIENTRY * qglRasterPos4i )(GLint x, GLint y, GLint z, GLint w);
extern  void ( APIENTRY * qglRasterPos4iv )(const GLint *v);
extern  void ( APIENTRY * qglRasterPos4s )(GLshort x, GLshort y, GLshort z, GLshort w);
extern  void ( APIENTRY * qglRasterPos4sv )(const GLshort *v);
extern  void ( APIENTRY * qglReadBuffer )(GLenum mode);
extern  void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
extern  void ( APIENTRY * qglRectd )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
extern  void ( APIENTRY * qglRectdv )(const GLdouble *v1, const GLdouble *v2);
extern  void ( APIENTRY * qglRectf )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
extern  void ( APIENTRY * qglRectfv )(const GLfloat *v1, const GLfloat *v2);
extern  void ( APIENTRY * qglRecti )(GLint x1, GLint y1, GLint x2, GLint y2);
extern  void ( APIENTRY * qglRectiv )(const GLint *v1, const GLint *v2);
extern  void ( APIENTRY * qglRects )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
extern  void ( APIENTRY * qglRectsv )(const GLshort *v1, const GLshort *v2);
extern  GLint ( APIENTRY * qglRenderMode )(GLenum mode);
extern  void ( APIENTRY * qglRotated )(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglRotatef )(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglScaled )(GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglScalef )(GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglSelectBuffer )(GLsizei size, GLuint *buffer);
extern  void ( APIENTRY * qglShadeModel )(GLenum mode);
extern  void ( APIENTRY * qglStencilFunc )(GLenum func, GLint ref, GLuint mask);
extern  void ( APIENTRY * qglStencilMask )(GLuint mask);
extern  void ( APIENTRY * qglStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
extern  void ( APIENTRY * qglTexCoord1d )(GLdouble s);
extern  void ( APIENTRY * qglTexCoord1dv )(const GLdouble *v);
extern  void ( APIENTRY * qglTexCoord1f )(GLfloat s);
extern  void ( APIENTRY * qglTexCoord1fv )(const GLfloat *v);
extern  void ( APIENTRY * qglTexCoord1i )(GLint s);
extern  void ( APIENTRY * qglTexCoord1iv )(const GLint *v);
extern  void ( APIENTRY * qglTexCoord1s )(GLshort s);
extern  void ( APIENTRY * qglTexCoord1sv )(const GLshort *v);
extern  void ( APIENTRY * qglTexCoord2d )(GLdouble s, GLdouble t);
extern  void ( APIENTRY * qglTexCoord2dv )(const GLdouble *v);
extern  void ( APIENTRY * qglTexCoord2f )(GLfloat s, GLfloat t);
extern  void ( APIENTRY * qglTexCoord2fv )(const GLfloat *v);
extern  void ( APIENTRY * qglTexCoord2i )(GLint s, GLint t);
extern  void ( APIENTRY * qglTexCoord2iv )(const GLint *v);
extern  void ( APIENTRY * qglTexCoord2s )(GLshort s, GLshort t);
extern  void ( APIENTRY * qglTexCoord2sv )(const GLshort *v);
extern  void ( APIENTRY * qglTexCoord3d )(GLdouble s, GLdouble t, GLdouble r);
extern  void ( APIENTRY * qglTexCoord3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglTexCoord3f )(GLfloat s, GLfloat t, GLfloat r);
extern  void ( APIENTRY * qglTexCoord3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglTexCoord3i )(GLint s, GLint t, GLint r);
extern  void ( APIENTRY * qglTexCoord3iv )(const GLint *v);
extern  void ( APIENTRY * qglTexCoord3s )(GLshort s, GLshort t, GLshort r);
extern  void ( APIENTRY * qglTexCoord3sv )(const GLshort *v);
extern  void ( APIENTRY * qglTexCoord4d )(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern  void ( APIENTRY * qglTexCoord4dv )(const GLdouble *v);
extern  void ( APIENTRY * qglTexCoord4f )(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern  void ( APIENTRY * qglTexCoord4fv )(const GLfloat *v);
extern  void ( APIENTRY * qglTexCoord4i )(GLint s, GLint t, GLint r, GLint q);
extern  void ( APIENTRY * qglTexCoord4iv )(const GLint *v);
extern  void ( APIENTRY * qglTexCoord4s )(GLshort s, GLshort t, GLshort r, GLshort q);
extern  void ( APIENTRY * qglTexCoord4sv )(const GLshort *v);
extern  void ( APIENTRY * qglTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglTexEnvf )(GLenum target, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglTexEnvfv )(GLenum target, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglTexEnvi )(GLenum target, GLenum pname, GLint param);
extern  void ( APIENTRY * qglTexEnviv )(GLenum target, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglTexGend )(GLenum coord, GLenum pname, GLdouble param);
extern  void ( APIENTRY * qglTexGendv )(GLenum coord, GLenum pname, const GLdouble *params);
extern  void ( APIENTRY * qglTexGenf )(GLenum coord, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglTexGenfv )(GLenum coord, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglTexGeni )(GLenum coord, GLenum pname, GLint param);
extern  void ( APIENTRY * qglTexGeniv )(GLenum coord, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglTexImage1D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglTexParameterf )(GLenum target, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglTexParameterfv )(GLenum target, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglTexParameteri )(GLenum target, GLenum pname, GLint param);
extern  void ( APIENTRY * qglTexParameteriv )(GLenum target, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglTranslated )(GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglTranslatef )(GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglVertex2d )(GLdouble x, GLdouble y);
extern  void ( APIENTRY * qglVertex2dv )(const GLdouble *v);
extern  void ( APIENTRY * qglVertex2f )(GLfloat x, GLfloat y);
extern  void ( APIENTRY * qglVertex2fv )(const GLfloat *v);
extern  void ( APIENTRY * qglVertex2i )(GLint x, GLint y);
extern  void ( APIENTRY * qglVertex2iv )(const GLint *v);
extern  void ( APIENTRY * qglVertex2s )(GLshort x, GLshort y);
extern  void ( APIENTRY * qglVertex2sv )(const GLshort *v);
extern  void ( APIENTRY * qglVertex3d )(GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglVertex3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglVertex3f )(GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglVertex3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglVertex3i )(GLint x, GLint y, GLint z);
extern  void ( APIENTRY * qglVertex3iv )(const GLint *v);
extern  void ( APIENTRY * qglVertex3s )(GLshort x, GLshort y, GLshort z);
extern  void ( APIENTRY * qglVertex3sv )(const GLshort *v);
extern  void ( APIENTRY * qglVertex4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern  void ( APIENTRY * qglVertex4dv )(const GLdouble *v);
extern  void ( APIENTRY * qglVertex4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern  void ( APIENTRY * qglVertex4fv )(const GLfloat *v);
extern  void ( APIENTRY * qglVertex4i )(GLint x, GLint y, GLint z, GLint w);
extern  void ( APIENTRY * qglVertex4iv )(const GLint *v);
extern  void ( APIENTRY * qglVertex4s )(GLshort x, GLshort y, GLshort z, GLshort w);
extern  void ( APIENTRY * qglVertex4sv )(const GLshort *v);
extern  void ( APIENTRY * qglVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

extern	void ( APIENTRY * qglPointParameterfEXT)( GLenum param, GLfloat value );
extern	void ( APIENTRY * qglPointParameterfvEXT)( GLenum param, const GLfloat *value );
extern	void ( APIENTRY * qglColorTableEXT)( int, int, int, int, int, const void * );

extern	void ( APIENTRY * qglLockArraysEXT) (int , int);
extern	void ( APIENTRY * qglUnlockArraysEXT) (void);

extern	void ( APIENTRY * qglMTexCoord2fSGIS)( GLenum, GLfloat, GLfloat );
extern	void ( APIENTRY * qglSelectTextureSGIS)( GLenum );

extern	void ( APIENTRY * qglActiveTextureARB)( GLenum );
extern	void ( APIENTRY * qglClientActiveTextureARB)( GLenum );

#ifdef _WIN32

extern  int   ( WINAPI * qwglChoosePixelFormat )(HDC, CONST PIXELFORMATDESCRIPTOR *);
extern  int   ( WINAPI * qwglDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
extern  int   ( WINAPI * qwglGetPixelFormat)(HDC);
extern  BOOL  ( WINAPI * qwglSetPixelFormat)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
extern  BOOL  ( WINAPI * qwglSwapBuffers)(HDC);

extern BOOL  ( WINAPI * qwglCopyContext)(HGLRC, HGLRC, UINT);
extern HGLRC ( WINAPI * qwglCreateContext)(HDC);
extern HGLRC ( WINAPI * qwglCreateLayerContext)(HDC, int);
extern BOOL  ( WINAPI * qwglDeleteContext)(HGLRC);
extern HGLRC ( WINAPI * qwglGetCurrentContext)(VOID);
extern HDC   ( WINAPI * qwglGetCurrentDC)(VOID);
extern PROC  ( WINAPI * qwglGetProcAddress)(LPCSTR);
extern BOOL  ( WINAPI * qwglMakeCurrent)(HDC, HGLRC);
extern BOOL  ( WINAPI * qwglShareLists)(HGLRC, HGLRC);
extern BOOL  ( WINAPI * qwglUseFontBitmaps)(HDC, DWORD, DWORD, DWORD);

extern BOOL  ( WINAPI * qwglUseFontOutlines)(HDC, DWORD, DWORD, DWORD, FLOAT,
                                           FLOAT, int, LPGLYPHMETRICSFLOAT);

extern BOOL ( WINAPI * qwglDescribeLayerPlane)(HDC, int, int, UINT,
                                            LPLAYERPLANEDESCRIPTOR);
extern int  ( WINAPI * qwglSetLayerPaletteEntries)(HDC, int, int, int,
                                                CONST COLORREF *);
extern int  ( WINAPI * qwglGetLayerPaletteEntries)(HDC, int, int, int,
                                                COLORREF *);
extern BOOL ( WINAPI * qwglRealizeLayerPalette)(HDC, int, BOOL);
extern BOOL ( WINAPI * qwglSwapLayerBuffers)(HDC, UINT);

extern BOOL ( WINAPI * qwglSwapIntervalEXT)( int interval );

extern BOOL (APIENTRY * qwglGetDeviceGammaRamp) (HDC, LPVOID);
extern BOOL (APIENTRY * qwglSetDeviceGammaRamp) (HDC, LPVOID);
extern BOOL APIENTRY qwglSetDeviceGammaRampTexture(HDC, LPWORD);
/*
** WGL_ARB_extensions_string
*/
extern  const char *( WINAPI * qwglGetExtensionsStringARB)(HDC);

/*
** WGL_ARB_pixel_format
*/
#define	WGL_NUMBER_PIXEL_FORMATS_ARB	0x2000
#define	WGL_DRAW_TO_WINDOW_ARB			0x2001
#define	WGL_DRAW_TO_BITMAP_ARB			0x2002
#define	WGL_ACCELERATION_ARB			0x2003
#define	WGL_NEED_PALETTE_ARB			0x2004
#define	WGL_NEED_SYSTEM_PALETTE_ARB		0x2005
#define	WGL_SWAP_LAYER_BUFFERS_ARB		0x2006
#define	WGL_SWAP_METHOD_ARB				0x2007
#define	WGL_NUMBER_OVERLAYS_ARB			0x2008
#define	WGL_NUMBER_UNDERLAYS_ARB		0x2009
#define	WGL_TRANSPARENT_ARB				0x200A
#define	WGL_TRANSPARENT_RED_VALUE_ARB	0x2037
#define	WGL_TRANSPARENT_GREEN_VALUE_ARB	0x2038
#define	WGL_TRANSPARENT_BLUE_VALUE_ARB	0x2039
#define	WGL_TRANSPARENT_ALPHA_VALUE_ARB	0x203A
#define	WGL_TRANSPARENT_INDEX_VALUE_ARB	0x203B
#define	WGL_SHARE_DEPTH_ARB				0x200C
#define	WGL_SHARE_STENCIL_ARB			0x200D
#define	WGL_SHARE_ACCUM_ARB				0x200E
#define	WGL_SUPPORT_GDI_ARB				0x200F
#define	WGL_SUPPORT_OPENGL_ARB			0x2010
#define	WGL_DOUBLE_BUFFER_ARB			0x2011
#define	WGL_STEREO_ARB					0x2012
#define	WGL_PIXEL_TYPE_ARB				0x2013
#define	WGL_COLOR_BITS_ARB				0x2014
#define	WGL_RED_BITS_ARB				0x2015
#define	WGL_RED_SHIFT_ARB				0x2016
#define	WGL_GREEN_BITS_ARB				0x2017
#define	WGL_GREEN_SHIFT_ARB				0x2018
#define	WGL_BLUE_BITS_ARB				0x2019
#define	WGL_BLUE_SHIFT_ARB				0x201A
#define	WGL_ALPHA_BITS_ARB				0x201B
#define	WGL_ALPHA_SHIFT_ARB				0x201C
#define	WGL_ACCUM_BITS_ARB				0x201D
#define	WGL_ACCUM_RED_BITS_ARB			0x201E
#define	WGL_ACCUM_GREEN_BITS_ARB		0x201F
#define	WGL_ACCUM_BLUE_BITS_ARB			0x2020
#define	WGL_ACCUM_ALPHA_BITS_ARB		0x2021
#define	WGL_DEPTH_BITS_ARB				0x2022
#define	WGL_STENCIL_BITS_ARB			0x2023
#define	WGL_AUX_BUFFERS_ARB				0x2024
#define	WGL_NO_ACCELERATION_ARB			0x2025
#define	WGL_GENERIC_ACCELERATION_ARB	0x2026
#define	WGL_FULL_ACCELERATION_ARB		0x2027
#define	WGL_SWAP_EXCHANGE_ARB			0x2028
#define	WGL_SWAP_COPY_ARB				0x2029
#define	WGL_SWAP_UNDEFINED_ARB			0x202A
#define	WGL_TYPE_RGBA_ARB				0x202B
#define	WGL_TYPE_COLORINDEX_ARB			0x202C

extern  BOOL ( APIENTRY * qwglGetPixelFormatAttribivARB ) (HDC, GLint, GLint, GLuint,
													const GLint *, GLint *);
extern  BOOL ( APIENTRY * qwglGetPixelFormatAttribfvARB ) (HDC, GLint, GLint, GLuint,
													const GLint *, GLfloat *);
extern  BOOL ( APIENTRY * qwglChoosePixelFormatARB ) (HDC hdc, const GLint *,
													const GLfloat *, GLuint,
													GLint *, GLuint *);
/*
** WGL_3DFX_multisample
*/
#define	WGL_SAMPLE_BUFFERS_3DFX			0x2060
#define	WGL_SAMPLES_3DFX				0x2061
extern qboolean use_WGL_3DFX_multisample;

#endif

/*
** GL_3DFX_multisample
*/
#define	MULTISAMPLE_3DFX				0x86B2
#define	SAMPLE_BUFFERS_3DFX				0x86B3
#define	SAMPLES_3DFX					0x86B4
#define	MULTISAMPLE_BIT_3DFX			0x20000000

/*
** GL_3DFX_tbuffer
*/
extern  void ( APIENTRY * qglTBufferMask3DFX) (GLuint mask);
extern  void ( __stdcall * qgrTBufferWriteMaskExt) (unsigned int);


/*
** GL_SGIS_generate_mipmap
*/
#ifndef GL_SGIS_generate_mipmap
#define GL_SGIS_generate_mipmap 1
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192
#endif


/*
** WGL_ARB_multisample
*/
#ifndef WGL_ARB_multisample
#define WGL_ARB_multisample 1
#define WGL_SAMPLE_BUFFERS_ARB         0x2041
#define WGL_SAMPLES_ARB                0x2042
#endif
extern qboolean use_WGL_ARB_multisample;


/*
** GL_ARB_multisample
*/
#ifndef GL_ARB_multisample
#define GL_ARB_multisample 1
#define GL_MULTISAMPLE_ARB                0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB   0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_ARB        0x809F
#define GL_SAMPLE_COVERAGE_ARB            0x80A0
#define GL_SAMPLE_BUFFERS_ARB             0x80A8
#define GL_SAMPLES_ARB                    0x80A9
#define GL_SAMPLE_COVERAGE_VALUE_ARB      0x80AA
#define GL_SAMPLE_COVERAGE_INVERT_ARB     0x80AB
#define GL_MULTISAMPLE_BIT_ARB            0x20000000
typedef void (APIENTRY * PFNGLSAMPLECOVERAGEARBPROC) (GLclampf value, GLboolean invert);
#endif


/*
** GL_ARB_texture_cube_map
*/
#ifndef GL_ARB_texture_cube_map
#define GL_ARB_texture_cube_map 1
#define GL_NORMAL_MAP_ARB                 0x8511
#define GL_REFLECTION_MAP_ARB             0x8512
#define GL_TEXTURE_CUBE_MAP_ARB           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  0x851C
#endif


/*
** GL_ARB_texture_env_combine
*/
#ifndef GL_ARB_texture_env_combine
#define GL_ARB_texture_env_combine 1
#define GL_COMBINE_ARB                    0x8570
#define GL_COMBINE_RGB_ARB                0x8571
#define GL_COMBINE_ALPHA_ARB              0x8572
#define GL_SOURCE0_RGB_ARB                0x8580
#define GL_SOURCE1_RGB_ARB                0x8581
#define GL_SOURCE2_RGB_ARB                0x8582
#define GL_SOURCE0_ALPHA_ARB              0x8588
#define GL_SOURCE1_ALPHA_ARB              0x8589
#define GL_SOURCE2_ALPHA_ARB              0x858A
#define GL_OPERAND0_RGB_ARB               0x8590
#define GL_OPERAND1_RGB_ARB               0x8591
#define GL_OPERAND2_RGB_ARB               0x8592
#define GL_OPERAND0_ALPHA_ARB             0x8598
#define GL_OPERAND1_ALPHA_ARB             0x8599
#define GL_OPERAND2_ALPHA_ARB             0x859A
#define GL_RGB_SCALE_ARB                  0x8573
#define GL_ADD_SIGNED_ARB                 0x8574
#define GL_INTERPOLATE_ARB                0x8575
#define GL_SUBTRACT_ARB                   0x84E7
#define GL_CONSTANT_ARB                   0x8576
#define GL_PRIMARY_COLOR_ARB              0x8577
#define GL_PREVIOUS_ARB                   0x8578
#endif


/*
** GL_ARB_texture_env_dot3
*/
#ifndef GL_ARB_texture_env_dot3
#define GL_ARB_texture_env_dot3 1
#define GL_DOT3_RGB_ARB                   0x86AE
#define GL_DOT3_RGBA_ARB                  0x86AF
#endif

#ifdef __linux__

// local function in dll
extern void *qwglGetProcAddress(char *symbol);

extern void (*qgl3DfxSetPaletteEXT)(GLuint *);

/*
//FX Mesa Functions
extern fxMesaContext (*qfxMesaCreateContext)(GLuint win, GrScreenResolution_t, GrScreenRefresh_t, const GLint attribList[]);
extern fxMesaContext (*qfxMesaCreateBestContext)(GLuint win, GLint width, GLint height, const GLint attribList[]);
extern void (*qfxMesaDestroyContext)(fxMesaContext ctx);
extern void (*qfxMesaMakeCurrent)(fxMesaContext ctx);
extern fxMesaContext (*qfxMesaGetCurrentContext)(void);
extern void (*qfxMesaSwapBuffers)(void);
*/

//GLX Functions
extern XVisualInfo * (*qglXChooseVisual)( Display *dpy, int screen, int *attribList );
extern GLXContext (*qglXCreateContext)( Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct );
extern void (*qglXDestroyContext)( Display *dpy, GLXContext ctx );
extern Bool (*qglXMakeCurrent)( Display *dpy, GLXDrawable drawable, GLXContext ctx);
extern void (*qglXCopyContext)( Display *dpy, GLXContext src, GLXContext dst, GLuint mask );
extern void (*qglXSwapBuffers)( Display *dpy, GLXDrawable drawable );

// 3dfxSetPaletteEXT shunt
void Fake_glColorTableEXT( GLenum target, GLenum internalformat,
                             GLsizei width, GLenum format, GLenum type,
                             const GLvoid *table );

#endif // linux

/*
** extension constants
*/
#define GL_POINT_SIZE_MIN_EXT				0x8126
#define GL_POINT_SIZE_MAX_EXT				0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT	0x8128
#define GL_DISTANCE_ATTENUATION_EXT			0x8129

#ifdef __sgi
#define GL_SHARED_TEXTURE_PALETTE_EXT		GL_TEXTURE_COLOR_TABLE_SGI
#else
#define GL_SHARED_TEXTURE_PALETTE_EXT		0x81FB
#endif

#define GL_TEXTURE0_SGIS					0x835E
#define GL_TEXTURE1_SGIS					0x835F
#define GL_TEXTURE0_ARB						0x84C0
#define GL_TEXTURE1_ARB						0x84C1
#undef GL_TEXTURE0
#undef GL_TEXTURE1

extern int GL_TEXTURE0, GL_TEXTURE1;

// GL_ARB_fragment_program
extern PFNGLPROGRAMSTRINGARBPROC qglProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC qglBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC qglDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC qglGenProgramsARB;
extern PFNGLPROGRAMENVPARAMETER4DARBPROC qglProgramEnvParameter4dARB;
extern PFNGLPROGRAMENVPARAMETER4DVARBPROC qglProgramEnvParameter4dvARB;
extern PFNGLPROGRAMENVPARAMETER4FARBPROC qglProgramEnvParameter4fARB;
extern PFNGLPROGRAMENVPARAMETER4FVARBPROC qglProgramEnvParameter4fvARB;
extern PFNGLPROGRAMLOCALPARAMETER4DARBPROC qglProgramLocalParameter4dARB;
extern PFNGLPROGRAMLOCALPARAMETER4DVARBPROC qglProgramLocalParameter4dvARB;
extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC qglProgramLocalParameter4fARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC qglProgramLocalParameter4fvARB;
extern PFNGLGETPROGRAMENVPARAMETERDVARBPROC qglGetProgramEnvParameterdvARB;
extern PFNGLGETPROGRAMENVPARAMETERFVARBPROC qglGetProgramEnvParameterfvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC qglGetProgramLocalParameterdvARB;
extern PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC qglGetProgramLocalParameterfvARB;
extern PFNGLGETPROGRAMIVARBPROC qglGetProgramivARB;
extern PFNGLGETPROGRAMSTRINGARBPROC qglGetProgramStringARB;
extern PFNGLISPROGRAMARBPROC qglIsProgramARB;

#endif
