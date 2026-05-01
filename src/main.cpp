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
float transX = 0, transY = 0, transZ = 0;

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

void loadOBJ(const char* filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Erro ao abrir arquivo OBJ: " << filename << endl;
        return;
    }

    string line;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        stringstream ss(line);
        string type;
        ss >> type;

        if (type == "v") {
            Vec3 v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (type == "vn") {
            Vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "vt") {
            Vec2 t;
            ss >> t.u >> t.v;
            textures.push_back(t);
        }
        else if (type == "f") {
            vector<string> verts;
            string vert;

            // Read ALL vertices of the face (not just 3)
            while (ss >> vert) {
                verts.push_back(vert);
            }

            // Triangulate (fan method)
            for (size_t i = 1; i + 1 < verts.size(); i++) {
                Face f;

                string vStr[3] = { verts[0], verts[i], verts[i + 1] };

                for (int k = 0; k < 3; k++) {
                    int v = 0, t = 0, n = 0;

                    // Try all formats
                    if (sscanf(vStr[k].c_str(), "%d/%d/%d", &v, &t, &n) == 3) {
                        // v/t/n
                    }
                    else if (sscanf(vStr[k].c_str(), "%d//%d", &v, &n) == 2) {
                        t = 0;
                    }
                    else if (sscanf(vStr[k].c_str(), "%d/%d", &v, &t) == 2) {
                        n = 0;
                    }
                    else if (sscanf(vStr[k].c_str(), "%d", &v) == 1) {
                        t = n = 0;
                    }
                    else {
                        v = t = n = 0;
                    }

                    f.v[k] = (v > 0) ? v - 1 : (v < 0 ? (int)vertices.size() + v : -1);
                    f.t[k] = (t > 0) ? t - 1 : (t < 0 ? (int)textures.size() + t : -1);
                    f.n[k] = (n > 0) ? n - 1 : (n < 0 ? (int)normals.size() + n : -1);
                }

                faces.push_back(f);
            }
        }
    }

    cout << "OBJ carregado: "
        << vertices.size() << " vertices, "
        << normals.size() << " normals, "
        << textures.size() << " texcoords, "
        << faces.size() << " faces\n";
}

void drawModel() {
    glBegin(GL_TRIANGLES);

    for (auto& f : faces) {
        for (int i = 0; i < 3; i++) {
            if (f.v[i] < 0 || f.v[i] >= vertices.size()) {
                cout << "Invalid vertex index: " << f.v[i] << endl;
                continue;
            }

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


void setupLights() {
    if (!useLighting) {
        glDisable(GL_LIGHTING);
        return;
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    // EXACT match to reference project behavior
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    // Global ambient (same idea as reference)
    GLfloat globalAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // IMPORTANT: set lights in WORLD SPACE (identity matrix)
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // ---------------- LIGHT 0 (RED) ----------------
    if (light0) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);

    GLfloat pos0[] = { 0.5f, 0.5f, 0.5f, 1.0f };   // like reference (small scene coords)
    GLfloat amb0[] = { 0.2f, 0.0f, 0.0f, 1.0f };
    GLfloat diff0[] = { 0.8f, 0.0f, 0.0f, 1.0f };
    GLfloat spec0[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);

    // ---------------- LIGHT 1 (GREEN) ----------------
    if (light1) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);

    GLfloat pos1[] = { -0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat amb1[] = { 0.0f, 0.2f, 0.0f, 1.0f };
    GLfloat diff1[] = { 0.0f, 0.8f, 0.0f, 1.0f };
    GLfloat spec1[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, amb1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diff1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spec1);

    // ---------------- LIGHT 2 (BLUE) ----------------
    if (light2) glEnable(GL_LIGHT2); else glDisable(GL_LIGHT2);

    GLfloat pos2[] = { 0.0f, -0.5f, 0.5f, 1.0f };
    GLfloat amb2[] = { 0.0f, 0.0f, 0.2f, 1.0f };
    GLfloat diff2[] = { 0.0f, 0.0f, 0.8f, 1.0f };
    GLfloat spec2[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT2, GL_POSITION, pos2);
    glLightfv(GL_LIGHT2, GL_AMBIENT, amb2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diff2);
    glLightfv(GL_LIGHT2, GL_SPECULAR, spec2);

    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

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

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glColor3f(1.0f, 1.0f, 1.0f);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0f);

    glClearColor(0.02f, 0.02f, 0.02f, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 800.0 / 600.0, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
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