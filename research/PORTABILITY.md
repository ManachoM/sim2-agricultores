# Trabajo portable entre macOS y Linux

## Qué queda portable ahora

Documentación en Markdown/CSV/JSON, rutas relativas al repositorio y utilidades Python 3.10+ sin dependencias. El contenedor usa Linux tanto desde Docker Desktop en macOS como desde Docker Engine en Linux. No requiere copiar binarios, `obj/`, `libs_build/` ni rutas `/home/...`.

Esto no garantiza igualdad bit a bit entre arquitecturas. RNG, orden de estructuras, empates de eventos, compilador y bibliotecas deben certificarse antes de réplicas comparables. Una ejecución emulada ARM↔x86 no es un benchmark HPC comparable.

## Entorno aislado secuencial

Desde la raíz:

```sh
cp .env.example .env
mkdir -p runs
export SIM_UID="$(id -u)" SIM_GID="$(id -g)"
docker compose config --quiet
docker compose build simulator
docker compose up -d db
docker compose run --rm simulator python3 tools/research.py prepare --config sim_config_files/sim_config_1.json
docker compose run --rm simulator
python3 tools/research.py snapshot --config runs/local-config.json --output runs/environment.json
docker compose exec -T db pg_dump -U simulator -d continuation -Fc > runs/continuation.dump
docker compose stop
```

`prepare` escribe una configuración local nueva con conexión al servicio `db` y rutas de salida en `runs/`; rechaza sobrescrituras. La configuración 1 usa productores simples y no necesita entradas climáticas; **el horizonte original sigue siendo 13.800 horas** y no es una prueba rápida. Para validar inicialización con pocas entidades puede prepararse `sim_config.json`, que usa ferias y terrenos reducidos. Los agricultores de tipo `risk` sí requieren `frost_file`, `oc_file` y `spi_file` en `Simulation::initialize_agents`.

PostgreSQL usa una base y volumen separados (`continuation`, `continuation_pgdata`), sin publicar puerto en el host. El esquema heredado se aplica solamente a un volumen vacío mediante `docker-entrypoint-initdb.d`. No ejecutar repetidamente `make init_db`: sus INSERT y ALTER no son idempotentes. `docker compose down` conserva el volumen; `down -v` lo borra, por lo que no forma parte del flujo normal.

Las imágenes `ubuntu:24.04` y `postgres:16` y paquetes apt constituyen un entorno de arranque, **no un lock de publicación**. Tras construir y validar, archivar digest de imágenes, `dpkg-query -W`, versiones, `uname -m` y compilación exacta; conservar la imagen o `docker save` fuera de Git. El Dockerfile conserva el Makefile heredado y solo sobreescribe sus rutas y flags al compilar. No pretende reconstruir el entorno 2025 ni cubre BSPlib/MPI.

## Pasar al equipo Linux

1. En el Ubuntu original, **respaldar primero la carpeta de trabajo completa**, incluidos `.git`, archivos modificados, no rastreados e ignorados, además de PostgreSQL por separado. Allí están los terrenos OH y datos/decisiones no commiteados confirmados por el autor. No hacer checkout, pull, reset ni limpieza sobre esa carpeta antes de preservar su estado. Un `git bundle` o un clon no conserva por sí solo los cambios sin commit.
2. Preparar la continuidad mediante `git clone` en **otra carpeta**, sin reutilizar ni sobrescribir el directorio original. En ese clon: `git fetch --all --tags` y `git switch research/interregional-continuation`. Para cada experimento registrar SHA y huellas de las entradas efectivas; documentar filtros y decisiones recuperadas en la bitácora.
3. Ejecutar `check`, pruebas y contenedores en el clon nuevo. `check` valida el snapshot Git registrado, no la entrada local original todavía sin importar: una diferencia de huella en Ubuntu no debe corregirse reemplazando datos originales por los del repositorio. Capturar un `snapshot` en cada máquina y verificar entradas antes de comparar salidas.
4. Copiar por un canal propio `runs/`, datos externos, imágenes archivadas y respaldos `.dump`; verificar `shasum -a 256 archivo` en macOS o `sha256sum archivo` en Linux. Estos archivos y `.env` se excluyen de Git. Un clon por sí solo no transporta PostgreSQL ni datos no versionados.
5. Restaurar un respaldo solo en una **base nueva y vacía**, con `pg_restore --exit-on-error --no-owner --no-privileges`; no usar `--clean` sobre la base original. Comprobar roles, versión y extensiones antes. La copia Docker nueva ya tiene tablas: para un restore integral crear otra base vacía.
6. El autor conserva el entorno **Ubuntu 24**, PostgreSQL y scripts originales. Exportar su estado antes de reinstalar nada: `/etc/os-release`, `git rev-parse HEAD`, `git status --short`, `git diff --binary HEAD`, `git ls-files --others --exclude-standard`, `g++ --version`, `mpirun --version`, `psql --version`, `pg_config --version`, versión libpqxx, `lscpu`, flags de compilación, scripts de lanzamiento y dumps. El diff no incluye archivos no rastreados ni ignorados: conservarlos en el respaldo completo y seleccionar después los científicos para versionar. Revisar secretos antes de compartir o subir archivos.

## Variantes paralelas

Usar checkouts separados para `main-parallel` y `main-parallel-credit`; no mezclar sus objetos con `main`. El Makefile paralelo referencia `libs_build/bsponmpi/bin/bspcxx`, bsponmpi v1.1.1 y libpqxx 7.9.2; requiere además `database/add_proc_id_column.sql`. Recuperar la instalación original antes de escribir un contenedor MPI. `mpirun -np 1` de una variante paralela no debe asumirse idéntico al secuencial.

## Flujo de registros

Antes de experimentar: confirmar D002 → commit limpio → registrar configuración, entradas y entorno → ejecutar → respaldar DB y enlazar execution_id → anotar resultado y decisión. `snapshot` marca explícitamente que el RNG del núcleo aún no se controla. Agregar un número de seed a un manifiesto no cambia el RNG C++.
