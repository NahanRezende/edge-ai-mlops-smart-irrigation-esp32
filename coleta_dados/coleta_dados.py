import requests
import pandas as pd
from pathlib import Path
from datetime import datetime
from zoneinfo import ZoneInfo

# Configurações
latitude = -19.82   # João Monlevade - MG
longitude = -43.17
start_year = 2020
end_year = datetime.now(ZoneInfo("America/Sao_Paulo")).year  # ano atual

today = datetime.now(ZoneInfo("America/Sao_Paulo")).date()
df_total = pd.DataFrame()

for year in range(start_year, end_year + 1):
    start_date = f"{year}-01-01"
    end_date = today.isoformat() if year == end_year else f"{year}-12-31"

    url = (
        "https://archive-api.open-meteo.com/v1/archive"
        f"?latitude={latitude}&longitude={longitude}"
        f"&start_date={start_date}&end_date={end_date}"
        "&daily=temperature_2m_max,temperature_2m_min,precipitation_sum"
        "&timezone=America/Sao_Paulo"
    )

    try:
        r = requests.get(url, timeout=30)
        r.raise_for_status()
        data = r.json()
    except Exception as e:
        print(f"❌ Falha HTTP no ano {year}: {e}")
        continue

    daily = data.get("daily")
    if not daily:
        print(f"❌ Sem bloco 'daily' no ano {year}. Payload: {data.keys()}")
        continue

    df = pd.DataFrame(daily)

    # Sanitiza colunas esperadas
    if not {"time", "temperature_2m_max", "temperature_2m_min", "precipitation_sum"} <= set(df.columns):
        print(f"❌ Colunas inesperadas no ano {year}: {df.columns.tolist()}")
        continue

    # Converte tipos e remove linhas ruins
    df["time"] = pd.to_datetime(df["time"], errors="coerce", utc=False)
    # precipitação não-negativa
    df["precipitation_sum"] = pd.to_numeric(df["precipitation_sum"], errors="coerce").clip(lower=0.0)
    df["temperature_2m_max"] = pd.to_numeric(df["temperature_2m_max"], errors="coerce")
    df["temperature_2m_min"] = pd.to_numeric(df["temperature_2m_min"], errors="coerce")
    df = df.dropna(subset=["time", "temperature_2m_max", "temperature_2m_min", "precipitation_sum"])

    if df.empty:
        print(f"⚠️ Sem linhas válidas no ano {year}")
        continue

    df_total = pd.concat([df_total, df], ignore_index=True)
    print(f"✅ Dados de {year} coletados: {len(df)} linhas")

# Verificações antes de salvar
if df_total.empty:
    raise RuntimeError("Nenhum dado coletado. Verifique a rede/URL/intervalos.")

df_total = df_total.rename(columns={
    "time": "data",
    "temperature_2m_max": "temp_max",
    "temperature_2m_min": "temp_min",
    "precipitation_sum": "chuva_mm"
})[["data", "temp_max", "temp_min", "chuva_mm"]]

output_path = Path(__file__).resolve().parent / "dados_climaticos_tratados.csv"
df_total.to_csv(output_path, index=False, float_format="%.3f")
print(f"📁 Dados tratados salvos em: {output_path} | linhas={len(df_total)}")
