# /// script
# dependencies = [
#     "cma",
# ]
# ///
"""CMA-ESでbeam/beam_tuned AIの評価重み(Weights)を最適化する。

使い方: uv run scripts/tune_weights.py --ai beam_tuned
"""

import argparse
import subprocess
import sys

import cma

# 各AIのWeights構造体のフィールド順。beam_tunedはbeamにbumpiness/aggregateHeight/holeDepthを追加したもの。
WEIGHT_NAMES_BY_AI = {
    "beam": ["landingHeight", "erodedPieceCells", "rowTransitions", "columnTransitions", "holes", "cumulativeWells"],
    "beam_tuned": [
        "landingHeight", "erodedPieceCells", "rowTransitions", "columnTransitions", "holes", "cumulativeWells",
        "bumpiness", "aggregateHeight", "holeDepth",
    ],
}
DEFAULT_WEIGHTS_BY_AI = {
    "beam": [-1.0, 1.0, -1.0, -1.0, -4.0, -1.0],
    "beam_tuned": [
        -4.222872604628931, 0.5267286904410413, -9.981653421474142, -4.244974414578996, -4.064196036562922,
        -2.874873002269033, -0.816394344335359, -3.343403961839296, -2.486832413166647,
    ],
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ai", choices=WEIGHT_NAMES_BY_AI.keys(), default="beam")
    parser.add_argument("--benchmark-path", default="./build/Benchmark")
    parser.add_argument("--games", type=int, default=3, help="1回の評価あたりのゲーム数(学習用)")
    parser.add_argument("--seed-start", type=int, default=1, help="学習用seedの開始値")
    parser.add_argument("--beam-width", type=int, default=20)
    parser.add_argument("--beam-depth", type=int, default=2)
    parser.add_argument("--sigma0", type=float, default=1.0, help="CMA-ESの初期ステップ幅")
    parser.add_argument("--max-generations", type=int, default=2)
    parser.add_argument("--log", default="tune_weights_log.csv")
    parser.add_argument(
        "--x0",
        default=None,
        help="初期値をカンマ区切りで指定してウォームスタートする(次元数はWeightsと同じ)。未指定ならデフォルト重み",
    )
    return parser.parse_args()


def average_score(args: argparse.Namespace, weights: list) -> float:
    weights_arg = ",".join(str(w) for w in weights)
    result = subprocess.run(
        [
            args.benchmark_path,
            "--ai", args.ai,
            "--beam-width", str(args.beam_width),
            "--beam-depth", str(args.beam_depth),
            "--games", str(args.games),
            "--seed-start", str(args.seed_start),
            "--weights", weights_arg,
        ],
        capture_output=True,
        text=True,
        check=True,
    )
    for line in result.stdout.splitlines():
        if line.startswith("average score:"):
            return float(line.split(":", 1)[1])
    raise RuntimeError(f"average scoreが出力に見つかりません: {result.stdout}")


def main() -> None:
    args = parse_args()
    weight_names = WEIGHT_NAMES_BY_AI[args.ai]

    if args.x0 is not None:
        x0 = [float(v) for v in args.x0.split(",")]
        if len(x0) != len(weight_names):
            raise ValueError(f"--x0は{len(weight_names)}個のカンマ区切り数値である必要があります: {args.x0}")
    else:
        x0 = DEFAULT_WEIGHTS_BY_AI[args.ai]

    es = cma.CMAEvolutionStrategy(
        x0,
        args.sigma0,
        {"bounds": [-10, 10], "maxiter": args.max_generations},
    )

    generation = 0
    with open(args.log, "w") as log:
        log.write(
            "generation,gen_best_score,gen_mean_score,overall_best_score,"
            + ",".join(weight_names)
            + "\n"
        )

        try:
            while not es.stop():
                candidates = es.ask()
                scores = [average_score(args, c) for c in candidates]
                fitnesses = [-s for s in scores]  # cmaは最小化するため符号反転
                es.tell(candidates, fitnesses)
                es.disp()

                # overall_best_scoreとその後の重み列は常にes.result.xbestの組(=同じ解)を指すため、
                # overall_best_scoreは世代を追うごとに単調増加(悪化しない)になる。
                gen_best_score = max(scores)
                gen_mean_score = sum(scores) / len(scores)
                overall_best_score = -es.result.fbest
                log.write(
                    f"{generation},{gen_best_score},{gen_mean_score},{overall_best_score},"
                    + ",".join(str(w) for w in es.result.xbest)
                    + "\n"
                )
                log.flush()
                generation += 1
        except KeyboardInterrupt:
            print("中断されました。ここまでの最良結果を報告します。", file=sys.stderr)

    print("best weights:", dict(zip(weight_names, es.result.xbest)))
    print("best average score:", -es.result.fbest)


if __name__ == "__main__":
    main()
