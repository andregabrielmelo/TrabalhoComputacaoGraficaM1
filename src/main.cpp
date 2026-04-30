#include <GL/freeglut.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

struct Vec3 { float x, y, z; };
struct Vec2 { float u, v; };

struct Face {
    int v[3];
    int t[3];
    int n[3];
};

vector<Vec3> vertices;
vector<Vec3> normals;
vector<Vec2> textures;
vector<Face> faces;

// Transformações
float rotX = 0, rotY = 0, rotZ = 0;
float scale = 0.1f;
float transX = 0, transY = 0, transZ = -50;

// Luzes
bool light0 = true;
bool light1 = true;
bool light2 = true;
bool useLighting = true;

// Textura
GLuint textureID;

void keyboard(unsigned char key, int, int) {
    switch (key) {
    case 'w': rotX += 5; break;
    case 's': rotX -= 5; break;
    case 'a': rotY += 5; break;
    case 'd': rotY -= 5; break;
    case 'q': rotZ += 5; break;
    case 'e': rotZ -= 5; break;

    case '+': scale += 0.05f; break;
    case '-': scale -= 0.05f; break;

    case 'i': transY += 1.0f; break;
    case 'k': transY -= 1.0f; break;
    case 'j': transX -= 1.0f; break;
    case 'l': transX += 1.0f; break;
    case 'u': transZ -= 1.0f; break;
    case 'o': transZ += 1.0f; break;

    case '1': light0 = !light0; break;
    case '2': light1 = !light1; break;
    case '3': light2 = !light2; break;

    case 'z': useLighting = !useLighting; break;
    }

    if (scale < 0.01f) scale = 0.01f;

    glutPostRedisplay();
}

// Loader simples de BMP (24 bits)
void loadTexture(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        cout << "Erro ao carregar textura\n";
        return;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];

    int size = 3 * width * height;
    unsigned char* data = new unsigned char[size];

    fread(data, sizeof(unsigned char), size, file);
    fclose(file);

    // BGR → RGB
    for (int i = 0; i < size; i += 3) {
        swap(data[i], data[i + 2]);
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    delete[] data;
}

void loadOBJ(const char* filename) {
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string type;
        ss >> type;

        if (type == "v") {
            Vec3 v; ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (type == "vn") {
            Vec3 n; ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "vt") {
            Vec2 t; ss >> t.u >> t.v;
            textures.push_back(t);
        }
        else if (type == "f") {
            Face f;

            for (int i = 0; i < 3; i++) {
                string vert;
                ss >> vert;

                int v = 0, t = 0, n = 0;

                sscanf(vert.c_str(), "%d/%d/%d", &v, &t, &n);

                f.v[i] = v - 1;
                f.t[i] = t - 1;
                f.n[i] = n - 1;
            }

            faces.push_back(f);
        }
    }
}

void setupLights() {
    if (!useLighting) {
        glDisable(GL_LIGHTING);
        return;
    }

    glEnable(GL_LIGHTING);

    GLfloat globalAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    GLfloat pos0[] = { 10, 10, 10, 1 };
    GLfloat pos1[] = { -10, 10, 10, 1 };
    GLfloat pos2[] = { 0, -10, 10, 1 };

    // LIGHT 0
    if (light0) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    GLfloat diff0[] = { 1, 0, 0, 1 };
    GLfloat amb0[] = { 0.2f, 0, 0, 1 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, diff0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);

    // LIGHT 1
    if (light1) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);

    GLfloat diff1[] = { 0, 1, 0, 1 };
    GLfloat amb1[] = { 0, 0.2f, 0, 1 };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diff1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, diff1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, amb1);

    // LIGHT 2
    if (light2) glEnable(GL_LIGHT2); else glDisable(GL_LIGHT2);
    glLightfv(GL_LIGHT2, GL_POSITION, pos2);

    GLfloat diff2[] = { 0, 0, 1, 1 };
    GLfloat amb2[] = { 0, 0, 0.2f, 1 };
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diff2);
    glLightfv(GL_LIGHT2, GL_SPECULAR, diff2);
    glLightfv(GL_LIGHT2, GL_AMBIENT, amb2);
}

void drawModel() {
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_TRIANGLES);

    for (auto& f : faces) {
        for (int i = 0; i < 3; i++) {

            if (f.n[i] >= 0 && f.n[i] < normals.size()) {
                Vec3 n = normals[f.n[i]];
                glNormal3f(n.x, n.y, n.z);
            }

            if (f.t[i] >= 0 && f.t[i] < textures.size()) {
                Vec2 t = textures[f.t[i]];
                glTexCoord2f(t.u, t.v);
            }

            Vec3 v = vertices[f.v[i]];
            glVertex3f(v.x, v.y, v.z);
        }
    }

    glEnd();
}


void drawText(float x, float y, const string& text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);

    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);

    for (char c : text)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);

    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

void drawVariables() {
    drawText(10, 580, "RotX: " + to_string((int)rotX));
    drawText(10, 560, "RotY: " + to_string((int)rotY));
    drawText(10, 540, "RotZ: " + to_string((int)rotZ));

    drawText(10, 520, "Scale: " + to_string(scale));

    drawText(10, 500, "TransX: " + to_string(transX));
    drawText(10, 480, "TransY: " + to_string(transY));
    drawText(10, 460, "TransZ: " + to_string(transZ));

    drawText(10, 440, string("Light0(red): ") + (light0 ? "ON" : "OFF"));
    drawText(10, 420, string("Light1(green): ") + (light1 ? "ON" : "OFF"));
    drawText(10, 400, string("Light2(blue): ") + (light2 ? "ON" : "OFF"));

    drawText(10, 380, string("Lighting: ") + (useLighting ? "ON" : "OFF"));
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);

    glTranslatef(0, 0, -50);

    setupLights();

    glPushMatrix();

    glTranslatef(transX, transY, transZ);
    glScalef(scale, scale, scale);
    glRotatef(rotX, 1, 0, 0);
    glRotatef(rotY, 0, 1, 0);
    glRotatef(rotZ, 0, 0, 1);

    drawModel();

    glPopMatrix();

    drawVariables();

    glutSwapBuffers();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glColor3f(1.0f, 1.0f, 1.0f);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GLfloat matAmbient[] = { 0.3f, 0.3f, 0.3f, 1 };
    GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1 };
    GLfloat matSpecular[] = { 1, 1, 1, 1 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glClearColor(0.02f, 0.02f, 0.02f, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 800.0 / 600.0, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);

    loadTexture("../textures/grass.bmp");
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Viewer");

    init();
    loadOBJ("../objects/porsche.obj");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(display);

    glutMainLoop();
    return 0;
}