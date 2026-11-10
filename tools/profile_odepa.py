"""Perfila CSV ODEPA 2025 sin convertir ni sumar unidades incompatibles."""
import argparse
import csv
import hashlib
import json
from collections import Counter, defaultdict
from decimal import Decimal
from pathlib import Path

PRODUCTS = {"Espinaca", "Tomate", "Zanahoria", "Ajo", "Lechuga"}


def number(value):
    result = Decimal(value.replace(",", "."))
    if not result.is_finite() or result < 0:
        raise ValueError(f"Volumen inválido: {value}")
    return result


def profile(path, kind):
    totals = defaultdict(Decimal)
    positive = Counter()
    rows = nonpositive = selected = 0
    dates = set()
    with path.open(encoding="utf-8-sig", newline="") as stream:
        reader = csv.DictReader(stream)
        required = ({"Fecha", "ID region", "Producto", "Mercado", "Origen", "Unidad de comercializacion", "Volumen"}
                    if kind == "wholesale" else {"Anio", "Mes", "Codigo producto", "Producto", "Pais origen", "Unidad", "Volumen"})
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"Faltan columnas: {sorted(required - set(reader.fieldnames or []))}")
        for row in reader:
            rows += 1
            volume = number(row["Volumen"])
            nonpositive += volume == 0
            if kind == "wholesale":
                if row["ID region"] != "13" or row["Producto"] not in PRODUCTS:
                    continue
                key = (row["Producto"], row["Mercado"], row["Origen"], row["Unidad de comercializacion"])
                dates.add(row["Fecha"])
            else:
                # Capítulo 07 completo: inspección de códigos, no selección automática de frescos.
                if not row["Codigo producto"].startswith("07"):
                    continue
                key = (row["Codigo producto"], row["Producto"], row["Pais origen"], row["Unidad"])
                dates.add(row["Anio"] + "-" + row["Mes"].zfill(2))
            selected += 1
            totals[key] += volume
            positive[key] += volume > 0
    return {
        "source_filename": path.name,
        "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        "kind": kind, "rows": rows, "zero_volume_rows_all_products": nonpositive,
        "selected_rows": selected, "selected_dates": sorted(dates),
        "scope": "RM: five candidate products" if kind == "wholesale" else "Chile imports: HS chapter 07, includes non-fresh products",
        "warning": "Volumes retain original commercial units. No mass conversion, coverage inference or metropolitan allocation performed. Zero rows require closure/missingness audit.",
        "groups": [{"key": list(key), "reported_volume": str(value), "positive_rows": positive[key]}
                   for key, value in sorted(totals.items())],
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("kind", choices=("wholesale", "imports"))
    parser.add_argument("csv", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = profile(args.csv, args.kind)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
