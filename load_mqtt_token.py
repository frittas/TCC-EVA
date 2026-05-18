Import("env")
import os

# Caminho do arquivo que contém o token (na raiz do projeto)
TOKEN_FILE = "mqtt_token.txt"

mqtt_token = None

if os.path.exists(TOKEN_FILE):
    try:
        with open(TOKEN_FILE, "r") as f:
            mqtt_token = f.read().strip()
    except Exception as e:
        print(f"[BUILD] Erro ao ler {TOKEN_FILE}: {e}")

if not mqtt_token:
    raise Exception(f"MQTT_TOKEN não definido. Crie o arquivo '{TOKEN_FILE}' na raiz do projeto com o seu token.")

# Exporta como macro de pré-processamento para o código.
env.Append(CPPDEFINES=["MQTT_TOKEN={}".format(mqtt_token)])
print(f"[BUILD] MQTT_TOKEN carregado a partir de {TOKEN_FILE}.")
