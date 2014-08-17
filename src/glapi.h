/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#pragma once

#define GL_GLEXT_PROTOTYPES

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#define __IPHONEOS__
#include "SDL_opengles.h"

#define CreateVertexBuffers(nobuf, bufptr) glGenBuffers(nobuf, bufptr)
#define DeleteVertexBuffers(nobuf, bufptr) glDeleteBuffers(nobuf, bufptr)
#else
#include "SDL_opengl.h"

#define CreateVertexBuffers(nobuf, bufptr) glGenBuffersARB(nobuf, bufptr)
#define DeleteVertexBuffers(nobuf, bufptr) glDeleteBuffersARB(nobuf, bufptr)

#define glOrthof glOrtho
#define glDepthRangef glDepthRange
#define glFrustumf glFrustum
#endif
