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
./build/Benchmark --ai beam --weights "-1,1,-1,-1,-4,-1"  # beamの評価重みを変更(landingHeight,erodedPieceCells,rowTransitions,columnTransitions,holes,cumulativeWellsの順)
./build/Benchmark --ai beam_tuned --weights "-1,1,-1,-1,-4,-1,0,0,0"  # beam_tuned: beamにbumpiness/aggregateHeight/holeDepth特徴量を追加したAI(重みは9個、末尾3つがbumpiness,aggregateHeight,holeDepth)
```

```bash
uv run scripts/plot_benchmark.py result.csv    # result.csv.{score_hist,lines_hist,lines_vs_score}.png を生成
```

`beam`/`beam_tuned`の評価重み(Weights)は、CMA-ESで最適化できる。

```bash
uv run scripts/tune_weights.py                 # デフォルトは動作確認用の小さい設定(すぐ終わる)、対象はbeam
uv run scripts/tune_weights.py --ai beam_tuned --games 30 --beam-width 50 --beam-depth 5 --max-generations 40  # 本番設定
uv run scripts/tune_weights.py --ai beam_tuned --x0="-1,1,-1,-1,-4,-1,-1.5,0,0" --games 30 --beam-width 50 --beam-depth 5 --max-generations 40  # --x0で初期値を指定してウォームスタート
```

世代ごとの結果は`tune_weights_log.csv`に記録される。
