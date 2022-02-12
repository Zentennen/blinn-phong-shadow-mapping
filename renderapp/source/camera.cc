// camera.cc

#include "main.hpp"
#include "camera.hpp"

namespace avocado {
   camera::camera()
      : pitch_(0.0f)
      , yaw_(0.0f)
      , position_(0.0f, 0.0f, 0.0f)
      , x_axis_(1.0f, 0.0f, 0.0f)
      , y_axis_(0.0f, 1.0f, 0.0f)
      , z_axis_(0.0f, 0.0f, 1.0f)
      , projection_(1.0f)
      , view_(1.0f)
   {
   }

   void camera::update()
   {
      glm::vec3 ax(1.0f, 0.0f, 0.0f);
      glm::vec3 ay(0.0f, 1.0f, 0.0f);
      glm::vec3 az(0.0f, 0.0f, 1.0f);

      glm::mat4 ry = glm::rotate(glm::mat4(1.0f), yaw_, ay);
      ax = glm::normalize(glm::mat3(ry) * ax);
      az = glm::normalize(glm::mat3(ry) * az);

      glm::mat4 rx = glm::rotate(glm::mat4(1.0f), pitch_, ax);
      ay = glm::normalize(glm::mat3(rx) * ay);
      az = glm::normalize(glm::mat3(rx) * az);

      view_[0][0] = ax.x;    view_[0][1] = ay.x;    view_[0][2] = az.x;
      view_[1][0] = ax.y;    view_[1][1] = ay.y;    view_[1][2] = az.y;
      view_[2][0] = ax.z;    view_[2][1] = ay.z;    view_[2][2] = az.z;
      view_[3][0] = -glm::dot(position_, ax);
      view_[3][1] = -glm::dot(position_, ay);
      view_[3][2] = -glm::dot(position_, az);

      x_axis_ = ax;
      y_axis_ = ay;
      z_axis_ = az;
   }

   void camera::set_projection(const glm::mat4 &projection)
   {
      projection_ = projection;
   }

   void camera::set_position(const glm::vec3 &position)
   {
      position_ = position;
   }

   void camera::move_x(const float amount)
   {
      position_ += x_axis_ * amount;
   }

   void camera::move_y(const float amount)
   {
      position_ += y_axis_ * amount;
   }

   void camera::move_z(const float amount)
   {
      position_ += z_axis_ * amount;
   }

   void camera::rotate_x(const float amount)
   {
      pitch_ += amount;
   }

   void camera::rotate_y(const float amount)
   {
      yaw_ += amount;
   }

   controller::controller(camera &camera)
      : camera_(camera)
      , camera_speed_(10.0f)
      , mouse_yaw_(0.022f)
      , mouse_pitch_(-0.022f)
      , mouse_sensitivity_(0.02f)
   {
   }

   void controller::update(const keyboard &kb, const mouse &m, const time &deltatime)
   {
      // note: keyboard
      if (kb.key_down(keyboard::key::w)) {
         camera_.move_z(-camera_speed_ * deltatime.as_seconds());
      }
      if (kb.key_down(keyboard::key::s)) {
         camera_.move_z(camera_speed_ * deltatime.as_seconds());
      }
      if (kb.key_down(keyboard::key::a)) {
         camera_.move_x(-camera_speed_ * deltatime.as_seconds());
      }
      if (kb.key_down(keyboard::key::d)) {
         camera_.move_x(camera_speed_ * deltatime.as_seconds());
      }
      if (kb.key_down(keyboard::key::q)) {
         camera_.move_y(-camera_speed_ * deltatime.as_seconds());
      }
      if (kb.key_down(keyboard::key::e)) {
         camera_.move_y(camera_speed_ * deltatime.as_seconds());
      }

      // note: mouse
      const glm::vec2 mouse_position(float(m.position().x_), float(m.position().y_));
      const glm::vec2 mouse_delta = mouse_position - previous_mouse_position_;
      if (m.button_down(mouse::button::left)) {
         camera_.rotate_x(-mouse_delta.y * mouse_sensitivity_ * mouse_pitch_);
         camera_.rotate_y(mouse_delta.x * mouse_sensitivity_ * mouse_yaw_);
      }

      previous_mouse_position_ = mouse_position;
      //if (camera_.position_.x > 45.0f)  camera_.position_.x = 45.0f;
      //if (camera_.position_.x < -45.0f) camera_.position_.x = -45.0f;
      //if (camera_.position_.y > 45.0f)  camera_.position_.y = 45.0f;
      //if (camera_.position_.y < 3.0f)   camera_.position_.y = 3.0f;
      //if (camera_.position_.z > 45.0f)  camera_.position_.z = 45.0f;
      //if (camera_.position_.z < -45.0f) camera_.position_.z = -45.0f;
      camera_.update();
   }

   void controller::set_camera_speed(const float camera_speed)
   {
      camera_speed_ = camera_speed;
   }

   void controller::set_mouse_invert_yaw(const bool invert)
   {
      mouse_yaw_ = invert ? -mouse_yaw_ : mouse_yaw_;
   }

   void controller::set_mouse_invert_pitch(const bool invert)
   {
      mouse_pitch_ = invert ? -mouse_pitch_ : mouse_pitch_;
   }

   void controller::set_mouse_sensitivity(const float mouse_sensitivity) 
   {
      mouse_sensitivity_ = mouse_sensitivity;
   }
} // !avocado
