#include <AndroidLog.h>
#include <Android.h>
#include <unistd.h>
#include "AndroidMessage.h"

#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>

extern "C"
{
	void android_main();
}

bool InitGL();
void DestroyOpenGL();

bool bIsVisible = false;
bool bShouldQuit = false;
void callback( const AndroidMessage& message )
{
	if ( message.iMessageID == AndroidMessage_ApplicationShutdown )
	{
		// Shutdown the app
		bShouldQuit = true;
	}

	else if ( message.iMessageID == AndroidMessage_SurfaceCreated )
	{
		// Surface created, init OpenGL
		InitGL();
	}

	else if ( message.iMessageID == AndroidMessage_SurfaceDestroyed )
	{
		// Surface destroyed, destroy OpenGL
		DestroyOpenGL();
	}

	else if ( message.iMessageID == AndroidMessage_SurfaceChanged )
	{
		// Surface format and size may have changed.
		AndroidSurfaceChanged* pSurface;
		pSurface = (AndroidSurfaceChanged*)message.pData;

		// Set new viewport
		glViewport( 0, 0, pSurface->iWidth, pSurface->iHeight );
	}

	else if ( message.iMessageID == AndroidMessage_WindowVisible )
	{
		// We should now draw!
		bIsVisible = true;
	}

	else if ( message.iMessageID == AndroidMessage_WindowHidden )
	{
		// We shouldn't draw now...
		bIsVisible = false;
	}

	else if ( message.iMessageID == AndroidMessage_OnTouch )
	{
		// It touched me... eek!
		AndroidTouch* pTouch = (AndroidTouch*)message.pData;

		LOGV( "[Example]: Touch: %i, x: %f y:, %f action:, %i.", pTouch->iPointerID, pTouch->fPosX, pTouch->fPosY, pTouch->iAction );

		if ( pTouch->iAction == 0 )
		{
			Android::ShowKeyboard();
		}

		else if ( pTouch->iAction == 1 )
		{
			Android::HideKeyboard();
		}
	}

	else if ( message.iMessageID == AndroidMessage_OnKey )
	{
		// We have been pressed!
		AndroidKey* pKey = (AndroidKey*)message.pData;
		LOGV( "[Example]: Got key! %i %c", pKey->iKeyCode, pKey->iUnicodeChar );
	}
}

EGLDisplay _display = NULL;
EGLSurface _surface;
EGLContext _context;
bool bOpenGLInit = false;
void android_main()
{
	LOGV( "[Example]: android_main() called!" );

	// Set callback functions for events
	Android::SetEventCallback( callback );

	// While app is alive...
	while ( !bShouldQuit )
	{
		// Poll events
		Android::PollEvents();

		// If we have OpenGL setup and the window is visible
		if ( bOpenGLInit && bIsVisible )
		{
			// Draw

			glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

			if ( !eglSwapBuffers(_display, _surface ))
			{
					LOGE("eglSwapBuffers() returned error %d", eglGetError());
			}
		}
	}
}

void DestroyOpenGL()
{
	LOGV( "Destroying context!" );

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

	    if ( (display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY )
	    {
	        LOGE("eglGetDisplay() returned error %d", eglGetError());
	        return false;
	    }

	    if ( !eglInitialize(display, 0, 0) )
	    {
	    	LOGE("eglInitialize() returned error %d", eglGetError());
	        return false;
	    }

	    if ( !eglChooseConfig(display, attribs, &config, 1, &numConfigs) )
	    {
	    	LOGE("eglChooseConfig() returned error %d", eglGetError());
	    	DestroyOpenGL();
	        return false;
	    }

	    if ( !eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format) )
	    {
	    	LOGE("eglGetConfigAttrib() returned error %d", eglGetError());
	    	DestroyOpenGL();
	        return false;
	    }

	    // Android, set buffer geometry using our window which is saved in Android
	    ANativeWindow_setBuffersGeometry( Android::GetWindow(), 0, 0, format );

	    if (!(surface = eglCreateWindowSurface(display, config, Android::GetWindow(), 0)))
	    {
	    	LOGE("eglCreateWindowSurface() returned error %d", eglGetError());
	    	DestroyOpenGL();
	        return false;
	    }

	    if ( !(context = eglCreateContext(display, config, 0, 0)) )
	    {
	    	LOGE("eglCreateContext() returned error %d", eglGetError());
	    	DestroyOpenGL();
	        return false;
	    }

	    if ( !eglMakeCurrent(display, surface, surface, context) )
	    {
	    	LOGE("eglMakeCurrent() returned error %d", eglGetError());
	    	DestroyOpenGL();
	        return false;
	    }

	    if ( !eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
	        !eglQuerySurface(display, surface, EGL_HEIGHT, &height) )
	    {
	    	LOGE("eglQuerySurface() returned error %d", eglGetError());
	    	DestroyOpenGL();
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
