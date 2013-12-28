#include <AndroidLog.h>
#include <NativeActivity.h>
#include <unistd.h>
#include "AndroidMessage.h"

#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>

extern "C"
{
	void android_main();
}

bool InitGL();
void destroy();

bool bIsVisible = false;
void callback( const AndroidMessage& message )
{
	LOGE( "NEW Message!! Whoohoo %i", message.iMessageID );

	if ( message.iMessageID == SurfaceCreated )
	{
		InitGL();
	}

	else if ( message.iMessageID == SurfaceDestroyed )
	{
		destroy();
	}

	else if ( message.iMessageID == WindowVisible )
	{
		bIsVisible = true;
	}

	else if ( message.iMessageID == WindowHidden )
	{
		bIsVisible = false;
	}
}

EGLDisplay _display = NULL;
EGLSurface _surface;
EGLContext _context;
bool bOpenGLInit = false;
void android_main()
{
	LOGE( "Hiiiiiiiii!" );

	NativeActivity::SetEventCallback( callback );

	while ( true )
	{
		NativeActivity::PollEvents();

		if ( bOpenGLInit && bIsVisible )
		{
			if (_display)
			{
				glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				if (!eglSwapBuffers(_display, _surface))
				{
					LOGE("eglSwapBuffers() returned error %d", eglGetError());
				}
			}
		}

		sleep( 1 );
	}
}

void destroy()
{
	LOGE( "Destroying context!" );
	bOpenGLInit = false;
	eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	    eglDestroyContext(_display, _context);
	    eglDestroySurface(_display, _surface);
	    eglTerminate(_display);

	    _display = EGL_NO_DISPLAY;
	    _surface = EGL_NO_SURFACE;
	_context = NULL;
}


bool InitGL()
{
	const EGLint attribs[] = {
	        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	        EGL_BLUE_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_RED_SIZE, 8,
	        EGL_NONE
	    };
	    EGLDisplay display;
	    EGLConfig config;
	    EGLint numConfigs;
	    EGLint format;
	    EGLSurface surface;
	    EGLContext context;
	    EGLint width;
	    EGLint height;
	    GLfloat ratio;

	    LOGV( "Initializing context" );

	    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
	        LOGE("eglGetDisplay() returned error %d", eglGetError());
	        return false;
	    }
	    if (!eglInitialize(display, 0, 0)) {
	    	LOGE("eglInitialize() returned error %d", eglGetError());
	        return false;
	    }

	    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
	    	LOGE("eglChooseConfig() returned error %d", eglGetError());
	        destroy();
	        return false;
	    }

	    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
	    	LOGE("eglGetConfigAttrib() returned error %d", eglGetError());
	        destroy();
	        return false;
	    }

	    ANativeWindow_setBuffersGeometry( NativeActivity::GetWindow(), 0, 0, format );

	    if (!(surface = eglCreateWindowSurface(display, config, NativeActivity::GetWindow(), 0)))
	    {
	    	LOGE("eglCreateWindowSurface() returned error %d", eglGetError());
	        destroy();
	        return false;
	    }

	    if (!(context = eglCreateContext(display, config, 0, 0))) {
	    	LOGE("eglCreateContext() returned error %d", eglGetError());
	        destroy();
	        return false;
	    }

	    if (!eglMakeCurrent(display, surface, surface, context)) {
	    	LOGE("eglMakeCurrent() returned error %d", eglGetError());
	        destroy();
	        return false;
	    }

	    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
	        !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
	    	LOGE("eglQuerySurface() returned error %d", eglGetError());
	        destroy();
	        return false;
	    }

	    _display = display;
	    _surface = surface;
	    _context = context;

	    glDisable(GL_DITHER);
	    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	    glClearColor(0, 0, 0, 0);
	    glEnable(GL_CULL_FACE);
	    //glShadeModel(GL_SMOOTH);
	    glEnable(GL_DEPTH_TEST);

	    bOpenGLInit = true;

	    return true;
}
