#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
//

const int NumVertices = 108; // 12 faces * 3 tri/face * 3 vtx/tri

vec4 points[NumVertices];
vec4 colors[NumVertices];

int Index = 0;
GLfloat pii = (1 + sqrt(5)) / 2;

vec4 vertices[20] = {
    vec4(-1.0, -1.0,  1.0, 1.0),
    vec4(-1.0,  1.0,  1.0, 1.0),
    vec4(1.0,  1.0,  1.0, 1.0),
    vec4(1.0, -1.0,  1.0, 1.0),

    vec4(-1.0, -1.0,  -1.0, 1.0),
    vec4(-1.0,  1.0,  -1.0, 1.0),
    vec4(1.0,  1.0,  -1.0, 1.0),
    vec4(1.0, -1.0,  -1.0, 1.0),

    vec4(0, -pii, 1/pii, 1.0),
    vec4(0, pii, 1/pii, 1.0),
    vec4(0, -pii, -1/pii, 1.0),
    vec4(0, pii, -1/pii, 1.0),

    vec4(-1 / pii, 0, pii, 1.0),
    vec4(1 / pii, 0, pii, 1.0),
    vec4(-1 / pii, 0, -pii, 1.0),
    vec4(1 / pii, 0, -pii, 1.0),

    vec4(-pii, -1 / pii, 0, 1.0),
    vec4(-pii, 1 / pii, 0, 1.0),
    vec4(pii, 1 / pii, 0, 1.0),
    vec4(pii, -1 / pii, 0, 1.0)
};

// RGBA colors
vec4 vertex_colors[12] = {
    vec4(0.5, 0.5, 0.5, 1.0),  // gray
    vec4(1.0, 0.0, 0.0, 1.0),  // red
    vec4(1.0, 1.0, 0.0, 1.0),  // yellow
    vec4(0.0, 1.0, 0.0, 1.0),  // green
    vec4(0.0, 0.0, 1.0, 1.0),  // blue
    vec4(1.0, 0.0, 1.0, 1.0),  // magenta
    vec4(1.0, 1.0, 1.0, 1.0),  // white
    vec4(0.0, 1.0, 1.0, 1.0),   // cyan

    vec4(1.0, 0.4, 0.4, 1.0),  // pink
    vec4(0.6, 0.1, 0.7, 1.0),  // purple
    vec4(0.5, 0.0, 0.0, 1.0),  // brown
    vec4(0.5, 0.5, 0.0, 1.0)  // olive
};

enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int Axis = Xaxis;
GLfloat Theta[NumAxes] = { 0.0, 0.0, 0.0 };

GLuint matrix_loc;

//make three triangls that divide a penta
void penta(int a, int b, int c, int d, int e)
{
    //one triangle a-b-c
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;

    //one triangle a-c-d
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;

    //one triangle a-d-e
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[e]; Index++;
}

// generate a dodec
void colordodec()
{
    penta(0, 12, 1, 17, 16);
    penta(1, 9, 11, 5, 17);
    penta(2, 13, 3, 19, 18);
    penta(3, 13, 12, 0, 8);
    penta(4, 16, 17, 5, 14);
    penta(5, 11, 6, 15, 14);

    penta(6, 18, 19, 7, 15);
    penta(7, 10, 4, 14, 15);
    penta(8, 10, 7, 19, 3);
    penta(9, 1, 12, 13, 2);
    penta(10, 8, 0, 16, 4);
    penta(11, 9, 2, 18, 6);
}
void init(void)
{
    colordodec();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL,
        GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    //load shaders
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    //initialize vertex position attribute from vertex shader
    GLuint vPos = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPos);
    glVertexAttribPointer(vPos, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    //initialize vertex color attribute from vertex shader
    GLuint vCol = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vCol);
    glVertexAttribPointer(vCol, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

    //initialize uniform variable from vertex shander
    matrix_loc = glGetUniformLocation(program, "rotation");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

GLfloat speed = 0.0;
GLfloat speed_save = 0.5;
GLfloat scaling = 0.4;
GLfloat translation[3] = { 0, 0, 0 };

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mat4 ctm = Translate(translation[0], translation[1], translation[2]);
    ctm *= (Scale(scaling, scaling, scaling));
    ctm *= RotateX(Theta[Xaxis]) *
        RotateY(Theta[Yaxis]) *
        RotateZ(Theta[Zaxis]);
    
    glUniformMatrix4fv(matrix_loc, 1, GL_TRUE, ctm);

    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    glutSwapBuffers();
}


void
keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 033:
        exit(EXIT_SUCCESS);
        break;
    case 'q':
        printf("q");
        exit(EXIT_SUCCESS);
        break;
    case 'Q':
        printf("Q");
        exit(EXIT_SUCCESS);
        break;
    case 040:
        printf("space bar\n");
        if (speed > 0.0) {
            speed_save = speed;
            speed = 0.0;
        }
        else speed = speed_save;
        break;
    case 'j': // 속도 줄이기
        if (speed == 0.0) {
            if (speed_save >= 0.2) {
                speed_save -= 0.1;
                printf("down speed(save)\n");
            }
        }
        else if (speed >= 0.2) {
            speed -= 0.1;
            printf("down speed\n");
        }
        break;
    case 'k': // 속도 높이기
        if (speed == 0.0) {
            if (speed_save <= 0.9) {
                speed_save += 0.1;
                printf("up speed(save)\n");
            }
        }
        else if (speed <= 0.9) {
            speed += 0.1;
            printf("up speed\n");
        }
        break;
    case 'z': // 축소
        if (scaling >= 0.3) scaling -= 0.1;
        break;
    case 'x': // 확대
        if (scaling <= 0.6) scaling += 0.1;
        break;
    case 'a': // 왼쪽 X-
        if (translation[0] > -1) translation[0] -= 0.1;
        break;
    case 'd': // 오른쪽 X+
        if (translation[0] < 1) translation[0] += 0.1;
        break;
    case 's': // 아래 Y-
        if (translation[1] > -1) translation[1] -= 0.1;
        break;
    case 'w': // 위 Y+
        if (translation[1] < 1) translation[1] += 0.1;
        break;
    }
}

void idle()
{
    Theta[Axis] += speed;

    if (Theta[Axis] > 360.0) {
        Theta[Axis] -= 360.0;
    }
    glutPostRedisplay();
}

void
mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        switch (button) {
        case GLUT_LEFT_BUTTON: Axis = Xaxis; break;
        case GLUT_MIDDLE_BUTTON: Axis = Yaxis; break;
        case GLUT_RIGHT_BUTTON: Axis = Zaxis; break;
        }
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);

    glutInitContextVersion(4, 3);
    //In case of MacOS
    //glutInitContextProfie(GLUT_CORE_PROFILE);
    glutCreateWindow("2020062324");

    glewInit();
    init();

    /*
    printf("OpenGL %s, GLSL %s\n",
        glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));
    */

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}