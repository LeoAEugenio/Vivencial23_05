#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <string>

using namespace std;

GLFWwindow *Window = nullptr;
GLuint Shader_programm = 0;
GLuint Vao_esfera = 0;
int Num_vertices_esfera = 0;

int WIDTH = 800;
int HEIGHT = 600;

float Tempo_entre_frames = 0.0f;

float Cam_speed = 10.0f;
glm::vec3 Cam_pos(0.0f, 0.0f, 4.0f);
float Cam_yaw = -90.0f;
float Cam_pitch = 0.0f;

double lastX = WIDTH / 2.0;
double lastY = HEIGHT / 2.0;
bool primeiro_mouse = true;

glm::vec3 objetoPos(0.0f, 0.0f, 0.0f);
glm::vec3 objetoEscala(1.0f, 1.0f, 1.0f);

bool luzPrincipalLigada = true;
bool luzPreenchimentoLigada = true;
bool luzFundoLigada = true;

void redimensionaCallback(GLFWwindow *window, int w, int h)
{
    WIDTH = w;
    HEIGHT = h;
    glViewport(0, 0, w, h);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (primeiro_mouse)
    {
        lastX = xpos;
        lastY = ypos;
        primeiro_mouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensibilidade = 0.1f;
    xoffset *= sensibilidade;
    yoffset *= sensibilidade;

    Cam_yaw += xoffset;
    Cam_pitch += yoffset;

    if (Cam_pitch > 89.0f)
        Cam_pitch = 89.0f;

    if (Cam_pitch < -89.0f)
        Cam_pitch = -89.0f;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_1)
        {
            luzPrincipalLigada = !luzPrincipalLigada;
            cout << "Luz principal: " << (luzPrincipalLigada ? "ligada" : "desligada") << endl;
        }

        if (key == GLFW_KEY_2)
        {
            luzPreenchimentoLigada = !luzPreenchimentoLigada;
            cout << "Luz preenchimento: " << (luzPreenchimentoLigada ? "ligada" : "desligada") << endl;
        }

        if (key == GLFW_KEY_3)
        {
            luzFundoLigada = !luzFundoLigada;
            cout << "Luz de fundo: " << (luzFundoLigada ? "ligada" : "desligada") << endl;
        }
    }
}

void inicializaOpenGL()
{
    glfwInit();

    Window = glfwCreateWindow(WIDTH, HEIGHT, "Vivencial 2 - Iluminacao 3 Pontos", nullptr, nullptr);

    if (Window == nullptr)
    {
        cout << "Erro ao criar janela GLFW" << endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(Window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Erro ao inicializar GLAD" << endl;
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(Window, redimensionaCallback);
    glfwSetCursorPosCallback(Window, mouse_callback);
    glfwSetKeyCallback(Window, key_callback);
    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    cout << "Placa de video: " << glGetString(GL_RENDERER) << endl;
    cout << "Versao do OpenGL: " << glGetString(GL_VERSION) << endl;
}

void inicializaEsfera()
{
    vector<float> points;

    int stacks = 30;
    int sectors = 30;
    float radius = 0.5f;
    const float PI = 3.14159265359f;

    for (int i = 0; i < stacks; ++i)
    {
        float phi1 = PI * float(i) / stacks;
        float phi2 = PI * float(i + 1) / stacks;

        for (int j = 0; j < sectors; ++j)
        {
            float theta1 = 2.0f * PI * float(j) / sectors;
            float theta2 = 2.0f * PI * float(j + 1) / sectors;

            auto addVertex = [&](float p, float t)
            {
                float x = radius * sin(p) * cos(t);
                float y = radius * cos(p);
                float z = radius * sin(p) * sin(t);

                points.push_back(x);
                points.push_back(y);
                points.push_back(z);

                points.push_back(x / radius);
                points.push_back(y / radius);
                points.push_back(z / radius);
            };

            addVertex(phi1, theta1);
            addVertex(phi2, theta1);
            addVertex(phi1, theta2);

            addVertex(phi1, theta2);
            addVertex(phi2, theta1);
            addVertex(phi2, theta2);
        }
    }

    Num_vertices_esfera = points.size() / 6;

    GLuint VBO;
    glGenVertexArrays(1, &Vao_esfera);
    glGenBuffers(1, &VBO);

    glBindVertexArray(Vao_esfera);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void *)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void *)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);
}

GLuint compilaShader(const char *source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        cout << "Erro ao compilar shader: " << infoLog << endl;
    }

    return shader;
}

void inicializaShaders()
{
    const char *vertex_shader = R"(
        #version 330 core

        layout(location = 0) in vec3 vertex_posicao;
        layout(location = 1) in vec3 vertex_normal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 proj;

        out vec3 fragPos;
        out vec3 normal;

        void main()
        {
            vec4 worldPos = model * vec4(vertex_posicao, 1.0);
            fragPos = worldPos.xyz;
            normal = mat3(transpose(inverse(model))) * vertex_normal;
            gl_Position = proj * view * worldPos;
        }
    )";

    const char *fragment_shader = R"(
        #version 330 core

        struct PointLight
        {
            vec3 position;
            vec3 color;
            float intensity;
            float enabled;
        };

        in vec3 fragPos;
        in vec3 normal;

        out vec4 frag_colour;

        uniform PointLight lights[3];

        uniform vec3 viewPos;
        uniform vec3 objectColor;

        uniform float Ka;
        uniform float Kd;
        uniform float Ks;
        uniform float shininess;

        vec3 calculaLuz(PointLight light, vec3 N, vec3 V)
        {
            if (light.enabled < 0.5)
            {
                return vec3(0.0);
            }

            vec3 L = normalize(light.position - fragPos);
            vec3 R = normalize(reflect(-L, N));

            float d = length(light.position - fragPos);

            float Kc = 1.0;
            float Kl = 0.09;
            float Kq = 0.032;

            float attenuation = 1.0 / (Kc + Kl * d + Kq * d * d);

            float diff = max(dot(N, L), 0.0);
            vec3 diffuse = Kd * diff * light.color * light.intensity;

            float spec = pow(max(dot(V, R), 0.0), shininess);
            vec3 specular = Ks * spec * light.color * light.intensity;

            // Atenuação na reflexão difusa
            diffuse *= attenuation;

            // Atenuação também na especular
            specular *= attenuation;

            return diffuse + specular;
        }

        void main()
        {
            vec3 N = normalize(normal);
            vec3 V = normalize(viewPos - fragPos);

            vec3 ambient = Ka * vec3(1.0);

            vec3 result = ambient * objectColor;

            result += calculaLuz(lights[0], N, V) * objectColor;
            result += calculaLuz(lights[1], N, V) * objectColor;
            result += calculaLuz(lights[2], N, V) * objectColor;

            frag_colour = vec4(result, 1.0);
        }
    )";

    GLuint vs = compilaShader(vertex_shader, GL_VERTEX_SHADER);
    GLuint fs = compilaShader(fragment_shader, GL_FRAGMENT_SHADER);

    Shader_programm = glCreateProgram();

    glAttachShader(Shader_programm, vs);
    glAttachShader(Shader_programm, fs);

    glLinkProgram(Shader_programm);

    GLint success;
    glGetProgramiv(Shader_programm, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(Shader_programm, 512, nullptr, infoLog);
        cout << "Erro ao linkar shader program: " << infoLog << endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void transformacaoGenerica(float Tx, float Ty, float Tz,
                           float Sx, float Sy, float Sz,
                           float Rx, float Ry, float Rz)
{
    glm::mat4 transform(1.0f);

    transform = glm::translate(transform, glm::vec3(Tx, Ty, Tz));
    transform = glm::rotate(transform, glm::radians(Rz), glm::vec3(0, 0, 1));
    transform = glm::rotate(transform, glm::radians(Ry), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(Rx), glm::vec3(1, 0, 0));
    transform = glm::scale(transform, glm::vec3(Sx, Sy, Sz));

    GLuint loc = glGetUniformLocation(Shader_programm, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transform));
}

void especificaMatrizVisualizacao()
{
    glm::vec3 front;

    front.x = cos(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    front.y = sin(glm::radians(Cam_pitch));
    front.z = sin(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));

    front = glm::normalize(front);

    glm::mat4 view = glm::lookAt(
        Cam_pos,
        Cam_pos + front,
        glm::vec3(0, 1, 0)
    );

    GLuint loc = glGetUniformLocation(Shader_programm, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
}

void especificaMatrizProjecao()
{
    glm::mat4 proj = glm::perspective(
        glm::radians(67.0f),
        (float)WIDTH / HEIGHT,
        0.1f,
        100.0f
    );

    GLuint loc = glGetUniformLocation(Shader_programm, "proj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
}

void inicializaCamera()
{
    especificaMatrizVisualizacao();
    especificaMatrizProjecao();
}

void trataTeclado()
{
    float velocidade = Cam_speed * Tempo_entre_frames;

    glm::vec3 frente;

    frente.x = cos(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    frente.y = sin(glm::radians(Cam_pitch));
    frente.z = sin(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));

    frente = glm::normalize(frente);

    glm::vec3 direita = glm::normalize(glm::cross(frente, glm::vec3(0, 1, 0)));

    if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
        Cam_pos += frente * velocidade;

    if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
        Cam_pos -= frente * velocidade;

    if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
        Cam_pos -= direita * velocidade;

    if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
        Cam_pos += direita * velocidade;

    if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(Window, true);
}

void defineMaterial(float r, float g, float b,
                    float ka, float kd, float ks,
                    float shininess)
{
    glUniform3f(
        glGetUniformLocation(Shader_programm, "objectColor"),
        r,
        g,
        b
    );

    glUniform1f(glGetUniformLocation(Shader_programm, "Ka"), ka);
    glUniform1f(glGetUniformLocation(Shader_programm, "Kd"), kd);
    glUniform1f(glGetUniformLocation(Shader_programm, "Ks"), ks);
    glUniform1f(glGetUniformLocation(Shader_programm, "shininess"), shininess);
}

void enviaLuz(int indice, glm::vec3 pos, glm::vec3 cor, float intensidade, bool ligada)
{
    string base = "lights[" + to_string(indice) + "]";

    glUniform3fv(
        glGetUniformLocation(Shader_programm, (base + ".position").c_str()),
        1,
        glm::value_ptr(pos)
    );

    glUniform3fv(
        glGetUniformLocation(Shader_programm, (base + ".color").c_str()),
        1,
        glm::value_ptr(cor)
    );

    glUniform1f(
        glGetUniformLocation(Shader_programm, (base + ".intensity").c_str()),
        intensidade
    );

    glUniform1f(
        glGetUniformLocation(Shader_programm, (base + ".enabled").c_str()),
        ligada ? 1.0f : 0.0f
    );
}

void enviaLuzesTresPontos()
{
    float size = objetoEscala.x;

    glm::vec3 keyPos = objetoPos + glm::vec3(
        -2.5f * size,
        2.0f * size,
        2.5f * size
    );

    glm::vec3 fillPos = objetoPos + glm::vec3(
        2.5f * size,
        1.3f * size,
        2.0f * size
    );

    glm::vec3 backPos = objetoPos + glm::vec3(
        0.0f,
        2.2f * size,
        -2.8f * size
    );

    // Luz principal
    enviaLuz(
        0,
        keyPos,
        glm::vec3(1.0f, 0.92f, 0.82f),
        1.4f,
        luzPrincipalLigada
    );

    // Luz de preenchimento
    enviaLuz(
        1,
        fillPos,
        glm::vec3(0.75f, 0.85f, 1.0f),
        0.55f,
        luzPreenchimentoLigada
    );

    // Luz de fundo
    enviaLuz(
        2,
        backPos,
        glm::vec3(1.0f, 1.0f, 0.9f),
        0.9f,
        luzFundoLigada
    );
}

void inicializaRenderizacao()
{
    float tempo_anterior = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(Window))
    {
        float tempo_atual = glfwGetTime();
        Tempo_entre_frames = tempo_atual - tempo_anterior;
        tempo_anterior = tempo_atual;

        // Cor do fundo: escuro
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(Shader_programm);

        inicializaCamera();

        glUniform3fv(
            glGetUniformLocation(Shader_programm, "viewPos"),
            1,
            glm::value_ptr(Cam_pos)
        );

        enviaLuzesTresPontos();

        // Cor da esfera: azul
        defineMaterial(
            0.2f, 0.45f, 1.0f,
            0.1f,
            0.7f,
            1.0f,
            32.0f
        );

        glBindVertexArray(Vao_esfera);

        transformacaoGenerica(
            objetoPos.x,
            objetoPos.y,
            objetoPos.z,
            objetoEscala.x,
            objetoEscala.y,
            objetoEscala.z,
            0.0f,
            0.0f,
            0.0f
        );

        glDrawArrays(GL_TRIANGLES, 0, Num_vertices_esfera);

        glfwSwapBuffers(Window);
        glfwPollEvents();

        trataTeclado();
    }

    glfwTerminate();
}

int main()
{
    inicializaOpenGL();
    inicializaEsfera();
    inicializaShaders();
    inicializaRenderizacao();

    return 0;
}