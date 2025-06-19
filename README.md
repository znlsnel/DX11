![Typing SVG](https://readme-typing-svg.demolab.com?font=Fira+Code&size=30&pause=1000&width=435&lines=3D+Rendering+Engine)

---
# Description
- **프로젝트 소개** <br>
  3D RealTime Rendering Engine 개발을 목적으로 시작한 프로젝트입니다.<br>
  게임 엔진에서 볼 수 있는 기능들을 직접 구현해보는 것을 목표로 개발했습니다.

- **개발 기간** : 2023.10.04 - 2024.02.27
- **사용 기술** <br>
-언어 : C++<br>
-그래픽 API : DirectX 11 <br>
-개발 환경 : Windows 11
<br>

## 프로젝트 소개 영상
[![Link](https://github.com/user-attachments/assets/9f577d23-f927-4bd6-b597-b83e9bae1b2b)](https://www.youtube.com/watch?v=7WnX4B5JKIU)
- 이미지를 클릭하면 유튜브 링크로 이동합니다.

<br>

---
## 목차
- 기획 의도
- 핵심 기능
<br>

---
## 기획 의도
- 게임 엔진을 구성하는 다양한 기능들을 직접 구현하고 이해하는 것을 목표로 개발
- 안정적으로 구동되는 리얼타임 랜더링 엔진 구현
<br>

---
## 핵심 기능
### Terrain
- Mesh의 디테일이 동적으로 변하는 Tessellation 지형 구현
![image](https://github.com/user-attachments/assets/54455143-a419-4eb7-8969-296a2785d5c6)<br>
  <img src="https://github.com/user-attachments/assets/84af9297-ce13-4d5a-a983-0cc9d79d3e10" width="700">

  CPU에서는 적당 수준의 정점 정보만 들고 있게 하고, GPU에서 정점의 수를 늘려주는 방식을 사용했습니다. <br>
  이 방식을 통해 CPU에서 알고 있는 지형에 대한 정보를 바탕으로 레이케스팅을 가능하게 하였습니다.

<br>

### Bounding Volume Hierarchy
- Mesh의 정점이나 Object 그룹을 계층으로 관리하는 시스템 개발
![image](https://github.com/user-attachments/assets/c88fb983-70ca-406e-a99c-0630731c4c1c) <br>
  <img src="https://github.com/user-attachments/assets/9c55f470-6582-45a4-ae52-25a5d62637d6" width="700">
- BVH를 통해 정점 혹은 오브젝트를 N번 순회하여 탐색하는 방식에서 LogN번의 연산으로 탐색할 수 있게 하여 성능을 개선했습니다.

<br>


### Ray Casting
- BVH를 이용하여 마우스로 클릭한 위치의 월드 좌표값 추출하는 기능
  <img src="https://github.com/user-attachments/assets/3adce4bc-3bae-454a-913b-8559364c25fe" width="700">

<br>


### Texture Painting
- Ray Casting을 통해 구한 월드 좌표에서, 현재 사용되는 texture를 실시간으로 수정하는 기능 구현
 ![image](https://github.com/user-attachments/assets/75bf1e76-64c5-4c20-8261-a228ad51aff4)<br>
  <img src="https://github.com/user-attachments/assets/fb62bef0-86b1-4a73-a5ae-83cb74c64ee8" width="700">

- 서로 다른 Texture가 자연스럽게 섞이도록 색상을 보간하는 작업을 해주었습니다.

<br>

### Foliage - Billboard Rendering
- Unreal의 Foliage 기능 구현, 멀리있는 객체는 2D 이미지로 렌더링하여 성능 최적화.
![image](https://github.com/user-attachments/assets/61e72c1f-ee23-4d2c-94ad-67237f070cdb)
  <img src="https://github.com/user-attachments/assets/43420f4a-b86d-4448-91d0-e03dd24b8188" width="700">

<br>

### Mirror
- 거울에 반사된 객체를 한번 더 렌더링 하는 방식으로 거울 구현
![image](https://github.com/user-attachments/assets/2e183d50-8eec-459f-bc16-1ff3ac2bf942)
  <img src="https://github.com/user-attachments/assets/dfe9e058-fd48-42c8-985b-92c877cf719f" width="700">

<br>

### Frustum Culling
- BVH를 이용하여 카메라 절두체 안의 객체만 렌더링하는 Frustum Culling 구현
  <img src="https://github.com/user-attachments/assets/5ac4f797-6f38-4310-87c1-cc45f750c779" width="700">

<br>

### Mesh LOD 시스템
- Mesh 간소화 알고리즘을 사용하여 Mesh Lod를 자동으로 생성하는 기능<br>
  <img src="https://github.com/user-attachments/assets/fc3155dd-ce57-4170-8de0-b06320dffdad" width="700">

<br>

### Physically Based Rendering
- PBR을 프로젝트에 적용하여 사실적인 그래픽 구현
![image](https://github.com/user-attachments/assets/8f609587-9619-4784-81a3-ab2d7225a202) <br>
  <img src="https://github.com/user-attachments/assets/88d788dd-a383-45d7-87a5-6ba6b5f3cc91" width="700">

<br>

### Shadow
- 물체와 빛까지의 거리를 담는 Shadow Map을 이용하여 그림자 구현 <br>
  <img src="https://github.com/user-attachments/assets/abedb99d-f457-4445-acc9-ee51b82f32d9" width="700">

<br>

### Skeletal Animation
- 각 정점에 영향을 주는 뼈의 정보를 계속 업데이트하고<br>
  Vertex Shader에서 그 정보를 토대로 위치를 보정해주는 방식으로 구현
<img src="https://github.com/user-attachments/assets/6606b5b4-f33a-41be-b4eb-3fd23c1c0a53" width="700"><br>
 연결된 애니메이션으로 넘어갈 때, 자연스럽게 넘어가도록 보간된 값이 적용되도록 했습니다.<br><br>

  <img src="https://github.com/user-attachments/assets/9657fc8c-a1a3-4b7e-8264-97cbc9e77fd2" width="700"><br>
 애니메이션의 프레임과 프레임 사이에 보간된 값을 넣어서 자연스러운 애니메이션을 구현했습니다.

<br>

### Fog
- 깊이 맵을 통해 거리에 따라 시야가 제한되는 안개 기능을 만들었습니다.
  <img src="https://github.com/user-attachments/assets/19d64b03-0a87-49f0-a384-a35becf563a7" width="700"><br>

<br>

### 기타
- Particle <br>
  <img src="https://github.com/user-attachments/assets/12f747ef-5e1b-46f7-b318-19e3fa5103ca" width="700"><br>

- Fluid simulation <br>
  <img src="https://github.com/user-attachments/assets/c58eecb5-0982-4a08-882b-f2020d1b4dc8" width="700"><br>

- Smoke simulation <br>
  <img src="https://github.com/user-attachments/assets/bd6cf968-48ca-4506-a868-0be035b5d6e2" width="700"><br>



