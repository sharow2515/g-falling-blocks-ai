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
./build/FallingBlocks                                # 人間がプレイする
./build/FallingBlocks --visualize --ai beam_unique   # AIのプレイを見る
./build/Benchmark --ai beam_unique --games 10 --max-lines 100   # AIの性能を計測する
```

## AI一覧

AIの名前を `--ai` で指定する。

| 名前 | 説明 |
|---|---|
| `simple` | 現在のピースだけを見る。回転 → 左右移動 → ハードドロップで置ける場所から選ぶ。 |
| `reachability` | simple と同じ評価。実際の操作(ウォールキックを含む)で届く着地先を、探索で全て調べる。 |
| `beam` | reachability と同様のことをビームサーチで行う。 |
| `beam_tuned` | beam と同じ探索を行う。特徴量を増やし、重みを調整している。 |
| `beam_unique` | beam_tuned に、同じ状態の候補を1つにまとめる処理を加えた。 |

`--weights` には、重みをカンマ区切りで、次の順に並べて渡す。

- `beam`(6個): landingHeight, erodedPieceCells, rowTransitions, columnTransitions, holes, cumulativeWells
- `beam_tuned` / `beam_unique`(9個): 上の6個に続けて bumpiness, aggregateHeight, holeDepth

## オプション

`FallingBlocks` と `Benchmark` のどちらも、未知のオプション、値の欠落、不正な値はエラーで終了する。

### 共通のオプション

| フラグ | 値 | 既定値 | 説明 |
|---|---|---|---|
| `--easy` | なし | | ブロック7種類(I, J, L, O, S, T, Z)を使用する。 |
| `--hard` | なし | 指定なしと同じ | ブロック20種類を使用する。 |
| `--ai` | AI名 | `simple` | 使用するAI。AI一覧の名前から選ぶ。 |
| `--beam-width` | 整数(1以上) | 20 | ビームサーチで残す候補の数。 |
| `--beam-depth` | 整数(0〜5) | 2 | 先読みする Next ピースの数。 |
| `--weights` | カンマ区切りの小数 | AIごとの既定の重み | 評価の重み。個数はAIごとに違う(AI一覧を参照)。 |

`--beam-width` / `--beam-depth` / `--weights` は beam 系(`beam` / `beam_tuned` / `beam_unique`)のみ有効で、`simple` / `reachability` では指定しても無視される。

### FallingBlocks のオプション

```bash
./build/FallingBlocks --easy --seed 123                             # ブロック7種類、出現順を固定してプレイ
./build/FallingBlocks --visualize --ai reachability --ai-delay 0.1  # AIのプレイを0.1秒間隔で見る
```

| フラグ | 値 | 既定値 | 説明 |
|---|---|---|---|
| `--visualize` | なし | 指定なし(人間がプレイ) | AIのプレイを見るモードで起動する。 |
| `--seed` | 整数(0以上) | 未指定(ランダム) | ブロックの出現順を固定する。リスタートしても同じ順になる。 |
| `--ai-delay` | 小数(秒) | 0.3 | `--visualize` のとき、AIが1操作ごとに置く間隔。 |

`--ai` / `--beam-width` / `--beam-depth` / `--weights` / `--ai-delay` は、`--visualize` と一緒にのみ指定できる(なければエラー)。

### Benchmark のオプション

AIの性能を、ウィンドウなしで複数ゲーム連続実行して計測する。

```bash
./build/Benchmark --ai reachability --games 20 --max-lines 100
./build/Benchmark --ai beam_unique --beam-width 20 --beam-depth 2 --games 10 --max-lines 100 --output result.csv
./build/Benchmark --ai beam --weights "-1,1,-1,-1,-4,-1" --games 5 --max-lines 100
```

| フラグ | 値 | 既定値 | 説明 |
|---|---|---|---|
| `--games` | 整数 | 100 | 実行するゲーム数。 |
| `--seed-start` | 整数(0以上) | 1 | 最初のゲームのシード。以降は1ずつ増える。 |
| `--max-lines` | 整数 | 500 | 1ゲームで消去するライン数の上限。達したらそのゲームを終了する。 |
| `--output` | ファイルのパス | 未指定(出力しない) | ゲームごとの結果をCSVに書き出す。 |

```bash
uv run scripts/plot_benchmark.py result.csv    # result.csv.{score_hist,lines_hist,lines_vs_score}.png を生成
```

`beam` / `beam_tuned` の評価の重みは、CMA-ES で最適化できる。

```bash
uv run scripts/tune_weights.py                 # 既定は動作確認用の小さい設定(すぐ終わる)、対象は beam
uv run scripts/tune_weights.py --ai beam_tuned --games 30 --beam-width 50 --beam-depth 5 --max-generations 40  # 本番設定
uv run scripts/tune_weights.py --ai beam_tuned --x0="-1,1,-1,-1,-4,-1,-1.5,0,0" --games 30 --beam-width 50 --beam-depth 5 --max-generations 40  # --x0 で初期値を指定してウォームスタート
```

世代ごとの結果は `tune_weights_log.csv` に記録される。
