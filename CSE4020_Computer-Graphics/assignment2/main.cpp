#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "ObjLoader.h"

//viewing transformation parameters

GLfloat radius = 1.0;
GLfloat theta = 0.0;
GLfloat phi = 0.0;

const GLfloat dr = 5.0 * DegreesToRadians;

GLuint view; // model-view matrix uniform shader variable location
GLuint model;

// Projection transformation parameters
GLfloat fovy = 45.0; //field-of-view in y direction angle (in degrees)
GLfloat aspect; //Viewport aspect ratio
GLfloat zNear = 0.1, zFar = 10.0;

GLuint projection; //projection matrix uniform shader variable location
GLuint color_loc;

mat4 viewMat[2];
mat4 projMat[2];

GLuint wire;

int Width;
int Height;

class TriMesh
{
public:
    int NumVertices;
    int NumTris;

    GLuint vao;
    GLuint vbo;
    GLuint ebo; // element buffer object for indices

    //vec4* vertices;
    //vec4 vertices[8];
    std::vector<vec4> vertices;

    vec3 color;
    mat4 modelTransform;

    //unsigned int* indices;
    //unsigned int indices[6];
    std::vector<unsigned int> indices;

    TriMesh() {
        NumVertices = 0;
        NumTris = 0;
        //vertices = NULL;
        //indices = NULL;
    }

    ~TriMesh() {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);

        //if (vertices != NULL)
         //   delete[] vertices;
        //if (indices != NULL)
          //  delete[] indices;
    }

    void init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glBindVertexArray(0);
    }

    void Render() {
        //set a constant color
        glUniform3fv(color_loc, 1, color);
        glUniformMatrix4fv(model, 1, GL_TRUE, modelTransform);


        glBindVertexArray(vao);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform1i(wire, 0);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniform1i(wire, 1);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

TriMesh bunny;




const int NumCrvVertices = 1024;

int Index = 0;

GLfloat BezierBasis[NumCrvVertices][4];

// 3차 베지에 커브.
struct BezierCurve {
    vec4 CtrlPts[4];

    vec4 points[NumCrvVertices + 4];
    vec3 colors[NumCrvVertices + 4];

    GLuint vao;
    GLuint vbo;

    BezierCurve() {
    }
    BezierCurve(const vec4& p0, const vec4& p1, const vec4& p2, const vec4& p3)
    {
        CtrlPts[0] = p0;
        CtrlPts[1] = p1;
        CtrlPts[2] = p2;
        CtrlPts[3] = p3;
    }

    ~BezierCurve() {

    }
    vec4 evaluate(const GLfloat t) {

    }
    // C에 해당하는 C_t 값만 따로 계산.
    vec4 evaluate(const int i) {
        vec4 pts;
        for (int j = 0; j < 3; j++) {
            pts[j] = BezierBasis[i][0] * CtrlPts[0][j]
                + BezierBasis[i][1] * CtrlPts[1][j]
                + BezierBasis[i][2] * CtrlPts[2][j]
                + BezierBasis[i][3] * CtrlPts[3][j];
        }
        pts[3] = 1.0;
        return pts;
    }

    void evalulate() {
        for (int i = 0; i < NumCrvVertices; i++) {
            for (int j = 0; j < 3; j++)
                points[i][j] = BezierBasis[i][0] * CtrlPts[0][j]
                + BezierBasis[i][1] * CtrlPts[1][j]
                + BezierBasis[i][2] * CtrlPts[2][j]
                + BezierBasis[i][3] * CtrlPts[3][j];
            points[i][3] = 1.0;
        }
    }
    void updateForRendering() {
        evalulate(); // 0~1024 까지의 벡터 어레이 곡선은 그려짐.

        // for drawing tangent handles
        // 1024 부터 거꾸로 저장.
        points[NumCrvVertices] = CtrlPts[3];
        points[NumCrvVertices + 1] = CtrlPts[2];
        points[NumCrvVertices + 2] = CtrlPts[1];
        points[NumCrvVertices + 3] = CtrlPts[0];
    }
};

BezierCurve curve;
int crv_edit_handle = -1;

// precompute polynomial bases for Hermite spline
void precomputeBezierBasis()
{
    GLfloat t, t2, t3;
    for (int i = 0; i < NumCrvVertices; i++) {
        t = i * 1.0 / (GLfloat)(NumCrvVertices - 1.0);
        t2 = t * t;
        t3 = 1 - t;
        BezierBasis[i][0] = t3 * t3 * t3;
        BezierBasis[i][1] = 3.0 * t3 * t3 * t;
        BezierBasis[i][2] = 3.0 * t3 * t2;
        BezierBasis[i][3] = t * t2;
    }
}


GLfloat left_ = -1.0, right_ = 1.0;
GLfloat bottom = -1.0, top = 1.0;




void init(void)
{
    /* 커브 */
    precomputeBezierBasis();
    curve = BezierCurve(vec4(0.1, 0.0, 0.0, 1.0), vec4(0.15, 0.0, 0.13, 1.0), vec4(0.0, 0.0, 0.15, 1.0), vec4(-0.1, 0.0, 0.1, 1.0));

    curve.updateForRendering();

    //for colors
    for (int i = 0; i < NumCrvVertices; i++)
        curve.colors[i] = vec3(0.0, 0.0, 0.0);
    curve.colors[NumCrvVertices] = vec3(1.0, 0.0, 0.0);
    curve.colors[NumCrvVertices + 1] = vec3(1.0, 0.0, 0.0);
    curve.colors[NumCrvVertices + 2] = vec3(1.0, 0.0, 0.0);
    curve.colors[NumCrvVertices + 3] = vec3(1.0, 0.0, 0.0);

    glGenVertexArrays(1, &(curve.vao));
    glBindVertexArray(curve.vao);

    glGenBuffers(1, &(curve.vbo));
    glBindBuffer(GL_ARRAY_BUFFER, curve.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(curve.points) + sizeof(curve.colors), NULL,
        GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve.points), curve.points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(curve.points), sizeof(curve.colors), curve.colors);

    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve.points)));


    CObjLoader* loader = new CObjLoader();
    loader->Load("bunny.obj", NULL);

    bunny.NumVertices = loader->vertexes.size();
    bunny.NumTris = loader->parts[0].faces.size();

    bunny.vertices.resize(bunny.NumVertices);
    for (int i = 0; i < bunny.NumVertices; i++) {
        tVertex vtx = loader->getVertex(i);
        bunny.vertices[i] = vec4(vtx.x, vtx.y, vtx.z, 1.0);
    }

    bunny.indices.resize(bunny.NumTris * 3);
    for (int i = 0; i < bunny.NumTris; i++) {
        tFace* face = &(loader->parts[0].faces[i]);

        for (int j = 0; j < 3; j++)
            bunny.indices[3 * i + j] = (unsigned int)(face->v[j] - 1);
    }
    delete loader;

    bunny.color = vec3(0, 0.7, 0.8);
    bunny.modelTransform = Scale(2.0);


    bunny.init();

    //initialize vertex position attribute from vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));



    vec4 eye(radius * sin(theta) * sin(phi),
        radius * cos(theta),
        radius * sin(theta) * cos(phi),
        1.0);
    vec4 at(0.0, 0.0, 0.0, 1.0);
    vec4 up(0.0, 0.0, -1.0, 0.0);

    viewMat[0] = LookAt(eye, at, up);
    projMat[0] = Perspective(fovy, aspect, zNear, zFar);


    vec4 eye2(radius * sin(theta) * sin(phi),
        radius * cos(theta),
        radius * sin(theta) * cos(phi),
        1.0);
    vec4 at2(0.0, 0.0, 0.0, 1.0);
    vec4 up2(0.0, 0.0, -1.0, 0.0);

    viewMat[1] = LookAt(eye2, at2, up2);
    projMat[1] = Perspective(fovy, aspect, zNear, zFar);


    //initialize uniform variable from vertex shander
    model = glGetUniformLocation(program, "model");
    view = glGetUniformLocation(program, "view");
    projection = glGetUniformLocation(program, "projection");
    color_loc = glGetUniformLocation(program, "segColor");

    wire = glGetUniformLocation(program, "wired");

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 0.0);
}

int temp = 0;
int cnt = 3;
bool isRotate = true;
void idle()
{
    if (isRotate) {
        temp += cnt;

        if (temp >= NumCrvVertices - 3 || temp <= 2) {
            cnt *= -1;
        }
    }

    glutPostRedisplay();
}
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //* 우측 화면 */
    vec4 eye2(curve.points[temp][0],
        0.0,
        curve.points[temp][2],
        1.0);
    vec4 at2(0.0, 0.0, 0.0, 1.0);
    vec4 up2(0.0, 1.0, 0.0, 0.0);

    viewMat[1] = LookAt(eye2, at2, up2);
    projMat[1] = Perspective(fovy, aspect, zNear, zFar);


    glViewport(Width / 2, 0, Width / 2, Height);
    glUniformMatrix4fv(view, 1, GL_TRUE, viewMat[1]);
    glUniformMatrix4fv(projection, 1, GL_TRUE, projMat[1]);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bunny.Render();

    /* 좌측 화면 */
    glViewport(0, 0, Width / 2, Height);
    glUniformMatrix4fv(view, 1, GL_TRUE, viewMat[0]);
    glUniformMatrix4fv(projection, 1, GL_TRUE, projMat[0]);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bunny.Render();

    /* 커브 그리기 */
    glBindVertexArray(curve.vao);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve.points), curve.points);

    //glLineWidth(2.0f);
    glDrawArrays(GL_LINE_STRIP, 0, NumCrvVertices + 4);
    //glBindVertexArray(0);





    glutSwapBuffers();
}



void
keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 033:
        exit(EXIT_SUCCESS);
        break;


    case ' ':  // reset values to their defaults
        printf("spacebar");
        isRotate = !isRotate;
        break;
    }
    glutPostRedisplay();
}



int HitIndex(BezierCurve* curve, int x, int y)
{

    int ret = -1;
    GLfloat dist[4], mindist = FLT_MAX;

    // a size of one pixel in the curve coordinate system.
    vec2 pixelLen((right_ - left_) / (GLfloat)(Width / 2), (top - bottom) / (GLfloat)Height);

    // the current mouse point in the curve coordinate
    vec2 mousePt, tmpVec;
    mousePt.x = left_ + pixelLen[0] * (GLfloat)x;
    mousePt.y = bottom + pixelLen[1] * (GLfloat)y;
    printf("%f, %f \n", mousePt.x, mousePt.y);
    // Measure the squared distance between mouse point and handles
    for (int i = 0; i < 4; i++) {
        tmpVec = vec2(curve->CtrlPts[i][0], curve->CtrlPts[i][2]) - mousePt;
        dist[i] = dot(tmpVec, tmpVec);
        printf("dist : %f\n", dist[i]);
    }

    for (int i = 0; i < 4; i++) {
        if (mindist > dist[i]) {
            ret = i;
            mindist = dist[i];
        }
    }

    // if clicked within 10 pixel radius of one of handles then return
    if (mindist < 100.0 * dot(pixelLen, pixelLen)) {
        printf("[click : %d]\n", ret);
        return ret;
    }
    else
        return -1;
}

void mouse(GLint button, GLint action, GLint x, GLint y)
{
    if (!isRotate && GLUT_LEFT_BUTTON == button)
    {
        printf("click\n");
        switch (action)
        {
        case GLUT_DOWN: // 왼쪽 버튼이 눌리면
            crv_edit_handle = HitIndex(&curve, x, Height - y);
            break;
        case GLUT_UP:
            crv_edit_handle = -1;
            break;
        default: break;
        }
    }
    glutPostRedisplay();
}

//void mouseMove(GLint x, GLint y)
//{
//    if (crv_edit_handle != -1) {
//        vec2 pixelLen((right_ - left_) / (GLfloat)(Width), (top - bottom) / (GLfloat)Height);
//        vec3 mousePt;
//        mousePt.x = left_ + (GLfloat)x * pixelLen[0];
//        mousePt.z = bottom + (GLfloat)(Height - y) * pixelLen[1];
//
//        // P0, P1, P2 가 모두 포인트이기 때문에 if 처리 X.
//        curve.CtrlPts[crv_edit_handle] = mousePt;
//
//        curve.updateForRendering();
//
//        glutPostRedisplay();
//    }
//}



int main(int argc, char** argv)
{
    Width = 1024;
    Height = 512;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(Width, Height);

    aspect = 1;

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
    //glutMotionFunc(mouseMove);
    glutIdleFunc(idle);

    glutMainLoop();

    return 0;
}