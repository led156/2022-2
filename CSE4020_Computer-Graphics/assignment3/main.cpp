// rotating cube with two texture objects
// change textures with 1 and 2 keys

#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef vec4 point4;
typedef vec4 color4;

// Texture objects and storage for texture image
GLuint textures[3];

std::vector<vec4> points;
std::vector<vec3> normals;
std::vector<vec2> texCoords;
std::vector<int> indices;


// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Yaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

GLuint ModelView, Projection, LightPosition;
GLfloat fovy = 45.0;
GLfloat aspect;
GLfloat zNear = 0.1, zFar = 10.0;
//----------------------------------------------------------------------------

int Index = 0;

double c_pos[3][2] = { {-0.8, 0.2},
                       {-0.5, -0.2},
                       {-0.3, 0.8} };
int c = 0;
double c1 = c_pos[c][0], c2 = c_pos[c][1];


//----------------------------------------------------------------------------

void createSphere(GLfloat radius, int NumSectors, int NumStacks)
{
    GLfloat x, y, z, xy;

    GLfloat stackStep = M_PI / (GLfloat)NumStacks;
    GLfloat sectorStep = 2.0 * M_PI / (GLfloat)NumSectors;

    // compute vertices
    for (int i = 0; i <= NumStacks; i++) {
        vec4 pt;
        vec3 nor;
        vec2 tex;

        float stackAngle = M_PI_2 - (GLfloat)i * stackStep;
        xy = cosf(stackAngle);
        z = sinf(stackAngle);

        for (int j = 0; j <= NumSectors; j++) {
            float sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            pt = vec4(radius * x, radius * y, radius * z, 1.0);
            nor = vec3(x, y, z);
            tex = vec2((GLfloat)j / NumSectors, (GLfloat)i / NumStacks);

            points.push_back(pt);
            normals.push_back(nor);
            texCoords.push_back(tex);
        }
    }

    //create indices
    // k1 - k1 + 1
    // |   /   |
    // k2 - k2 + 1
    for (int i = 0; i < NumStacks; i++) {
        int k1 = i * (NumSectors + 1);
        int k2 = k1 + NumSectors + 1;
        for (int j = 0; j < NumSectors; j++, k1++, k2++) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != NumStacks - 1) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}


//----------------------------------------------------------------------------
void
init()
{
    createSphere(0.5, 18, 36);


    // Initialize texture objects
    glGenTextures(3, textures);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    int texWidth, texHeight, texChannels;
    unsigned char* data = stbi_load("earth_basic.jpg", &texWidth, &texHeight, &texChannels, 0);

    std::cout << texChannels << "\n";
    if (data) {

        //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
            //	  GL_RGB, GL_UNSIGNED_BYTE, image );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Fail to load earth_basic.jpg\n";
    }
    stbi_image_free(data);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    data = stbi_load("cloud.jpg", &texWidth, &texHeight, &texChannels, 0);
    if (data) {

        //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
            //	  GL_RGB, GL_UNSIGNED_BYTE, image );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texWidth, texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Fail to load cloud.jpg\n";
    }
    stbi_image_free(data);


    //+++++++++++++++++++++++++++++++++++++++++++++
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    data = stbi_load("night.png", &texWidth, &texHeight, &texChannels, 0);
    if (data) {

        //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
            //	  GL_RGB, GL_UNSIGNED_BYTE, image );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texWidth, texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Fail to load night.png\n";
    }
    stbi_image_free(data);
    //+++++++++++++++++++++++++++++++++++++++++++++\



    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and initialize a buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(vec4) * points.size() +
        sizeof(vec3) * normals.size() +
        sizeof(vec2) * texCoords.size(),
        NULL, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
        indices.data(), GL_STATIC_DRAW);



    // Specify an offset to keep track of where we're placing data in our
    //   vertex array buffer.  We'll use the same technique when we
    //   associate the offsets with vertex attribute pointers.
    GLintptr offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec4) * points.size(),
        points.data());
    offset += sizeof(vec4) * points.size();

    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec3) * normals.size(),
        normals.data());
    offset += sizeof(vec3) * normals.size();

    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec2) * texCoords.size(),
        texCoords.data());

    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    // set up vertex arrays
    offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));
    offset += sizeof(vec4) * points.size();

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));
    offset += sizeof(vec3) * normals.size();

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     // Initialize shader lighting parameters
    point4 light_position(c1, c2, 0.0, 0.0);
    color4 light_ambient(0.2, 0.2, 0.2, 1.0);
    color4 light_diffuse(1.0, 1.0, 1.0, 1.0); // 광원 색상 변경
    color4 light_specular(1.0, 1.0, 1.0, 1.0);

    color4 material_ambient(0.5, 0.5, 0.5, 1.0); // 물체 재질 색 값
    color4 material_diffuse(1.0, 1.0, 1.0, 1.0);
    color4 material_specular(1.0, 1.0, 1.0, 1.0);
    float  material_shininess = 100.0;

    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;

    glUniform4fv(glGetUniformLocation(program, "AmbientProduct"),
        1, ambient_product);
    glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"),
        1, diffuse_product);
    glUniform4fv(glGetUniformLocation(program, "SpecularProduct"),
        1, specular_product);

    LightPosition = glGetUniformLocation(program, "LightPosition");
    glUniform4fv(LightPosition, 1, light_position);
    //glUniformMatrix4fv(LightPosition, 1, GL_TRUE, light_position);

    glUniform1f(glGetUniformLocation(program, "Shininess"),
        material_shininess);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");
    LightPosition = glGetUniformLocation(program, "LightPosition");


    // Set the value of the fragment shader texture sampler variable
    //   ("texture") to the the appropriate texture unit. In this case,
    //   zero, for GL_TEXTURE0 which was previously set by calling
    //   glActiveTexture().
    glUniform1i(glGetUniformLocation(program, "mainTex"), 0);
    glUniform1i(glGetUniformLocation(program, "cloudTex"), 1);
    glUniform1i(glGetUniformLocation(program, "nightTex"), 2);

    //theta = glGetUniformLocation( program, "theta" );

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void
display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glUniform3fv( theta, 1, Theta );

    const vec3 viewer_pos(0.0, 0.0, 2.0);
    mat4  model_view = (Translate(-viewer_pos) *
        RotateX(Theta[Xaxis]) *
        RotateY(Theta[Yaxis]) *
        RotateZ(Theta[Zaxis]));
    model_view *= RotateX(270) * Scale(1.5, 1.5, 1.5);
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

    mat4 p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, p);

    point4 light_position(c1, c2, 0.0, 0.0);
    //glUniformMatrix4fv(LightPosition, 1, GL_TRUE, light_position);
    glUniform4fv(LightPosition, 1, light_position);

    //glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
    
}

//--------------------------------------------------------------------



//----------------------------------------------------------------------------
double speed = 0.1;
void
idle(void)
{
    Theta[Axis] += speed;

    if (Theta[Axis] > 360.0) {
        Theta[Axis] -= 360.0;
    }

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int mousex, int mousey)
{
    switch (key) {
    case 033: // Escape Key
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
        //case '1':
          //  glBindTexture( GL_TEXTURE_2D, textures[0] );
            //break;

        //case '2':
          //  glBindTexture( GL_TEXTURE_2D, textures[1] );
            //break;
    case 'u':
        if (speed <= 1)
            speed += 0.05;
        printf("%f\n", speed);
        break;
    case 'd':
        if (speed > 0.01)
            speed -= 0.05;
        printf("%f\n", speed);
        break;
    case 'm':
        c = (c + 1) % 3;
        c1 = c_pos[c][0]; c2 = c_pos[c][1];
        break;
    }


    glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = GLfloat(width) / height;
}
//----------------------------------------------------------------------------

int
main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);

    aspect = 512.0 / 512.0;

    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("2020062324");

    glewInit();

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
