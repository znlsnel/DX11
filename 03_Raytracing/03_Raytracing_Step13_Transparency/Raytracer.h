﻿#pragma once

#include "Light.h"
#include "Ray.h"
#include "Sphere.h"
#include "Square.h"
#include "Triangle.h"
#include "box.h"

#include <vector>

namespace hlab {
// 일반적으로는 헤더파일에서 using namespace std를 사용하지 않습니다.
// 여기서는 강의 동영상에 녹화되는 코드 길이를 줄이기 위해서 사용하였습니다.
// 예: std::vector -> vector
using namespace std;
using namespace glm;

class Raytracer {
public:
  int width, height;
  Light light;
  vector<shared_ptr<Object>> objects;

  Raytracer(const int &width, const int &height)
      : width(width), height(height) {
    auto sphere1 = make_shared<Sphere>(vec3(0.0f, -0.1f, 3.f), 1.0f);

    sphere1->amb = vec3(0.f, 0.2f, 0.f);
    sphere1->dif = vec3(0.0f, 0.0f, 1.0f);
    sphere1->spec = vec3(0.8f);
    sphere1->alpha = 50.0f;
    sphere1->reflection = 0.2f;
    sphere1->transparency = 0.1f;

    auto sphere2 = make_shared<Sphere>(vec3(3.0f, -0.1f, 3.f), 1.0f);

    sphere2->amb = vec3(0.1f, 0.f, 0.f);
    sphere2->dif = vec3(0.7f);
    sphere2->spec = vec3(0.8f);
    sphere2->alpha = 100.f;
    sphere2->reflection = 0.2f;
    sphere2->transparency = 0.f;

    auto sphere3 = make_shared<Sphere>(vec3(-3.0f, -0.1f, 3.f), 1.0f);

    sphere3->amb = vec3(0.0f, 0.0f, 0.1f);
    sphere3->dif = vec3(0.0f, 0.0f, 1.0f);
    sphere3->spec = vec3(0.8f);
    sphere3->alpha = 100.0f;
    sphere3->reflection = 0.1f;
    sphere3->transparency = 0.0f;

    objects.push_back(sphere1);
    objects.push_back(sphere2);
    objects.push_back(sphere3);

    auto groundTexture = std::make_shared<Texture>("ground.png");

    auto ground = make_shared<Square>(
        vec3(-10.0f, -1.5f, 0.0f), vec3(-10.0f, -1.5f, 10.0f),
        vec3(10.0f, -1.5f, 10.0f), vec3(10.0f, -1.5f, 0.0f), vec2(0.0f, 0.0f),
        vec2(1.0f, 0.0f), vec2(1.0f, 1.0f), vec2(0.0f, 1.0f));

    ground->amb = vec3(1.0f);
    ground->dif = vec3(1.0f);
    ground->spec = vec3(1.0f);
    ground->alpha = 10.0f;
    ground->reflection = 0.5f;
    ground->ambTexture = groundTexture;
    ground->difTexture = groundTexture;

    objects.push_back(ground);

    auto skyFrontTexture = std::make_shared<Texture>("front.jpg");
    auto skyBackTexture = std::make_shared<Texture>("back.jpg");
    auto skyLeftTexture = std::make_shared<Texture>("left.jpg");
    auto skyRightTexture = std::make_shared<Texture>("right.jpg");
    auto skyTopTexture = std::make_shared<Texture>("top.jpg");
    auto skyBottomTexture = std::make_shared<Texture>("bottom.jpg");

    //
    auto myBox = make_shared<Box>(
        vec3(-20.0f, 10.0f, 10.0f), vec3(20.0f, 10.0f, 10.0f),
        vec3(20.0f, -10.0f, 10.0f), vec3(-20.0f, -10.0f, 10.0f),
        vec3(20.0f, 10.0f, -10.0f), vec3(-20.0f, 10.0f, -10.0f),
        vec3(-20.0f, -10.0f, -10.0f), vec3(20.0f, -10.0f, -10.0f));
    myBox->InitBox(vec3(1.f), vec3(0.f), vec3(0.f), 10.f, 0.f);
    myBox->InitTexture(skyFrontTexture, skyBackTexture, skyLeftTexture,
                       skyRightTexture, skyTopTexture, skyBottomTexture);

    for (auto square : myBox->Squares) {
      objects.push_back(square);
    }

    // auto boxTeature = std::make_shared<Texture>("shadertoy_abstract1.jpg");
    // auto box = make_shared<Box>(vec3(-2.0f, 3.0f, 3.0f),
    // vec3(-1.0f, 3.0f, 3.0f), vec3(-1.0f, 2.0f, 3.0f),
    // vec3(-2.0f, 2.0f, 3.0f), 	vec3(-1.0f, 3.0f, 2.0f),
    //vec3(-2.0f, 3.0f, 2.0f), vec3(-2.0f, 2.0f, 2.0f),
    //vec3(-1.0f, 2.0f, 2.0f)); box->InitBox(vec3(1.f), vec3(0.f),
    // vec3(0.f), 10.f, 0.f); box->InitTexture(boxTeature);

    // for (auto square : box->Squares) {
    //	objects.push_back(square);
    // }

    light = Light{{0.0f, 0.3f, -0.5f}}; // 화면 뒷쪽
  }

  Hit FindClosestCollision(Ray &ray) {
    float closestD = 1000.0; // inf
    Hit closestHit = Hit{-1.0, dvec3(0.0), dvec3(0.0)};

    for (int l = 0; l < objects.size(); l++) {
      auto hit = objects[l]->CheckRayCollision(ray);

      if (hit.d >= 0.0f) {
        if (hit.d < closestD) {
          closestD = hit.d;
          closestHit = hit;
          closestHit.obj = objects[l];

          // 텍스춰 좌표
          closestHit.uv = hit.uv;
        }
      }
    }

    return closestHit;
  }

  // 광선이 물체에 닿으면 그 물체의 색 반환
  vec3 traceRay(Ray &ray, const int recurseLevel) {
    if (recurseLevel < 0)
      return vec3(0.0f);

    // Render first hit
    const auto hit = FindClosestCollision(ray);

    if (hit.d >= 0.0f) {
      glm::vec3 color(0.0f);

      // Diffuse
      const vec3 dirToLight = glm::normalize(light.pos - hit.point);
      Ray shadowRay{hit.point + dirToLight * 1e-4f, dirToLight};
      Hit ShadowHit = FindClosestCollision(shadowRay);

      if (ShadowHit.obj != nullptr && ShadowHit.obj->isCubeMap == false)
        return ShadowHit.obj->amb;

      glm::vec3 phongColor(0.0f);

      const float diff = glm::max(dot(hit.normal, dirToLight), 0.0f);

      // Specular
      const vec3 reflectDir =
          hit.normal * 2.0f * dot(dirToLight, hit.normal) - dirToLight;
      const float specular = glm::pow(
          glm::max(glm::dot(-ray.dir, reflectDir), 0.0f), hit.obj->alpha);

      if (hit.obj->ambTexture) {
        phongColor += hit.obj->amb * hit.obj->ambTexture->SampleLinear(hit.uv);
      } else {
        phongColor += hit.obj->amb;
      }

      if (hit.obj->difTexture) {
        phongColor +=
            diff * hit.obj->dif * hit.obj->difTexture->SampleLinear(hit.uv);
      } else {
        phongColor += diff * hit.obj->dif;
      }

      phongColor += hit.obj->spec * specular;

      color +=
          phongColor * (1.0f - hit.obj->reflection - hit.obj->transparency);

      if (hit.obj->reflection) {
        const auto reflectedDirection = glm::normalize(
            2.0f * hit.normal * dot(-ray.dir, hit.normal) + ray.dir);
        Ray reflection_ray{
            hit.point + reflectedDirection * 1e-4f,
            reflectedDirection}; // add a small vector to avoid numerical issue

        color +=
            traceRay(reflection_ray, recurseLevel - 1) * hit.obj->reflection;
      }

      // 참고
      // https://samdriver.xyz/article/refraction-sphere (그림들이 좋아요)
      // https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
      // (오류있음)
      // https://web.cse.ohio-state.edu/~shen.94/681/Site/Slides_files/reflection_refraction.pdf
      // (슬라이드가 보기 좋지는 않지만 정확해요)
      if (hit.obj->transparency) {
        const float ior = 1.3f; // Index of refraction (유리: 1.5, 물: 1.3)

        float eta; // sinTheta1 / sinTheta2
        vec3 normal;

        if (glm::dot(ray.dir, hit.normal) <
            0.0f) // 밖에서 안에서 들어가는 경우 (예: 공기->유리)
        {
          eta = ior;
          normal = hit.normal;
        } else // 안에서 밖으로 나가는 경우 (예: 유리->공기)
        {
          eta = 1.0f / ior;
          normal = -hit.normal;
        }

        const float cosTheta1 = glm::dot(-ray.dir, normal);
        const float sinTheta1 =
            glm::sqrt(1.f - cosTheta1 * cosTheta1); // cos^2 + sin^2 = 1
        const float sinTheta2 =
            sinTheta1 /
            eta; // eta = sinTheta1 / sinTheta2 // sinTheta2 = sinTheta1/eta
        const float cosTheta2 =
            glm::sqrt(1.f - sinTheta2 * sinTheta2); // cos^2 + sin^2  = 1

        const vec3 m =
            glm::normalize(glm::dot(normal, -ray.dir) * normal + ray.dir);
        const vec3 a = m * sinTheta2;
        const vec3 b = -normal * cosTheta2;
        const vec3 refractedDirection = glm::normalize(a + b); // transmission

        Ray refractedRay{hit.point + refractedDirection * 1e-2f,
                         refractedDirection};
        color +=
            traceRay(refractedRay, recurseLevel - 1) * hit.obj->transparency;

        // Fresnel 효과는 생략되었습니다.
      }

      return color;
    }

    return vec3(0.0f);
  }

  void Render(std::vector<glm::vec4> &pixels) {
    std::fill(pixels.begin(), pixels.end(), vec4(0.0f, 0.0f, 0.0f, 1.0f));

    const vec3 eyePos(0.0f, 0.0f, -1.5f);

#pragma omp parallel for
    for (int j = 0; j < height; j++)
      for (int i = 0; i < width; i++) {
        const vec3 pixelPosWorld = TransformScreenToWorld(vec2(i, j));
        Ray pixelRay{pixelPosWorld, glm::normalize(pixelPosWorld - eyePos)};
        pixels[i + width * j] =
            vec4(glm::clamp(traceRay(pixelRay, 5), 0.0f, 1.0f), 1.0f);
      }
  }

  vec3 TransformScreenToWorld(vec2 posScreen) {
    const float xScale = 2.0f / this->width;
    const float yScale = 2.0f / this->height;
    const float aspect = float(this->width) / this->height;

    // 3차원 공간으로 확장 (z좌표는 0.0)
    return vec3((posScreen.x * xScale - 1.0f) * aspect,
                -posScreen.y * yScale + 1.0f, 0.0f);
  }
};
} // namespace hlab