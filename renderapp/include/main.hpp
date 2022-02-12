// main.hpp

#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4201)
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)
#include "camera.hpp"
#include <avocado.hpp>
#include <avocado_render.hpp>

namespace avocado {
   const glm::mat4 identity(1.0f);
   const glm::vec3 center(0.0f, 0.0f, 0.0f);
   const glm::mat4 centerMatrix = glm::translate(identity, center);
   const float cameraSpeed = 15.0f;
   const float cameraSpeedMult = 3.0f;
   const float mouseSensitivity = 0.18f;
   const float pi = 3.141592f;
   const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
   const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
   const glm::vec3 zAxis(0.0f, 0.0f, 1.0f);

   struct vertex {
       glm::vec3 position;
       glm::vec3 normal;
       glm::vec2 texcoord;
   };

   struct Lighting {
       glm::vec4 ambient;
       glm::vec4 diffuse;
       glm::vec4 specular;
       Lighting() 
           : ambient(0.0f)
           , diffuse(0.0f)
           , specular(0.0f)
       {

       }
       Lighting(const glm::vec4& a, const glm::vec4& d, const glm::vec4& s) 
           : ambient(a)
           , diffuse(d)
           , specular(s)
       {

       }
   };

   struct Material {
       Lighting lighting;
       float shiny = 32.0f;
       texture tex;
   };

   struct renderapp final : application {
      renderapp();

      virtual bool on_init();
      virtual void on_exit();
      virtual bool on_tick(const time &deltatime);
      virtual void on_draw();

      bool createTexture(texture& tex, const string& filePath);
      bool loadShader(shader_program& shader, const string& vertexShaderPath, const string& fragmentShaderPath);
      void drawObjects(const dynamic_array<glm::mat4>& matrices, const uint32 vertexCount, const Material& material);
      void drawObjects(const dynamic_array<glm::mat4>& matrices, const uint32 vertexCount);
      void drawAllObjects();

      renderer rend;
      camera cam;
      controller camController;
      shader_program blinnPhongShader;
      vertex_layout mainLayout;
      sampler_state shadowSampler;
      sampler_state mainSampler;
      glm::mat4 cameraMatrix;

      glm::mat4 lightView = identity;
      glm::mat4 lightProjection = identity;
      framebuffer shadowFrameBuffer;
      shader_program shadowShader;

      vertex_buffer bgVertices;
      vertex_buffer boxVertices;
      uint32 bgVertexCount = 0;
      uint32 boxVertexCount = 0;

      Lighting light = {};
      glm::vec3 lightDirection = {};

      glm::mat4 crate = identity;
      glm::mat4 metalBox = identity;
      glm::mat4 earth = identity;
      glm::mat4 ground = identity;
      glm::mat4 wallBack = identity;

      Material groundMat;
      Material wallMat;
      Material crateMat;
      Material metalMat;
      Material earthMat;

      vertex_layout lightLayout;
      vertex_buffer lightVertices;
      shader_program lightShader;
   };

   float readNextFloat(string& str);
} // !avocado
