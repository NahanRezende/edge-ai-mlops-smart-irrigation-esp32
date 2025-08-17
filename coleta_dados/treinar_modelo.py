# coleta_dados/treinar_modelo.py
# Etapas 2+: cria alvo, split, normaliza, treina Regressão Logística,
# avalia e exporta um header .h para uso no ESP32.

import json
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
OUT = BASE_DIR                                   # salva artefatos na MESMA pasta dos .py
THRESH_CHUVA = 5.0
TEST_SIZE = 0.2
RANDOM_STATE = 42
FEATURES = ["temp_max", "temp_min", "chuva_mm"]

# -------------------- Export helper (C) -------------------
def _arr(name, arr):
    flat = np.array(arr, dtype=float).reshape(-1)
    return f"static const float {name}[] = {{{', '.join(f'{x:.8f}f' for x in flat)}}};\n"

def export_logreg_to_header(clf: LogisticRegression, scaler: StandardScaler, out_path: Path):
    W = np.array(clf.coef_, dtype=float)        # shape (1, 3)
    b = np.array(clf.intercept_, dtype=float)   # shape (1,)
    header = []
    header.append("// Parâmetros Regressão Logística (gerado automaticamente)\n")
    header.append("#pragma once\n")
    header.append("#define N_IN 3\n#define N_OUT 1\n\n")
    header.append(_arr("SCALER_MEAN", scaler.mean_))
    header.append(_arr("SCALER_SCALE", scaler.scale_))
    header.append(_arr("W", W))
    header.append(_arr("B", b))
    header.append("// p = sigmoid(dot(W, x_norm) + B)\n")
    out_path.write_text("".join(header), encoding="utf-8")

# ------------------------- Main ---------------------------
def main():
    assert CSV.exists(), f"CSV não encontrado: {CSV}"
    df = pd.read_csv(CSV, parse_dates=["data"])

    # 2) Alvo binário
    df["irrigar"] = (df["chuva_mm"] < THRESH_CHUVA).astype(int)

    # 3) Split
    X = df[FEATURES].astype(float).values
    y = df["irrigar"].values
    X_tr, X_te, y_tr, y_te = train_test_split(
        X, y, test_size=TEST_SIZE, stratify=y, random_state=RANDOM_STATE
    )

    # 4) Normalização
    scaler = StandardScaler()
    X_tr_s = scaler.fit_transform(X_tr)
    X_te_s = scaler.transform(X_te)

    # 5) Treino (Regressão Logística)
    clf = LogisticRegression(max_iter=1000, random_state=RANDOM_STATE)
    clf.fit(X_tr_s, y_tr)

    # 6) Avaliação
    y_pred = clf.predict(X_te_s)
    acc = accuracy_score(y_te, y_pred)
    f1 = f1_score(y_te, y_pred)
    cm = confusion_matrix(y_te, y_pred)
    report = classification_report(y_te, y_pred, digits=4)

    # 7) Salvar artefatos (na pasta dos .py)
    joblib.dump(clf, OUT / "model.pkl")
    joblib.dump(scaler, OUT / "scaler.pkl")
    with open(OUT / "metrics.txt", "w", encoding="utf-8") as f:
        f.write("Modelo: regressao_logistica\n")
        f.write(f"Acurácia: {acc:.4f}\nF1: {f1:.4f}\n")
        f.write(f"Matriz de confusão:\n{cm}\n\n{report}\n")

    # 8) Exportar header C e JSON auxiliares
    export_logreg_to_header(clf, scaler, OUT / "ia_params.h")
    pesos = {
        "type": "logreg",
        "W": np.array(clf.coef_).tolist(),
        "B": np.array(clf.intercept_).tolist(),
        "scaler_mean": scaler.mean_.tolist(),
        "scaler_scale": scaler.scale_.tolist(),
        "features": FEATURES,
        "threshold_chuva_mm": THRESH_CHUVA,
    }
    with open(OUT / "pesos_bias.json", "w", encoding="utf-8") as f:
        json.dump(pesos, f, indent=2)

    print(f"✔ Treino concluído. Acc={acc:.4f} | F1={f1:.4f}")
    print(f"Arquivos salvos em: {OUT}")

if __name__ == "__main__":
    main()
