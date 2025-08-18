# coleta_dados/treinar_modelo.py
# Treina regressão logística e exporta um ia_params.h "completo":
#  - Modelo (SCALER_MEAN/SCALE, W, B) compatível com a ESP32
#  - Série de chuva_mm_24h embutida (sem SPIFFS)
#  - FEATURE_ORDER definido e N_FEATS coerente

import json
from datetime import datetime
from pathlib import Path
import numpy as np
import pandas as pd
import joblib

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score, f1_score, confusion_matrix, classification_report

# ------------------------- Config -------------------------
BASE_DIR = Path(__file__).resolve().parent
CSV = BASE_DIR / "dados_climaticos_tratados.csv"  # gerado pelo seu coletor
OUT = BASE_DIR                                    # artefatos (pkl/metrics) ficam ao lado do .py

# Limiar só para construir um alvo simples (pode ajustar depois)
THRESH_CHUVA = 5.0
TEST_SIZE = 0.2
RANDOM_STATE = 42

# >>> ORDEM DAS FEATURES (o mesmo que a ESP32 vai montar)
FEATURES = ["soil_pct", "temp_c", "rh_pct", "chuva_mm_24h"]  # N_FEATS = 4

# Scaler "canônico" (evita scale=0 quando tiver feature constante)
SCALER_MEAN = np.array([30.0, 25.0, 60.0, 5.0], dtype=float)   # soil, temp, rh, chuva24h
SCALER_SCALE = np.array([20.0,  7.0, 15.0, 7.0], dtype=float)

# Série de previsão a embutir (grade temporal): use 1440 para diária
FORECAST_STEP_MIN = 1440
FORECAST_DIAS = 7  # quantos pontos embutir (7 = 7 dias se passo é diário)

# -------------------- Export helper (C) -------------------
def _fmt_list(arr) -> str:
    return ", ".join(f"{float(x):.8f}f" for x in np.array(arr, dtype=float).ravel())

def export_to_header(
    clf: LogisticRegression,
    scaler: StandardScaler,
    features,
    forecast_t0_epoch: int,
    forecast_step_min: int,
    forecast_vals: np.ndarray,
    out_path: Path,
):
    W_vec = np.array(clf.coef_, dtype=float).ravel()
    b = float(np.array(clf.intercept_, dtype=float).ravel()[0])

    n = len(features)
    feat_order_str = ", ".join(features)
    ia_version = datetime.now().strftime("%Y%m%d")
    forecast_vals = np.array(forecast_vals, dtype=float)
    forecast_vals = np.clip(forecast_vals, 0.0, 1000.0)  # sanidade básica

    header = f"""/* IA params (gerado por treinar_modelo.py) */
#pragma once

#define IA_VERSION {ia_version}
// FEATURE_ORDER: {feat_order_str}

#define N_FEATS {n}
static const float SCALER_MEAN[{n}]  = {{ {_fmt_list(scaler.mean_)} }};
static const float SCALER_SCALE[{n}] = {{ {_fmt_list(scaler.scale_)} }};
static const float W[{n}]            = {{ {_fmt_list(W_vec)} }};
static const float B[1]              = {{ {b:.8f}f }};

/* Série de previsão embutida (chuva acumulada nas próximas 24h)
   Índice aproximado em runtime:
     idx ≈ (now_epoch - FORECAST_T0_EPOCH) / (FORECAST_STEP_MIN * 60)
     idx clamp em [0, FORECAST_LEN-1]
*/
#define FORECAST_T0_EPOCH  {int(forecast_t0_epoch)}
#define FORECAST_STEP_MIN  {int(forecast_step_min)}
#define FORECAST_LEN       {int(len(forecast_vals))}
static const float FORECAST_CHUVA_MM24[FORECAST_LEN] = {{ {_fmt_list(forecast_vals)} }};
"""
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(header, encoding="utf-8")

# ------------------------- Main ---------------------------
def main():
    assert CSV.exists(), f"CSV não encontrado: {CSV}"
    df = pd.read_csv(CSV, parse_dates=["data"]).sort_values("data")

    # -------- Criar/ajustar colunas para bater com FEATURES (proxies)
    # 1) chuva_mm_24h: se já tiver 'chuva_mm' diária, é a própria 24h
    if "chuva_mm_24h" not in df.columns:
        if "chuva_mm" in df.columns:
            df["chuva_mm_24h"] = df["chuva_mm"].astype(float).clip(lower=0.0)
        else:
            # TODO: se for série horária/3h, some janela de 24h aqui
            df["chuva_mm_24h"] = 0.0

    # 2) proxies para sensores (sem tirar nada da placa)
    if "soil_pct" not in df.columns:
        df["soil_pct"] = 30.0
    if "temp_c" not in df.columns:
        if {"temp_max", "temp_min"} <= set(df.columns):
            df["temp_c"] = ((df["temp_max"] + df["temp_min"]) / 2.0).fillna(25.0)
        else:
            df["temp_c"] = 25.0
    if "rh_pct" not in df.columns:
        df["rh_pct"] = 60.0

    # -------- Alvo binário (mesma lógica que você já usava)
    df["irrigar"] = (df["chuva_mm_24h"] < THRESH_CHUVA).astype(int)

    # -------- Split
    X = df[FEATURES].astype(float).values
    y = df["irrigar"].values
    X_tr, X_te, y_tr, y_te = train_test_split(
        X, y, test_size=TEST_SIZE, stratify=y, random_state=RANDOM_STATE
    )

    # -------- Normalização fixa (o mesmo scaler que a ESP32 usará)
    MEAN  = SCALER_MEAN[:len(FEATURES)].astype(float)
    SCALE = SCALER_SCALE[:len(FEATURES)].astype(float)

    X_tr_s = (X_tr - MEAN) / SCALE
    X_te_s = (X_te - MEAN) / SCALE

    # Guardar os números para exportar no header
    scaler = StandardScaler()
    scaler.mean_  = MEAN
    scaler.scale_ = SCALE

    # -------- Treino
    clf = LogisticRegression(max_iter=1000, random_state=RANDOM_STATE)
    clf.fit(X_tr_s, y_tr)

    # -------- Avaliação
    y_pred = clf.predict(X_te_s)
    acc = accuracy_score(y_te, y_pred)
    f1 = f1_score(y_te, y_pred)
    cm = confusion_matrix(y_te, y_pred)
    report = classification_report(y_te, y_pred, digits=4)

    # -------- Artefatos
    joblib.dump(clf, OUT / "model.pkl")
    joblib.dump(scaler, OUT / "scaler.pkl")
    with open(OUT / "metrics.txt", "w", encoding="utf-8") as f:
        f.write("Modelo: regressao_logistica\n")
        f.write(f"Acurácia: {acc:.4f}\nF1: {f1:.4f}\n")
        f.write(f"Matriz de confusão:\n{cm}\n\n{report}\n")

    # -------- Série de previsão a embutir no header
    df_prev = df[["data", "chuva_mm_24h"]].dropna().sort_values("data")
    if len(df_prev) == 0:
        raise RuntimeError("Sem dados para embutir a previsão de chuva_mm_24h.")
    # diário (um ponto por dia): pegue os últimos FORECAST_DIAS registros
    tail = df_prev.tail(FORECAST_DIAS)
    forecast_vals = tail["chuva_mm_24h"].astype(float).to_numpy()
    # Epoch do primeiro ponto exportado (se 'data' for utc-naive, isso vira epoch local)
    t0_epoch = int(pd.Timestamp(tail["data"].iloc[0]).timestamp())

    # -------- Exportar header na pasta local (coleta_dados/)
    OUT_H = BASE_DIR / "ia_params.h"
    export_to_header(clf, scaler, FEATURES, t0_epoch, FORECAST_STEP_MIN, forecast_vals, OUT_H)

    # -------- JSON auxiliar
    pesos = {
        "type": "logreg",
        "W": np.array(clf.coef_).tolist(),
        "B": np.array(clf.intercept_).tolist(),
        "scaler_mean": scaler.mean_.tolist(),
        "scaler_scale": scaler.scale_.tolist(),
        "features": FEATURES,
        "threshold_chuva_mm": THRESH_CHUVA,
        "forecast": {
            "t0_epoch": t0_epoch,
            "step_min": FORECAST_STEP_MIN,
            "len": int(len(forecast_vals)),
        },
    }
    with open(OUT / "pesos_bias.json", "w", encoding="utf-8") as f:
        json.dump(pesos, f, indent=2)

    print(f"✔ Treino concluído. Acc={acc:.4f} | F1={f1:.4f}")
    print(f"Header gerado em: {OUT_H}")
    print(f"FEATURE_ORDER: {', '.join(FEATURES)}")
    print(f"Forecast: t0={t0_epoch} step_min={FORECAST_STEP_MIN} len={len(forecast_vals)}")

if __name__ == "__main__":
    main()
