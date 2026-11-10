import importlib.util
import json
import os
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch


def load(name):
    spec = importlib.util.spec_from_file_location(name, Path(__file__).resolve().parents[1] / "tools" / f"{name}.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


r = load("research")
p = load("profile_odepa")


class ResearchChecks(unittest.TestCase):
    def test_record_does_not_expose_connection_or_overwrite(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "inputs").mkdir()
            (root / "inputs/heatwave_levels.json").write_text("{}")
            (root / "data.json").write_text("[]")
            config = {key: "data.json" for key in r.INPUT_KEYS}
            config["DB_URL"] = "postgresql://user:DO_NOT_RECORD@db/test"
            path = root / "config.json"
            path.write_text(json.dumps(config))
            with patch.object(r, "ROOT", root):
                result = r.snapshot(path)
                self.assertNotIn("DO_NOT_RECORD", json.dumps(result))
                self.assertFalse(result["rng_controlled"])
                self.assertEqual(result["inputs_sha256"]["data.json"], r.digest(root / "data.json"))
                risk = dict(config, tipo_agricultor="risk")
                with self.assertRaises(KeyError):
                    r.inputs(risk)
                risk.update({key: "inputs/heatwave_levels.json" for key in ("frost_file", "oc_file", "spi_file")})
                self.assertIn("inputs/heatwave_levels.json", r.inputs(risk))
                r.output(root / "record.json", result)
                with self.assertRaises(FileExistsError):
                    r.output(root / "record.json", {})
                with patch.dict(os.environ, {"SIM_DB_PASSWORD": "a@b:c"}):
                    r.prepare(path)
                local = json.loads((root / "runs/local-config.json").read_text())
                self.assertIn("a%40b%3Ac@db", local["DB_URL"])

    def test_units_and_origins_stay_separate(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "test.csv"
            path.write_text('Fecha,ID region,Producto,Mercado,Origen,Unidad de comercializacion,Volumen\n'
                            '2025-01-01,13,Tomate,M,A,$/caja 10 kilos,"1,5"\n'
                            '2025-01-01,13,Tomate,M,A,$/kilo,2\n'
                            '2025-01-02,13,Tomate,M,B,$/kilo,0\n'
                            '2025-01-02,6,Tomate,M,A,$/kilo,9\n')
            result = p.profile(path, "wholesale")
            self.assertEqual(result["selected_rows"], 3)
            self.assertEqual(len(result["groups"]), 3)
            self.assertEqual(result["groups"][0]["reported_volume"], "1.5")
            self.assertEqual(result["zero_volume_rows_all_products"], 1)
            with self.assertRaises(ValueError):
                p.number("NaN")
            with self.assertRaises(ValueError):
                p.number("-1")


if __name__ == "__main__":
    unittest.main()
