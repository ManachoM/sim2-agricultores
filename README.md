# sim2-agricultores

Continuación del simulador publicado en [Future Internet 17(7), 316 (2025)](https://doi.org/10.3390/fi17070316), de Manuel Ignacio Manríquez, Veronica Gil-Costa y Mauricio Marin.

La base C++ original se conserva. La continuidad propuesta estudia abastecimiento interregional e internacional de hortalizas, restricciones logísticas y resiliencia.

- [Evaluación del handoff y propuesta científica](research/REVIEW.md)
- [Datos disponibles, limitaciones y contratos](research/DATA_PLAN.md)
- [Plan de trabajo y criterios de avance](research/PLAN.md)
- [Decisiones](research/DECISIONS.md) y [bitácora](research/LOGBOOK.md)
- [Trazabilidad artículo → código → experimento](research/traceability.csv)
- [Trabajo entre macOS y Linux](research/PORTABILITY.md)

## Comprobación sin instalar bibliotecas científicas

Desde la raíz del repositorio, con Python 3.10 o posterior:

```sh
python3 tools/research.py check
python3 -m unittest discover -s tests
python3 tools/research.py inventory --output runs/inventory.json
python3 tools/research.py snapshot --config sim_config_files/sim_config_1.json --output runs/environment.json
```

`check` verifica la integridad del estado candidato registrado; **no certifica la reproducción del artículo**. `snapshot` registra entorno y huellas de entradas sin guardar la URL de conexión. No ejecuta el simulador ni controla sus semillas.

## Estado

La rama de continuidad parte de `main`; las variantes paralelas permanecen en `main-parallel` y `main-parallel-credit`. Sus SHA están en [baseline.json](research/baseline.json). El autor conserva en Ubuntu 24 los terrenos de O'Higgins y otros datos y decisiones sin commit: Git es un snapshot parcial del paquete experimental. Primero se respaldará ese trabajo local y se identificarán las entradas efectivas. No se creó una etiqueta que afirme una certificación inexistente.

El entorno de contenedores cubre inicialmente la variante secuencial. Las nuevas capas M0/M1/M2 son un protocolo de investigación, todavía no una implementación.
