# g-falling-blocks-ai

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
./build/FallingBlocks # ブロック20種類使用
./build/FallingBlocks --easy  # ブロック7種類使用
./build/FallingBlocks --hard # ブロック20種類使用
```
