#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<glm/gtx/intersect.hpp>

#include <shader.h>
#include <camera.h>
#include <model.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
/*
glm::vec4 ray_trace(Model model, glm::vec3 origin, glm::vec3 ray){
     //std::cout << "x:" <<ray.x << " y:"<< ray.y <<" z:" << ray.z <<" "; 
     
     for(auto& mesh : ourModel.meshes){
        auto& indices = mesh.indices;
        auto& vertices = mesh.vertices;
        // triangles loop
        for(int i = 0; i<indices.size(); i+=3){
            glm::vec3 A = vertices[indices[i]].Position;
            glm::vec3 B = vertices[indices[i+1]].Position;
            glm::vec3 C = vertices[indices[i+2]].Position;

            glm::vec3 cross;
            if(glm::intersectRayTriangle(origin, ray, A, B, C, cross)){
                std::cout << "X";
                return {1., 1., 1., 0.};
            }
        }
    }
    std::cout << ".";
    return {0., 0., 0., 0.};
}
*/
int main(int argc, char *argv[]){
    viewerInit();

    Shader shader("shader/simple_vs.glsl", "shader/simple_fs.glsl");
    Model ourModel("data/nanosuit.obj");
    
    // glm::vec4 rays[SCR_WIDTH][SCR_HEIGHT];
    // camera.Position.z = 3.;
    camera.Position.z = 8.;
    camera.Position.y = 6.5;

    glm::vec3 left_upper;
    float screen_size = 100.;
    float dx = 2. * screen_size / (float) SCR_WIDTH;
    float dy = dx * (float) SCR_HEIGHT / (float) (SCR_WIDTH);

    left_upper.x = camera.Position.x - screen_size + 0.5 * dx;
    left_upper.y = camera.Position.y + screen_size - 0.5 * dy;
    // left_upper.z = camera.Position.z - 1;
    left_upper.z = camera.Position.z - screen_size ;

    // camera.Position.z = 0.5;


    glm::vec3 origin = camera.Position;



    glm::vec3 cross;
    glm::vec3 current = left_upper;
    for(int y = 0; y < SCR_HEIGHT; y++){
        current.x = left_upper.x;  
        for(int x = 0; x < SCR_WIDTH; x++){
            bool b = true;
            for(auto& mesh : ourModel.meshes){
                 auto& indices = mesh.indices;
                 auto& vertices = mesh.vertices;
        // triangles loop
    //  std::cout << "x:" <<current.x << " y:"<< current.y <<" z:" << current.z <<" "; 
            for(int i = 0; i<indices.size() && b; i+=3){
                auto&  A = vertices[indices[i]].Position;
                auto&  B = vertices[indices[i+1]].Position;
                auto&  C = vertices[indices[i+2]].Position;

                glm::vec3 cross;
                
            if( glm::intersectRayTriangle(origin, current, A, B, C, cross)){
                std::cout << "X";
                b = 0;
            // }else{
                // std::cout << "A{x:" << A.x << ",y:" << A.y <<",z:" << A.z << "} ";
                // std::cout << "B{x:" << B.x << ",y:" << B.y <<",z:" << B.z << "} ";
                // std::cout << "C{x:" << C.x << ",y:" << C.y <<",z:" << C.z << "} ";
                // std::cout << "c{x:" << cross.x << ",y:" << cross.y <<",z:" << cross.z << "}\n";
            }else{
                // std::cout << "current{x:" << current.x << ",y:" << current.y <<",z:" << current.z << "}\n";
            }
            
            //){
                // std::cout << "X";
            //    b= false;
           // }
        }
    }
    if(b){
    std::cout << " ";
      }  
      
      current.x += dx;
    }
        std::cout <<"\n";
        current.y -= dy; 
    }
   

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model, view, projection;
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, camera.NearPlane, 100.0f);
		//model = glm::rotate(model, rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.2f));



        shader.use();
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		ourModel.Draw(shader);
		

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // glDeleteVertexArrays(vertices.size(), VAO);
    // glDeleteBuffers(vertices.size(), VBO);
    // glDeleteBuffers(10, EBO);
    glfwTerminate();
    return 0;

}


