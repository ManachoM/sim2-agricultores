# Datos disponibles y límites de inferencia

Consulta y descarga: 2026-11-10. Se guardan perfiles y muestras pequeñas en `research/data/`; los CSV completos están en `data/raw/` (ignorados por Git). El catálogo [sources.json](sources.json) registra URL, licencia, huella y estado. Atribución de los datos externos: Oficina de Estudios y Políticas Agrarias, con información del Servicio Nacional de Aduanas para COMEX, licencia Creative Commons Attribution indicada en sus catálogos.

## Datos ya presentes en el proyecto

| Entrada | Contenido comprobado | Uso posible / cautela |
|---|---|---|
| `inputs/prods*.json` | 20 productos con meses, unidad, rendimiento, consumo y precios | Candidatos de continuidad. `prods_calibrated.json` y su copia difieren: seleccionar explícitamente; no combinar calibraciones |
| `inputs/ferias*.json` | RM: 343 ferias / 14.697 puestos; OH: 132 / 10.284; caso pequeño: 3 ferias | Calendario y escala; no son volúmenes ni flujos interregionales |
| `inputs/terrenos*.json` | Versión Git: RM 4.695 terrenos; OH 40.863; pequeño 23 | Los terrenos OH del trabajo están en Ubuntu y no se commitearon; recuperar esa entrada antes de reconstruir la oferta publicada |
| `inputs/frost_probs*.json`, SPI y calor | Perfiles mensuales y niveles; la serie calor se comparte en configuraciones auditadas | Reutilizar para control; no identifican correlación espacial diaria ni shocks conjuntos extremos |
| `data_analysis/volumenes_reales_mes.csv` | 260 filas, producto/mes/volumen esperado; sin origen ni año en columnas | Referencia heredada de calibración, no nueva evidencia observacional OD; recuperar derivación y unidades |
| `data_analysis/configs_results.csv` | 440 filas con errores y configuraciones | Resultados resumidos previos; no reemplazan réplicas individuales ni metadatos experimentales |
| Notebooks, Excel y GeoJSON | Preparación, parcelas OH, ferias, clima y análisis; varios notebooks dependen de PostgreSQL | Recuperar versiones y tablas originales; no ejecutar notebooks destructivos como paso de instalación |
| SQL / DB original | Esquema y dump histórico en Git; el autor conserva PostgreSQL original en Ubuntu 24 | Exportar con herramientas PostgreSQL compatibles y reconciliar IDs; no restaurar sobre la base fuente |

Inventario completo con conteos y huellas: [local-inventory.json](data/local-inventory.json). Licencias/procedencia de entradas históricas no están completamente documentadas: su presencia pública en Git no implica permiso general para redistribuir las fuentes originales.

## Datos externos comprobados

### Mayoristas ODEPA 2025

El [catálogo oficial](https://datos.odepa.gob.cl/dataset/precios-mayoristas-de-frutas-y-hortalizas) describe precios/volúmenes por mercado y origen. Se descargó el archivo anual completo y se perfiló con `tools/profile_odepa.py`: **202.029 filas**, 201 con volumen cero en el archivo completo. Para RM y cinco candidatos (ajo, espinaca, lechuga, tomate, zanahoria) hay **12.843 filas y 247 fechas distintas** entre 2 de enero y 31 de diciembre. Esto no significa cobertura diaria completa por producto/origen: esos conteos corresponden al conjunto seleccionado.

[Perfil agrupado](data/wholesale-2025-profile.json), sin conversiones de masa ni inferencia de camiones:

| Producto | Evidencia de origen con volumen positivo en RM 2025 | Recomendación |
|---|---|---|
| Tomate | Arica y Parinacota, O'Higgins, RM y provincias/localidades como Quillota, Limache, Huasco, Copiapó, Limarí y Talca | Caso principal interregional; normalizar localidad→región y comparar corredores cortos/largos |
| Zanahoria | RM, Coquimbo/Elquí, Biobío, Ñuble y Araucanía, entre otros | Segundo producto doméstico; revisar vida útil y estacionalidad |
| Espinaca | RM y provincia de Chacabuco | Control local; no imponer dos abastecedores externos no observados |
| Ajo | China, O'Higgins, RM, Talagante, Arica y Parinacota | Mejor puente internacional observado en este perfil; unidad heredada requiere correspondencia a kg |
| Lechuga | Múltiples regiones/provincias; cajas de 10–24 unidades | Buen contraste perecible futuro; no sumar unidades a kg sin peso por unidad y su incertidumbre |

`ID region` corresponde a la plaza de comercialización, no necesariamente al origen. `Origen` mezcla país, región, provincia y localidad: se necesita un diccionario explícito, no extraer un número por heurística. Conservar origen crudo y nivel de resolución; no geocodificar una provincia como si fuera una granja.

Los volúmenes preservan la unidad comercial. Hay textos como `$/paquete 20 unidades (volumen en unidades)`: **no aplicar automáticamente un multiplicador 20 al volumen**. Crear tabla de conversión revisada y contrastar totales con boletines oficiales. Separar unidades del precio y del volumen. El [boletín semanal oficial](https://www.odepa.gob.cl/wp-content/uploads/2024/08/Boletin_Semanal_Precios_Mayoristas_20240816.pdf) sirve para cotejar presentaciones y cantidades; no reemplaza la validación por producto/mercado del archivo 2025.

El perfil conserva grupos por producto, mercado, origen y unidad, agregando variedad/calidad únicamente para inventario descriptivo. La tabla analítica futura debe retener variedad y calidad, revisar duplicados de clave y cambios de muestreo antes de agregar. Un día ausente, un cero y un mercado cerrado no son automáticamente equivalentes. La información de los mercados cubiertos no es el consumo total de Santiago ni identifica trayectorias individuales o toda la producción familiar.

### COMEX ODEPA 2025

Se descargó el [recurso anual de importaciones](https://datos.odepa.gob.cl/dataset/f9fc0580-37dd-4034-8690-1c04bfbf33af/resource/670c25a8-ff81-48ab-be4d-da0746bc15ec): **43.843 filas**; 1.302 del capítulo 07 en el perfil. Cobertura nacional mensual, no destino de consumo. Los campos numéricos de la descarga son texto con coma decimal: el script usa `Decimal`, conserva códigos como texto y rechaza volúmenes negativos o no finitos.

Totales descriptivos seleccionados del [perfil](data/imports-2025-profile.json):

| Código | Descripción abreviada | País | Volumen declarado, kg netos |
|---|---|---|---:|
| 07020000 | Tomate fresco/refrigerado | Perú | 3.443.400 |
| 07032010 | Ajo orgánico fresco/refrigerado | China | 1.359.500 |
| 07032090 | Otros ajos frescos/refrigerados | China | 10.209.090,7439 |
| 07032090 | Otros ajos frescos/refrigerados | Argentina | 92.120 |
| 07032090 | Otros ajos frescos/refrigerados | Perú | 9.870 |

Estos totales justifican estudiar el canal internacional; no establecen su cuota de Santiago. El capítulo 07 incluye congelados, secos y legumbres: la espinaca identificada es congelada (07103000), que no sirve de oferta fresca equivalente. Zanahoria/nabo comparten 07061000. CIF/kg puede describir valor unitario aduanero, pero no es precio mayorista, flete aislado ni costo marginal de despacho.

Asignación internacional recomendada: `importación nacional × fracción destinada a RM × fracción del canal mayorista modelado`, con fracciones observadas si existen; si no, escenarios y sensibilidad [0,1] físicamente acotados. Reconciliar este flujo con origen extranjero observado en mayoristas para impedir doble conteo. Tampoco se infieren demoras aduaneras o puerto de ingreso de estos campos.

### Fuentes candidatas todavía sin descarga/calibración

| Necesidad | Fuente y estado | Qué no permite afirmar por sí sola |
|---|---|---|
| Superficie regional | [INE Hortalizas](https://www.ine.gob.cl/estadisticas-por-tema/agricultura-y-medio-ambiente/hortalizas), catálogo consultado | Superficie × rendimiento supuesto no es flujo real a Santiago |
| Carga e infraestructura | [API Observatorio Logístico](https://filesprod.observatoriologistico.cl/assets/frontend-graficos/api-datastream-docs/datastreams_api_docs.html), documentación consultada | Aún no se verificó una serie OD de hortalizas por corredor ni tiempos por camión |
| Topología/distancias | OpenStreetMap, candidato sin extracción en esta preparación | Distancia/vmax no es distribución empírica de viaje ni capacidad de carretera |
| Descarga, colas y capacidad de hub | Registros operativos/entrevistas/aforos por obtener | Los volúmenes diarios no identifican número de andenes, horarios y tiempo de servicio de manera única |
| Vida útil | [UC Davis tomate](https://postharvest.ucdavis.edu/produce-facts-sheets/tomato), ficha consultada; otras fichas por revisar | Vida útil bajo condiciones controladas no valida automáticamente decaimiento exponencial ni cadena fría local |
| Demanda | ENCA original y ventas heredadas; recuperar derivación y evaluar actualización | Ventas observadas con escasez no equivalen a demanda latente |

## Contratos para las tablas de trabajo

Mantener raw inmutable, transformación por script y tabla limpia versionada por huella. No se inventaron valores de capacidad, lead time ni conversión para llenar archivos.

| Tabla futura | Clave y campos mínimos | Unidades / verificaciones |
|---|---|---|
| `market_arrivals` | fecha, mercado, producto, variedad, calidad, origen_crudo, región_origen, tipo_origen, volumen_crudo, unidad_cruda, cantidad_kg, precio_CLP_kg, fuente_fila | fecha ISO; kg solo con conversión validada; null explícito si falta; clave única o duplicados explicados |
| `trade_imports` | año, mes, código_HS, producto, país, volumen, unidad, CIF_USD | separar frescos; código string con versión; unidad homogénea antes de sumar |
| `regional_supply` | región, producto, período, área_ha, rendimiento_kg_ha, disponibilidad_kg, cuota_destino | distinguir observación, estimación y oferta de escenario; no exceder producción disponible |
| `corridors` | origen, destino, distancia_km, tau_min_h, distribución_tiempo_h, capacidad_kg_h, fuente, categoría_evidencia | capacidad y tiempos >0 cuando corresponda; horarios explícitos; límites no identificados como rangos |
| `hub_service` | hub, andenes, horario, distribución_servicio_h, almacenamiento_kg | FCFS inicial; mismos parámetros entre modelos; evidencia o escenario declarado |
| `product_life` | producto, condición, vida_remanente_h, conversión_kg_unidad, fuente, rango | fecha/edad coherente; no aplicar un peso fijo sin origen documentado |

Todo parámetro debe marcarse `observed`, `estimated`, `literature` o `scenario`, con fuente/fecha/resolución, transformación, licencia y banda de incertidumbre. No llamar calibración a escoger una cifra plausible.

## Secuencia de adquisición y validación

1. Respaldar el paquete Ubuntu 24 completo, incluidos terrenos OH, datos y decisiones no commiteados, y recuperar la transformación de volúmenes, presupuesto y configuración original. Documentar las decisiones locales con su evidencia antes de seleccionar la entrada canónica; no sustituirlas por el snapshot de Git.
2. Descargar al menos años completos adicionales (por ejemplo 2023–2024) para calibración y reservar 2025 para validación temporal; revisar comparabilidad. Los perfiles actuales de 2025 son exploratorios y no una validación independiente.
3. Normalizar una pareja de mercados de Santiago por separado. Elegir explícitamente el hub que representa cada uno o definir un agregado; no bautizar como Lo Valledor el total de todas las plazas.
4. Documentar conversiones y obtener series origen–producto–mes con cobertura, faltantes y cuota desconocida. Seleccionar regiones por cobertura y peso observado, no por conveniencia de la topología.
5. Obtener al menos un dato independiente de viaje y uno de servicio/cola. Si no existen, desarrollar un estudio de mecanismos con capacidad/tiempos sintéticos y no afirmar validación operacional chilena.
6. Para importaciones, validar correspondencia HS↔producto y asignación metropolitana antes de cuantificar resiliencia internacional. Variar reparto y lead time por separado.

## Regenerar los perfiles

Las URL exactas están en `sources.json`. Descargar sin alterar nombres a `data/raw/` (por ejemplo `curl -fL URL -o data/raw/odepa-wholesale-2025.csv`) y verificar primero SHA-256 contra el catálogo. Si la fuente cambia, guardar nueva versión y registrar decisión; no sobrescribir silenciosamente.

```sh
python3 tools/profile_odepa.py wholesale data/raw/odepa-wholesale-2025.csv --output runs/wholesale-profile.json
python3 tools/profile_odepa.py imports data/raw/odepa-imports-2025.csv --output runs/imports-profile.json
```

El script no descarga ni transforma datos automáticamente. Los archivos de salida se crean de manera exclusiva para proteger evidencia anterior. Comparar el JSON regenerado con los perfiles versionados; el contenido debe ser idéntico si archivo y código son iguales.
