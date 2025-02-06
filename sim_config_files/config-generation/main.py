"""Script para la generación dinámica de escenarios"""

import json
import itertools


def main():
    # Leer inputs

    with open("../../inputs/prods.json", "r") as file:
        prods = json.load(file)

    cultivos_por_mes = [0] * 12
    productos_por_mes = [0] * 12
    for prod in prods:
        for mes in prod["meses_siembra"]:
            cultivos_por_mes[mes] += 1
        for mes in prod["meses_venta"]:
            productos_por_mes[mes] += 1
    # print(cultivos_por_mes)
    # print(productos_por_mes)

    with open("../../inputs/ferias.json", "r") as file:
        ferias = json.load(file)

    ferias_por_dia = [0] * 7

    for feria in ferias:
        for dia in feria["dias"]:
            ferias_por_dia[dia] += 1

    print(ferias_por_dia)
    # Generar combinaciones
    # Primero, generamos los productos uniformes por mes
    uniform_prods = []
    for prod in prods:
        new_prod = prod
        new_prod["meses_siembra"] = [i for i in range(12)]
        new_prod["meses_venta"] = [i for i in range(12)]
        uniform_prods.append(new_prod)

    uniform_prod_venta = []
    for prod in prods:
        new_prod = prod
        new_prod["meses_venta"] = [i for i in range(12)]
        uniform_prod_venta.append(new_prod)

    with open("../../inputs/uniform_prods.json", "w") as file:
        json.dump(uniform_prods, file)

    with open("../../inputs/uniform_ventas_prods.json", "w") as file:
        json.dump(uniform_prod_venta, file)

    # Luego, generar ferias uniformes
    uniform_ferias = []
    for feria in ferias:
        new_feria = feria
        new_feria["dias"] = [i for i in range(7)]
        uniform_ferias.append(new_feria)

    with open("../../inputs/uniform_ferias.json", "w") as file:
        json.dump(uniform_ferias, file)

    augmented_uniform_ferias = []
    for feria in ferias:
        new_feria = feria
        new_feria["dias"] = [i for i in range(7)]
        new_feria["cantidad_puestos"] = 50
        augmented_uniform_ferias.append(new_feria)

    with open("../../inputs/augmented_uniform_ferias.json", "w") as file:
        json.dump(augmented_uniform_ferias, file)

    big_augmented_uniform_feria = []
    for feria in augmented_uniform_ferias:
        new_feria = feria
        new_feria["cantidad_puestos"] = 100
        big_augmented_uniform_feria.append(new_feria)

    with open("../../inputs/big_augmented_ferias.json", "w") as file:
        json.dump(big_augmented_uniform_feria, file)

    tipos_feriante = ("static", "seasonal", "seasonal-proportional")
    tipos_agricultor = (
        "simple",
        "ganancia",
    )
    tipos_consumidor = ("family", "family_budget", "single")
    cons_to_fer_ratio = (2, 4, 8, 16, 32)

    prod_files = (
        "./inputs/prods.json",
        "./inputs/uniform_prods.json",
        "./inputs/uniform_ventas_prods.json",
    )
    ferias_files = (
        "./inputs/uniform_ferias.json",
        "./inputs/augmented_uniform_ferias.json",
        "./inputs/big_augmented_ferias.json",
    )
    i = 1
    # Guardar nuevos archivos
    for (
        tipo_feriante,
        tipo_agr,
        tipo_cons,
        cons_ratio,
        prod,
        feria,
    ) in itertools.product(
        tipos_feriante,
        tipos_agricultor,
        tipos_consumidor,
        cons_to_fer_ratio,
        prod_files,
        ferias_files,
    ):
        config = {
            "out_path": "./out/",
            "prod_file": prod,
            "feriantes_prods_amount": 6,
            "ferias_file": feria,
            "terrenos_file": "./inputs/terrenos.json",
            "debug_level": "N",
            "consumidor_feriante_ratio": cons_ratio,
            "output_path": "./out/sim_synth_1",
            "output_prefix": "sim_synth_1",
            "tipo_feriante": tipo_feriante,
            "tipo_consumidor": tipo_cons,
            "tipo_agricultor": tipo_agr,
            "DB_URL": "postgresql://postgres:secret@localhost:5432/sim-db",
        }

        filename = f"../../sim_config_files/synth_sim_config_{i}.json"
        with open(filename, "w+") as file:
            json.dump(config, file)
        i += 1


if __name__ == "__main__":
    main()
