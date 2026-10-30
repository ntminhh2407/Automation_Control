"""Plot tilt-angle measurements recorded in data.txt."""

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


SERIES = (
    ("roll_acc", "tab:blue"),
    ("roll_filtered", "tab:orange"),
    ("pitch_acc", "tab:green"),
    ("pitch_filtered", "tab:red"),
)


def load_data(data_path: Path) -> dict[str, list[float]]:
    """Load sample numbers and angle series from the CSV data file."""
    with data_path.open(newline="", encoding="utf-8") as data_file:
        reader = csv.DictReader(data_file)
        required_columns = {"n", *(name for name, _ in SERIES)}
        if reader.fieldnames is None or not required_columns.issubset(reader.fieldnames):
            missing_columns = required_columns.difference(reader.fieldnames or ())
            missing = ", ".join(sorted(missing_columns))
            raise ValueError(f"Missing required CSV column(s): {missing}")

        data = {column: [] for column in ("n", *(name for name, _ in SERIES))}
        for row_number, row in enumerate(reader, start=2):
            try:
                for column in data:
                    data[column].append(float(row[column]))
            except (TypeError, ValueError) as error:
                raise ValueError(
                    f"Invalid numeric value at CSV row {row_number}"
                ) from error

    if not data["n"]:
        raise ValueError("The CSV data file does not contain any samples")
    return data


def plot_data(data: dict[str, list[float]]) -> None:
    """Plot all measured angle series against their sample number."""
    figure, axis = plt.subplots(figsize=(11, 6))
    for column, color in SERIES:
        axis.plot(data["n"], data[column], color=color, linewidth=1.5, label=column)

    axis.set_title("Tilt angle by sample")
    axis.set_xlabel("Sample number (n)")
    axis.set_ylabel("Tilt angle (degrees)")
    axis.grid(True, alpha=0.3)
    axis.legend()
    figure.tight_layout()
    plt.show()


def main() -> None:
    parser = argparse.ArgumentParser(description="Plot tilt-angle data.")
    parser.add_argument(
        "data_file",
        nargs="?",
        type=Path,
        default=Path(__file__).parent / "src" / "data.txt",
        help="CSV file to plot (default: src/data.txt)",
    )
    args = parser.parse_args()
    plot_data(load_data(args.data_file))


if __name__ == "__main__":
    main()
