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

INSERT INTO execution_status (execution_status_id, status) VALUES (0, 'REGISTRADA'), (1, 'EN EJECUCIÓN'), (2, 'TERMINADA'), (3, 'ERROR');

COMMENT ON COLUMN "execution"."execution_time" IS 'Tiempo de ejecución medido en el mismo proceso.';

ALTER TABLE "execution_params" ADD FOREIGN KEY ("execution_id") REFERENCES "execution" ("execution_id");

ALTER TABLE "aggregated_product_results" ADD FOREIGN KEY ("execution_id") REFERENCES "execution" ("execution_id");

ALTER TABLE "aggregated_agent_results" ADD FOREIGN KEY ("execution_id") REFERENCES "execution" ("execution_id");

ALTER TABLE "execution" ADD FOREIGN KEY ("execution_status_id") REFERENCES "execution_status" ("execution_status_id");
