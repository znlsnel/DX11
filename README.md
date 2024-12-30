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
![image](https://github.com/user-attachments/assets/54455143-a419-4eb7-8969-296a2785d5c6)
<br>

### Bounding Volume Hierarchy
- Mesh의 정점이나 Object 그룹을 계층으로 관리하는 시스템 개발
![image](https://github.com/user-attachments/assets/c88fb983-70ca-406e-a99c-0630731c4c1c)
<br>

### Ray Casting
- BVH를 이용하여 마우스로 클릭한 위치의 월드 좌표값 추출하는 기능
[![Video](https://github.com/user-attachments/assets/a0a560ec-b352-4de2-a9b3-cdf0c540a349)](https://www.youtube.com/watch?time_continue=21&v=Ml560jTlHRk&embeds_referring_euri=https%3A%2F%2Fwww.notion.so%2F&source_ve_path=MjM4NTE)
<br>


### Texture Painting
- Ray Casting을 통해 구한 월드 좌표에서, 현재 사용되는 texture를 실시간으로 수정하는 기능 구현
 ![image](https://github.com/user-attachments/assets/75bf1e76-64c5-4c20-8261-a228ad51aff4)
<br>

### Foliage - Billboard Rendering
- Unreal의 Foliage 기능 구현, 멀리있는 객체는 2D 이미지로 렌더링하여 성능 최적화.
![image](https://github.com/user-attachments/assets/61e72c1f-ee23-4d2c-94ad-67237f070cdb)
<br>

### Mirror
- 거울에 반사된 객체를 한번 더 렌더링 하는 방식으로 거울 구현
![image](https://github.com/user-attachments/assets/2e183d50-8eec-459f-bc16-1ff3ac2bf942)
<br>

### Frustum Culling
- BVH를 이용하여 카메라 절두체 안의 객체만 렌더링하는 Frustum Culling 구현
- [![Video](https://github.com/user-attachments/assets/8ccc8316-5975-4315-88c7-f17422dedb45)](https://www.youtube.com/watch?time_continue=21&v=bqTtRdFjOnk&embeds_referring_euri=https%3A%2F%2Fwww.notion.so%2F&source_ve_path=MjM4NTE)
<br>

### Mesh LOD 시스템
- Mesh 간소화 알고리즘을 사용하여 Mesh Lod를 자동으로 생성하는 기능
[![Video](https://github.com/user-attachments/assets/6525b4af-0c6e-4f70-8f23-ec887abdac30)](https://www.youtube.com/watch?time_continue=21&v=AlE7ONOuRWQ&embeds_referring_euri=https%3A%2F%2Fwww.notion.so%2F&source_ve_path=MjM4NTE)
<br>

### Physically Based Rendering
- PBR을 프로젝트에 적용하여 사실적인 그래픽 구현
![image](https://github.com/user-attachments/assets/8f609587-9619-4784-81a3-ab2d7225a202)
<br>

### Shadow
- 물체와 빛까지의 거리를 담는 Shadow Map을 이용하여 그림자 구현
![image](https://github.com/user-attachments/assets/a1fe390d-db50-464d-b120-921373cc02c3)
<br>

### Skeletal Animation
- 각 정점에 영향을 주는 뼈의 정보를 계속 업데이트하고<br>
  Vertex Shader에서 그 정보를 토대로 위치를 보정해주는 방식으로 구현
[![Video](https://github.com/user-attachments/assets/242dc133-8a7d-4976-970a-ac0a56cad0d9)](https://www.youtube.com/watch?time_continue=21&v=QVtPTIfkE-E&embeds_referring_euri=https%3A%2F%2Fwww.notion.so%2F&source_ve_path=MjM4NTE)

