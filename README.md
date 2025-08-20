# 🌱 Sistema Inteligente de Irrigação com ESP32-S3 e MLOps

Projeto de TCC (Sistemas de Informação) para automação de irrigação com ESP32-S3, sensores ambientais e inferência de um modelo de ML treinado externamente. O pipeline em Python gera um header (`ia_params.h`) com os pesos do modelo, normalização e uma pequena série de previsão de chuva; o firmware consome esse header para decidir **ligar/desligar** a bomba com regras de segurança (janela quente, histerese, anti-chattering, etc.).

---

## 📁 Estrutura do Projeto

```bash
tcc_repository/
├── coleta_dados/                      # Treino/ETL (Python) — gera o ia_params.h
│   ├── coleta_dados.py                # Baixa e salva dados climáticos em CSV
│   ├── treinar_modelo.py              # Treina, avalia e exporta ia_params.h
│   ├── dados_climaticos_tratados.csv  # (gerado) CSV consolidado
│   ├── ia_params.h                    # (gerado) SCALER/W/B + forecast (deploy copia p/ firmware)
│   ├── model.pkl                      # (gerado) modelo scikit-learn
│   ├── scaler.pkl                     # (gerado) scaler
│   ├── metrics.txt                    # (gerado) métricas do treino
│   └── pesos_bias.json                # (gerado) espelho dos parâmetros
│
├── esp32_firmware/                    # Firmware da ESP32-S3 (ESP-IDF)
│   ├── CMakeLists.txt                 # Projeto ESP-IDF
│   ├── sdkconfig                      # Gerado via `idf.py menuconfig`
│   ├── .gitignore                     # Ignora build/ etc.
│   ├── build/                         # (gerado) artefatos de compilação
│   └── main/                          # Componente principal
│       ├── CMakeLists.txt
│       ├── app_config.h               # Thresholds, janelas, mapeamento de leituras
|       ├── wifi/                      # Modulo para conexão wifi da placa
|       │   ├── wifi.c
|       │   └── wifi.h
│       ├── main.c                     # Loop principal: sensores → decisão → bomba
│       │
│       ├── ia/                        # Inferência do modelo
│       │   ├── ia.c                   # Sigmoid, normalização, dot(W,x)+B
│       │   ├── ia_infer.h             # API da IA p/ restante do firmware
│       │   └── ia_params.h            # (injetado pelo deploy a partir de coleta_dados/)
│       │
│       ├── forecast/                  # Série embutida de chuva_mm_24h
│       │   ├── forecast.c             # lookup (t0/step/len) → chuva_24h “agora”
│       │   ├── forecast.h
│       │   └── time_utils.h           # Helpers de hora/janela quente
│       │
│       ├── decider/                   # Regras + IA + histerese + anti-chattering
│       │   ├── decider.c
│       │   └── decider.h
│       │
│       ├── pump/                      # Driver do relé/bomba
│       │   ├── pump.c
│       │   └── pump.h
│       │
│       └── sensors/                   # Drivers dos sensores
│           ├── dht22/ {dht.c, dht.h}
│           ├── ldr/   {ldr.c, ldr.h}
│           └── moisture/ {moisture.c, moisture.h}
│
├── .gitignore
└── README.md
```

> **Fluxo:** `treinar_modelo.py` gera `coleta_dados/ia_params.h`. O workflow de CI copia esse header para `esp32_firmware/main/ia/ia_params.h` e dispara `idf.py build` (e opcionalmente OTA). Para testes locais, copie manualmente.

---

## 🧰 Comandos Úteis

### 🔧 Compilar o firmware (ESP-IDF)
> Execute **dentro de `esp32_firmware/`**:
```bash
idf.py build
```
A saída vai para `esp32_firmware/build/`.

### 🚀 Gravar firmware e abrir monitor serial
> Em `esp32_firmware/` (ajuste a porta: `/dev/ttyUSB0`, `/dev/ttyACM0` ou `COMx`):
```bash
idf.py -p /dev/ttyACM0 flash monitor
```

### ⚙️ Abrir menu de configuração
```bash
idf.py menuconfig
```

### 💾 (Opcional) Gravar arquivos para SPIFFS
```bash
idf.py -p /dev/ttyACM0 spiffs-flash
```
Usa os arquivos de `esp32_firmware/data/` (se a partição estiver configurada).

---

## 🧪 Pipeline de dados e treino (Python)

### 1) Ambiente Python (recomendado usar venv)
> Na **raiz** (`tcc_repository/`):
```bash
python -m venv .venv
# Windows: .\.venv\Scripts\Activate.ps1
# Linux/macOS: source .venv/bin/activate
python -m pip install --upgrade pip
pip install pandas requests numpy tzdata scikit-learn joblib
```

### 2) Coletar dados históricos
```bash
cd coleta_dados
python coleta_dados.py
```
Gera `coleta_dados/dados_climaticos_tratados.csv`.

### 3) Treinar modelo e exportar header
```bash
python treinar_modelo.py
```
Gera `coleta_dados/ia_params.h` (SCALER/W/B + forecast de chuva_24h).

**Teste local (sem CI):**
```bash
cp ia_params.h ../esp32_firmware/main/ia/ia_params.h
cd ../esp32_firmware && idf.py build
```

---

## 🧠 Como a decisão é tomada

1. A ESP32 lê **umidade do solo**, **temperatura**, **umidade do ar** (+ opcional **chuva_24h** do forecast embutido).
2. Normaliza com `SCALER_MEAN/SCALER_SCALE` e aplica o modelo (regressão logística).
3. Regras operacionais (janela quente, previsão de chuva forte, histerese e anti-chattering) garantem segurança e estabilidade do liga/desliga.

---

## 📌 Observações

- O arquivo `esp32_firmware/main/ia/ia_params.h` é **gerado**; não edite manualmente.
- Thresholds e janelas estão em `app_config.h`.
- A previsão de chuva embutida tem grade simples (`t0`, `step_min`, `len`); dá pra evoluir sem mudar o firmware.