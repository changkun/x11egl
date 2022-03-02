// Copyright 2022 Changkun Ou <changkun.de>. All rights reserved.
// Use of this source code is governed by a GPLv3 license that
// can be found in the LICENSE file.

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <EGL/egl.h>
#include <GL/gl.h>

int main() {
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("x11: cannot connect to the X server");
        return -1;
    }

    XSetWindowAttributes swa;
    swa.event_mask = ExposureMask | FocusChangeMask |
        KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask;
    swa.background_pixmap = None;
    swa.override_redirect = False;

    Window oswin = XCreateWindow(display,
        XDefaultRootWindow(display),
        0, 0, 1280, 720,
        0, CopyFromParent, InputOutput, NULL,
        CWEventMask|CWBackPixmap|CWOverrideRedirect, &swa);

    Atom evDelWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
    Atom utf8string = XInternAtom(display, "UTF8_STRING", False);
    Atom wmName = XInternAtom(display, "_NET_WM_NAME", False);

    XTextProperty prop;
    prop.value  =     "";
    prop.encoding  =  utf8string;
    prop.format  =    8;
    prop.nitems  =    0;

    XSetWMProtocols(display, oswin, &evDelWindow, 1);
    XStoreName(display, oswin, "hello");
    XSetTextProperty(display, oswin, &prop, wmName);

    XMapWindow(display, oswin);
    XClearWindow(display, oswin);

    EGLDisplay eglDisp = eglGetDisplay((EGLNativeDisplayType)display);

    EGLint major;
    EGLint minor;
    EGLBoolean ret = eglInitialize(eglDisp, &major, &minor);
    if (ret != 1) {
        printf("egl: Initialize error: %d", eglGetError());
        return -1;
    }

    EGLint attr[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_CONFIG_CAVEAT, EGL_NONE, EGL_ALPHA_SIZE, 8, EGL_NONE,
    };

    EGLConfig cfg;
    EGLint ncfg;
    if (eglChooseConfig(eglDisp, attr, &cfg, 1, &ncfg) != EGL_TRUE) {
        printf("egl: ChooseConfig error: %d", eglGetError());
        return -1;
    };

    EGLSurface eglSurface = eglCreateWindowSurface(eglDisp, cfg, oswin, NULL);
    if (eglSurface == EGL_NO_SURFACE ) {
        printf("egl: CreateWindowSurface error: %d\n", eglGetError());
        return -1;
    }

    EGLContext nilCtx;
    EGLint ctxAttr[] = {
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE,
	};
    EGLContext eglCtx = eglCreateContext(eglDisp, cfg, nilCtx, ctxAttr);
    if (eglCtx == EGL_NO_CONTEXT) {
        printf("egl: CreateContext error: %d\n", eglGetError() );
        return 0;
    }
    eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx);

    XEvent ev;
    while (1) {
        XNextEvent(display, &ev);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1, 1, 1, 1);
        eglSwapBuffers (eglDisp, eglSurface);
    }
}
