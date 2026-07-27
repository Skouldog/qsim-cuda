"""Explore Google Benchmark results.

    ./.venv/bin/python scripts/plot_benchmarks.py --list
    ./.venv/bin/python scripts/plot_benchmarks.py --table --no-plot
    ./.venv/bin/python scripts/plot_benchmarks.py
    ./.venv/bin/python scripts/plot_benchmarks.py BM_ghzState --show
    ./.venv/bin/python scripts/plot_benchmarks.py --save cpu-baseline

Reads the JSON written by `cmake --build ... -t bench` and turns every
benchmark family into a table and a plot, whatever arguments it was
registered with.

Without --save, plots land in a scratch directory and are overwritten every
run.  With --save LABEL the run is kept: a dated folder is created holding
the JSON and every plot generated from it.
"""
# ./.venv/bin/python scripts/plot_benchmarks.py
# ./.venv/bin/python scripts/plot_benchmarks.py --save LABEL
import argparse
import json
import math
import os
import re
import shutil
from collections import OrderedDict

import matplotlib.pyplot as plt

DEFAULT_RESULTS = "benchmarks/results/latest.json"
DEFAULT_OUTDIR = "benchmarks/results/latest"
DEFAULT_SAVEDIR = "benchmarks/results"
BYTES_PER_AMPLITUDE = 16  # std::complex<double>

# Per-run aggregates google/benchmark emits when --benchmark_repetitions > 1.
STAT_ROWS = ("mean", "median", "stddev", "cv")

AXIS_LABELS = {
    "real_time": ("time (ns)", 1.0),
    "bytes_per_second": ("GB/s", 1e9),
    "items_per_second": ("G items/s", 1e9),
}


# --------------------------------------------------------------------------
# parsing
# --------------------------------------------------------------------------


def parse_run_name(run_name):
    """'BM_foo/qubits:3/k:8' -> ('BM_foo', OrderedDict(qubits=3, k=8)).

    Argument labels come from ArgName()/ArgNames() on the C++ side, so they
    may contain spaces; only the final colon separates label from value.
    """
    family, *arg_parts = run_name.split("/")
    args = OrderedDict()
    for part in arg_parts:
        label, _, value = part.rpartition(":")
        if not label:  # benchmark had no ArgName()
            label, value = "arg%d" % len(args), part
        label = re.sub(r"\s+", " ", label).strip()
        try:
            args[label] = int(value)
        except ValueError:
            args[label] = value
    return family, args


def load(path):
    """Return (context, families)."""
    with open(path) as handle:
        data = json.load(handle)

    families = OrderedDict()
    bigo = {}

    for row in data["benchmarks"]:
        family, args = parse_run_name(row["run_name"])
        aggregate = row.get("aggregate_name")

        if aggregate in ("BigO", "RMS"):
            entry = bigo.setdefault(family, {})
            if aggregate == "BigO":
                entry["big_o"] = row.get("big_o")
                entry["coefficient"] = row.get("real_coefficient")
                entry["time_unit"] = row.get("time_unit")
            else:
                entry["rms"] = row.get("rms")
            continue

        if aggregate not in STAT_ROWS:
            continue  # raw per-repetition row

        fam = families.setdefault(
            family, {"arg_names": list(args), "points": OrderedDict(), "bigo": None}
        )
        point = fam["points"].setdefault(tuple(args.values()), {"args": args})
        point[aggregate] = row

    for family, fam in families.items():
        fam["bigo"] = bigo.get(family)
        fam["points"] = [fam["points"][k] for k in sorted(fam["points"])]

    return data["context"], families


def metric(point, stat, key):
    row = point.get(stat)
    return None if row is None else row.get(key)


def available_metrics(fam):
    """Which rate columns this family actually reported."""
    return [
        key
        for key in AXIS_LABELS
        if any(metric(p, "mean", key) is not None for p in fam["points"])
    ]


def format_fit(fam):
    big = fam["bigo"]
    if not big:
        return None
    return "fit O(%s)  coeff=%.4g %s  rms=%.1f%%" % (
        big.get("big_o"),
        big.get("coefficient", float("nan")),
        big.get("time_unit", ""),
        100 * big.get("rms", float("nan")),
    )


# --------------------------------------------------------------------------
# keeping a run
# --------------------------------------------------------------------------


def slugify(text):
    """'CPU Baseline!' -> 'cpu-baseline'."""
    slug = re.sub(r"[^a-z0-9]+", "-", str(text).lower()).strip("-")
    return slug or "run"


def make_run_dir(savedir, label, context):
    """Create <savedir>/<date>-<label>/, uniquified if it already exists.

    The date comes from the benchmark JSON itself, not from the clock, so
    re-plotting an old run still files it under the day it was measured.
    The host lives inside results.json; put the machine in the label if you
    benchmark on more than one.
    """
    date = str(context.get("date", ""))[:10] or "undated"
    base = os.path.join(savedir, "%s-%s" % (date, slugify(label)))
    path, suffix = base, 2
    while os.path.exists(path):
        path = "%s-%d" % (base, suffix)
        suffix += 1
    os.makedirs(path)
    return path


# --------------------------------------------------------------------------
# text output
# --------------------------------------------------------------------------


def print_list(context, families):
    print("host   %s" % context.get("host_name"))
    print("date   %s" % context.get("date"))
    print("cpus   %s @ %s MHz" % (context.get("num_cpus"), context.get("mhz_per_cpu")))
    if context.get("cpu_scaling_enabled"):
        print("       cpu_scaling_enabled = true")
    for cache in context.get("caches", []):
        print(
            "cache  L%d %-11s %8.1f KiB  shared by %d"
            % (
                cache["level"],
                cache["type"],
                cache["size"] / 1024,
                cache["num_sharing"],
            )
        )
    print()
    for family, fam in families.items():
        print(
            "%-26s %3d runs   args: %s"
            % (family, len(fam["points"]), ", ".join(fam["arg_names"]) or "(none)")
        )
        fit = format_fit(fam)
        if fit:
            print("%-26s            %s" % ("", fit))


def print_table(family, fam):
    cols = available_metrics(fam)
    headers = list(fam["arg_names"]) + [AXIS_LABELS[c][0] for c in cols] + ["cv%"]

    rows = []
    for point in fam["points"]:
        cells = [str(v) for v in point["args"].values()]
        for col in cols:
            value = metric(point, "mean", col)
            cells.append(
                "-" if value is None else "%.4f" % (value / AXIS_LABELS[col][1])
            )
        cv = metric(point, "cv", "real_time")
        cells.append("-" if cv is None else "%.1f" % (100 * cv))
        rows.append(cells)

    widths = [
        max(len(headers[i]), max((len(r[i]) for r in rows), default=0))
        for i in range(len(headers))
    ]
    header_line = "  ".join(h.rjust(w) for h, w in zip(headers, widths))
    print("\n%s" % family)
    print(header_line)
    print("-" * len(header_line))
    for row in rows:
        print("  ".join(c.rjust(w) for c, w in zip(row, widths)))
    fit = format_fit(fam)
    if fit:
        print(fit)


# --------------------------------------------------------------------------
# plotting
# --------------------------------------------------------------------------


def cache_edges(context):
    """Qubit count at which the statevector first exceeds each cache level."""
    return [
        ("L%d" % c["level"], math.log2(c["size"] / BYTES_PER_AMPLITUDE))
        for c in context.get("caches", [])
        if c.get("type") in ("Data", "Unified")
    ]


def series(fam):
    """One argument -> a single series. More -> last arg is x, the rest label."""
    names = fam["arg_names"]
    if len(names) <= 1:
        return [(None, fam["points"])]

    grouped = OrderedDict()
    for point in fam["points"]:
        values = list(point["args"].values())
        label = ", ".join("%s=%s" % (n, v) for n, v in zip(names[:-1], values[:-1]))
        grouped.setdefault(label, []).append(point)
    return list(grouped.items())


def plot_family(family, fam, context, outdir, show, caches):
    cols = available_metrics(fam)
    if not cols:
        return None

    x_name = fam["arg_names"][-1] if fam["arg_names"] else "run"
    groups = series(fam)

    fig, axes = plt.subplots(
        1, len(cols), figsize=(5.0 * len(cols), 4.0), squeeze=False
    )

    for ax, col in zip(axes[0], cols):
        label, scale = AXIS_LABELS[col]
        for name, points in groups:
            xs, ys, errs = [], [], []
            for point in points:
                value = metric(point, "mean", col)
                if value is None:
                    continue
                xs.append(list(point["args"].values())[-1])
                ys.append(value / scale)
                sd = metric(point, "stddev", col)
                errs.append(0.0 if sd is None else sd / scale)
            if xs:
                ax.errorbar(xs, ys, yerr=errs, marker="o", capsize=3, label=name)

        ax.set_xlabel(x_name)
        ax.set_ylabel(label)
        ax.set_title(label)
        ax.grid(True, which="both", alpha=0.3)
        if col == "real_time":
            ax.set_yscale("log")
        else:
            ax.set_ylim(bottom=0)
        if any(name for name, _ in groups):
            ax.legend(fontsize=8)
        if caches and x_name == "qubits":
            for level, edge in cache_edges(context):
                ax.axvline(edge, color="grey", linestyle="--", alpha=0.5)
                ax.text(
                    edge,
                    ax.get_ylim()[1],
                    level,
                    ha="center",
                    va="bottom",
                    fontsize=8,
                    color="grey",
                )

    title = family
    fit = format_fit(fam)
    if fit:
        title += "   —   " + fit
    fig.suptitle("%s   (%s)" % (title, context.get("host_name", "")))
    fig.tight_layout()

    path = os.path.join(outdir, "%s.png" % family)
    fig.savefig(path, dpi=150)
    if show:
        plt.show()
    plt.close(fig)
    return path


# --------------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("families", nargs="*", help="only these (default: all)")
    parser.add_argument("--results", default=DEFAULT_RESULTS)
    parser.add_argument("--outdir", default=DEFAULT_OUTDIR, help="scratch plot dir")
    parser.add_argument("--save", metavar="LABEL", help="keep this run under a label")
    parser.add_argument("--savedir", default=DEFAULT_SAVEDIR, help="where saved runs go")
    parser.add_argument("--list", action="store_true", help="summarise and exit")
    parser.add_argument("--table", action="store_true", help="print the numbers")
    parser.add_argument("--no-plot", action="store_true")
    parser.add_argument("--show", action="store_true", help="open a window")
    parser.add_argument("--caches", action="store_true", help="mark cache boundaries")
    opts = parser.parse_args()

    context, families = load(opts.results)

    if opts.families:
        missing = [f for f in opts.families if f not in families]
        if missing:
            parser.error(
                "no such family: %s (have: %s)"
                % (", ".join(missing), ", ".join(families))
            )
        families = OrderedDict((f, families[f]) for f in opts.families)

    if opts.list:
        print_list(context, families)
        return

    if opts.table:
        for family, fam in families.items():
            print_table(family, fam)

    if opts.save:
        outdir = make_run_dir(opts.savedir, opts.save, context)
        shutil.copyfile(opts.results, os.path.join(outdir, "results.json"))
        print("keeping run in %s" % outdir)
    else:
        outdir = opts.outdir
        os.makedirs(outdir, exist_ok=True)

    if not opts.no_plot:
        for family, fam in families.items():
            path = plot_family(family, fam, context, outdir, opts.show, opts.caches)
            if path:
                print("wrote %s" % path)


if __name__ == "__main__":
    main()
