import requests
import pandas as pd
from datetime import datetime
from pathlib import Path

# Configurações
latitude = -19.82   # João Monlevade - MG
longitude = -43.17
start_year = 2020
end_year = 2024

# Coleta dos dados por ano
df_total = pd.DataFrame()

for year in range(start_year, end_year + 1):
    start_date = f"{year}-01-01"
    end_date = f"{year}-12-31"

    url = (
        f"https://archive-api.open-meteo.com/v1/archive?"
        f"latitude={latitude}&longitude={longitude}"
        f"&start_date={start_date}&end_date={end_date}"
        f"&daily=temperature_2m_max,temperature_2m_min,precipitation_sum"
        f"&timezone=America/Sao_Paulo"
    )

    response = requests.get(url)
    data = response.json()

    if "daily" in data:
        df = pd.DataFrame(data["daily"])
        df_total = pd.concat([df_total, df], ignore_index=True)
        print(f"✅ Dados de {year} coletados com sucesso.")
    else:
        print(f"❌ Falha ao coletar dados de {year}")

# Limpeza e padronização
df_total.dropna(inplace=True)
df_total["time"] = pd.to_datetime(df_total["time"])
df_total.rename(columns={
    "time": "data",
    "temperature_2m_max": "temp_max",
    "temperature_2m_min": "temp_min",
    "precipitation_sum": "chuva_mm"
}, inplace=True)

# Seleção de colunas relevantes
colunas = ["data", "temp_max", "temp_min", "chuva_mm"]
df_final = df_total[colunas]

# Caminho para salvar dentro da mesma pasta do script
output_path = Path(__file__).resolve().parent / "dados_climaticos_tratados.csv"

df_final.to_csv(output_path, index=False)
print(f"📁 Dados tratados salvos em: {output_path}")
