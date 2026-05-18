---

## 📡 wifi_management

**Responsabilidade:** Gestão de conectividade WiFi e credenciais.

### Funções Públicas

```cpp
void initWiFi()
// Tenta conectar em modo Station. Fallback para AP se falhar.

bool ensureWiFiConnected()
// Verifica conexão e tenta reconectar se necessário.
```

---

## ☁️ mqtt

### Arquivo: `mqtt.h/cpp`

**Responsabilidade:** Comunicação com o broker ThingsBoard.

### Funções Públicas

void initMQTT()
// Configura broker e tenta conexão inicial.
