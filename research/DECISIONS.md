# Registro de decisiones

Fecha inicial: 2026-11-10, America/Santiago. Las decisiones metodológicas siguientes son propuestas operativas revisables; no constituyen aprobación del investigador ni prerregistro externo.

| ID | Estado | Decisión | Evidencia / motivo | Reabrir cuando |
|---|---|---|---|---|
| D001 | Adoptada para esta preparación | Conservar C++ y las tres variantes; agregar infraestructura sin cambiar semántica | Código y ramas existentes | Una regresión demuestre necesidad de cambiar el núcleo |
| D002 | Pendiente | Identificar commits, configuraciones, datos y entorno efectivamente usados en 2025 antes de etiquetar `paper-2025-baseline` | Discrepancias de cantidades y RNG; ver REVIEW | Se recuperen artefactos Linux y se reproduzcan resultados |
| D003 | Propuesta | Primer artículo sobre diversificación, logística y suficiencia de modelos M0/M1/M2 | Una pregunta falsable reduce dispersión del handoff | Revisión de literatura y piloto |
| D004 | Propuesta | Internacional agregado en la evaluación de datos; detalle portuario/fronterizo condicionado | ODEPA COMEX tiene país/mes/producto; no destino metropolitano ni tiempo de despacho | Evidencia de participación relevante y datos de tiempos/capacidad |
| D005 | Reemplazada por D011 | Terna doméstica inicial: espinaca, tomate y zanahoria; ajo candidato internacional | Inventario heredado, anterior al perfil de flujos | Perfil anual completado en esta sesión |
| D006 | Propuesta | Abastecimiento fijo y FEFO antes de elección adaptativa o logit | No hay observaciones que identifiquen coeficientes de utilidad | Datos conductuales o necesidad demostrada por sensibilidad |
| D007 | Propuesta | A0 secuencial y A1 réplicas independientes antes de nuevos algoritmos HPC | El costo por réplica aún no está medido en esta continuación | Perfil y tiempo a precisión insuficientes con A1 |
| D008 | Adoptada para esta preparación | Python stdlib para huellas, inventarios y pruebas; Linux en contenedor para ejecución inicial | macOS sin libpqxx/libpq; rutas originales Linux | Se valide una compilación nativa adicional |
| D009 | Propuesta | Comparación de fidelidad por intervalos de equivalencia, además de diferencias | Una diferencia no significativa no prueba equivalencia | Justificación de márgenes con actores/datos |
| D010 | Propuesta | No convertir producción regional o importación nacional directamente en oferta de Santiago | Coberturas y etapas distintas | Matriz de asignación observada o supuesto explícito con sensibilidad |
| D011 | Propuesta, reemplaza D005 | Tomate principal y zanahoria contraste interregional; espinaca control local; ajo extensión internacional | Perfil RM 2025: espinaca solo local; ajo chino observado; COMEX identifica importaciones frescas | Normalización completa y revisión de cobertura |
| D012 | Adoptada por aclaración del autor | Tratar Git como snapshot parcial del trabajo publicado; recuperar terrenos OH y datos/decisiones no commiteados desde Ubuntu antes de fijar el control | Aclaración del autor del 2026-11-10; los archivos locales aún no se inspeccionaron | Se respalde y documente el paquete experimental original |

## Plantilla para una nueva decisión

### D013 — Título

- Fecha y responsable:
- Estado: propuesta / adoptada / rechazada / reemplazada por Dxxx.
- Pregunta y alternativas:
- Evidencia: fuente, commit, dataset o experimento.
- Decisión y razón:
- Supuestos / consecuencias:
- Experimento que puede refutarla y criterio para reabrir:

Agregar nuevas entradas; preservar las anteriores y señalar reemplazos. Las discusiones pueden vivir en GitHub, pero la decisión y su evidencia deben quedar aquí.
