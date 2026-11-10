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
