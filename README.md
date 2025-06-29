# 🌱 Sistema Inteligente de Irrigação com ESP32-S3 e MLOps

Este projeto faz parte do Trabalho de Conclusão de Curso (TCC) de Engenharia da Computação. O objetivo é desenvolver um sistema de irrigação automatizado utilizando a ESP32-S3 para coleta de dados de sensores ambientais, com análise inteligente baseada em técnicas de Machine Learning e práticas de MLOps.

---

## 📁 Estrutura do Projeto

```bash
tcc_repository/
├── esp32_firmware/         # Firmware da ESP32-S3 (ESP-IDF)
│   ├── main/               # Código principal em C (ESP-IDF)
│   │   └── sensores/       # Lógica modular para DHT22, LDR e umidade do solo
│   ├── data/               # (opcional) CSVs a serem gravados via SPIFFS
│   ├── build/              # Gerada após a compilação (gitignored)
│   ├── CMakeLists.txt      # Configuração de build CMake
│   ├── sdkconfig           # Configurações geradas pelo menuconfig
│   └── .gitignore
├── coleta_dados/           # Scripts Python para coleta de dados históricos
│   └── coleta_dados.py
└── README.md               # Este arquivo
```

---

## 🧰 Comandos Úteis

### 🔧 Compilar o projeto

idf.py build

Compila o código C usando a ESP-IDF. A saída é gerada na pasta `build/`.

---

### 🚀 Gravar firmware e abrir monitor serial

idf.py -p /dev/ttyACM0 flash monitor

Grava o firmware na ESP32 e abre o monitor serial para acompanhar a execução.

---

### ⚙️ Abrir menu de configuração da ESP-IDF

idf.py menuconfig

Interface para configurar pinos, sensores e opções do projeto.

---

### 💾 Gravar arquivos CSV na memória SPIFFS da ESP32

idf.py -p /dev/ttyACM0 spiffs-flash

Grava os arquivos da pasta `esp32_firmware/data/` na flash da ESP32 usando SPIFFS.

### 📦 Instalar dependências Python

> Execute dentro da pasta `coleta_dados/`:

pip install requests pandas


### 📊 Executar script Python para coletar dados históricos

cd coleta_dados/
python coleta_dados.py

Executa o script para baixar dados climáticos e salvar em CSV.
