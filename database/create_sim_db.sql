CREATE TABLE IF NOT EXISTS "execution_params" (
  "execution_id" integer,
  "param_name" varchar,
  "param_value" varchar
);

CREATE TABLE IF NOT EXISTS "execution" (
  "execution_id" serial PRIMARY KEY,
  "execution_status_id" integer,
  "start_time" timestamp,
  "end_time" timestamp,
  "execution_time" DOUBLE PRECISION
);

CREATE TABLE IF NOT EXISTS "aggregated_product_results" (
  "execution_id" integer,
  "process" varchar,
  "variable_type" varchar,
  "time_granularity" varchar,
  "time" integer,
  "product_id" integer,
  "value" DOUBLE PRECISION
);

CREATE TABLE IF NOT EXISTS "execution_status" (
  "execution_status_id" integer PRIMARY KEY unique,
  "status" varchar not null
);

CREATE TABLE IF NOT EXISTS "aggregated_agent_results" (
  "execution_id" integer,
  "agent_type" varchar,
  "variable_type" varchar,
  "time_granularity" varchar,
  "time" integer,
  "agent_id" integer,
  "value" DOUBLE PRECISION
);

INSERT INTO
  execution_status (execution_status_id, status)
VALUES
  (0, 'REGISTRADA'),
  (1, 'EN EJECUCIÓN'),
  (2, 'TERMINADA'),
  (3, 'ERROR');

COMMENT ON COLUMN "execution"."execution_time" IS 'Tiempo de ejecución medido en el mismo proceso.';

ALTER TABLE
  "execution_params"
ADD
  FOREIGN KEY ("execution_id") REFERENCES "execution" ("execution_id");

ALTER TABLE
  "aggregated_product_results"
ADD
  FOREIGN KEY ("execution_id") REFERENCES "execution" ("execution_id");

ALTER TABLE
  "aggregated_agent_results"
ADD
  FOREIGN KEY ("execution_id") REFERENCES "execution" ("execution_id");

ALTER TABLE
  "execution"
ADD
  FOREIGN KEY ("execution_status_id") REFERENCES "execution_status" ("execution_status_id");

--- Creamos tablas con contenido de la simulación
CREATE TABLE IF NOT EXISTS "product" (
  "product_id" integer PRIMARY KEY,
  "nombre" varchar UNIQUE,
  "meses_siembra" integer ARRAY,
  "meses_venta" integer ARRAY,
  "dias_cosecha" DOUBLE PRECISION,
  "unidad" varchar,
  "unit_ha" DOUBLE PRECISION,
  "volumen_feriante" DOUBLE PRECISION,
  "volumen_un_consumidor" DOUBLE PRECISION,
  "prob_consumir" DOUBLE PRECISION,
  "heladas" DOUBLE PRECISION ARRAY,
  "oc" DOUBLE PRECISION ARRAY,
  "sequias" DOUBLE PRECISION ARRAY,
  "plagas" DOUBLE PRECISION ARRAY,
  "prob_compra_feriante" DOUBLE PRECISION
);

INSERT INTO product (
    product_id,
    nombre,
    meses_siembra,
    meses_venta,
    dias_cosecha,
    unidad,
    unit_ha,
    volumen_feriante,
    volumen_un_consumidor,
    prob_consumir,
    heladas,
    oc,
    sequias,
    plagas,
    prob_compra_feriante
) VALUES
(0, 'Ajo', ARRAY[3, 4, 5, 6, 7], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 180.0, 'unidades', 194800.0, 180.0, 6.0, 0.64, ARRAY[1.0, 1.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.03445717033079672),
(1, 'Alcachofa', ARRAY[1, 2], ARRAY[4, 5, 6, 7, 8, 9, 10], 90.0, 'unidades', 49390.0, 300.0, 6.0, 0.34, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.0034049053429333104),
(2, 'Apio', ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 70.0, 'unidades', 37270.0, 36.0, 1.0, 0.37, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.03491494404156389),
(3, 'Arveja Verde', ARRAY[4, 5, 6, 7], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 90.0, 'kilos', 6690.0, 15.0, 1.0, 0.23, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.02174037182321378),
(4, 'Brócoli', ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 90.0, 'unidades', 35000.0, 300.0, 4.0, 0.36, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], 0.04561697958266661),
(5, 'Choclo', ARRAY[8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 120.0, 'unidades', 46200.0, 500.0, 16.0, 0.57, ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.09659428758762899),
(6, 'Coliflor', ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 130.0, 'unidades', 30000.0, 100.0, 2.0, 0.4, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], 0.1363249800309577),
(7, 'Espinaca', ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 90.0, 'kilos', 16800.0, 51.0, 2.0, 0.26, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.06389297838257009),
(8, 'Haba', ARRAY[3, 4, 5], ARRAY[0, 4, 5, 6, 7, 8, 9, 10, 11], 90.0, 'kilos', 12600.0, 20.0, 1.0, 0.27, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.228972203024576),
(9, 'Lechuga', ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 90.0, 'unidades', 44020.0, 48.0, 8.0, 0.9, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.05877232530516449),
(10, 'Melón', ARRAY[0, 1, 8, 9, 10, 11], ARRAY[0, 1, 2, 11], 120.0, 'unidades', 25190.0, 1000.0, 3.0, 0.29, ARRAY[3.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.0010778941524511602),
(11, 'Pepino ensalada', ARRAY[8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 120.0, 'unidades', 119000.0, 120.0, 12.0, 0.43, ARRAY[3.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.016261053271624356),
(12, 'Poroto granado', ARRAY[0, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 11], 80.0, 'kilos', 7440.0, 66.0, 1.0, 0.2, ARRAY[3.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.07569136656768123),
(13, 'Poroto verde', ARRAY[0, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 80.0, 'kilos', 7770.0, 44.0, 1.0, 0.57, ARRAY[3.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.009220719312794827),
(14, 'Repollo', ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 100.0, 'unidades', 24910.0, 6.0, 1.0, 0.64, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 3.0, 3.0], ARRAY[2.0, 2.0, 3.0], 0.0004613004727915714),
(15, 'Sandia', ARRAY[0, 1, 8, 9, 10, 11], ARRAY[0, 1, 2, 11], 120.0, 'unidades', 9070.0, 50.0, 2.0, 0.19, ARRAY[3.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.015537429418048265),
(16, 'Tomate', ARRAY[7, 8], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 90.0, 'kilos', 61870.0, 1800.0, 6.0, 0.92, ARRAY[3.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 3.0, 3.0], ARRAY[2.0, 2.0, 3.0], 0.007136055922990994),
(17, 'Zanahoria', ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 120.0, 'kilos', 31390.0, 102.0, 2.0, 0.86, ARRAY[1.0, 2.0, 2.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], 0.09795243783444108),
(18, 'Zapallo italiano', ARRAY[0, 8, 9, 10, 11], ARRAY[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11], 100.0, 'unidades', 90000.0, 180.0, 4.0, 0.65, ARRAY[3.0, 3.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], 0.045426669861010724),
(19, 'Frutilla', ARRAY[3, 4], ARRAY[0, 1, 2, 3, 4, 9, 10, 11], 120.0, 'kilos', 30000.0, 1050.0, 8.0, 0.62, ARRAY[1.0, 2.0, 3.0], ARRAY[1.0, 2.0, 3.0], ARRAY[2.0, 3.0, 3.0], ARRAY[2.0, 2.0, 3.0], 0.006543927734094409);


ALTER TABLE aggregated_product_results
ALTER TABLE
  "aggregated_product_results"
ADD
  FOREIGN KEY ("product_id") REFERENCES "product" ("product_id");