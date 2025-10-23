# GPU-Based Grass Animation with Wind Physics

DirectX 12 Compute Shader를 활용한 실시간 물리 기반 풀 애니메이션 시스템

## 프로젝트 개요

![Video Lavel](https://img.youtube.com/vi/2k9AXCyJiXg/0.jpg)

"더 라스트 가디언"에서 영감을 받아, 풀이 바람에 따라 흔들리는 시스템을 DirectX12로 구현했습니다.

## 핵심 특징
- GPU Compute Shader로 160개 풀잎 동시 계산 (32 blades x 5 segments)
- 모든 물리계산을 GPU에서 처리하고있기 때문에 CPU 부하 제로
- Geometry Shader로 빌보드 폴리곤 동적 생성

## 기술스택
| 분야 | 기술 |
|------|------|
| **Graphics API** | DirectX 12 |
| **Shaders** | HLSL (Compute, Geometry, Vertex, Pixel) |
| **Physics** | 관성 모멘트, 토크, 쿼터니언 회전 |
| **Language** | C++ |
| **Tools** | Visual Studio 2022 |

## 성능
| 항목 | 수치 |
|------|------|
| **풀잎 개수** | 160 (32 × 5 segments) |
| **GPU 메모리** | ~2MB (Bone 버퍼) |
| **CPU 사용률** | 0% (물리 계산 전부 GPU) |

**최적화 포인트:**
- Compute Shader로 병렬 처리 -> CPU부하 90%이상 감소
- 구조화 버퍼(Structured Buffer)로 GPU 메모리 효율화
- Geometry Shader로 버텍스 버퍼 동적 생성 -> 메모리 절약

## 구현 과정에서 배운 점
### 1. GPU 기반 물리 시뮬레이션의 장단점
장점:

- 대량의 객체를 동시에 처리 가능
- CPU 부하 최소화

단점:

- 디버깅 어려움 (GPU 내부 상태 확인 힘듦)
- CPU-GPU 동기화 이슈

### 2. Compute Shader와 Geometry Shader 협업

- Compute Shader: 물리 계산 (Bone 위치/회전)
- Geometry Shader: 렌더링 (빌보드 생성)
- UAV(Unordered Access View)로 데이터 공유

### 3. 물리 시뮬레이션 안정성

- 감쇠(Damping) 값 조정으로 진동 제어
- 복원력(Restore Force)으로 원래 형태 유지
- deltaTime 기반 프레임 독립적 시뮬레이션

## 조작 방법
- W/S: 카메라 전진/후진
- A/D: 카메라 좌/우 이동
- Q/E: 카메라 상/하 이동
- 1: 와이어프레임 모드
- 2: 바람 On/Off
- 마우스 좌클릭, 이동: 시야 변경

## 요구사항
- Windows 10/11
- Visual Studio 2022
- DirectX 12 지원 GPU
- Windows SDK 10.0.19041.0 이상

## 참고자료
https://ko.wikipedia.org/wiki/%EA%B4%80%EC%84%B1_%EB%AA%A8%EB%A9%98%ED%8A%B8
https://namu.wiki/w/%EA%B4%80%EC%84%B1%20%EB%AA%A8%EB%A9%98%ED%8A%B8
