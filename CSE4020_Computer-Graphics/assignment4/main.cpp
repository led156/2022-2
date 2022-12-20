
#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <stack>

typedef vec4 point4;
typedef vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4(-0.5, -0.5,  0.5, 1.0),
    point4(-0.5,  0.5,  0.5, 1.0),
    point4(0.5,  0.5,  0.5, 1.0),
    point4(0.5, -0.5,  0.5, 1.0),
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5,  0.5, -0.5, 1.0),
    point4(0.5,  0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA olors
color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0),  // black
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(0.0, 1.0, 0.0, 1.0),  // green
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
    color4(1.0, 1.0, 1.0, 1.0),  // white
    color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

//----------------------------------------------------------------------------

class MatrixStack {
    std::stack<mat4> _matrices;

public:
    void push(const mat4& m) {
        _matrices.push(m);
    }
    mat4& pop(void) {
        mat4 ret = _matrices.top();
        _matrices.pop();
        return ret;
    }
};

MatrixStack  mvstack;

// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT = 1.0;
const GLfloat BASE_WIDTH = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH = 1.5;
const GLfloat UPPER_ARM_HEIGHT = 1.0;
const GLfloat UPPER_ARM_WIDTH = 9.0;

const GLfloat LEFT_ARM_HEIGHT = 5.0;
const GLfloat LEFT_ARM_WIDTH = 1.5;
const GLfloat RIGHT_ARM_HEIGHT = 5.0;
const GLfloat RIGHT_ARM_WIDTH = 1.5;

const GLfloat FAN_CENTER_WIDTH = 0.9;
const GLfloat FAN_WING_HEIGHT = 2.0;
const GLfloat FAN_WING_WIDTH = 0.3;

// Shader transformation matrices
mat4  model_view;
GLuint ModelView, Projection;

// Array of rotation angles (in degrees) for each rotation axis
enum {
    Base = 0, 
    LowerArm = 1, 
    
    UpperArm = 2,
    LeftArm = 3,
    RightArm = 4,

    LeftFanCenter = 5,
    LeftFanWing1 = 6,
    LeftFanWing2 = 7,
    LeftFanWing3 = 8,
    LeftFanWing4 = 9,

    RightFanCenter = 10,
    RightFanWing1 = 11,
    RightFanWing2 = 12,
    RightFanWing3 = 13,
    RightFanWing4 = 14,

    LowerArmY = 15,
    LeftRightArm = 16,

    NumAngles = 17,
    Quit
};
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 
    0.0, // Base
    0.0, // LowerArm
    0.0, // UpperArm
    0.0, // LeftArm
    0.0, // LeftFanCenter
    0.0, // LeftFanWing1
    90.0, // 2
    180.0, // 3
    270.0, // 4
    0.0, // RightFanCenter
    0.0, // RightFanWing1
    90.0, // 2
    180.0, // 3
    270.0, // 4

    0.0, // LowerArmY
    0.0 // LeftRightArm
};

//----------------------------------------------------------------------------

struct Node {
    mat4  transform;
    void  (*render)(void);
    Node* sibling;
    Node* child;

    Node() :
        render(NULL), sibling(NULL), child(NULL) {}

    Node(mat4& m, void (*render)(void), Node* sibling, Node* child) :
        transform(m), render(render), sibling(sibling), child(child) {}
};

Node  nodes[NumAngles];

//----------------------------------------------------------------------------

int Index = 0;

void
quad(int a, int b, int c, int d)
{
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

void
colorcube()
{
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

void
traverse(Node* node)
{
    if (node == NULL) { return; }

    mvstack.push(model_view);

    model_view *= node->transform;
    node->render();

    if (node->child) { traverse(node->child); }

    model_view = mvstack.pop();

    if (node->sibling) { traverse(node->sibling); }
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void
base()
{
    mvstack.push(model_view);

    mat4 instance = (Translate(0.0, 0.5 * BASE_HEIGHT, 0.0) *
        Scale(BASE_WIDTH, BASE_HEIGHT, BASE_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    model_view = mvstack.pop();
}

//----------------------------------------------------------------------------

void
upper_arm()
{
    mvstack.push(model_view);

    mat4 instance = (Translate(0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0) *
        Scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, 1.0));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    model_view = mvstack.pop();
}

//----------------------------------------------------------------------------

void
lower_arm()
{
    mvstack.push(model_view);

    mat4 instance = (Translate(0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0) *
        Scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    model_view = mvstack.pop();
}

//----------------------------------------------------------------------------

void
left_arm()
{
    mvstack.push(model_view);

    mat4 instance = (Translate(0.0, 0.5 * LEFT_ARM_HEIGHT, 0.0) *
        Scale(LEFT_ARM_WIDTH, LEFT_ARM_HEIGHT, 1.0));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    model_view = mvstack.pop();
}

void
right_arm()
{
    mvstack.push(model_view);

    mat4 instance = (Translate(0.0, 0.5 * RIGHT_ARM_HEIGHT, 0.0) *
        Scale(RIGHT_ARM_WIDTH, RIGHT_ARM_HEIGHT, 1.0));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    model_view = mvstack.pop();
}


void
fan_center()
{
    mvstack.push(model_view);

    mat4 instance = (Translate(0.0, 0.0, 0.0) *
        Scale(1.0, FAN_CENTER_WIDTH, FAN_CENTER_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    model_view = mvstack.pop();
}

//void
//left_fan_center()
//{
//    mvstack.push(model_view);
//
//    mat4 instance = (Translate(0.0, 0.8 * FAN_CENTER_WIDTH, 0.0) *
//        Scale(FAN_CENTER_WIDTH, FAN_CENTER_WIDTH, FAN_CENTER_WIDTH));
//
//    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
//    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
//
//    model_view = mvstack.pop();
//}
//
//void
//right_fan_center()
//{
//    mvstack.push(model_view);
//
//    mat4 instance = (Translate(0.0, 0.8 * FAN_CENTER_WIDTH, 0.0) *
//        Scale(FAN_CENTER_WIDTH, FAN_CENTER_WIDTH, FAN_CENTER_WIDTH));
//
//    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
//    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
//
//    model_view = mvstack.pop();
//}

void
fan_wing()
{
    mvstack.push(model_view);

    mat4 instance = (Translate(0.0, 1.5*FAN_CENTER_WIDTH, 0.0) *
        Scale(FAN_WING_WIDTH, FAN_WING_HEIGHT, FAN_WING_WIDTH));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    model_view = mvstack.pop();
}



//----------------------------------------------------------------------------

void
display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    traverse(&nodes[Base]);

    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void
initNodes(void)
{
    mat4  m;

    // 1 Base
    m = RotateY(Theta[Base]);
    nodes[Base] = 
        Node(m, base, NULL, &nodes[LowerArm]);

    // 2 Lower
    m = Translate(0.0, 0.5 * BASE_HEIGHT, 0.0)*
        RotateZ(Theta[LowerArm])*
        RotateY(Theta[LowerArmY]);
    nodes[LowerArm] = 
        Node(m, lower_arm, NULL, &nodes[UpperArm]);

    // 3 Upper
    m = Translate(0.0, 0.8*LOWER_ARM_HEIGHT, 0.0) *
        RotateZ(Theta[UpperArm]);
    nodes[UpperArm] =
        Node(m, upper_arm, NULL, &nodes[LeftArm]);

    // 4 Left
    m = Translate(-(UPPER_ARM_WIDTH / 2 - LEFT_ARM_WIDTH / 2), 0.1 * UPPER_ARM_HEIGHT, 0.0) *
        RotateZ(Theta[LeftRightArm]);
    nodes[LeftArm] =
        Node(m, left_arm,
            &nodes[RightArm], &nodes[LeftFanCenter]);

    // 4 Right
    m = Translate(UPPER_ARM_WIDTH / 2 - RIGHT_ARM_WIDTH / 2, 0.1 * UPPER_ARM_HEIGHT, 0.0) *
        RotateZ(Theta[LeftRightArm]);
    nodes[RightArm] =
        Node(m, right_arm, NULL, &nodes[RightFanCenter]);


    // -----------------------


    // LEFT FAN
    // - center
    m = Translate(0.0, 0.8 * LEFT_ARM_HEIGHT, LEFT_ARM_WIDTH / 2) *
        RotateZ(Theta[LeftFanCenter]);
    nodes[LeftFanCenter] =
        Node(m, fan_center, NULL, &nodes[LeftFanCenter+1]);
    // - wing
    for (int i = 1; i < 5; i++) 
    {
        m = Translate(0, 0.0, 0.0) *
            RotateZ(Theta[LeftFanCenter+i]);
        Node* siblings = (i == 4) ? NULL : &nodes[LeftFanCenter + i + 1];
        nodes[LeftFanCenter+i] =
            Node(m, fan_wing, siblings, NULL);
    }




    // RIGHT FAN
    // - center
    m = Translate(0.0, 0.8 * RIGHT_ARM_HEIGHT, RIGHT_ARM_WIDTH / 2) *
        RotateZ(Theta[RightFanCenter]);
    nodes[RightFanCenter] =
        Node(m, fan_center, NULL, &nodes[RightFanCenter + 1]);
    // - wing
    for (int i = 1; i < 5; i++)
    {
        m = Translate(0, 0.0, 0.0) *
            RotateZ(Theta[RightFanCenter + i]);
        Node* siblings = (i == 4) ? NULL : &nodes[RightFanCenter + i + 1];
        nodes[RightFanCenter + i] =
            Node(m, fan_wing, siblings, NULL);
    }



}


//----------------------------------------------------------------------------

void
init(void)
{
    colorcube();

    // Initialize tree
    initNodes();

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(points)));

    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------
GLfloat speed[3] = { 1.0, 0.0, -1.0 };
int speed_idx = 0;
void idle()
{
    //printf("idle");
    Theta[LeftFanCenter] += speed[speed_idx];
    Theta[RightFanCenter] -= speed[speed_idx];

    if (Theta[LeftFanCenter] > 360.0) {
        Theta[LeftFanCenter] -= 360.0;
    }
    if (Theta[RightFanCenter] > 360.0) {
        Theta[RightFanCenter] -= 360.0;
    }

    if (Theta[LeftFanCenter] < 0.0) {
        Theta[LeftFanCenter] += 360.0;
    }
    if (Theta[RightFanCenter] < 0.0) {
        Theta[RightFanCenter] += 360.0;
    }

    nodes[LeftFanCenter].transform =
        Translate(0.0, 0.8 * LEFT_ARM_HEIGHT, LEFT_ARM_WIDTH/2) *
        RotateZ(Theta[LeftFanCenter]);
    nodes[RightFanCenter].transform =
        Translate(0.0, 0.8 * RIGHT_ARM_HEIGHT, RIGHT_ARM_WIDTH / 2) *
        RotateZ(Theta[RightFanCenter]);

    glutPostRedisplay();
}

GLfloat angle_lower = 15.0;
GLfloat angle_lr = 15.0;

void
mouse(int button, int state, int x, int y)
{

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Incrase the joint angle
        if (Axis == LowerArm) { // 90 ~ 0, 360 ~ 270
            Theta[Axis] += angle_lower;
            if (angle_lower > 0) {
                if (Theta[Axis] > 360.0) {
                    Theta[Axis] -= 360;
                }
                else if (Theta[Axis] > 90.0 && Theta[Axis] < 270.0) {
                    angle_lower *= -1;
                    Theta[Axis] += 2 * angle_lower;
                }
            }
            else {
                if (Theta[Axis] < 0.0) {
                    Theta[Axis] += 360;
                }
                else if (Theta[Axis] > 90.0 && Theta[Axis] < 270.0) { 
                    angle_lower *= -1;
                    Theta[Axis] += 2*angle_lower;
                }
            }
        }
        else if (Axis == LeftRightArm) {
            Theta[Axis] += angle_lr;
            if (angle_lr > 0) {
                if (Theta[Axis] > 360.0) {
                    Theta[Axis] -= 360;
                }
                else if (Theta[Axis] > 90.0 && Theta[Axis] < 270.0) {
                    angle_lr *= -1;
                    Theta[Axis] += 2 * angle_lr;
                }
            }
            else {
                if (Theta[Axis] < 0.0) {
                    Theta[Axis] += 360;
                }
                else if (Theta[Axis] > 90.0 && Theta[Axis] < 270.0) {
                    angle_lr *= -1;
                    Theta[Axis] += 2 * angle_lr;
                }
            }
        }
        else {
            Theta[Axis] += 10.0;
            if (Theta[Axis] > 360.0) { Theta[Axis] -= 360.0; }
        }
    }

    

    mvstack.push(model_view);
    switch (Axis) {
    case LowerArm:
        nodes[LowerArm].transform = 
            Translate(0.0, 0.5 * BASE_HEIGHT, 0.0) *
            RotateZ(Theta[LowerArm]) *
            RotateY(Theta[LowerArmY]);
        break;
    case LowerArmY:
        nodes[LowerArm].transform =
            Translate(0.0, 0.5 * BASE_HEIGHT, 0.0) *
            RotateZ(Theta[LowerArm]) *
            RotateY(Theta[LowerArmY]);
        break;

    case LeftRightArm:
        nodes[LeftArm].transform =
            Translate(-(UPPER_ARM_WIDTH / 2 - LEFT_ARM_WIDTH / 2), 0.1 * UPPER_ARM_HEIGHT, 0.0) *
            RotateZ(Theta[LeftRightArm]);
        nodes[RightArm].transform =
            Translate(UPPER_ARM_WIDTH / 2 - RIGHT_ARM_WIDTH / 2, 0.1 * UPPER_ARM_HEIGHT, 0.0) *
            RotateZ(Theta[LeftRightArm]);
        break;
    }



    model_view = mvstack.pop();
}

//----------------------------------------------------------------------------

void
menu(int option)
{
    if (option == Quit) {
        exit(EXIT_SUCCESS);
    }
    else {
        Axis = option;
    }
}

//----------------------------------------------------------------------------

void
reshape(int width, int height)
{
    glViewport(0, 0, width, height);

    GLfloat  left = -10.0, right = 10.0;
    GLfloat  bottom = -5.0, top = 15.0;
    GLfloat  zNear = -10.0, zFar = 10.0;

    GLfloat aspect = GLfloat(width) / height;

    if (aspect > 1.0) {
        left *= aspect;
        right *= aspect;
    }
    else {
        bottom /= aspect;
        top /= aspect;
    }

    mat4 projection = Ortho(left, right, bottom, top, zNear, zFar);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

    model_view = mat4(1.0);  // An Identity matrix
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 033: // Escape Key
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
    case ' ':
        speed_idx += 1;
        if (speed_idx >= 3) speed_idx = 0;
        break;
    }
}

//----------------------------------------------------------------------------

int
main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("2020062324");

    glewInit();

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);

    glutCreateMenu(menu);
    // Set the menu values to the relevant rotation axis values (or Quit)
    glutAddMenuEntry("lower arm - z", LowerArm); // z축 회전
    glutAddMenuEntry("lower arm - y", LowerArmY); // y축 회전
    glutAddMenuEntry("Left, Right", LeftRightArm); // z축

    glutAddMenuEntry("quit", Quit);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
