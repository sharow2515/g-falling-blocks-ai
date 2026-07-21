# g-falling-blocks-ai

「激キショテトリス」（作者: 物理鉄人）をC++に移植し、AIを実装するリポジトリです。
- 元作品: https://unityroom.com/games/kishotet

## 必要なツール

- CMake
- vcpkg
- Ninja

### Linux 追加で必要

```bash
sudo apt-get update
sudo apt-get install libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
```

## ビルドと実行

```bash
cmake --preset=default
cmake --build build
./build/FallingBlocks                    # ブロック20種類使用、AI: heuristic(デフォルト)
./build/FallingBlocks --easy             # ブロック7種類使用
./build/FallingBlocks --hard             # ブロック20種類使用
./build/FallingBlocks --ai reachability  # 自動プレイに使うAIを切り替える
```

ゲーム中にAキーを押すと、指定したAIによる自動プレイのON/OFFを切り替えられる。

## AIベンチマーク

`ai::AiPlayer`実装(`heuristic` / `reachability`)の性能を、ウィンドウなしで複数ゲーム連続実行して計測する。

```bash
./build/Benchmark                              # デフォルト設定(後述)で実行
./build/Benchmark --games 100 --seed-start 1   # 100ゲーム、シード1から連番で実行
./build/Benchmark --easy                       # ブロック7種類で実行
./build/Benchmark --hard                       # ブロック20種類で実行(デフォルト)
./build/Benchmark --max-lines 1000             # 1ゲームあたりの消去ライン上限を変更
./build/Benchmark --ai reachability            # 使用するAIを切り替える
```
