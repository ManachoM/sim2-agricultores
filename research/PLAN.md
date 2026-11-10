# Plan de continuidad y protocolo inicial

Estado: propuesta del 2026-11-10; no hay resultados de nuevos experimentos. La primera salida es un artículo científico; un artículo HPC es condicional. Las fases no son estimaciones contractuales: tiempos y presupuesto se ajustan después del perfil de ejecución.

## Pregunta y alcance mínimo

¿En qué condiciones la diversificación de orígenes reduce demanda no satisfecha y cuándo la capacidad de descarga, el viaje y el envejecimiento neutralizan ese efecto?

Mercado focal: Santiago. Tomate como caso interregional principal; zanahoria como contraste; espinaca como control local observado. Un hub inicial, dos orígenes externos seleccionados tras limpieza y oferta RM. Ajo como caso internacional adicional sujeto a normalización de kg/unidad y asignación a Santiago; no hacerlo requisito del primer resultado doméstico. La versión nacional no debe extrapolarse a las importaciones sin ese experimento.

No se implementan en esta preparación nuevas reglas C++, modelos de elección, routing, predictores o algoritmos de sincronización. `experiments/pilot.json` especifica un diseño futuro y no puede entregarse al ejecutable actual como si fuera una configuración válida.

## Fases y puertas

| Fase | Trabajo y archivos afectados en la futura implementación | Entradas / artefactos | Comprobación y puerta | Dependencia y vuelta atrás |
|---|---|---|---|---|
| F0 Recuperar control | Exportar entorno Ubuntu 24 conservado, scripts, configs, DB y SHA de cada rama; sin alterar instalación | Baseline candidato, dumps, figuras y scripts originales → paquete congelado y discrepancias resueltas | Reproducir una configuración RM y una OH; explicar cantidades y métricas. No se etiqueta publicado hasta resolverlo | Primera fase. Si no se recupera exactamente, declarar réplica reconstruida y su diferencia |
| F1 Reproducibilidad y observabilidad | `Simulation::run`, `HeapFEL`, comparador, inicializadores y todos los RNG/callers; monitor y ramas paralelas por separado | Control F0 → seeds efectivas, trazas pequeñas, contadores/timers y regresiones | Replay determinista pequeño; horario y balances; comparar viejo/nuevo perfil con semillas/control documentados | F0. Cambios que alteren ciencia se separan de instrumentación; conservar checkout original |
| F2 Datos y frontera | Scripts de transformación, catálogo, diccionario de orígenes/unidades y perfiles | ODEPA mayoristas/COMEX, insumos heredados → series limpias con cobertura y splits temporales | Conversiones y duplicados revisados, totales contra fuente; destino/stock sin doble conteo; incertidumbre explícita | Puede avanzar junto a F0. Sin datos de servicio, mantener estudio de mecanismos |
| F3 M0/M1 | `mercado_mayorista`, `feriante`, inventarios, configuración y registro de métricas; preservar factories | Oferta observada/escenario, stocks y lead time → modos comparables | Oferta adicional cero recupera control bajo semántica heredada; stock no negativo y mismo presupuesto/oferta entre políticas | F1–F2. Mantener original como modo/checkout de control |
| F4 M2 mínimo | Lote/envío y eventos dentro del FEL existente: despacho, llegada, cola, descarga, habilitación; FEFO y vencimiento | F3, servicio y vida útil por producto → red manual y trazas | Un camión/lote no se duplica; un andén no atiende dos a la vez; balances y edad; si capacidad infinita, converger a M1 con iguales tiempos | F3. Si no aporta, conservar M1 y documentar suficiencia |
| F5 Piloto y confirmación | Diseño de escenarios, launcher de procesos independientes y scripts de análisis | Piloto versionado → varianza pareada, tamaño confirmatorio, IC/equivalencia y curvas | Resultado primario y umbrales fijados antes de confirmación; escenarios de validación separados; no seleccionar solo resultados favorables | F4. Si IC ancho, más precisión o conclusión inconclusa; si equivalentes, publicar adecuación de M1 |
| F6 Internacional agregado | Fuente país/mes, cupo asignado y lead time de importación; conserva resto del mercado | COMEX, ajo mayorista, regla de asignación → experimento adicional trazable | Consistencia fresco/HS/unidad; no duplicar entrada aduanera y mayorista; variar reparto nacional→RM | F2–F5, opcional para cierre doméstico. Sin asignación defendible, reportar solo escenarios |
| F7 Cierre artículo A | Congelar configs, código, scripts, seeds y datos; generar figuras desde artefactos | F5 y F6 si aporta → paquete y manuscrito | Cada afirmación tiene experimento/IC/artefacto; declarar incertidumbre y condiciones de generalización | No depende de nuevos algoritmos HPC. M1 suficiente es un resultado válido |
| H1 HPC condicional | Ramas paralelas: `parallel_simulation`, mensajes, particionador y estimador; benchmark A0/A1/A2 | Perfil F1, workload F4/F5 → curva tiempo/precisión y error | Igual presupuesto total de núcleos, trabajo y réplicas; ventaja fuera de ruido y sin alterar conclusiones | Solo si A1 no alcanza presupuesto científico o hay pregunta metodológica independiente |

En F1 no hacer una gran refactorización del RNG: localizar todos los generadores y documentar qué se modifica. Streams lógicos por réplica, entidad y mecanismo, independientes del número de LP; orden estable de entidades y desempate estable de eventos. Preservar modo heredado si la política cambia distribuciones. Una prueba de seed necesita comparar salidas, no solo imprimir la semilla.

## Diseño propuesto del piloto

Factores: cuotas locales planificadas {0, 0,5, 1}; shocks {normal, pérdida local, corredor, compuesto}; modelos {M0, M1, M2}; semillas {101…110}. Son **360 ejecuciones de red con productos simultáneos**, no 360 por producto. Previo: pruebas deterministas con red manual y capacidad infinita; no cuentan como réplicas científicas.

Horizonte medido 90 días, calentamiento inicial propuesto 180 días (máximo ciclo de los productos heredados), sujeto a diagnóstico de estabilización. No confundir warmup con fecha de inicio estacional. Shocks: inicio día 30 medido, duración inicial 7 días, severidad de 50% como **escenario**, no estimación histórica. Documentar volumen/edad de stock a tiempo cero. Fronteras lambda=0/1 pueden ser contrafactuales fuera de capacidad real: etiquetarlas como tales o ajustar límites a oferta observada para la confirmación.

Piloto de 10 réplicas no garantiza precisión final. Estimar desviación estándar pareada `s_d`; tamaño inicial confirmatorio por celda `ceil((1,96*s_d/h)^2)` para semiancho objetivo `h=0,01` de demanda no satisfecha; recalcular con t y revisar colas/asimetrías. Usar semillas nuevas y un tamaño fijo; si el presupuesto no alcanza, acotar la pregunta o reportar incertidumbre, sin buscar significación por ensayo/error.

Confirmación candidata: cinco cuotas {0, 0,25, 0,5, 0,75, 1}, cuatro shocks, tres modos y dos ventanas estacionales = **120 × n réplicas** (3.600 si n=30 solo como presupuesto ilustrativo). Añadir una severidad o correlación a la vez mediante screening; evitar el factorial completo del handoff. La ablación sin deterioro y la capa internacional requieren presupuestos separados. Un solo año no identifica correlación extrema espacial; probar bandas bajo etiquetas de escenario.

Costo piloto = `360 × duración_por_réplica`; confirmar una duración representativa antes de lanzar. A1 distribuye ejecuciones independientes y cada corrida tiene ID y salida propia; limitar concurrencia también por memoria y carga PostgreSQL, no solo núcleos. Registrar tiempos totales, no sumar solo tiempo útil de eventos. No se crea todavía un launcher A1 porque el núcleo no acepta seeds controladas.

## Umbrales propuestos para revisar antes de confirmar

| Comprobación | Criterio propuesto | Interpretación |
|---|---|---|
| Balance por producto | Residuo absoluto ≤ `1e-9 × max(1, entradas acumuladas)` en casos pequeños double | Tolerancia numérica; revisar según escala/unidad antes de producción |
| Capacidad y edad | Sin solapamiento de andenes; tránsito/cola contabilizados; edad no decrece | Condición de validez, no resultado estadístico |
| Suficiencia M1 | IC 95% de diferencia de demanda insatisfecha dentro de ±0,02 y pérdida dentro de ±0,01; recomendación de política estable | Suficiente solo en esa clase. Fuera del margen: diferencia; cruzando margen: inconcluso |
| Resultado de diversificación | Diferencia pareada con IC y efecto práctico; robustez a asignación/vida útil/servicio | No requiere signo predeterminado ni óptimo interior |
| Recuperación | ≥95% de servicio del control sin shock durante siete días; censura explícita | Evita confundir un día afortunado con recuperación |
| HPC nuevo | Mejora de tiempo a precisión con IC bajo presupuesto idéntico y error científico bajo márgenes | El umbral de ganancia se fija después del perfil y antes del benchmark final |

## Paquete de evidencia para una corrida real

Manifiesto con SHA limpio y ref, config efectiva sin secretos, hashes de inputs, seed y política de streams efectivamente aplicada, modo, horizonte, calendario, shock, hardware/OS/compiler, digest de imagen y paquetes, comando exacto, horas de inicio/fin, exit code, execution_id/proc_id, logs, exportación DB y hash de salidas. `tools/research.py snapshot` aporta entorno/config/huellas; aún hay que completar resultados y procedencia del ejecutable. No interpretar un snapshot como una ejecución.

## Matriz de evidencia para el artículo

| Afirmación | Heredado / nuevo | Experimento | Métrica | Umbral / criterio | Artefacto futuro | Sección |
|---|---|---|---|---|---|---|
| Continuidad del estudio 2025 | ABM/FEL/DB heredados | E00 control RM/OH | cantidades y resultados publicados | Correspondencia resuelta y tolerancia predefinida | paquete baseline + reporte de diferencias | Métodos |
| Abastecimiento externo modifica servicio | Mercado heredado + nuevas fuentes | E10 barrido de cuota con oferta total fija | déficit acumulado / tasa de servicio | Efecto e IC; sin signo exigido | tabla cuota→resultado | Resultados |
| Cola/pérdida altera valor de diversificación | FEL heredado + lotes/hub nuevos | E20 M0/M1/M2 y ablación | demanda no satisfecha, pérdida | Márgenes de 2 pp / 1 pp propuestos | mapa de suficiencia | Métodos / resultados |
| Resultado robusto a incertidumbre | Escenarios nuevos | E30 sensibilidad fuera de calibración | signo/orden de política e intervalos | Condiciones de estabilidad reportadas | curva y región de incertidumbre | Validación |
| Internacional puede complementar abastecimiento | Nuevos cupos país/mes | E40 ajo, reparto y demora | déficit, pérdida y cuota entregada | Sin doble conteo; límites de asignación explícitos | contraste internacional | Extensión / discusión |
| Estrategia HPC mejora costo computacional | Runtime heredado + cambios condicionales | H10 A0/A1/A2 presupuesto igual | tiempo a precisión / error | Ventaja medida con incertidumbre, sin semántica ficticia | benchmark | Apéndice o artículo B |
