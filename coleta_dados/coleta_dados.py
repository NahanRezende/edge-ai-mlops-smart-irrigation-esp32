import requests
import pandas as pd
from datetime import datetime

# Configurações
latitude = -19.82   # Latitude de João Monlevade
longitude = -43.17  # Longitude de João Monlevade
start_year = 2020
end_year = 2024

# Lista para armazenar os dados
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

# Salva como CSV
df.to_csv('../esp32_firmware/data/dados_temperatura.csv', index=False)
print("📁 Arquivo salvo como dados_climaticos_historicos.csv")
