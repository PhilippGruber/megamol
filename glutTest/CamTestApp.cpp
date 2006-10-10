/*
 * CamTestApp.cpp
 *
 * Copyright (C) 2006 by Universitaet Stuttgart (VIS). Alle Rechte vorbehalten.
 */
#include "CamTestApp.h"

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdio>

#include "vislib/graphicstypes.h"
#include "vislib/PerformanceCounter.h"
#include "vislib/Rectangle.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "vislogo.h"
#include <cstdlib>


/*
 * CamTestApp::Lens::Lens
 */
CamTestApp::Lens::Lens(void) {
    this->w = 0.25f;
    this->h = 0.25f;
    this->x = float(::rand() % 1000) / 1000.0f * (1.0f - this->w);
    this->y = float(::rand() % 1000) / 1000.0f * (1.0f - this->h);
    this->ax = float(::rand() % 2000) / 2000.0f * M_PI;
    this->ay = sin(this->ax);
    this->ax = cos(this->ax);
}


/*
 * CamTestApp::Lens::~Lens
 */
CamTestApp::Lens::~Lens(void) {
}


/*
 * CamTestApp::Lens::Update
 */
void CamTestApp::Lens::Update(float sec, vislib::graphics::CameraOpenGL camera) {
    bool pong = false;

    this->x += ax * sec;
    this->y += ay * sec;

    if (((this->x < 0.0f) && (this->ax < 0.0f)) || ((this->x > 1.0f - this->w) && (this->ax > 0.0f))) {
        this->ax = -this->ax;
        pong = true;
    }
    if (((this->y < 0.0f) && (this->ay < 0.0f)) || ((this->y > 1.0f - this->h) && (this->ay > 0.0f))) {
        this->ay = -this->ay;
        pong = true;
    }

    if (pong) {
        double d = atan2(this->ay, this->ax);
        d += (double(::rand() % 2001 - 1000) / 1000.0f) * 0.5f;
        this->ax = cos(d);
        this->ay = sin(d);
    }

    this->camera = camera;
}


/*
 * CamTestApp::Lens::BeginDraw
 */
void CamTestApp::Lens::BeginDraw(unsigned int ww, unsigned int wh, bool ortho) {
    glViewport(0, 0, ww, wh);

    this->camera.SetTileRectangle(
        vislib::math::Rectangle<vislib::graphics::ImageSpaceType>
        (this->x * this->camera.GetVirtualWidth(), this->y * this->camera.GetVirtualHeight(), 
        (this->x + this->w) * this->camera.GetVirtualWidth(), (this->y + this->h) * this->camera.GetVirtualHeight()));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(-1.0f, -1.0f, 0.0f);
    glScalef(2.0f, 2.0f, 1.0f);
    glTranslatef(this->x, this->y, 0.0f);

    glDisable(GL_DEPTH_TEST);
    glColor3ub(255, 255, 0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(this->w, 0.0f);
        glVertex2f(this->w, this->h);
        glVertex2f(0.0f, this->h);
    glEnd();
    glEnable(GL_DEPTH_TEST);

    glViewport(int(this->x * ww), int(this->y * wh), int(this->w * ww), int(this->h * wh));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    this->camera.glMultProjectionMatrix();
    //gluPerspective(40.0, float(this->w * ww) / float(this->h * wh), 1.0, 10.0); // TODO: Parameters!

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    this->camera.glMultViewMatrix();
    //gluLookAt(0.0, -2.5, 0.0,  // TODO: Parameters!
    //    0.0, 0.0, 0.0,  // TODO: Parameters!
    //    0.0, 0.0, 1.0); // TODO: Parameters!
}


/*
 * CamTestApp::Lens::EndDraw
 */
void CamTestApp::Lens::EndDraw(void) {
	glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
	glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}


/*
 * CamTestApp::CamTestApp
 */
CamTestApp::CamTestApp(void) : AbstractGlutApp() {
    this->lensCount = 7;
    this->lenses = new Lens[this->lensCount];
    this->walkSpeed = 0.25f;
    this->rotSpeed = 0.5f;

    this->ortho = false;
    this->nativeFull = true;

    this->beholder.SetView(
        vislib::math::Point3D<double>(0.0, -2.5, 0.0),
        vislib::math::Point3D<double>(0.0, 0.0, 0.0),
        vislib::math::Vector3D<double>(0.0, 0.0, 1.0));

    this->camera.SetBeholder(&this->beholder);
    this->camera.SetNearClipDistance(1.0f);
    this->camera.SetFarClipDistance(5.0f);
    this->camera.SetFocalDistance(2.5f);
    this->camera.SetApertureAngle(40.0f);
    this->camera.SetVirtualWidth(10.0f);
    this->camera.SetVirtualHeight(10.0f);

    this->camera.SetProjectionType(this->ortho 
        ? vislib::graphics::Camera::MONO_ORTHOGRAPHIC
        : vislib::graphics::Camera::MONO_PERSPECTIVE);
}


/*
 * CamTestApp::~CamTestApp
 */
CamTestApp::~CamTestApp(void) {
    delete[] this->lenses;
}


/*
 * CamTestApp::PreGLInit
 */
int CamTestApp::PreGLInit(void) {
    VisLogoDoStuff();
    VisLogoTwistLogo();
    return 0;
}


/*
 * CamTestApp::PostGLInit
 */
int CamTestApp::PostGLInit(void) {
    glEnable(GL_DEPTH_TEST);
    this->lastTime = vislib::sys::PerformanceCounter::Query();
    return 0;
}


/*
 * CamTestApp::Resize
 */
void CamTestApp::Resize(unsigned int w, unsigned int h) {
    AbstractGlutApp::Resize(w, h);
    this->camera.SetVirtualWidth(float(w) / 500.0f);
    this->camera.SetVirtualHeight(float(h) / 500.0f);
}


/*
 * CamTestApp::KeyPress
 */
bool CamTestApp::KeyPress(unsigned char key) {
    switch(key) {
        case 'o':
            this->ortho = !this->ortho;
            this->camera.SetProjectionType(this->ortho 
                ? vislib::graphics::Camera::MONO_ORTHOGRAPHIC
                : vislib::graphics::Camera::MONO_PERSPECTIVE);
            printf("Orthographic projection is %s\n", this->ortho ? "on" : "off");
            return true;
        case 'n':
            this->nativeFull = !this->nativeFull;
            printf("Native calls for full image are %s\n", this->nativeFull ? "on" : "off");
            return true;
        default: return false;            
    }
}


/*
 * CamTestApp::Render
 */
void CamTestApp::Render(void) {
    UINT64 time = vislib::sys::PerformanceCounter::Query();
    float sec = this->walkSpeed * float(time - this->lastTime) / 1000.0f;

    this->lastTime = time;
    this->angle = static_cast<float>(static_cast<int>(static_cast<float>(time) * this->rotSpeed) % 3600) * 0.1f;

    glViewport(0, 0, this->GetWidth(), this->GetHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    if (this->nativeFull) {
        if (this->ortho) {
            double w, h;
            w = 0.5 * this->camera.GetVirtualWidth();
            h = 0.5 * this->camera.GetVirtualHeight();
            glOrtho(-w, w, -h, h,
                this->camera.GetNearClipDistance(), this->camera.GetFarClipDistance());
        } else {
            gluPerspective(this->camera.GetApertureAngle(), this->GetAspectRatio(),
                this->camera.GetNearClipDistance(), this->camera.GetFarClipDistance());
        }
    } else {
        this->camera.glMultProjectionMatrix();
    }

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    if (this->nativeFull) {
        const vislib::math::Point3D<double> &pos = this->beholder.GetPosition();
        const vislib::math::Point3D<double> &lat = this->beholder.GetLookAt();
        const vislib::math::Vector3D<double> &up = this->beholder.GetUpVector();
        gluLookAt(pos.X(), pos.Y(), pos.Z(), lat.X(), lat.Y(), lat.Z(), up.X(), up.Y(), up.Z());
    } else {
        this->camera.glMultViewMatrix();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    this->RenderLogo();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClear(GL_DEPTH_BUFFER_BIT);

    for (unsigned int i = 0; i < this->lensCount; i++) {
        this->lenses[i].Update(sec, this->camera);
        this->lenses[i].BeginDraw(this->GetWidth(), this->GetHeight(), this->ortho);
        this->RenderLogo();
        this->lenses[i].EndDraw();
    }

	glFlush();

	glutSwapBuffers();
    glutPostRedisplay();
}


/*
 * CamTestApp::RenderLogo
 */
void CamTestApp::RenderLogo(void) {
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(this->angle, 0.0f, -1.0f, 0.0f);
    unsigned int vCount = VisLogoCountVertices();
	unsigned int p;
	glBegin(GL_QUAD_STRIP);
	for (unsigned int i = 0; i < 20; i++) {
		for (unsigned int j = 0; j < vCount / 20; j++) {
			p = (i + j * 20) % vCount;
			glColor3dv(VisLogoVertexColor(p)->f);
			glNormal3dv(VisLogoVertexNormal(p)->f);
			glVertex3dv(VisLogoVertex(p)->f);
			p = ((i + 1) % 20 + j * 20) % vCount;
			glColor3dv(VisLogoVertexColor(p)->f);
			glNormal3dv(VisLogoVertexNormal(p)->f);
			glVertex3dv(VisLogoVertex(p)->f);
		}
	}
	p = 0; // closing strip
	glColor3dv(VisLogoVertexColor(p)->f);
	glNormal3dv(VisLogoVertexNormal(p)->f);
	glVertex3dv(VisLogoVertex(p)->f);
	p = 1;
	glColor3dv(VisLogoVertexColor(p)->f);
	glNormal3dv(VisLogoVertexNormal(p)->f);
	glVertex3dv(VisLogoVertex(p)->f);
	glEnd();    
    glPopMatrix();
}
