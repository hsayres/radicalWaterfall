//
//  radicalWaterfall.cpp
//  Example of usage for chuck_fft
//  
//  Haley Sayres || Music 256a || October 2013
// 
//  Third party code used:
//  - chuck_fft.h/cpp based on the FFT implementation in CARL
//  - Thread.h/cpp from STK
//  - FFT created by Jorge Herrera


#ifdef __MACOSX_CORE__
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>1
#endif


#include <iostream>
#include <math.h>
#include "RtAudio.h"

#include "chuck_fft.h"
#include "Thread.h"


#define MY_SRATE 44100
#define SAMPLE double 
#define ZPF 1
// corresponding format for RtAudio
#define MY_FORMAT RTAUDIO_FLOAT64

using namespace std;

// ===========
// = Globals =
// ===========
GLsizei g_width = 800; 
GLsizei g_height = 600;

Mutex g_mutex;

float * g_fftBuff;
float * g_window;
bool g_useWindow = false;
bool g_displayBars = false;
unsigned int g_buffSize = 512;
GLfloat g_Xinc = 0.0;
GLfloat g_Yinc = 0.0;
GLfloat g_lastWidth = g_width;
GLfloat g_lastHeight = g_height;


//-----------------------------------------------------------------------------
// Defines a point in a 3D space (coords x, y and z)
//-----------------------------------------------------------------------------
struct pt3d
{
    pt3d( GLfloat x, GLfloat y, GLfloat z ) : x(x), y(y), z(z) {};
    
    float x;
    float y;
    float z;
};



// =======================
// = Function prototypes =
// =======================
void idleFunc( );
void displayFunc( );
void reshapeFunc( int width, int height );
void keyboardFunc( unsigned char, int, int );
void mouseFunc( int button, int state, int x, int y );
void specialFunc( int key, int x, int y );
void initialize( );
void changeLookAt( pt3d look_from, pt3d look_to, pt3d head_up );
void drawAxis();



// ==========================
// = Cammera placement vars =
// ==========================
// Camera control global variables
pt3d g_look_from( 1.0, .5 + g_Xinc, 2.5+ g_Yinc);
pt3d g_look_to( 1, .3, 0 );
pt3d g_head_up( 0, 5, 0 );



// ============
// = GL stuff =
// ============
//-----------------------------------------------------------------------------
// Name: initialize( )
// Desc: sets initial OpenGL states
//       also initializes any application data
//-----------------------------------------------------------------------------
void initialize()
{

    // =================
    // = OpenGL & GLUT =
    // =================

    // set the GL clear color - use when the color buffer is cleared
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        
    // set the shading model to 'smooth'
    glShadeModel( GL_SMOOTH );
    // enable depth
    glEnable( GL_DEPTH_TEST );
    // set the front faces of polygons
    glFrontFace( GL_CCW );
    // set fill mode
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          
    // seed random number generator
    srand( time(NULL) );
    
    //print statements giving user instructions on how to use program
    printf( "--------------------------------------------------------------------------------------------\n");
    printf( "left/ right arrows change the depth of where viewer looks from\n");
    printf( "up/ down arrows change the y-axis of where viewer looks from\n");
    printf( "magenta line is drawn where maximum frequency occurs, with maximum amplitude of that freq\n");
    printf( "enter 'F' or 'f' to enter fullscreen, and 'B' or 'b' to go back to original screen dimensions\n");
    printf( "--------------------------------------------------------------------------------------------\n");
}


//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
void reshapeFunc( int w, int h )
{
    // save the new window size
    g_width = (GLsizei)w; 
    g_height = (GLsizei)h;
    // map the view port to the client area
    glViewport( 0, 0, (GLsizei)w, (GLsizei)h );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // create the viewing frustum
    gluPerspective( 45.0, (GLfloat) w / (GLfloat) h, 0.1, 300.0 );
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    changeLookAt( g_look_from,g_look_to, g_head_up );
}


//-----------------------------------------------------------------------------
// Name: keyboardFunc( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboardFunc( unsigned char key, int x, int y )
{
    switch( key )
    {
    case 'Q':
    case 'q':
        cerr << "Bye!" << endl;
        exit(1);
        break;
    case 'L':
    case 'l':
        g_look_from = pt3d( -1, 0, 0 );
        cerr << "Looking from the left" << endl;
        break;
    case 'R':
    case 'r':
        g_look_from = pt3d( 1, 0, 0 );
        cerr << "Looking from the right" << endl;
        break;
    case 'F':
    case 'f':
            glutFullScreen(); //enter full screen
        break;
    case 'B':
    case 'b':
            glutReshapeWindow (g_lastWidth, g_lastHeight); //back to reg width and height
        break;
    }
    glutPostRedisplay( );
}



//-----------------------------------------------------------------------------
// Name: mouseFunc( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouseFunc( int button, int state, int x, int y )
{
    if( button == GLUT_LEFT_BUTTON )
    {
        // when left mouse button is down, move left
        if( state == GLUT_DOWN )
        {
        }
        else
        {
        }
    }
    else if ( button == GLUT_RIGHT_BUTTON )
    {
        // when right mouse button down, move right
        if( state == GLUT_DOWN )
        {
        }
        else
        {
        }
    }
    else
    {
    }
    glutPostRedisplay( );
}


//-----------------------------------------------------------------------------
// Name: specialFunc( )
// Desc: handles special function keys
//-----------------------------------------------------------------------------
void specialFunc( int key, int x, int y )
{
    if( key == GLUT_KEY_LEFT)
    {
        std::cerr << "Left arrow";
        g_Yinc -= .1f;
    }
    if( key == GLUT_KEY_RIGHT)
    {
        std::cerr << "Right arrow";
        g_Yinc += .1f;
    }
    if( key == GLUT_KEY_DOWN)
    {
        std::cerr << "Down arrow";
        g_Xinc -= .1f;
    }
    if( key == GLUT_KEY_UP)
    {
        std::cerr << "Up arrow";
        g_Xinc += .1f;
    }
    
    reshapeFunc( g_width, g_height);
    glutPostRedisplay( );
}


//-----------------------------------------------------------------------------
// Name: idleFunc( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idleFunc( )
{
    // render the scene
    glutPostRedisplay( );
}


//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void displayFunc( )
{
    
    const size_t MAX_TIME = 10000;
    static int now = 0, t = 0, prevAmp;
    static float maxAmp[MAX_TIME];
    static float maxFreq[MAX_TIME];
    
    static float spec[MAX_TIME][512][2]; //x, y
    
    // clear the color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // save the current transformation
    glPushMatrix();
    
    // set the view point (camera)
    changeLookAt( g_look_from, g_look_to, g_head_up );
    
    glLineWidth( 1.0f );
    glTranslatef( -.5, -0.5, 0 );
    
    glColor3f(.5, 1.0, 1.0 );
   
    
    // cast to use the complex data type defined in chuck_fft
    complex * fft = (complex *)g_fftBuff;
    
    if( g_fftBuff )
    {
        g_mutex.lock();
        
        //loop through time
        for (t = now; t >= 0; t--)
        {
            glBegin(GL_LINE_STRIP);
            
            maxAmp[now] =0;
            maxFreq[now] = 0;
            
            
            for(size_t f = 0; f < ZPF * g_buffSize / 2; ++f)
            {
                if (prevAmp < 1.) {
                    glColor3f(1.0, 1.0, 1.0 );
                } else if (prevAmp < 2.) {
                    glColor3f(1.0, 1.0, 0 );
                }else if (prevAmp < 3.) {
                    glColor3f(1.0, .75, 0 );
                }else if (prevAmp < 4.) {
                    glColor3f(1.0, .5, 0 );
                }else if (prevAmp < 5.) {
                    glColor3f(1.0, 0, 0 );
                }else if (prevAmp < 6.) {
                    glColor3f(.85, .25, .25 );
                }else if (prevAmp < 7.) {
                    glColor3f(.73, .56, .56 );
                }else if (prevAmp < 8.) {
                    glColor3f(.9, .2, .8 );
                }else if (prevAmp < 9.) {
                    glColor3f(1.0, 0, 1.0 );
                }else if (prevAmp < 10.) {
                    glColor3f(.4, .14, .55 );
                }
                
                spec[now][f][0] = 5* (float)f * 2.0 / (ZPF * (float)g_buffSize); // x-coordinate calculations
                spec[now][f][1] = 10 * 2 * ZPF * cmp_abs( fft[f] ); // y-coordinate calculations

                
                glVertex3f( spec[t][f][0] * 5, spec[t][f][1],  (t - now) * .2); //draw the correct wave
                
                //loop through to find maxAmp and maxFreq
                if (maxAmp[now] < spec[now][f][1]) {
                    maxAmp[now] = spec[now][f][1];
                    maxFreq[now] = spec[now][f][0];
                }
                prevAmp= maxAmp[now];
            }
            
            
            glEnd();
            
            glPushMatrix();
            glTranslatef(maxFreq[now], maxAmp[now], 0);
            glutSolidSphere(.1, 10, 10);
            glPopMatrix();
        }
    
        
        //draws magenta color line at maxFreq with maxAmp
        glColor4f(1, 0, 1, 1);
        glBegin(GL_LINE_STRIP);
        glVertex3f( maxFreq[now], 0,  0);
        glVertex3f( maxFreq[now], maxAmp[now],  0);
        glEnd();
        g_mutex.unlock();
        
        now++;

    
    

    }
    
    // restore state
    glPopMatrix();
    
    // flush!
    glFlush();
    // swap the double buffer
    glutSwapBuffers( );
    
}

//-----------------------------------------------------------------------------
// name: changeLookAt()
// desc: changes the point of view
//-----------------------------------------------------------------------------
void changeLookAt( pt3d look_from, pt3d look_to, pt3d head_up )
{
    gluLookAt(  look_from.x, look_from.y + g_Xinc, look_from.z + g_Yinc,
                look_to.x, look_to.y, look_to.z,
                head_up.x, head_up.y, head_up.z );
}



//-----------------------------------------------------------------------------
// name: drawAxis()
// desc: draw cartesian axis (x,y,z) using (r,g,b) respectively
//-----------------------------------------------------------------------------
void drawAxis()
{
    glPushMatrix();
        
    glBegin( GL_LINES );
    
        // x axis
        glColor4f( 1, 0, 0, 1 );
        glVertex3f( 0, 0, 0 );
        glVertex3f( 1, 0, 0 );

        // y axis
        glColor4f( 0, 1, 0, 1 );
        glVertex3f( 0, 0, 0 );
        glVertex3f( 0, 1, 0 );

        // z axis
        glColor4f( 0, 0, 1, 1 );
        glVertex3f( 0, 0, 0 );
        glVertex3f( 0, 0, 1 );

    glEnd();
    glPopMatrix();
}



// =========
// = Audio =
// =========
// Audio callback
int audioCallback( void * outputBuffer, void * inputBuffer, 
            unsigned int bufferSize, double streamTime,
            RtAudioStreamStatus status, void * userData )
{
    SAMPLE * out = (SAMPLE *)outputBuffer;
    SAMPLE * in = (SAMPLE *)inputBuffer;
        
    g_mutex.lock();
        
    // zero out the fft buffer (required only if ZPF > 1)
    memset( g_fftBuff, 0, ZPF * bufferSize * sizeof(float) );
    
    for(size_t i = 0; i < bufferSize; ++i)
    {
        out[i] = 0;
        // "static" sine wave for testing
        //g_fftBuff[i] = ::sin( 2 * M_PI * 2200.0f * i / MY_SRATE );
        g_fftBuff[i] = in[i];
    }
    
    // apply window to the buffer of audio
    if( g_useWindow )
        apply_window( g_fftBuff, g_window, g_buffSize );        

    // compute the fft
    rfft( g_fftBuff, ZPF * bufferSize / 2, FFT_FORWARD );
    
    g_mutex.unlock();
    
    return 0;
}


// ========
// = Main =
// ========
// Entry point
int main (int argc, char *argv[])
{
    
    // RtAudio config + init

    // pointer to RtAudio object
    RtAudio *  audio = NULL;

    // create the object
    try
    {
        audio = new RtAudio();
    }
        catch( RtError & err ) {
        err.printMessage();
        exit(1);
    }

    if( audio->getDeviceCount() < 1 )
    {
        // nopes
        cout << "no audio devices found!" << endl;
        exit( 1 );
    }
        
    // let RtAudio print messages to stderr.
    audio->showWarnings( true );

    // set input and output parameters
    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = audio->getDefaultInputDevice();
    iParams.nChannels = 1;
    iParams.firstChannel = 0;
    oParams.deviceId = audio->getDefaultOutputDevice();
    oParams.nChannels = 1;
    oParams.firstChannel = 0;
        
    // create stream options
    RtAudio::StreamOptions options;

    // set the callback and start stream
    try
    {
        audio->openStream( &oParams, &iParams, RTAUDIO_FLOAT64, MY_SRATE, &g_buffSize, &audioCallback, NULL, &options);
        
        cerr << "Buffer size defined by RtAudio: " << g_buffSize << endl;
        
        // allocate the buffer for the fft
        g_fftBuff = new float[g_buffSize * ZPF];
        if ( g_fftBuff == NULL ) {
            cerr << "Something went wrong when creating the fft buffers" << endl;
            exit (1);
        }
        
        // allocate the buffer for the time domain window
        g_window = new float[g_buffSize];
        if ( g_window == NULL ) {
            cerr << "Something went wrong when creating the window" << endl;
            exit (1);
        }

        // create a hanning window
        make_window( g_window, g_buffSize );
        
        // start the audio stream
        audio->startStream();
        
        // test RtAudio functionality for reporting latency.
        cout << "stream latency: " << audio->getStreamLatency() << " frames" << endl;
    }
    catch( RtError & err )
    {
        err.printMessage();
        goto cleanup;
    }


    // ============
    // = GL stuff =
    // ============

    // initialize GLUT
    glutInit( &argc, argv );
    // double buffer, use rgb color, enable depth buffer
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    // initialize the window size
    glutInitWindowSize( g_width, g_height );
    // set the window postion
    glutInitWindowPosition( 100, 100 );
    // create the window
    glutCreateWindow( "radicalWaterfall.cpp" );
    //glutEnterGameMode();

    // set the idle function - called when idle
    glutIdleFunc( idleFunc );
    // set the display function - called when redrawing
    glutDisplayFunc( displayFunc );
    // set the reshape function - called when client area changes
    glutReshapeFunc( reshapeFunc );
    // set the keyboard function - called on keyboard events
    glutKeyboardFunc( keyboardFunc );
    // set the mouse function - called on mouse stuff
    glutMouseFunc( mouseFunc );
    // set the special function - called on special keys events (fn, arrows, pgDown, etc)
    glutSpecialFunc( specialFunc );

    // enable depth test
    glEnable( GL_DEPTH_TEST );
    
    // do our own initialization
    initialize();

    // let GLUT handle the current thread from here
    glutMainLoop();

        
    // if we get here, stop!
    try
    {
        audio->stopStream();
    }
    catch( RtError & err )
    {
        err.printMessage();
    }

    // Clean up
    cleanup:
    if(audio)
    {
        audio->closeStream();
        delete audio;
    }

    
    return 0;
}
