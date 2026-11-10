"""Registros portables de investigación; no ejecuta ni altera el simulador."""
import argparse
import hashlib
import json
import os
import platform
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import quote

ROOT = Path(__file__).resolve().parents[1]
INPUT_KEYS = ("prod_file", "ferias_file", "terrenos_file")


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest() if sys.version_info >= (3, 11) else hashlib.sha256(stream.read()).hexdigest()


def output(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with os.fdopen(os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600), "w", encoding="utf-8") as stream:
        json.dump(value, stream, ensure_ascii=False, indent=2)
        stream.write("\n")


def command(*args):
    try:
        result = subprocess.run(args, cwd=ROOT, text=True, capture_output=True, timeout=10)
        return result.stdout.strip() if result.returncode == 0 else "unavailable"
    except (OSError, subprocess.TimeoutExpired):
        return "unavailable"


def inputs(config):
    paths = [Path(config[key]) for key in INPUT_KEYS]
    if config.get("tipo_agricultor") == "risk":
        paths.extend(Path(config[key]) for key in ("frost_file", "oc_file", "spi_file"))
    return {str(p): digest(ROOT / p) for p in paths}


def inventory():
    result = {}
    for path in sorted((ROOT / "inputs").glob("*.json")):
        data = json.loads(path.read_text())
        entry = {"records": len(data), "sha256": digest(path)}
        if path.stem.startswith("ferias"):
            entry["puestos"] = sum(row["cantidad_puestos"] for row in data)
        if path.stem.startswith("terrenos"):
            entry["area_ha"] = sum(row["area_ha"] for row in data)
        if path.stem.startswith("prods"):
            entry["products"] = [{k: row[k] for k in ("id", "nombre", "unidad")} for row in data]
        result[path.relative_to(ROOT).as_posix()] = entry
    return result


def check():
    baseline = json.loads((ROOT / "research/baseline.json").read_text())
    failures = [name for name, sha in baseline["files_sha256"].items()
                if not (ROOT / name).is_file() or digest(ROOT / name) != sha]
    handoff = baseline["handoff"]
    if digest(ROOT / handoff["path"]) != handoff["sha256"]:
        failures.append(handoff["path"])
    sources = json.loads((ROOT / "research/sources.json").read_text())
    for source in sources["sources"]:
        if "sha256" not in source:
            continue
        profile = json.loads((ROOT / source["profile_path"]).read_text())
        raw = ROOT / source["raw_path"]
        if profile["sha256"] != source["sha256"] or (raw.exists() and digest(raw) != source["sha256"]):
            failures.append(source["id"])
    for path in (ROOT / "sim_config_files").rglob("*.json"):
        config = json.loads(path.read_text())
        try:
            inputs(config)
        except (KeyError, OSError) as exc:
            failures.append(f"{path.relative_to(ROOT)}: {exc}")
    pilot = json.loads((ROOT / "research/experiments/pilot.json").read_text())
    if pilot["status"] != "planned_not_executable":
        failures.append("Actualizar check antes de declarar ejecutable el protocolo piloto")
    if failures:
        raise ValueError("Integridad/entradas: " + "; ".join(failures))
    print(f"OK: {len(baseline['files_sha256'])} archivos candidatos y rutas de configuraciones. Baseline publicado NO certificado.")


def snapshot(path):
    path = Path(path)
    config = json.loads(path.read_text())
    return {
        "recorded_at_utc": datetime.now(timezone.utc).isoformat(),
        "git_commit": command("git", "rev-parse", "HEAD"),
        "git_status": command("git", "status", "--porcelain"),
        "tracked_diff_sha256": hashlib.sha256(command("git", "diff", "HEAD").encode()).hexdigest(),
        "platform": platform.platform(), "machine": platform.machine(),
        "python": platform.python_version(),
        "versions": {name: command(name, "--version").splitlines()[:1]
                     for name in ("g++", "make", "psql", "docker")},
        "config_path": str(path), "config_sha256": digest(path),
        "config_without_connection": {k: v for k, v in config.items() if k != "DB_URL"},
        "inputs_sha256": inputs(config),
        "rng_controlled": False, "seed": None,
        "status": "environment_record_only_not_a_simulation_result",
        "note": "Untracked content is not archived. Commit code first; attach container digest/package inventory and database execution_id to actual runs.",
    }


def prepare(config_path):
    password = os.environ["SIM_DB_PASSWORD"]
    config = json.loads(Path(config_path).read_text())
    inputs(config)
    config["DB_URL"] = f"postgresql://simulator:{quote(password, safe='')}@db:5432/continuation"
    config.update(out_path="./runs/", output_path="./runs/local", output_prefix="local")
    output(ROOT / "runs/local-config.json", config)
    (ROOT / "runs/local-config.json").chmod(0o600)
    (ROOT / "runs/local").mkdir(exist_ok=True)
    print("Configuración local creada; semilla y horizonte heredados sin modificación.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)
    sub.add_parser("check")
    for action in ("inventory", "snapshot", "prepare"):
        item = sub.add_parser(action)
        if action != "inventory":
            item.add_argument("--config", required=True)
        if action != "prepare":
            item.add_argument("--output", required=True)
    args = parser.parse_args()
    os.chdir(ROOT)
    try:
        if args.action == "check":
            check()
        elif args.action == "prepare":
            prepare(args.config)
        else:
            output(args.output, inventory() if args.action == "inventory" else snapshot(args.config))
    except (OSError, ValueError, KeyError) as exc:
        parser.exit(1, f"Error: {exc}\n")


if __name__ == "__main__":
    main()
