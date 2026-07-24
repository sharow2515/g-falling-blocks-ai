# /// script
# dependencies = [
#     "pandas",
#     "matplotlib",
# ]
# ///
"""Benchmarkの--outputで生成したCSVをグラフ化する。

使い方: uv run scripts/plot_benchmark.py <csv path>
"""

import sys

import matplotlib.pyplot as plt
import pandas as pd


def save_hist(series: pd.Series, title: str, out_path: str) -> None:
    fig, ax = plt.subplots()
    ax.hist(series, bins=20)
    ax.set_title(title)
    ax.set_xlabel(str(series.name))
    ax.set_ylabel("count")
    fig.savefig(out_path, bbox_inches="tight")
    print(f"saved: {out_path}")


def main() -> None:
    if len(sys.argv) != 2:
        print(f"usage: uv run {sys.argv[0]} <csv path>", file=sys.stderr)
        sys.exit(1)

    csv_path = sys.argv[1]
    df = pd.read_csv(csv_path)

    save_hist(df["score"], "Score distribution", f"{csv_path}.score_hist.png")
    save_hist(df["lines"], "Lines distribution", f"{csv_path}.lines_hist.png")

    fig, ax = plt.subplots()
    ax.scatter(df["lines"], df["score"])
    ax.set_title("Lines vs score")
    ax.set_xlabel("lines")
    ax.set_ylabel("score")
    scatter_path = f"{csv_path}.lines_vs_score.png"
    fig.savefig(scatter_path, bbox_inches="tight")
    print(f"saved: {scatter_path}")


if __name__ == "__main__":
    main()
