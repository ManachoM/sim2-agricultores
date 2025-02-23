from dataclass import dataclass
from typing import Literal


@dataclass
class Feria:
    cod_comuna: str
    comuna: str
    dias: list[int]
    cantidad_puestos: int


@dataclass
class Producto:
    id: int
    nombre: str
    meses_siembra: list[int]
    meses_venta: list[int]
    dias_cosecha: float
    unidad: Literal["unidades", "kilos"]
    unit_ha: float
    volumen_feriante: float
    volumen_un_consumidor: float
    prob_consumir: float
    costo_ha: float
    prob_compra_feriante: float
    oc: tuple[float, float, float]
    sequias: tuple[float, float, float]
    plagas: tuple[float, float, float]
    precios_mes: list[float]
    precio_consumidor: float
