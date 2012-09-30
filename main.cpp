/////////////////////////////////
// main.cpp
//
// CMPUT 411
// Assignment 1
//
// David Szepesvari.
/////////////////////////////////

// TA : Xida Chen xida@cs.ualberta.ca

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159265
#define DEBUG true

using namespace std;

// GLOBAL VARIABLES -------------------
static unsigned int modelList;
vector<float> inputVerCoords; // vertex coordinates as inputted
vector< vector<unsigned> > faces; // each face is a list of vertices

static float modelTrans[16]; // all transformations of model -- in matrix format
static float cameraTrans[16]; // all transformations of camera -- in matrix format

//static float cameraMoved[] = { 0.0f, 0.0f, 0.0f}; // position of model
//static float camlRot[16]; // rotation of camera -- in matrix stack format
static bool persp = false; // perspective or orthographic projection?

// rest the position and rotation of the model and the camera to the original settings
void resetScene() {
	// TODO have to write reset code!!!!!!!!!!!!!!!!!!!!!
	cout << "RESET CODE NEEDS TO BE WRITTEN!!!!!!!!!!!!!!!" << endl;
}

void drawAxes(float length) {
	// draw the 3 axis (x, y, z) <--> (red, green, blue)
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(length,0,0);
	glEnd();
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(0,length,0);
	glEnd();
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(0,0,length);
	glEnd();
	glColor3f(0.0, 0.0, 0.0);
}

// Drawing routine.
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
//	glLoadIdentity();
	glPushMatrix();

	// camera transformations -- these are already in the current modelview matrix
//	glTranslatef(-cameraMoved[0], -cameraMoved[1], -cameraMoved[2]); // movement is w.r.t rotation.. TODO check!

	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (DEBUG) drawAxes(10); // camera axes

	glMultMatrixf(cameraTrans);

	if (DEBUG) drawAxes(10); // world axes

	// Modeling transformations.
	glTranslatef(0.0, 0.0, -2.0); // add -2 at the end (TODO could be just included in modelTrans : first matrix == applied last)
	glMultMatrixf(modelTrans);

	glCallList(modelList); // Execute display list.
	if (DEBUG) drawAxes(1); // these rotate with the model

	glPopMatrix(); // reset to original form
//	glFlush();
	glutSwapBuffers();
}

// Initialization routine.
// returns non-zero if error
int setup(char* modelFile)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);

	// each vertex is consecutive 3 coord
	vector<float> vertexCoords; // translated and scaled
	double means[] = {0.0, 0.0, 0.0}; // TODO is it going to cause issues that we subtract and multiply
	float maxs[] = {0.0f, 0.0f, 0.0f}; // floats by doubles?
	float mins[] = {0.0f, 0.0f, 0.0f};

	cout << "Loading " << modelFile << endl;
	string line, lineType;
	unsigned int vertex;
	float x, y, z;
	ifstream myfile (modelFile);
	if (myfile.is_open()) {
		while ( myfile.good() ) {
			getline (myfile, line);

			if (line.size() == 0) break;

			// parse each line
			stringstream ss(line);
			ss >> lineType;
//			cout << "++ " << lineType << endl;
			if (lineType.compare("v") == 0) {
				ss >> x >> y >> z;
				if (DEBUG) cout << "Created vertex " << x << " " << y << " " << z << endl;
				means[0] += x; means[1] += y; means[2] += z;
				inputVerCoords.push_back(x);
				inputVerCoords.push_back(y);
				inputVerCoords.push_back(z);
			} else if (lineType.compare("f") == 0) {

				vector<unsigned> oneFace;
//				if (DEBUG) cout << "face ";
				while (ss >> vertex) {
//					if (DEBUG) cout << vertex << " ";
					if (vertex > inputVerCoords.size()) {
						cout << "The face '" << line << "' is referencing vertex " << vertex << " which has not been defined yet. TERMINATING." << endl;
						return 1;
					}
					oneFace.push_back(vertex);
				}
				faces.push_back(oneFace);
//				if (DEBUG) cout << endl;

			} else {
				if (DEBUG) 	cout << "Ignoring line: " << line << endl;
			}
		}
	    myfile.close();
	} else {
		cout << "Unable to open file. TERMINATING." << endl;
		return 1;
	}

	// now actually find the mean of the vertices by dividing by their number;
	for (int i = 0; i < 3; i++) means[i] /= inputVerCoords.size()/3;
	if (DEBUG) {
		cout << "mean of vertices:";
		for (int i = 0; i < 3; i++) cout << " " << means[i];
		cout << endl;
	}

	// translate vertices by -means[]. Also find min and max of each coordinate of the translated vertices
	for (unsigned i = 0; i < inputVerCoords.size(); i++) {
		float cur = inputVerCoords[i] - means[i%3];
		vertexCoords.push_back(cur);
		if (cur < mins[i%3]) mins[i%3] = cur;
		if (cur > maxs[i%3]) maxs[i%3] = cur;
	}

	float maxDiff = 0; // find maxDiff
	cout << "MAX\tMIN" << endl;
	for (int i = 0; i < 3; i++) {
		cout << maxs[i] << "\t" << mins[i] << endl;
		if (maxs[i] - mins[i] > maxDiff) maxDiff = maxs[i] - mins[i];
	}

	// scale by 1.0/maxDiff
	for (unsigned i = 0; i < inputVerCoords.size(); i++) {
		vertexCoords[i] /= maxDiff;
//		if ((i%3) == 2) vertexCoords[i] -= 2;
		// I decided not to translate here, as that makes it hard to rotate it around its center
		// Instead, it is translated in the drawScene function
		// TODO maybe do it here afterall, but then
	}


	// Set up a vertex array
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &vertexCoords[0]);

	// set up a display list
	modelList = glGenLists(1); // Return a list index.
	glNewList(modelList, GL_COMPILE); // Begin create a display list.
		for (unsigned i = 0; i < faces.size(); i++) {
			glBegin(GL_TRIANGLE_STRIP); // draw the model
			for (vector<unsigned>::iterator it = faces[i].begin(); it != faces[i].end(); it++)
				glArrayElement(*it - 1); // they are numbered 1 .. ?
			glEnd();
		}
	glEndList();
	// End create a display list.

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glGetFloatv(GL_MODELVIEW_MATRIX, modelTrans); // initialize it to identity --> move to resetScene()
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraTrans); // initialize it to identity --> move to resetScene()

	return 0;
}

// instead of the idle solution, use glutTimerFunc(period in ms, timer_function, value)
void doWhenIdle() {
//	modelRot[1] += 0.5;
//	glutPostRedisplay();
}

/* Adds the specified rotation to the model. */
void addModelRotation(float angle, float ax, float ay, float az) {
	// we let openGL do the multiplications.. so
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); // make a copy
	glLoadMatrixf(modelTrans); // load current rotations
	glRotatef(angle, ax, ay, az); // add new rotation
	glGetFloatv(GL_MODELVIEW_MATRIX, modelTrans); // save new rotation
	glPopMatrix(); // reset modelview to original settings
}

/* Adds the specified rotation to the model. */
void addModelTranslation(float x, float y, float z) {
	// we let openGL do the multiplications.. so
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); // make a copy
	glLoadMatrixf(modelTrans); // load current rotations
	glTranslatef(x, y, z); // add new translation
	glGetFloatv(GL_MODELVIEW_MATRIX, modelTrans); // save new rotation
	glPopMatrix(); // reset modelview to original settings
}

// translate the camera by the given amounts (with respect to its local fram)
void translateCamera(float x, float y, float z) {
	glMatrixMode(GL_MODELVIEW); // make sure we have the right matrix selected
	glPushMatrix(); // make a copy
	glLoadIdentity(); // do nothing
	glTranslatef(-x, -y, -z); // add the translation
	glMultMatrixf(cameraTrans); // now add already existing camera transformations, from the left!
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraTrans); // save new rotation
	glPopMatrix(); // reset modelview to original settings
}

// rotate the camera (with respect to its local frame) by the given amount,
// about the given vector
void rotateCamera(float angle, float x, float y, float z) {
	glMatrixMode(GL_MODELVIEW); // make sure we have the right matrix selected
	glPushMatrix(); // make a copy
	glLoadIdentity(); // do nothing
	glRotatef(-angle, x, y, z);
	glMultMatrixf(cameraTrans); // now add already existing camera transformations, from the left!
	glGetFloatv(GL_MODELVIEW_MATRIX, cameraTrans); // save new rotation
	glPopMatrix(); // reset modelview to original settings
}

void outputModel() {
	ofstream myfile;
	myfile.open ("output.obj");
	myfile.precision(6);
	myfile << fixed;
	// vertices first
	for (unsigned v = 0; v < inputVerCoords.size(); v += 3)
		myfile << "v " << inputVerCoords[v] << " " << inputVerCoords[v+1] << " " << inputVerCoords[v+2] << endl;
	// then faces
	for (unsigned i = 0; i < faces.size(); i++) {
		myfile << "f";
		for (vector<unsigned>::iterator it = faces[i].begin(); it != faces[i].end(); it++) {
			myfile << " " << *it;
		}
		myfile << endl;
	}
	myfile.close();
}

void updateProjection() {
	// syntax:
	//	glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
	//	glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
	cout << "Changing projection type to " << ((persp) ? "perspective" : "orthographic") << endl;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (persp) 	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
	else 		glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);

	glutPostRedisplay();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
//	glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 100.0); // original
	updateProjection();
}


// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'q': exit(0); break;
	case 's': resetScene(); break;
	case 'v':
		persp = false;
		updateProjection();
		break;
	case 'V':
		persp = true;
		updateProjection();
		break;
	case 'w':
		outputModel();
		break;
	// translations;
	case 'l': addModelTranslation(-0.1, 0.0, 0.0); break;
	case 'L': addModelTranslation(0.1, 0.0, 0.0); break;
	case 'd': addModelTranslation(0.0, -0.1, 0.0); break;
	case 'D': addModelTranslation(0.0, 0.1, 0.0); break;
	case 'n': addModelTranslation(0.0, 0.0, -0.1); break;
	case 'N': addModelTranslation(0.0, 0.0, 0.1); break;
	// rotations - change the MODELVIEW MATRIX (assume it's already selected)
	case 'p': addModelRotation(-10.0, 1, 0, 0); break;
	case 'P': addModelRotation(+10.0, 1, 0, 0); break;
	case 'y': addModelRotation(-10.0, 0, 1, 0); break;
	case 'Y': addModelRotation(+10.0, 0, 1, 0); break;
	case 'r': addModelRotation(-10.0, 0, 0, 1); break;
	case 'R': addModelRotation(+10.0, 0, 0, 1); break;
	// camera controls
//	case 'i': cameraMoved[2] -= 0.1; break;
//	case 'I': cameraMoved[2] += 0.1; break;
	case 'i': translateCamera(0.0, 0.0, -0.1); break;
	case 'I': translateCamera(0.0, 0.0, 0.1); break;
	case 't': rotateCamera(-10, 1, 0, 0); break;
	case 'T': rotateCamera(10, 1, 0, 0); break;
	case 'a': rotateCamera(-10, 0, 1, 0); break;
	case 'A': rotateCamera(10, 0, 1, 0); break;
	case 'c': rotateCamera(-10, 0, 0, 1); break;
	case 'C': rotateCamera(10, 0, 0, 1); break;
	default:
		break;
	}

	// for most actions this is required, other times it can't hurt
	glutPostRedisplay();
}

// Special keyboard input processing routine.
void specialKeyInput(int key, int x, int y) {
//	   if(key == GLUT_KEY_UP) cameraMoved[1] += 0.1;
//	   if(key == GLUT_KEY_DOWN) cameraMoved[1] -= 0.1;
//	   if(key == GLUT_KEY_LEFT) cameraMoved[0] -= 0.1;
//	   if(key == GLUT_KEY_RIGHT) cameraMoved[0] += 0.1;
	   if(key == GLUT_KEY_UP) translateCamera(0, 0.1, 0);
	   if(key == GLUT_KEY_DOWN) translateCamera(0, -0.1, 0);
	   if(key == GLUT_KEY_LEFT) translateCamera(-0.1, 0, 0);
	   if(key == GLUT_KEY_RIGHT) translateCamera(0.1, 0, 0);
	   glutPostRedisplay();
}

// Main routine.
int main(int argc, char **argv)
{
	if (argc != 2) {
		cout << "ERROR: this program takes exactly 1 argument: the .obj file to load." << endl
				<< "The program is going to terminate now." << endl;
		return 2;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
//	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("box.cpp");
	if (setup(argv[1]) != 0) {
		return 2;
	}
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
	glutIdleFunc(doWhenIdle); // TODO to test
	glutMainLoop();

   return 0;
}
