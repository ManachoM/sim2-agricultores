# Bitácora de investigación

## 2026-11-10 — Preparación de la continuidad

- Solicitud: evaluar handoff y publicación final; preparar trabajo portable entre laptop personal y Linux en `ManachoM/sim2-agricultores`.
- Se accedió al repositorio mediante el plugin GitHub y se clonó con Git para disponer de una copia de trabajo completa. Sin cambios en el simulador original.
- Fuentes: handoff v2 preservado en `references/`; artículo final de 27 páginas (DOI y SHA-256 en `baseline.json`); ramas y entradas auditadas; catálogos oficiales y muestras API ODEPA consultados.
- Hallazgos: `main` es secuencial; paralelismo y créditos están en ramas distintas. El mercado mayorista original es un índice, no un almacén físico. Hay parámetros de vencimiento heredados, sin una cadena completa de pérdidas al consumidor. Los precios son entradas, no un equilibrio endógeno.
- Auditoría de cantidades: RM tiene 343 registros de feria y 14.697 puestos; O'Higgins tiene 132 registros, 10.284 puestos y 40.863 terrenos. Debe reconciliarse con sección 6 del artículo (RM 14.375 feriantes / 29.394 consumidores; O'Higgins 4.863 agricultores).
- Entorno observado: cliente Docker instalado, daemon no disponible; `pkg-config` no encuentra libpqxx/libpq. Por ello no se declara ejecución ni reproducción científica local.
- Se agregaron evaluación, plan, registro de decisiones, trazabilidad, candidatos de datos, protocolo piloto y utilidades verificables. Validaciones efectivas y estado remoto: ver entrada de cierre más abajo.
- El autor confirmó Ubuntu 24 y conservación del entorno, PostgreSQL y scripts. Pendiente: exportar patch release/arquitectura, compilador, MPI/BSPlib/libpqxx, configuraciones efectivas, seeds si existen, scripts de figuras y respaldo de los experimentos originales.
- Se descargaron CSV anuales 2025 completos: 202.029 filas mayoristas y 43.843 COMEX. Raw local en `data/raw/`, perfiles y catálogo en Git. No se hizo calibración ni se asignó importación nacional a Santiago.

## 2026-11-10 — Validación y entrega de infraestructura

- Integridad: `python3 tools/research.py check` pasa para 59 archivos candidatos, configuración y huellas raw presentes.
- Utilidades: dos pruebas ejecutables pasan; cubren exclusión de credenciales, rechazo de sobrescrituras, entradas climáticas condicionales, contraseña con caracteres especiales y separación de unidad/origen al perfilar.
- Perfiles ODEPA regenerados desde los CSV conservados en `data/raw/`: idénticos a los JSON versionados. JSON, CSV de trazabilidad, enlaces relativos y conteo de 360 corridas del protocolo comprobados.
- `docker compose --env-file .env.example config --quiet` y `git diff --check` pasan. Se confirmó que no hay cambios en `src/`, `includes/`, `inputs/`, configuraciones heredadas, Makefile ni esquema original.
- [GitHub Actions del primer commit](https://github.com/ManachoM/sim2-agricultores/actions/runs/42853322479) pasó, incluyendo compilación completa del contenedor en Ubuntu 24.04. Tras un aviso de deprecación se actualizó checkout a v7; el resultado de esa revisión queda en los checks del PR.
- Publicación de trabajo: rama `research/interregional-continuation`, [PR #1 en borrador](https://github.com/ManachoM/sim2-agricultores/pull/1), sin merge. El plugin GitHub permitió leer pero devolvió 403 al escribir; la subida se realizó con la sesión SSH del laptop. La rama local sigue la remota y usa SSH para push.
- Límites: no se ejecutó una simulación completa, ni se validó conexión/escritura de una corrida contra PostgreSQL, ni se certificó el entorno de la máquina original. Docker local tiene daemon inactivo. Compilar en CI no demuestra equivalencia experimental ni portabilidad ARM validada.
- Próximo paso: F0, exportación del entorno Ubuntu 24 conservado y reconciliación de las cantidades del control; luego F2, normalización de origen/unidades.

## 2026-11-10 — Aclaración del autor sobre datos locales originales

- El autor indicó que los terrenos OH del trabajo están en el equipo Ubuntu y forman parte de datos y decisiones aún no commiteados. No se ha accedido a esos archivos desde este laptop.
- La comparación de 40.863 filas en Git con 4.863 agricultores en el artículo describe una brecha de versionado del paquete disponible, no evidencia de un error del trabajo publicado. Los conteos del inventario Git se conservan como observación de ese snapshot.
- Se registró D012 y se actualizaron evaluación, plan, catálogo de baseline y guía de traslado. Las observaciones de código/datos Git quedan limitadas a la versión inspeccionada.
- F0 comienza con respaldo completo del directorio Ubuntu, sus cambios locales y archivos ignorados/no rastreados, más exportación PostgreSQL. La continuidad se preparará en un clon separado; no se reemplazarán datos originales para hacer pasar el check del snapshot Git.

## Plantilla de sesión

- Fecha y responsable:
- Objetivo / decisión vinculada (Dxxx):
- Commit / rama / estado limpio o cambios identificados:
- Entradas y manifiesto (`runs/.../environment.json`):
- Acción / comando exacto:
- Resultado, incluyendo negativos:
- Evidencia / artefactos / execution_id de PostgreSQL:
- Qué cambió en nuestra interpretación:
- Próximo paso y condición de avance:

No guardar claves, URLs con contraseña ni datos personales en esta bitácora. Los resultados completos viven fuera de Git cuando son voluminosos; registrar su ubicación, SHA-256 y condiciones de acceso.
