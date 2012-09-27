/////////////////////////////////
// handin.cpp
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

// global variable
static unsigned int modelList;


// Drawing routine.
void drawScene(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(0.0, 0.0, 0.0);
   glLoadIdentity();

   // Modeling transformations.
   glTranslatef(0.0, 0.0, -20.0);

   glCallList(modelList); // Execute display list.

   glFlush();
}

// Initialization routine.
// returns non-zero if error
int setup(char* modelFile)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);

	// each face is a list of vertices
	vector< vector<unsigned> > faces;
	// each vertex is consecutive 3 coord
	vector<float> inputVerCoords; // as inputted
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
				if (DEBUG) cout << "face ";
				while (ss >> vertex) {
					if (DEBUG) cout << vertex << " ";
					if (vertex > inputVerCoords.size()) {
						cout << "The face '" << line << "' is referencing vertex " << vertex << " which has not been defined yet. TERMINATING." << endl;
						return 1;
					}
					oneFace.push_back(vertex);
				}
				faces.push_back(oneFace);
				if (DEBUG) cout << endl;

			} else {
				cout << "Ignoring line: " << line << endl;
			}
		}
	    myfile.close();
	} else {
		cout << "Unable to open file" << endl;
	}

	// now actually find the mean of the vertices by dividing by their number;
	for (int i = 0; i < 3; i++) means[i] /= inputVerCoords.size()/3;

	// translate vertices by means[]. Also find min and max of each coordinate of the translated vertices
	for (int i = 0; i < inputVerCoords.size(); i++) {
		float cur = inputVerCoords[i] / means[i%3];
		vertexCoords.push_back(cur);
		if (cur < mins[i%3]) mins[i%3] = cur;
		if (cur < maxs[i%3]) maxs[i%3] = cur;
	}

	// scale: note ... max of differences! TODO finish

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

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	return 0;
}

void outputModel() {
	ofstream myfile;
	myfile.open ("output.obj");
	myfile.precision(6);
	myfile << fixed;
	// vertices first
	for (unsigned v = 0; v < vertexCoords.size(); v += 3)
		myfile << "v " << vertexCoords[v] << " " << vertexCoords[v+1] << " " << vertexCoords[v+2] << endl;
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

// OpenGL window reshape routine.
void resize(int w, int h)
{
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 100.0);

   glMatrixMode(GL_MODELVIEW);
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 27:
		exit(0);
		break;
	case 'w':
		outputModel();
		break;
	default:
		break;
	}
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
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("box.cpp");
	if (setup(argv[1]) != 0) {
		return 2;
	}
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutMainLoop();

   return 0;
}
