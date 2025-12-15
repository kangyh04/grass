# GPU-Based Grass Animation with Wind Physics

DirectX 12 Compute Shader를 활용한 실시간 물리 기반 풀 애니메이션 시스템

---

## 📽️ 프로젝트 데모

[![Video Lavel](https://img.youtube.com/vi/2k9AXCyJiXg/0.jpg)](https://www.youtube.com/watch?v=2k9AXCyJiXg)

"더 라스트 가디언"에서 영감을 받아, 풀이 바람에 따라 흔들리는 시스템을 DirectX12로 구현했습니다.

---

## 🌐 다국어 README (클릭하여 펼치기)

<details>
<summary><strong>🇰🇷 한국어 (Korean)</strong></summary>
<br>

### GPU-Based Grass Animation with Wind Physics

#### 핵심 특징
- GPU Compute Shader로 **160개 풀잎 동시 계산** (32 blades x 5 segments)
- 모든 물리계산을 GPU에서 처리하고있기 때문에 **CPU 부하 제로**
- Geometry Shader로 빌보드 폴리곤 **동적 생성**

#### 기술스택
| 분야 | 기술 |
|------|------|
| **Graphics API** | DirectX 12 |
| **Shaders** | HLSL (Compute, Geometry, Vertex, Pixel) |
| **Physics** | 관성 모멘트, 토크, 쿼터니언 회전 |
| **Language** | C++ |
| **Tools** | Visual Studio 2022 |

#### 성능
| 항목 | 수치 |
|------|------|
| **풀잎 개수** | 160 (32 × 5 segments) |
| **GPU 메모리** | ~2MB (Bone 버퍼) |
| **CPU 사용률** | 0% (물리 계산 전부 GPU) |

**최적화 포인트:**
* Compute Shader로 병렬 처리 -> CPU부하 90%이상 감소
* 구조화 버퍼(Structured Buffer)로 GPU 메모리 효율화
* Geometry Shader로 버텍스 버퍼 동적 생성 -> 메모리 절약

#### 구현 과정에서 배운 점
1.  **GPU 기반 물리 시뮬레이션의 장단점**
    * **장점**: 대량의 객체를 동시에 처리 가능, CPU 부하 최소화
    * **단점**: 디버깅 어려움 (GPU 내부 상태 확인 힘듦), CPU-GPU 동기화 이슈
2.  **Compute Shader와 Geometry Shader 협업**
    * Compute Shader: 물리 계산 (Bone 위치/회전)
    * Geometry Shader: 렌더링 (빌보드 생성)
    * UAV(Unordered Access View)로 데이터 공유
3.  **물리 시뮬레이션 안정성**
    * 감쇠(Damping) 값 조정으로 진동 제어
    * 복원력(Restore Force)으로 원래 형태 유지
    * deltaTime 기반 프레임 독립적 시뮬레이션

#### 조작 방법
* W/S: 카메라 전진/후진
* A/D: 카메라 좌/우 이동
* Q/E: 카메라 상/하 이동
* 1: 와이어프레임 모드
* 2: 바람 On/Off
* 마우스 좌클릭, 이동: 시야 변경

#### 요구사항
* Windows 10/11
* Visual Studio 2022
* DirectX 12 지원 GPU
* Windows SDK 10.0.19041.0 이상

#### 참고자료
* [관성 모멘트 (Wikipedia)](https://ko.wikipedia.org/wiki/%EA%B4%80%EC%84%B1_%EB%AA%A8%EB%A9%98%ED%8A%B8)
* [관성 모멘트 (나무위키)](https://namu.wiki/w/%EA%B4%80%EC%84%B1%20%EB%AA%A8%EB%A9%98%ED%8A%B8)

</details>

<details>
<summary><strong>🇯🇵 日本語 (Japanese)</strong></summary>
<br>

### GPU-Based Grass Animation with Wind Physics

DirectX 12 Compute Shaderを活用したリアルタイム物理ベース草アニメーションシステム

#### 主な特徴
- GPU Compute Shaderで**160枚の草の葉を同時計算** (32 blades x 5 segments)
- すべての物理計算をGPUで処理しているため、**CPU負荷ゼロ**
- Geometry Shaderでビルボードポリゴンを**動的生成**

#### 技術スタック
| 分野 | 技術 |
|------|------|
| **Graphics API** | DirectX 12 |
| **Shaders** | HLSL (Compute, Geometry, Vertex, Pixel) |
| **Physics** | 慣性モーメント、トルク、クォータニオン回転 |
| **Language** | C++ |
| **Tools** | Visual Studio 2022 |

#### パフォーマンス
| 項目 | 数値 |
|------|------|
| **草の葉の数** | 160 (32 × 5 segments) |
| **GPUメモリ** | ~2MB (Boneバッファ) |
| **CPU使用率** | 0% (物理計算はすべてGPU) |

**最適化ポイント:**
* Compute Shaderによる並列処理 -> CPU負荷90%以上削減
* 構造化バッファ (Structured Buffer) でGPUメモリ効率化
* Geometry Shaderで頂点バッファを動的生成 -> メモリ節約

#### 実装プロセスで学んだこと
1.  **GPUベース物理シミュレーションの長所と短所**
    * **長所**: 大量のオブジェクトを同時処理可能、CPU負荷最小化
    * **短所**: デバッグの難しさ、CPU-GPU同期の問題
2.  **Compute ShaderとGeometry Shaderの連携**
    * Compute Shader: 物理計算 (Bone位置/回転)
    * Geometry Shader: レンダリング (ビルボード生成)
    * UAV(Unordered Access View) でデータ共有
3.  **物理シミュレーションの安定性**
    * 減衰(Damping)値の調整による振動制御
    * 復元力(Restore Force)による形状維持
    * deltaTimeベースのフレームレート非依存シミュレーション

#### 操作方法
* W/S: カメラ前進/後退
* A/D: カメラ左/右移動
* Q/E: カメラ上/下移動
* 1: ワイヤーフレームモード
* 2: 風 On/Off
* マウス左クリック、移動: 視点変更

#### 要件
* Windows 10/11
* Visual Studio 2022
* DirectX 12対応GPU
* Windows SDK 10.0.19041.0以上

#### 参考資料
* [慣性モーメント (Wikipedia)](https://ja.wikipedia.org/wiki/%E6%85%A3%E6%80%A7%E3%83%A2%E3%83%BC%E3%83%A1%E3%83%B3%E3%83%88)

</details>

<details>
<summary><strong>🇬🇧 English</strong></summary>
<br>

### GPU-Based Grass Animation with Wind Physics

Real-time Physics-based Grass Animation System utilizing DirectX 12 Compute Shader

#### Key Features
* Simultaneous calculation of **160 grass blades** (32 blades x 5 segments) using GPU Compute Shader
* **Zero CPU overhead** as all physics calculations are handled by the GPU
* **Dynamic generation** of billboard polygons using Geometry Shader

#### Tech Stack
| Category | Technology |
|------|------|
| **Graphics API** | DirectX 12 |
| **Shaders** | HLSL (Compute, Geometry, Vertex, Pixel) |
| **Physics** | Moment of Inertia, Torque, Quaternion Rotation |
| **Language** | C++ |
| **Tools** | Visual Studio 2022 |

#### Performance
| Item | Value |
|------|------|
| **Number of Blades** | 160 (32 × 5 segments) |
| **GPU Memory** | ~2MB (Bone Buffer) |
| **CPU Usage** | 0% (All physics calculation on GPU) |

**Optimization Points:**
* Parallel processing with Compute Shader -> Over 90% reduction in CPU load
* Efficient GPU memory usage with Structured Buffers
* Dynamic vertex buffer generation using Geometry Shader -> Memory saving

#### Lessons Learned
1.  **Pros and Cons of GPU-Based Physics Simulation**
    * **Pros**: Simultaneous processing of large number of objects, minimized CPU load
    * **Cons**: Difficult debugging (hard to check internal GPU state), CPU-GPU synchronization issues
2.  **Collaboration between Compute Shader and Geometry Shader**
    * Compute Shader: Physics calculation (Bone position/rotation)
    * Geometry Shader: Rendering (Billboard creation)
    * Data sharing via UAV (Unordered Access View)
3.  **Stability of Physics Simulation**
    * Vibration control through Damping adjustment
    * Shape maintenance with Restore Force
    * Frame-rate independent simulation based on deltaTime

#### Controls
* W/S: Camera Forward/Backward
* A/D: Camera Left/Right
* Q/E: Camera Up/Down
* 1: Wireframe Mode
* 2: Wind On/Off
* Mouse Left Click + Move: Viewpoint Change

#### Requirements
* Windows 10/11
* Visual Studio 2022
* DirectX 12 compatible GPU
* Windows SDK 10.0.19041.0 or higher

#### References
* [Moment of Inertia (Wikipedia)](https://en.wikipedia.org/wiki/Moment_of_inertia)

</details>
