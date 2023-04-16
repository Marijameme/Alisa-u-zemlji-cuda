#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "rg/Error.h"
#include "rg/Texture2D.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(std::vector<std::string> faces);

void configurate_instance_buffer(unsigned int num, Model model, glm::mat4 *modelMatrices, bool normal_mapping);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = false;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Alisa u zemlji cuda", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
//    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // build and compile shaders
    // -------------------------
    Shader platoShader("resources/shaders/plato.vs", "resources/shaders/plato.fs");
    Shader skyBoxShader("resources/shaders/sky_box.vs", "resources/shaders/sky_box.fs");
    Shader instanceShader("resources/shaders/instance.vs", "resources/shaders/instance.fs");
    Shader normalShader("resources/shaders/normal.vs", "resources/shaders/normal.fs");

    // load models
    // -----------
    Model amanitaModel("resources/objects/amanita/amanita_a_low.obj");
    amanitaModel.SetShaderTextureNamePrefix("material.");
    Model ambrelaModel("resources/objects/ambrela/Big_ambrella_low.obj");
    ambrelaModel.SetShaderTextureNamePrefix("material.");
    Model boletusModel("resources/objects/boletus/boletus_low.obj");
    boletusModel.SetShaderTextureNamePrefix("material.");
    Model chantarellModel("resources/objects/chantarelle/chanterelles_low.obj");
    chantarellModel.SetShaderTextureNamePrefix("material.");
    Model morelModel("resources/objects/morel/morel_low.obj");
    morelModel.SetShaderTextureNamePrefix("materal.");
    Model russulaModel("resources/objects/russula/russula_low.obj");
    russulaModel.SetShaderTextureNamePrefix("material.");
    Model rock5Model("resources/objects/Rock_5/Rock_5.obj");
    rock5Model.SetShaderTextureNamePrefix("material.");

    bool normal_mapping = false;
    //create model matrices for amanita
    unsigned int amanitaNum = 8;
    glm::mat4* amanitaModelMatrises;
    amanitaModelMatrises = new glm::mat4[amanitaNum];
    glm::vec3 amanitaTranslations[] = {
            glm::vec3(23.5f, 1.5f, -4.0f),
            glm::vec3(36.5f, 1.5f, -7.5f),
            glm::vec3(30.0f, 1.5f, -15.0f),
            glm::vec3(33.3f, 1.5f, -19.0f),
            glm::vec3(37.5f, 1.5f, -21.5f),
            glm::vec3(19.5f, 1.5f, -7.6f),
            glm::vec3(30.0f, 1.5f, -30.0f),
            glm::vec3(19.5f, 1.5f, -32.5f),

    };

    for(unsigned int i = 0; i < amanitaNum; i++){
        glm::mat4 model = glm::mat4 (1.0f);
        model = glm::translate(model, amanitaTranslations[i]);
        model = glm::scale(model, glm::vec3(3.2f));
        amanitaModelMatrises[i] = model;
    }
    configurate_instance_buffer(amanitaNum, amanitaModel, amanitaModelMatrises, normal_mapping);

    //create model matrices for amabrela
    unsigned int ambrelaNum = 4;
    glm::mat4* ambrelaModelMatrises;
    ambrelaModelMatrises = new glm::mat4[ambrelaNum];
    glm::vec3 ambrelaTranslations[] = {
            glm::vec3(20.f, 1.5f, -10.0f),
            glm::vec3(37.5f, 1.5f, -15.5f),
            glm::vec3(30.0f, 1.5f, -35.0f),
            glm::vec3(40.3f, 1.5f, -35.0f)
    };

    for(unsigned int i = 0; i < ambrelaNum; i++){
        glm::mat4 model = glm::mat4 (1.0f);
        model = glm::translate(model, ambrelaTranslations[i]);
        model = glm::scale(model, glm::vec3(3.2f));
        ambrelaModelMatrises[i] = model;
    }
    configurate_instance_buffer(ambrelaNum, ambrelaModel, ambrelaModelMatrises, normal_mapping);

    //create model matrices for boletus
    unsigned int boletusNum = 4;
    glm::mat4* boletusModelMatrises;
    boletusModelMatrises = new glm::mat4[boletusNum];
    glm::vec3 boletusTranslations[] = {
            glm::vec3(31.5f, 1.5f, -10.0f),
            glm::vec3(35.5f, 1.5f, -30.0f),
            glm::vec3(23.5f, 1.5f, -28.5f),
            glm::vec3(25.3f, 1.5f, -35.0f)
    };

    for(unsigned int i = 0; i < boletusNum; i++){
        glm::mat4 model = glm::mat4 (1.0f);
        model = glm::translate(model, boletusTranslations[i]);
        model = glm::scale(model, glm::vec3(3.2f));
        boletusModelMatrises[i] = model;
    }
    configurate_instance_buffer(boletusNum, boletusModel, boletusModelMatrises, normal_mapping);

    //create model matrices for chantarell
    unsigned int chantarellNum = 5;
    glm::mat4* chantarellModelMatrises;
    chantarellModelMatrises = new glm::mat4[chantarellNum];
    glm::vec3 chantarellTranslations[] = {
            glm::vec3(42.5f, 1.5f, -20.0f),
            glm::vec3(17.5f, 1.5f, -35.0f),
            glm::vec3(32.5f, 1.5f, -26.5f),
            glm::vec3(25.0f, 1.5f, -5.0f),
            glm::vec3(27.5f, 1.5f, -40.5)
    };

    for(unsigned int i = 0; i < chantarellNum; i++){
        glm::mat4 model = glm::mat4 (1.0f);
        model = glm::translate(model, chantarellTranslations[i]);
        model = glm::scale(model, glm::vec3(3.2f));
        chantarellModelMatrises[i] = model;
    }
    configurate_instance_buffer(chantarellNum, chantarellModel, chantarellModelMatrises, normal_mapping);


    //create model matrices for morel
    unsigned int morelNum = 5;
    glm::mat4* morelModelMatrises;
    morelModelMatrises = new glm::mat4[morelNum];
    glm::vec3 morelTranslations[] = {
            glm::vec3(20.0f, 1.5f, -5.0f),
            glm::vec3(42.5f, 1.5f, -17.5f),
            glm::vec3(30.0f, 1.5f, -21.0f),
            glm::vec3(40.0f, 1.5f, -30.0f),
            glm::vec3(23.5f, 1.5f, -37.5)
    };

    for(unsigned int i = 0; i < morelNum; i++){
        glm::mat4 model = glm::mat4 (1.0f);
        model = glm::translate(model, morelTranslations[i]);
        model = glm::scale(model, glm::vec3(3.2f));
        morelModelMatrises[i] = model;
    }
    configurate_instance_buffer(morelNum, morelModel, morelModelMatrises, normal_mapping);

    //create model matrices for russula
    unsigned int russulaNum = 7;
    glm::mat4* russulaModelMatrises;
    russulaModelMatrises = new glm::mat4[russulaNum];
    glm::vec3 russulaTranslations[] = {
            glm::vec3(30.0f, 1.5f, -12.0f),
            glm::vec3(19.5f, 1.5f, -20.5f),
            glm::vec3(15.0f, 1.5f, -28.0f),
            glm::vec3(17.0f, 1.5f, -30.0f),
            glm::vec3(30.5f, 1.5f, -36.5),
            glm::vec3(10.0f, 1.5f, -36.0f),
            glm::vec3(13.5f, 1.5f, -37.5)
    };

    for(unsigned int i = 0; i < russulaNum; i++){
        glm::mat4 model = glm::mat4 (1.0f);
        model = glm::translate(model, russulaTranslations[i]);
        model = glm::scale(model, glm::vec3(3.2f));
        russulaModelMatrises[i] = model;
    }
    configurate_instance_buffer(russulaNum, russulaModel, russulaModelMatrises, normal_mapping);

    //create model matrices for treeA
//    unsigned int treeANum = 5;
//    glm::mat4* treeAModelMatrises;
//    treeAModelMatrises = new glm::mat4[treeANum];
//    glm::vec3 treeATranslations[] = {
//            glm::vec3(40.0f, 1.0f, -10.0f),
//            glm::vec3(35.5f, 1.0f, -6.0f),
//            glm::vec3(20.0f, 1.0f, -8.0f),
//            glm::vec3(15.0f, 1.0f, -7.0f),
//            glm::vec3(15.5f, 1.0f, -30.5)
//    };
//
//    for(unsigned int i = 0; i < treeANum; i++){
//        glm::mat4 model = glm::mat4 (1.0f);
//        model = glm::translate(model, treeATranslations[i]);
//        model = glm::scale(model, glm::vec3(1.2f));
//        treeAModelMatrises[i] = model;
//    }
//    configurate_instance_buffer(treeANum, treeAModel, treeAModelMatrises, normal_mapping);

    /*****/
    //vertexes

    float vertices[] = {
            // positions                 // texture coords
            0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // top right
            0.5f, -0.5f, 0.0f,1.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float skyBoxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    unsigned int skyBoxVBO, skyBoxVAO;
    glGenVertexArrays(1, &skyBoxVAO);
    glGenBuffers(1, &skyBoxVBO);

    glBindVertexArray(skyBoxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyBoxVertices), skyBoxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float ), (void*)0);

    //load and create textures
    Texture2D grassDiffuse("resources/textures/grass_texture.jpg", GL_REPEAT, GL_LINEAR);
    Texture2D grassSpecular("resources/textures/grass_specular.jpg", GL_REPEAT, GL_LINEAR);
    Texture2D bloodSplatter("resources/textures/blood-splatter-png-44474.png", GL_REPEAT, GL_CLAMP_TO_EDGE);
    vector<std::string> faces
            {
                    "resources/textures/Apocalypse/vz_apocalypse_right.png",
                    "resources/textures/Apocalypse/vz_apocalypse_left.png",
                    "resources/textures/Apocalypse/vz_apocalypse_up.png",
                    "resources/textures/Apocalypse/vz_apocalypse_down.png",
                    "resources/textures/Apocalypse/vz_apocalypse_front.png",
                    "resources/textures/Apocalypse/vz_apocalypse_back.png"
            };
    unsigned int cubemapTexture = loadCubemap(faces);

    /*****/


    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.2, 0.2, 0.2);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.002f;
    pointLight.quadratic = 0.000032f;



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    skyBoxShader.use();
    skyBoxShader.setInt("skybox", 0);
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms

        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();

        platoShader.use();
        platoShader.setMat4("projection", projection);
        platoShader.setMat4("view", view);
        platoShader.setVec3("viewPos", programState->camera.Position);

        // light properties
        platoShader.setVec3("l.position", pointLight.position);
        platoShader.setVec3("l.ambient", 0.2f, 0.2f, 0.2f);
        platoShader.setVec3("l.diffuse", 0.5, 0.5, 0.5);
        platoShader.setVec3("l.specular", 1.0f, 1.0f, 1.0f);
        platoShader.setFloat("l.constant", pointLight.constant);
        platoShader.setFloat("l.linear", pointLight.linear);
        platoShader.setFloat("l.quadratic", pointLight.quadratic);

        // material properties
        platoShader.setInt("material.diffuse", 0);
        platoShader.setInt("material.specular", 1);
        platoShader.setFloat("material.shininess", 64.0f);


        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //draw plato
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        grassDiffuse.bind();
        glActiveTexture(GL_TEXTURE1);
        grassSpecular.bind();
        for(int i = 0; i < 50; i++) {
            for (int j = 0; j < 50; j++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i, 1.0, -j));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                platoShader.setMat4("model", model);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        //draw blood
        glActiveTexture(GL_TEXTURE0);
        bloodSplatter.bind();
        for(int i = 0; i < 50; i++){
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(i, 1.1, -i));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            platoShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        instanceShader.use();
        instanceShader.setVec3("l.position", pointLight.position);
        instanceShader.setVec3("l.ambient", 0.2f, 0.2f, 0.2f);
        instanceShader.setVec3("l.diffuse", 0.5, 0.5, 0.5);
        instanceShader.setVec3("l.specular", 1.0f, 1.0f, 1.0f);
        instanceShader.setFloat("l.constant", pointLight.constant);
        instanceShader.setFloat("l.linear", pointLight.linear);
        instanceShader.setFloat("l.quadratic", pointLight.quadratic);
        instanceShader.setMat4("view", view);
        instanceShader.setMat4("projection", projection);
        instanceShader.setInt("material.texture_diffuse", 0);
        instanceShader.setInt("material.texture_specular", 1);
        instanceShader.setInt("material.texture_normal", 2);

        //amanita
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, amanitaModel.textures_loaded[0].id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, amanitaModel.textures_loaded[1].id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, amanitaModel.textures_loaded[2].id);
        for (unsigned int i = 0; i < amanitaModel.meshes.size(); i++)
        {
            glBindVertexArray(amanitaModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, amanitaModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amanitaNum);
            glBindVertexArray(0);
        }

        //ambrela
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ambrelaModel.textures_loaded[0].id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ambrelaModel.textures_loaded[1].id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ambrelaModel.textures_loaded[2].id);
        for (unsigned int i = 0; i < ambrelaModel.meshes.size(); i++)
        {
            glBindVertexArray(ambrelaModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, ambrelaModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, ambrelaNum);
            glBindVertexArray(0);
        }

        //boletus
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, boletusModel.textures_loaded[0].id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, boletusModel.textures_loaded[1].id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, boletusModel.textures_loaded[2].id);
        for (unsigned int i = 0; i < boletusModel.meshes.size(); i++)
        {
            glBindVertexArray(boletusModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, boletusModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, boletusNum);
            glBindVertexArray(0);
        }
        //chantarell
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, chantarellModel.textures_loaded[0].id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, chantarellModel.textures_loaded[1].id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, chantarellModel.textures_loaded[2].id);
        for (unsigned int i = 0; i < chantarellModel.meshes.size(); i++)
        {
            glBindVertexArray(chantarellModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, chantarellModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, chantarellNum);
            glBindVertexArray(0);
        }
        //morel
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, morelModel.textures_loaded[0].id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, morelModel.textures_loaded[1].id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, morelModel.textures_loaded[2].id);
        for (unsigned int i = 0; i < morelModel.meshes.size(); i++)
        {
            glBindVertexArray(morelModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, morelModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, morelNum);
            glBindVertexArray(0);
        }
        //russula
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, russulaModel.textures_loaded[0].id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, russulaModel.textures_loaded[1].id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, russulaModel.textures_loaded[2].id);
        for (unsigned int i = 0; i < russulaModel.meshes.size(); i++)
        {
            glBindVertexArray(russulaModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, russulaModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, russulaNum);
            glBindVertexArray(0);
        }

        normalShader.use();
        normalShader.setVec3("viewPos", programState->camera.Position);
        normalShader.setFloat("material.shininess", 32.0f);
        normalShader.setVec3("light.direction", pointLight.position);
        normalShader.setVec3("light.ambient", pointLight.ambient);
        normalShader.setVec3("light.diffuse", pointLight.diffuse);
        normalShader.setVec3("light.specular", pointLight.specular);
        normalShader.setFloat("light.constant", pointLight.constant);
        normalShader.setFloat("light.linear", pointLight.linear);
        normalShader.setFloat("light.quadratic", pointLight.quadratic);
        rock5Model.Draw(normalShader);
        //draw sky box
        glDepthFunc(GL_LEQUAL);
        skyBoxShader.use();
        view = glm::mat4(glm::mat3(view)); //removing the translation part
        skyBoxShader.setMat4("view", view);
        skyBoxShader.setMat4("projection", projection);

        glBindVertexArray(skyBoxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &skyBoxVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &skyBoxVAO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        programState->CameraMouseMovementUpdateEnabled = !programState->CameraMouseMovementUpdateEnabled;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->backpackPosition);
        ImGui::DragFloat("Backpack scale", &programState->backpackScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadCubemap(vector<std::string> faces){
    unsigned int t_id;
    glGenTextures(1, &t_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, t_id);

    //load images
    int width, height, nChannels;
    for(unsigned int i = 0; i < faces.size(); i++){
        unsigned char *data = stbi_load(FileSystem::getPath(faces[i]).c_str(),
                                        &width, &height, &nChannels, 0);
        if(data){
            std::cout << "Tekstura za sky box je ucitana\n";
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0 , GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }else{
            ASSERT(false, "Tekstura za sky box nije mogla da se ucita");
        }
        stbi_image_free(data);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    return t_id;
}

void configurate_instance_buffer(unsigned int num, Model model, glm::mat4 *modelMatrices, bool normal_mapping){
    //configurate instance array
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, num*sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for (unsigned int i = 0; i < model.meshes.size(); i++)
    {
        unsigned int VAO = model.meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        if(normal_mapping == false) {
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) 0);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (sizeof(glm::vec4)));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);
        }else{
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
            glEnableVertexAttribArray(7);
            glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(8);
            glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);
            glVertexAttribDivisor(7, 1);
            glVertexAttribDivisor(8, 1);
        }

        glBindVertexArray(0);
    }
}


