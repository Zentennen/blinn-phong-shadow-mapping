// main.cc

#include "main.hpp"

namespace avocado {
   // note: application create implementation
   application *application::create(settings &settings)
   {
      settings.title_      = "renderapp";
      settings.width_      = 1280;
      settings.height_     = 720;
      settings.center_     = true;

      return new renderapp;
   }

   // note: renderapp class
   renderapp::renderapp()
       : camController(cam)
       , cameraMatrix(1.0f)
   {
       string lightData = "ERROR";
       file_system::read_file_content("../light.txt", lightData);
       {// direction
           size_t nl = lightData.find_first_of('\n');
           string str = lightData.substr(0, nl);
           lightData = lightData.substr(nl + 1);
           lightDirection.x = readNextFloat(str);
           lightDirection.y = readNextFloat(str);
           lightDirection.z = readNextFloat(str);
           lightDirection = glm::normalize(lightDirection);
       }
       {// ambient
           size_t nl = lightData.find_first_of('\n');
           string str = lightData.substr(0, nl);
           lightData = lightData.substr(nl + 1);
           light.ambient.r = readNextFloat(str);
           light.ambient.g = readNextFloat(str);
           light.ambient.b = readNextFloat(str);
           light.ambient.a = 1.0f;
       } 
       {// diffuse
           size_t nl = lightData.find_first_of('\n');
           string str = lightData.substr(0, nl);
           lightData = lightData.substr(nl + 1);
           light.diffuse.r = readNextFloat(str);
           light.diffuse.g = readNextFloat(str);
           light.diffuse.b = readNextFloat(str);
           light.ambient.a = 1.0f;
       }
       {// specular
           size_t nl = lightData.find_first_of('\n');
           string str = lightData.substr(0, nl);
           lightData = lightData.substr(nl + 1);
           light.specular.r = readNextFloat(str);
           light.specular.g = readNextFloat(str);
           light.specular.b = readNextFloat(str);
           light.ambient.a = 1.0f;
       }
   }

   bool renderapp::on_init()
   {
      if (!rend.initialize()) {
         return false;
      }
      loadShader(blinnPhongShader, "assets/blinnPhong.vs.txt", "assets/blinnPhong.fs.txt");
      loadShader(shadowShader, "assets/shadow.vs.txt", "assets/shadow.fs.txt");
      loadShader(lightShader, "assets/light.vs.txt", "assets/light.fs.txt");

      cam.set_projection(glm::perspective(pi * 0.25f, 16.0f / 9.0f, 1.0f, 300.0f));
      cam.set_position(glm::vec3(0.0f, 5.0f, 0.0f));
      camController.set_camera_speed(cameraSpeed);
      camController.set_mouse_sensitivity(mouseSensitivity);
      camController.set_mouse_invert_yaw(true);
      camController.set_mouse_invert_pitch(true);

      lightProjection = glm::ortho(-50.0f, 50.0f, -20.0f, 80.0f, 0.0f, 100.0f);
      lightView = glm::lookAt(-lightDirection * 50.0f, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
      const framebuffer_format format[] = { FRAMEBUFFER_FORMAT_D32 };
      if (!shadowFrameBuffer.create(2048, 2048, 1, format)) {
          return on_error("Could not create frame buffer");
      }

      createTexture(crateMat.tex, "assets/crate.png");
      crateMat.shiny = 128.0f;
      crateMat.lighting = {
          { 0.4f, 0.4f, 0.4f, 1.0f },
          { 0.5f, 0.5f, 0.5f, 1.0f },
          { 0.2f, 0.2f, 0.2f, 1.0f }
      };

      createTexture(wallMat.tex, "assets/stone.jpg");
      wallMat.shiny = 128.0f;
      wallMat.lighting = {
          { 0.8f, 0.8f, 0.8f, 1.0f },
          { 0.6f, 0.6f, 0.6f, 1.0f },
          { 0.1f, 0.1f, 0.1f, 1.0f }
      };

      createTexture(metalMat.tex, "assets/metal.jpg");
      metalMat.shiny = 1024.0f;
      metalMat.lighting = {
          { 0.5f, 0.5f, 0.5f, 1.0f },
          { 0.4f, 0.4f, 0.4f, 1.0 },
          { 0.7f, 0.7f, 0.7f, 1.0f }
      };

      createTexture(earthMat.tex, "assets/earth.png");
      earthMat.shiny = 256.0f;
      earthMat.lighting = {
          { 0.7f, 0.7f, 0.7f, 1.0f },
          { 0.5f, 0.5f, 0.5f, 1.0f },
          { 0.5f, 0.5f, 0.5f, 1.0f }
      };

      createTexture(groundMat.tex, "assets/grass.jpg");
      groundMat.shiny = 64.0f;
      groundMat.lighting = {
          { 0.6f, 0.6f, 0.6f, 1.0f },
          { 1.0f, 1.0f, 1.0f, 1.0f },
          { 0.15f, 0.15f, 0.15f, 1.0f }
      };

      mainLayout.add_attribute(0, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 3, false);
      mainLayout.add_attribute(1, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 3, false);
      mainLayout.add_attribute(2, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 2, false);

      const auto bgScale = glm::scale(identity, glm::vec3(50.0f, 1.0f, 50.0f));
      ground =      glm::translate(identity, glm::vec3(0.0f, 0.0f, 0.0f) + center) * bgScale;
      wallBack =    glm::translate(identity, glm::vec3(0.0f, 50.0f, -50.0f) + center) *  glm::rotate(identity, pi / 2.0f, xAxis) *   bgScale;
      

      if (!shadowSampler.create(SAMPLER_FILTER_MODE_NEAREST, SAMPLER_ADDRESS_MODE_CLAMP, SAMPLER_ADDRESS_MODE_CLAMP)) {
          return on_error("could not create sampler state!");
      }

      if (!mainSampler.create(SAMPLER_FILTER_MODE_LINEAR, SAMPLER_ADDRESS_MODE_CLAMP, SAMPLER_ADDRESS_MODE_CLAMP)) {
          return on_error("could not create sampler state!");
      }

      glm::vec3 frontTopLeft =  { -1.0f, 1.0f,  1.0f,  };
      glm::vec3 frontTopRight = { 1.0f,  1.0f,  1.0f,  };
      glm::vec3 frontBotLeft =  { -1.0f, -1.0f, 1.0f,  };
      glm::vec3 frontBotRight = { 1.0f,  -1.0f, 1.0f,  };
      glm::vec3 backTopLeft =   { -1.0f, 1.0f,  -1.0f, };
      glm::vec3 backTopRight =  { 1.0f,  1.0f,  -1.0f, };
      glm::vec3 backBotLeft =   { -1.0f, -1.0f, -1.0f, };
      glm::vec3 backBotRight =  { 1.0f,  -1.0f, -1.0f, };

      glm::vec3 normalBack =    { 0.0f,  0.0f,  1.0f };
      glm::vec3 normalForward = { 0.0f,  0.0f,  -1.0f };
      glm::vec3 normalLeft =    { -1.0f, 0.0f,  0.0f };
      glm::vec3 normalRight =   { 1.0f,  0.0f,  0.0f };
      glm::vec3 normalUp =      { 0.0f,  1.0f,  0.0f };
      glm::vec3 normalDown =    { 0.0f,  -1.0f, 0.0f };

      glm::vec2 texTopLeft =  { 0.0f, 0.0f };
      glm::vec2 texTopRight = { 1.0f, 0.0f };
      glm::vec2 texBotLeft =  { 0.0f, 1.0f };
      glm::vec2 texBotRight = { 1.0f, 1.0f };

      {// light quad
          const struct { glm::vec3 position; glm::vec2 texcoord; } vertices[] = {
              { backTopLeft, texTopLeft },
              { backBotRight, texBotRight },
              { backTopRight, texTopRight },
              { backBotLeft, texBotLeft },
              { backBotRight, texBotRight },
              { backTopLeft, texTopLeft }
          };
          lightVertices.create(BUFFER_ACCESS_MODE_STATIC, sizeof(vertices), vertices);
          lightLayout.add_attribute(0, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 3, false);
          lightLayout.add_attribute(1, vertex_layout::ATTRIBUTE_FORMAT_FLOAT, 2, false);
      }

      {// box
          const vertex vertices[] =
          {
              //front tri 1
              { frontBotLeft,  normalBack, texBotLeft,  },
              { frontBotRight, normalBack, texBotRight, },
              { frontTopLeft,  normalBack, texTopLeft,  },
              //front tri 2   
              { frontTopRight, normalBack, texTopRight, },
              { frontTopLeft,  normalBack, texTopLeft,  },
              { frontBotRight, normalBack, texBotRight, },
              //back tri 1  
              { backTopLeft,   normalForward, texTopLeft,  },
              { backBotRight,  normalForward, texBotRight, },
              { backBotLeft,   normalForward, texBotLeft,  },
              //back tri 2  
              { backTopRight,  normalForward, texTopRight, },
              { backBotRight,  normalForward, texBotRight, },
              { backTopLeft,   normalForward, texTopLeft,  },
              //left tri 1  
              { frontTopLeft,  normalLeft, texTopRight, },
              { backBotLeft,   normalLeft, texBotLeft,  },
              { frontBotLeft,  normalLeft, texBotRight, },
              //left tri 2    
              { frontTopLeft,  normalLeft, texTopRight, },
              { backTopLeft,   normalLeft, texTopLeft,  },
              { backBotLeft,   normalLeft, texBotLeft,  },
              //right tri 1  
              { frontTopRight, normalRight, texTopLeft,  },
              { frontBotRight, normalRight, texBotLeft,  },
              { backBotRight,  normalRight, texBotRight, },
              //right tri 2   
              { frontTopRight, normalRight, texTopLeft,  },
              { backBotRight,  normalRight, texBotRight, },
              { backTopRight,  normalRight, texTopRight, },
              //top tri 1    
              { backTopRight,  normalUp, texTopRight, },
              { backTopLeft,   normalUp, texTopLeft,  },
              { frontTopLeft,  normalUp, texBotLeft,  },
              //top tri 2     
              { frontTopRight, normalUp, texBotRight, },
              { backTopRight,  normalUp, texTopRight, },
              { frontTopLeft,  normalUp, texBotLeft,  },
              //bottom tri 1  
              { frontBotLeft,  normalDown, texTopLeft,  },
              { backBotLeft,   normalDown, texBotLeft,  },
              { frontBotRight, normalDown, texTopRight, },
              //bottom tri 2  
              { backBotRight,  normalDown, texBotRight, },
              { frontBotRight, normalDown, texTopRight, },
              { backBotLeft,   normalDown, texBotLeft,  },
          };

          if (!boxVertices.create(BUFFER_ACCESS_MODE_STATIC, sizeof(vertices), vertices))
          {
              return on_error("Could not create vertex buffer!");
          }
          boxVertexCount = sizeof(vertices) / sizeof(vertex);
      }

      {// ground
          const vertex vertices[] =
          {
              //tri 1
              { backTopRight,  normalUp, texTopRight},
              { backTopLeft,   normalUp, texTopLeft},
              { frontTopLeft,  normalUp, texBotLeft},
              //tri 2
              { frontTopRight, normalUp, texBotRight},
              { backTopRight,  normalUp, texTopRight},
              { frontTopLeft,  normalUp, texBotLeft},
          };

          if (!bgVertices.create(BUFFER_ACCESS_MODE_STATIC, sizeof(vertices), vertices))
          {
              return on_error("Could not create vertex buffer!");
          }
          bgVertexCount = sizeof(vertices) / sizeof(vertex);
      }

      return true;
   }

   void renderapp::on_exit()
   {
   }

   bool renderapp::on_tick(const time &deltatime)
   {
      if (keyboard_.key_pressed(keyboard::key::escape)) {
         return false;
      }
      if (keyboard_.key_pressed(keyboard::key::leftshift)) {
          camController.set_camera_speed(cameraSpeed * cameraSpeedMult);
      }
      if (keyboard_.key_released(keyboard::key::leftshift)) {
          camController.set_camera_speed(cameraSpeed);
      }
      camController.update(keyboard_, mouse_, deltatime);
      
      {// crate
          const auto r = glm::rotate(identity, time::now().as_seconds(), glm::vec3(0.0f, 1.0f, 0.0f));
          const auto t = glm::translate(identity, glm::vec3(0.0f, 10.0f + 2.0f * sinf(time::now().as_seconds()), -20.0f));
          const auto s = glm::scale(identity, glm::vec3(5.0f));
          crate = centerMatrix * t * r * s;
      }
      {// metal
          metalBox = glm::translate(identity, glm::vec3(20.0f, 0.0f, 0.0f)) * crate;
      }
      {// earth
          earth = glm::translate(identity, glm::vec3(-20.0f, 0.0f, 0.0f)) * crate;
      }
      
      return true;
   }

   void renderapp::on_draw()
   {
      rend.set_sampler_state(mainSampler, 0);
      rend.set_sampler_state(shadowSampler, 1);
      rend.set_depth_state(true, true);
      
      // shadow pass
      rend.set_rasterizer_state(CULL_MODE_FRONT);
      rend.set_framebuffer(shadowFrameBuffer);
      rend.clear(0.0f, 0.0f, 0.0f, 1.0f);
      rend.set_shader_program(shadowShader);
      rend.set_shader_uniform(shadowShader, UNIFORM_TYPE_MATRIX, "u_lightView", 1, glm::value_ptr(lightView));
      rend.set_shader_uniform(shadowShader, UNIFORM_TYPE_MATRIX, "u_lightProjection", 1, glm::value_ptr(lightProjection));
      rend.set_vertex_buffer(boxVertices);
      rend.set_vertex_layout(mainLayout);
      drawObjects({ crate, metalBox, earth }, boxVertexCount);

      //render pass
      rend.reset_framebuffer();
      rend.set_viewport(0, 0, 1280, 720);
      rend.clear(0.3f, 0.3f, 0.3f, 1.0f);
      rend.set_rasterizer_state(CULL_MODE_BACK);
      rend.set_shader_program(blinnPhongShader);
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_VEC3, "u_viewPosition", 1, glm::value_ptr(cam.position_));
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_VEC4, "u_lightAmbient", 1, glm::value_ptr(light.ambient));
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_VEC4, "u_lightDiffuse", 1, glm::value_ptr(light.diffuse));
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_VEC4, "u_lightSpecular", 1, glm::value_ptr(light.specular));
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_MATRIX, "u_view", 1, glm::value_ptr(cam.view_));
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_MATRIX, "u_projection", 1, glm::value_ptr(cam.projection_));
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_MATRIX, "u_lightView", 1, glm::value_ptr(lightView));
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_MATRIX, "u_lightProjection", 1, glm::value_ptr(lightProjection));
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_VEC3, "u_lightDirection", 1, glm::value_ptr(-lightDirection));
      uint32 texIndex0 = 0;
      uint32 texIndex1 = 1;
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_SAMPLER, "u_diffuse", 1, &texIndex0);
      rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_SAMPLER, "u_shadow", 1, &texIndex1);
      rend.set_texture(shadowFrameBuffer.color_attachment_as_texture(0), 1);
      cameraMatrix = cam.projection_ * cam.view_;

      // background
      rend.set_vertex_buffer(bgVertices);
      rend.set_vertex_layout(mainLayout);
      {// ground
          dynamic_array<glm::mat4> matrices = { ground };
          drawObjects(matrices, bgVertexCount, groundMat);
      }

      {// walls & ceiling
          dynamic_array<glm::mat4> matrices = { wallBack };
          drawObjects(matrices, bgVertexCount, wallMat);
      }

      // boxes
      rend.set_vertex_buffer(boxVertices);
      rend.set_vertex_layout(mainLayout);
      {// crate
          dynamic_array<glm::mat4> matrices = { crate };
          drawObjects(matrices, boxVertexCount, crateMat);
      }
      {// metal
          dynamic_array<glm::mat4> matrices = { metalBox };
          drawObjects(matrices, boxVertexCount, metalMat);
      }
      {// earth
          dynamic_array<glm::mat4> matrices = { earth };
          drawObjects(matrices, boxVertexCount, earthMat);
      }

      //render light
      rend.set_viewport(0, 720 - 256, 256, 256);
      rend.set_shader_program(lightShader);
      rend.set_texture(shadowFrameBuffer.color_attachment_as_texture(0));
      rend.set_vertex_buffer(lightVertices);
      rend.set_vertex_layout(lightLayout);
      rend.draw(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, 6);
   }

   void renderapp::drawObjects(const dynamic_array<glm::mat4>& matrices, const uint32 vertexCount, const Material& material)
   {
       rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_VEC4, "u_matAmbient", 1, glm::value_ptr(material.lighting.ambient));
       rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_VEC4, "u_matDiffuse", 1, glm::value_ptr(material.lighting.diffuse));
       rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_VEC4, "u_matSpecular", 1, glm::value_ptr(material.lighting.specular));
       rend.set_shader_uniform(blinnPhongShader, UNIFORM_TYPE_FLOAT, "u_shiny", 1, &material.shiny);
       rend.set_texture(material.tex, 0);

       for (uint32 i = 0; i < matrices.size(); i++) {
           rend.set_shader_uniform(blinnPhongShader, uniform_type::UNIFORM_TYPE_MATRIX, "u_objectMatrix", 1, glm::value_ptr(matrices[i]));
           rend.draw(primitive_topology::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, vertexCount);
       }
   }

   bool renderapp::createTexture(texture& tex, const string& filePath)
   {
       bitmap image;
       if (!image.create(filePath.c_str())) {
           return on_error("could not load bitmap image!");
       }

       texture_format format = texture::from_bitmap_format(image.pixel_format());
       int32 width = image.width();
       int32 height = image.height();
       const uint8* data = image.data();
       if (!tex.create(format, width, height, data)) {
           return on_error("could not create texture!");
       }

       image.destroy();
       return true;
   }

   bool renderapp::loadShader(shader_program& shader, const string& vertexShaderPath, const string& fragmentShaderPath)
   {
       string vertex_source;
       if (!file_system::read_file_content(vertexShaderPath, vertex_source)) {
           return on_error("Could not load vertex source");
       }
       string fragment_source;
       if (!file_system::read_file_content(fragmentShaderPath, fragment_source)) {
           return on_error("Could not load fragment source");
       }
       if (!shader.create(vertex_source.c_str(), fragment_source.c_str()))
       {
           return on_error("Could not create shader program!");
       }
       return true;
   }


   void renderapp::drawObjects(const dynamic_array<glm::mat4>& matrices, const uint32 vertexCount)
   {
       for (uint32 i = 0; i < matrices.size(); i++) {
           rend.set_shader_uniform(shadowShader, uniform_type::UNIFORM_TYPE_MATRIX, "u_objectMatrix", 1, glm::value_ptr(matrices[i]));
           rend.draw(primitive_topology::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, vertexCount);
       }
   }

   float readNextFloat(string& str)
   {
       size_t start = str.find_first_not_of(' ');
       size_t end = str.find_first_of(' ', start);
       string fl = str.substr(start, end - start);
       float ret = std::stof(fl);
       str = str.substr(end + 1);
       return ret;
   }
} // !avocado
