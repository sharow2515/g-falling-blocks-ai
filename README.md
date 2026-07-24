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
sudo apt-get install libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev pkg-config
```

## ビルドと実行

```bash
cmake --preset=default
cmake --build build
./build/FallingBlocks                    # ブロック20種類使用、AI: simple(デフォルト)
./build/FallingBlocks --easy             # ブロック7種類使用
./build/FallingBlocks --hard             # ブロック20種類使用
./build/FallingBlocks --ai reachability  # 自動プレイに使うAIを切り替える
./build/FallingBlocks --seed 123         # ミノの出現列を指定する(未指定ならランダム)
```

ゲーム中にAキーを押すと、指定したAIによる自動プレイのON/OFFを切り替えられる。

## AI可視化モード

Aiの動きを可視化するモード

```bash
./build/FallingBlocks --visualize                     # AI: simple(デフォルト)で可視化
./build/FallingBlocks --visualize --ai reachability   # AIを切り替える
./build/FallingBlocks --visualize --ai-delay 0.1      # 1操作ごとの間隔(秒)を変更(デフォルト0.3)
./build/FallingBlocks --visualize --seed 123          # ミノの出現列を指定する(未指定ならランダム)
```


## AIベンチマーク

`ai::AiPlayer`実装(`simple` / `reachability`)の性能を、ウィンドウなしで複数ゲーム連続実行して計測する。

```bash
./build/Benchmark                              # デフォルト設定(後述)で実行
./build/Benchmark --games 100 --seed-start 1   # 100ゲーム、シード1から連番で実行
./build/Benchmark --easy                       # ブロック7種類で実行
./build/Benchmark --hard                       # ブロック20種類で実行(デフォルト)
./build/Benchmark --max-lines 1000             # 1ゲームあたりの消去ライン上限を変更
./build/Benchmark --ai reachability            # 使用するAIを切り替える
./build/Benchmark --output result.csv          # ゲームごとの結果(seed,score,lines,hit_line_limit)をCSVに出力
```

```bash
uv run scripts/plot_benchmark.py result.csv    # result.csv.{score_hist,lines_hist,lines_vs_score}.png を生成
```
