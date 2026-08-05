"""Explore Google Benchmark results.

    ./.venv/bin/python scripts/plot_benchmarks.py --list
    ./.venv/bin/python scripts/plot_benchmarks.py --table --no-plot
    ./.venv/bin/python scripts/plot_benchmarks.py
    ./.venv/bin/python scripts/plot_benchmarks.py BM_ghzState --show
    ./.venv/bin/python scripts/plot_benchmarks.py --overlay --caches
    ./.venv/bin/python scripts/plot_benchmarks.py --save cpu-baseline

Reads the JSON written by `cmake --build ... -t bench` and turns every
benchmark family into a table and a plot, whatever arguments it was
registered with.

--overlay additionally draws every family that is measured against qubit
count onto one set of axes, so a ceiling family (see --reference) can be
read against the curves it bounds.

Every run also writes numbers.md next to the plots: all measured rows plus
derived ns-per-iteration and approximate cycles, so the benchmark log never
cites a number this script cannot regenerate.

Without --save, plots land in a scratch directory and are overwritten every
run.  With --save LABEL the run is kept: a dated folder is created holding
the JSON and every plot generated from it.
"""

# --------------------------------------------------------------------------
# The whole workflow, in order.  Run everything from the project root.
#
# 1. measure -> benchmarks/results/latest.json   (takes a few minutes)
#    Needs a Release configure with QSIM_BENCHMARK=ON, otherwise the
#    `bench` target does not exist at all.
#    cmake --build build --target bench
#
# 2. what is in the run
#    ./.venv/bin/python scripts/plot_benchmarks.py --list
#
# 3. the raw numbers
#    ./.venv/bin/python scripts/plot_benchmarks.py --table --no-plot
#
# 4. plots, including the roofline comparison
#    ./.venv/bin/python scripts/plot_benchmarks.py --overlay --caches
#
# 5. same, but keep this run under a label
#    ./.venv/bin/python scripts/plot_benchmarks.py --overlay --caches --save cpu-baseline
#
# Add --show to any of them to open a window instead of only writing files.
# --------------------------------------------------------------------------

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

# The family that measures the memory ceiling.  Curves in the overlay are
# expressed as a percentage of it when it is present.
DEFAULT_REFERENCE = "BM_roofline"

# Comparing families on shared axes only means something when their x axis
# means the same thing; every family in the overlay must end in this argument.
OVERLAY_X = "qubits"
OVERLAY_METRIC = "bytes_per_second"

# Per-run aggregates google/benchmark emits when --benchmark_repetitions > 1.
STAT_ROWS = ("mean", "median", "stddev", "cv")

AXIS_LABELS = {
    "real_time": ("time (ns)", 1.0),
    "bytes_per_second": ("GB/s", 1e9),
    "items_per_second": ("G items/s", 1e9),
}

# SetItemsProcessed() counts whatever the benchmark decided an "item" is, and
# that differs between families.  Name it per family so the plots say what the
# rate is a rate *of*; anything not listed falls back to DEFAULT_ITEM_LABEL.
DEFAULT_ITEM_LABEL = "G amplitudes/s"
ITEM_LABELS = {
    "BM_getPairIndices": "G index pairs/s",
}

# Innermost-loop iterations one benchmark call performs.  Time per iteration
# is the only rate that is comparable across families: bytes_per_second and
# items_per_second both follow per-family conventions (amplitudes touched,
# pairs, amplitudes updated).  Families without an entry get "-" in the
# derived columns rather than a guessed number.
ITERATIONS_PER_CALL = {
    "BM_applySingleQubitGate": lambda args: 2 ** (args["qubits"] - 1),
    "BM_applySingleQubitGateTargetSweep": lambda args: 2 ** (args["qubits"] - 1),
    "BM_applyCnotGate": lambda args: 2 ** (args["qubits"] - 1),
    "BM_ghzState": lambda args: args["qubits"] * 2 ** (args["qubits"] - 1),
    "BM_getPairIndices": lambda args: 2 ** (args["qubits"] - 1),
    "BM_roofline": lambda args: 2 ** args["qubits"],
}

# What a single iteration does and how many one call performs, printed into
# the report so the caveats and the arithmetic travel with the numbers
# instead of living in the log's prose.
ITERATION_NOTES = {
    "BM_applySingleQubitGate": "one pair: 2 loads, 2x2 complex mat-vec, "
    "2 stores; 2^(qubits-1) iterations per call",
    "BM_applySingleQubitGateTargetSweep": "one pair: same loop, varying "
    "target bit; 2^(qubits-1) iterations per call",
    "BM_applyCnotGate": "one pair: index calc and control test, swap on half "
    "of them; 2^(qubits-1) iterations per call",
    "BM_ghzState": "one pair inside one of the circuit's `qubits` gates; "
    "qubits * 2^(qubits-1) iterations per call",
    "BM_getPairIndices": "one index calculation; upper bound, dominated by "
    "the DoNotOptimize harness cost; 2^(qubits-1) iterations per call",
    "BM_roofline": "one amplitude (not a pair): load, scale by a double, "
    "store; 2^qubits iterations per call",
}


def label_for(family, col):
    """Column heading for one metric of one family."""
    if col == "items_per_second":
        return ITEM_LABELS.get(family, DEFAULT_ITEM_LABEL)
    return AXIS_LABELS[col][0]


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


def table_rows(family, fam, context):
    """Headers and formatted cells, shared by --table and the numbers report.

    Values are medians across repetitions; cv% is the spread.  ns/iter is
    real_time divided by the family's iteration count; cycles/iter uses the
    nominal mhz_per_cpu from the context and is therefore approximate.
    """
    cols = available_metrics(fam)
    iters = ITERATIONS_PER_CALL.get(family)
    mhz = context.get("mhz_per_cpu")

    headers = (
        list(fam["arg_names"])
        + [label_for(family, c) for c in cols]
        + ["iters/call", "ns/iter", "cycles/iter", "cv%"]
    )

    rows = []
    for point in fam["points"]:
        cells = [str(v) for v in point["args"].values()]
        for col in cols:
            value = metric(point, "median", col)
            cells.append(
                "-" if value is None else "%.4f" % (value / AXIS_LABELS[col][1])
            )

        time = metric(point, "median", "real_time")
        if iters is None or time is None:
            cells += ["-", "-", "-"]
        else:
            count = iters(point["args"])
            ns_per_iter = time / count
            cells.append(str(count))
            cells.append("%.3f" % ns_per_iter)
            cells.append("-" if not mhz else "%.2f" % (ns_per_iter * mhz / 1000.0))

        cv = metric(point, "cv", "real_time")
        cells.append("-" if cv is None else "%.1f" % (100 * cv))
        rows.append(cells)

    return headers, rows


def print_table(family, fam, context):
    headers, rows = table_rows(family, fam, context)
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


def write_numbers_report(context, families, outdir):
    """numbers.md next to the plots: every measured row plus derived columns.

    The file exists so the benchmark log never cites a number the tooling
    cannot regenerate.
    """
    lines = [
        "# Benchmark numbers",
        "",
        "Generated by scripts/plot_benchmarks.py -- do not edit by hand.",
        "",
        "- host: %s" % context.get("host_name"),
        "- date: %s" % context.get("date"),
        "- cpus: %s @ %s MHz (nominal)"
        % (context.get("num_cpus"), context.get("mhz_per_cpu")),
        "- cpu_scaling_enabled: %s" % context.get("cpu_scaling_enabled"),
    ]
    for cache in context.get("caches", []):
        lines.append(
            "- cache: L%d %s, %.1f KiB, shared by %d"
            % (
                cache["level"],
                cache["type"],
                cache["size"] / 1024,
                cache["num_sharing"],
            )
        )
    lines += [
        "",
        "Values are medians across repetitions; cv% is the spread.",
        "`ns/iter` is time per innermost-loop iteration, the only column that",
        "is comparable across families -- GB/s and items/s follow per-family",
        "conventions.  `cycles/iter` multiplies it by the nominal clock above;",
        "frequency scaling was enabled, so cycle counts are approximate.",
        "",
        "What one iteration does:",
        "",
    ]
    for family in families:
        note = ITERATION_NOTES.get(family)
        if note:
            lines.append("- `%s`: %s" % (family, note))
    lines.append("")

    for family, fam in families.items():
        headers, rows = table_rows(family, fam, context)
        if not rows:
            continue
        lines.append("## %s" % family)
        lines.append("")
        lines.append("| " + " | ".join(headers) + " |")
        lines.append("|" + "|".join(" ---: " for _ in headers) + "|")
        for row in rows:
            lines.append("| " + " | ".join(row) + " |")
        fit = format_fit(fam)
        if fit:
            lines.append("")
            lines.append(fit)
        lines.append("")

    path = os.path.join(outdir, "numbers.md")
    with open(path, "w") as handle:
        handle.write("\n".join(lines) + "\n")
    return path


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


def draw_cache_lines(ax, context):
    """Mark where the statevector outgrows each cache level.

    Only meaningful on axes whose x is a qubit count *and* whose family
    actually allocates a statevector -- BM_getPairIndices does neither.
    """
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
        label, scale = label_for(family, col), AXIS_LABELS[col][1]
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
            draw_cache_lines(ax, context)

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
# overlay: several families on one set of axes
# --------------------------------------------------------------------------


def overlay_curves(families):
    """[(label, xs, ys)] in GB/s for every family measured against qubits.

    Families whose last argument is not a qubit count are skipped: putting
    them on shared axes would silently compare two different x meanings.
    Families that never called SetBytesProcessed() drop out too, because
    they produce no points.
    """
    scale = AXIS_LABELS[OVERLAY_METRIC][1]
    curves = []
    for family, fam in families.items():
        if not fam["arg_names"] or fam["arg_names"][-1] != OVERLAY_X:
            continue
        for name, points in series(fam):
            xs, ys = [], []
            for point in points:
                value = metric(point, "mean", OVERLAY_METRIC)
                if value is None:
                    continue
                xs.append(list(point["args"].values())[-1])
                ys.append(value / scale)
            if xs:
                label = family if not name else "%s  %s" % (family, name)
                curves.append((label, xs, ys))
    return curves


def plot_overlay(families, context, outdir, show, caches, reference):
    """Throughput of every qubit-indexed family on one axes.

    When the reference family is present a second panel expresses each curve
    as a percentage of it -- that ratio, not the absolute GB/s, is what says
    how much of the machine a kernel is actually using.

    Read the percentage panel only where the curves have flattened out: at
    very small qubit counts the reference loop is dominated by its own loop
    overhead rather than by memory, so it is not a real ceiling there.
    """
    curves = overlay_curves(families)
    if not curves:
        return None

    ceiling = None
    for label, xs, ys in curves:
        if label == reference:
            ceiling = dict(zip(xs, ys))
            break

    panels = 2 if ceiling else 1
    fig, axes = plt.subplots(1, panels, figsize=(5.5 * panels, 4.2), squeeze=False)

    ax = axes[0][0]
    for label, xs, ys in curves:
        is_reference = label == reference
        ax.plot(
            xs,
            ys,
            marker="o",
            markersize=4,
            linewidth=2.0 if is_reference else 1.5,
            linestyle="--" if is_reference else "-",
            color="black" if is_reference else None,
            label=label,
        )
    ax.set_xlabel(OVERLAY_X)
    ax.set_ylabel(AXIS_LABELS[OVERLAY_METRIC][0])
    ax.set_title("throughput")
    ax.set_ylim(bottom=0)
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(fontsize=8)
    if caches:
        draw_cache_lines(ax, context)

    if ceiling:
        ax = axes[0][1]
        for label, xs, ys in curves:
            if label == reference:
                continue
            shared = [(x, y) for x, y in zip(xs, ys) if ceiling.get(x)]
            if not shared:
                continue
            ax.plot(
                [x for x, _ in shared],
                [100.0 * y / ceiling[x] for x, y in shared],
                marker="o",
                markersize=4,
                label=label,
            )
        ax.axhline(100.0, color="black", linestyle="--", linewidth=2.0)
        ax.set_xlabel(OVERLAY_X)
        ax.set_ylabel("%% of %s" % reference)
        ax.set_title("fraction of the ceiling")
        ax.set_ylim(bottom=0)
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(fontsize=8)
        if caches:
            draw_cache_lines(ax, context)
    else:
        print(
            "overlay: no %s family in this run, plotting absolutes only" % reference
        )

    fig.suptitle("family comparison   (%s)" % context.get("host_name", ""))
    fig.tight_layout()

    path = os.path.join(outdir, "overlay.png")
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
    parser.add_argument(
        "--overlay",
        action="store_true",
        help="also draw all qubit-indexed families on shared axes",
    )
    parser.add_argument(
        "--reference",
        default=DEFAULT_REFERENCE,
        help="family treated as the ceiling in the overlay",
    )
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
            print_table(family, fam, context)

    if opts.save:
        outdir = make_run_dir(opts.savedir, opts.save, context)
        shutil.copyfile(opts.results, os.path.join(outdir, "results.json"))
        print("keeping run in %s" % outdir)
    else:
        outdir = opts.outdir
        os.makedirs(outdir, exist_ok=True)

    report = write_numbers_report(context, families, outdir)
    print("wrote %s" % report)

    if not opts.no_plot:
        for family, fam in families.items():
            path = plot_family(family, fam, context, outdir, opts.show, opts.caches)
            if path:
                print("wrote %s" % path)
        if opts.overlay:
            path = plot_overlay(
                families, context, outdir, opts.show, opts.caches, opts.reference
            )
            if path:
                print("wrote %s" % path)


if __name__ == "__main__":
    main()
