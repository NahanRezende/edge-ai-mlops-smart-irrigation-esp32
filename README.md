# 🌱 Sistema Inteligente de Irrigação com ESP32-S3 e MLOps

Este projeto faz parte do Trabalho de Conclusão de Curso (TCC) de Engenharia da Computação. O objetivo é desenvolver um sistema de irrigação automatizado utilizando a ESP32-S3 para coleta de dados de sensores ambientais, com análise inteligente baseada em técnicas de Machine Learning e práticas de MLOps.

---

## 📁 Estrutura do Projeto

```bash
tcc_repository/
├── esp32_firmware/                # Firmware da ESP32-S3 (ESP-IDF)
│   ├── main/                      # Código principal do projeto
│   │   ├── main.c                 # Função app_main e controle dos sensores
│   │   └── sensores/              # Lógica modular dos sensores conectados
│   │       ├── dht22/
│   │       │   ├── dht.c          # Lê temperatura e umidade com o DHT22
│   │       │   └── dht.h          # Header com definições da API DHT22
│   │       ├── ldr/
│   │       │   ├── ldr.c          # Lê luminosidade via LDR
│   │       │   └── ldr.h          # Header da API do sensor LDR
│   │       └── moisture/
│   │           ├── moisture.c     # Lê umidade do solo
│   │           └── moisture.h     # Header do sensor de umidade do solo
│   ├── data/                      # (opcional) CSVs a serem gravados via SPIFFS
│   ├── build/                     # Gerada após compilação (gitignored)
│   ├── CMakeLists.txt             # Configuração do sistema de build ESP-IDF
│   ├── sdkconfig                  # Arquivo gerado via `idf.py menuconfig`
│   └── .gitignore                 # Ignora arquivos como build/
├── coleta_dados/                 # Scripts Python para coleta de dados históricos
│   └── coleta_dados.py            # Script que baixa e salva dados climáticos em CSV
└── README.md                      # Este arquivo
```

---

## 🧰 Comandos Úteis

### 🔧 Compilar o projeto
```bash
idf.py build
```
Compila o código C usando a ESP-IDF. A saída é gerada na pasta `build/`.

---

### 🚀 Gravar firmware e abrir monitor serial
```bash
idf.py -p /dev/ttyACM0 flash monitor
```
Grava o firmware na ESP32-S3 e abre o terminal serial para depuração.

---

### ⚙️ Abrir menu de configuração da ESP-IDF
```bash
idf.py menuconfig
```
Interface interativa para configurar pinos, sensores e definições do projeto.

---

### 💾 Gravar arquivos CSV na memória SPIFFS da ESP32
```bash
idf.py -p /dev/ttyACM0 spiffs-flash
```
Grava os arquivos da pasta `esp32_firmware/data/` na memória SPIFFS da ESP32.

---

### 📦 Instalar dependências Python
> Execute dentro da pasta `coleta_dados/`:
```bash
pip install requests pandas
```
Instala bibliotecas necessárias para rodar o script de coleta de dados.

---

### 📊 Executar script Python para coletar dados históricos
```bash
cd coleta_dados/
python coleta_dados.py
```
Executa a coleta e salva os dados climáticos (ex: temperatura) em CSV.