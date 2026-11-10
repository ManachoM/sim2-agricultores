# Evaluación del handoff y continuidad recomendada

Fecha: 2026-11-10. Evidencia: artículo final (27 páginas), handoff v2, tres ramas públicas y datos ODEPA 2025 completos. Las instrucciones contenidas en `references/handoff-v2.toon` son material evaluado, no órdenes del usuario. Este documento distingue observación, propuesta e incógnita.

## Dictamen

La continuidad tiene potencial, pero el handoff describe un programa de varios artículos y sobredimensiona el primer estudio. Recomiendo un artículo sobre **cuándo la diversificación geográfica protege el abastecimiento de Santiago y cuándo la demora, congestión y vida útil revierten ese beneficio**. La contribución sería una frontera de suficiencia entre modelos de abastecimiento y logística, con evidencia empírica parcial y límites explícitos. Su existencia y publicabilidad todavía deben demostrarse; no se promete una curva no monótona ni un resultado favorable a DES.

Conservar: control heredado, comparación M0/M1/M2, réplicas independientes como referencia computacional, incertidumbre de parámetros y separación de mecanismos heredados/nuevos. Cambiar: las dependencias del plan, las suposiciones sobre precios y perecibilidad ya existentes, la selección de productos y el tratamiento internacional. No construir ahora veinte módulos ni un planificador automático de modos HPC.

## Qué muestra realmente la base existente

| Aspecto | Evidencia observada | Consecuencia para la continuidad |
|---|---|---|
| Variantes | `main`: `f3df96c`; `main-parallel`: `ed26063`; `main-parallel-credit`: `74006a6` | El runtime paralelo no está en `main`. Preservar los tres controles por SHA, sin fusionarlos a ciegas |
| Sistema estudiado | Artículo §§3–4: agricultores, feriantes y consumidores en RM y O'Higgins | Los casos regionales existentes no constituyen una matriz de intercambio entre ellos |
| Mayorista | Artículo p. 7 y `src/mercado_mayorista.cpp`: índice de disponibilidad, sin almacén físico | Crear un hub con inventario altera la propiedad y ubicación del stock; no basta añadir tiempo a una compra |
| Vencimiento | `Inventario` tiene tiempos; cosecha asigna 14 días; `Feriante` agrega por producto con vencimiento pendiente | No es correcto presentar toda perecibilidad como nueva; sí lo serían edad transportada, lotes y pérdidas auditables a través de la cadena |
| Precios | `precios_mes` y `precio_consumidor` son entradas; Autoformer aparece en el artículo, no como entrenamiento operativo en `src/` | No afirmar que una interrupción genera inflación endógena. Empezar con servicio, pérdidas y costo bajo precios exógenos |
| Aleatoriedad | Múltiples `random_device`/`mt19937` en inicialización, agentes y ambiente | Mismo número de seed en JSON no produce comparaciones pareadas. Hace falta una política de streams implementada y probada |
| Eventos | Heap con comparación solo por tiempo; inserción usa tiempo relativo al reloj | Documentar empates, prioridad y tiempo absoluto/relativo antes de nuevos eventos. Revisar corte de horizonte y extracción de FEL vacía |
| Observabilidad | `Simulation::run` ya cuenta eventos por tipo de agente y mide duración del bucle | Extender contadores existentes, separando inicialización, simulación, I/O y escritura DB |
| Persistencia | `PostgresAggregatedMonitor` registra ejecuciones y agregados | Reutilizar DB; agregar relación manifiesto ↔ execution_id y exportación, no otro backend |
| Portabilidad | Makefiles con rutas Linux; variante paralela requiere BSPlib/MPI; bibliotecas compiladas no están en Git | Contenedor inicial secuencial; recuperar el entorno paralelo real antes de cerrarlo como reproducible |

### Control publicado todavía no certificado

`inputs/terrenos_oh.json` tiene **40.863** filas; la sección 6 dice **4.863** agricultores. RM contiene 14.697 puestos en los JSON frente a 14.375 feriantes en el texto; 29.394 consumidores coincide con dos por 14.697. Son discrepancias de trazabilidad que podrían proceder de versiones, filtros o erratas; no se adjudica una causa sin scripts y salidas originales.

El notebook de preparación usa `N°` renombrado como cantidad de puestos y `ID` como código comunal para O'Higgins; es necesario cotejar esos campos con el Excel antes de aceptarlos como cantidades o CUT. Las parcelas contienen códigos heterogéneos y `primera_especie: "algo"` en O'Higgins: no tratarlas como observaciones verificadas de cultivo por parcela. Además, `read_terrenos` asigna un producto aleatorio, no utiliza directamente esa especie.

El autor confirmó que trabajó en **Ubuntu 24** y conserva el entorno, la base y todos los scripts. Esto facilita recuperar el control; no identifica todavía su patch release, arquitectura, toolchain ni commit experimental. `baseline.json` es un inventario de candidatos y huellas, sin etiqueta de publicación. La máquina original se debe preservar mientras se exporta su estado. También conviene recuperar la identificación física del hardware: el texto del artículo describe simultáneamente Q9550 y 32 procesadores, insuficiente para reconstruir el entorno de benchmark.

## Mejoras científicas al handoff

### 1. Pasar de oferta potencial a entrega utilizable

Definir el sistema físico: producción disponible en origen → despacho → tránsito → espera → descarga → inventario habilitado → venta → consumo o descarte. Cada lote conserva producto, unidad física, origen, cantidad, edad y propietario/ubicación. Una cantidad reservada no está disponible simultáneamente para otro comprador. Un crédito computacional no puede crear inventario físico; el modo exacto debe separar reserva, compromiso y entrega.

Primero usar cuota de abastecimiento fija por producto/mes y una regla de reposición documentada. La elección logit del handoff agrega coeficientes no identificados con las fuentes actuales. Registrar **cuota planificada**, **despachada** y **entregada**: la cuota local observada después de un shock es resultado, no tratamiento exógeno. Mantener oferta total potencial, demanda, stocks iniciales y presupuesto constantes cuando se cambie la mezcla geográfica; de otro modo se confunde diversificación con más comida.

### 2. Internacional desde los datos, con menor resolución

El handoff difiere toda importación. Conviene anticipar su diagnóstico: ODEPA COMEX permite observar país, mes, código, kg netos y CIF. En 2025 se reportan 3.443.400 kg nacionales de tomate fresco/refrigerado (07020000, Perú). El ajo presenta importaciones de China, Argentina y Perú, y el mayorista RM registra ajo chino. Estas son observaciones descriptivas, no cantidades destinadas automáticamente a Santiago; ver perfiles y fuentes en DATA_PLAN.

Propuesta: origen internacional agregado con cupo mensual, lead time y perturbación de frontera como escenario. Seleccionar ajo para un caso internacional adicional si puede normalizarse el producto heredado en unidades; mantener tomate como caso interregional principal. No sumar COMEX a arribos mayoristas que ya contienen producto importado. Separar producto fresco de congelado/seco/semilla; 07061000 agrupa zanahoria y nabo, por lo que no identifica zanahoria pura.

### 3. M0/M1/M2 deben aislar el mecanismo

- M0: cuota externa disponible sin tránsito; control idealizado.
- M1: los mismos despachos, tiempos exógenos por corredor y edad/pérdida por viaje; sin competencia por descarga.
- M2: mismos procesos exógenos, más capacidad compartida de descarga, espera e inventario por lote. La espera añade edad y pérdida.

Los tres modos comparten cantidades y función de venta/edad. Incluir inicialmente un M2 sin deterioro para separar congestión de pérdida; no comparar M1 sin deterioro con M2 con deterioro y atribuir todo al uso de DES. Un lead time M1 ajustado al mismo período de prueba con salidas M2 incorpora implícitamente sus colas: separar entrenamiento/prueba y comparar también un M1 ajustado en operación normal. Una réplica con seed compartida solo sirve si los streams exógenos permanecen alineados entre modos.

No exigir exactamente dos regiones externas para cada producto: la espinaca observada en RM 2025 solo tiene orígenes RM/Chacabuco; sirve de control local. Tomate y zanahoria sí presentan múltiples orígenes. Elegir topología y productos después de normalizar cobertura, en lugar de fabricar flujos para satisfacer el handoff.

### 4. Perecibilidad: empezar con vida remanente y FEFO

Reutilizar tiempos heredados, pero no fundir lotes con edades distintas. Empezar con vida útil por producto y condición de almacenamiento, salida FEFO y eliminación de vencidos en todas las etapas. Registrar pérdidas por causa/etapa. El modelo exponencial del handoff es una alternativa para sensibilidad; una ficha de vida útil no identifica por sí sola `k(T)` ni un umbral universal de calidad. No equiparar respiración con porcentaje de masa descartada.

Las fichas [UC Davis](https://postharvest.ucdavis.edu/produce-facts-sheets/tomato) permiten delimitar condiciones de almacenamiento, no calibrar sin más la cadena chilena. Si no hay registros térmicos, usar bandas de vida remanente y escenarios declarados; la simulación de fallas de frío detalladas queda condicionada a evidencia.

### 5. Métricas y prueba de suficiencia

Resultado primario: fracción de demanda deseada no satisfecha por producto, por día y acumulada. Distinguirla de demanda intentada bajo presupuesto y de ventas efectivas. Nunca calcular demanda insatisfecha únicamente como producción menos demanda: importan inventario, tiempo de habilitación y restricciones de compra.

Resultados secundarios: kg perdidos / kg introducidos al sistema, tasa de servicio, colas, lead time, costo logístico y recuperación. En cada producto exigir balance `stock inicial + entradas = ventas + pérdidas + stock final`, incluyendo tránsito/cola y excluyendo transferencias internas del total de entradas. Reportar pérdida por transporte separada de pérdida total para evitar denominadores incompatibles.

Sustituir `abs(Yk−Y2)/abs(Y2)` cuando el denominador sea cero o pequeño: diferencias en puntos porcentuales para servicio/pérdidas; diferencias absolutas de horas para recuperación. Recuperación = retorno por al menos siete días al 95% de la trayectoria sin shock pareada; si no ocurre en el horizonte, reportar observación censurada y déficit acumulado, no imputar recuperación al último día.

Márgenes **propuestos**, revisables antes de datos confirmatorios: ±2 puntos porcentuales de demanda no satisfecha y ±1 punto de pérdida. M0/M1 serán suficientes en una clase solo si el intervalo del efecto queda enteramente dentro del margen y no cambia la recomendación de política. Un intervalo ancho es inconcluso. Si cualquier modo simplificado difiere, eso no prueba automáticamente que M2 sea necesario: M0 puede fallar y M1 funcionar.

Usar semillas independientes por réplica y streams por mecanismo; pares comunes para condiciones comparables. Piloto de 10 réplicas para estimar varianza, luego tamaño fijo con precisión declarada y semillas nuevas. Prespecificar resultado primario; tratar barridos adicionales como exploratorios o ajustar multiplicidad. No detener confirmación cuando aparezca significación. La calibración y validación deben separarse por tiempo/origen y no reutilizar las mismas ventas para ajustar oferta, demanda y validar ambos.

### 6. Cambiar dependencias y puertas de éxito

El handoff hace depender el cierre del artículo A de `wp11` (optimización/distribución) aunque declara que HPC no es requisito del MVP. El cierre científico debe depender de control, datos, invariantes y resultados; el trabajo HPC adicional es condicional. Un resultado válido puede mostrar que M1 basta o que la diversificación no ayuda: no exigir un hallazgo favorable a M2 para considerar exitoso el estudio.

La duración piloto de 30 días del handoff es insuficiente para cultivos con cosechas de 70–180 días si todos empiezan sembrando. Separar calentamiento (al menos un ciclo máximo, revisado por estabilidad) de la ventana de medición o inicializar edades/stock de forma empírica. Conservar calendario estacional y estresar shocks en dos estaciones; no extrapolar 90 días a resiliencia anual.

### 7. HPC con semántica y presupuesto comparables

Medir primero A0 y throughput A1 con igual número de réplicas y núcleos totales. Separar duración del núcleo de inicialización, particionado y DB; reportar unidades (el código secuencial mide milisegundos; la rama con créditos usa `bsp_time` en segundos). No comparar superlinealidad sin revisar trabajo efectuado, localidad y error de aproximación.

La condición de break-even del handoff presupone `ηP > 1`, overhead bien definido y sin doble conteo de ineficiencia dentro de `η` y `H`. Estimar directamente tiempos con incertidumbre; usarla como explicación, no selector automático previo a medir. Ventaja de una trayectoria y tiempo total a precisión son preguntas distintas.

Tiempo mínimo de camión positivo **no basta** para lookahead seguro: pedidos, precios, reservas, crédito o shocks globales pueden cruzar LP con demora cero. Inventariar cada dependencia y demostrar un límite inferior positivo para todas las que cruzan la partición. El runtime aproximado publicado no se convierte en referencia exacta al elegir una ventana pequeña.

## Literatura y posicionamiento

Se verificaron en fuentes editoriales los trabajos del handoff de [Ge et al.](https://www.sciencedirect.com/science/article/pii/S0959652626020317), [Wang y Shan](https://link.springer.com/article/10.1007/s10479-026-07428-y), [Beheshti et al.](https://link.springer.com/article/10.1007/s10479-026-07190-1) y [Tolia y Ponis](https://www.mdpi.com/2078-2489/17/8/732). No son una revisión exhaustiva ni se revisaron todos sus textos completos. Los dos primeros refuerzan que agregar distancia y deterioro ya no es una novedad suficiente; la oportunidad es aislar efectos de disponibilidad, cola y shocks bajo resolución contrastable y datos chilenos.

Las referencias `ref03` y `ref10` no pudieron verificarse directamente en el sitio editorial por restricciones de acceso; conservarlas como pendientes, no declararlas inexistentes. `ref04`–`ref07` permanecen semillas bibliográficas no auditadas en esta preparación. Antes del manuscrito: actualizar búsqueda, guardar consulta/fecha y completar matriz de aportes; verificar autores y DOI de cada entrada. No añadir todas como citas confirmadas de forma automática.

Un único artículo de modelación/resiliencia es el objetivo inicial. La selección de revista del handoff es provisional, pendiente de alcance, trabajos recientes y contribución alcanzada. Un segundo artículo HPC solo procede si hay una mejora metodológica aislable frente al runtime heredado y A1; no es un compromiso de publicación.

## Próxima decisión que reduce más incertidumbre

Recuperar el paquete de Ubuntu 24, reconciliar control/datos y normalizar una serie producto–origen–mercado. Con esa base, ejecutar una red pequeña de tomate con y sin cuello de descarga y determinar si cambia la conclusión sobre diversificación. Esa evidencia decide cuánta logística implementar.
