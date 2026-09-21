# MTS Router — Мультиагентная система разработки

## Обзор

Проект разделён на 6 независимых подпроектов, каждый из которых может обрабатываться
отдельным агентом/командой параллельно.

## Архитектура мультиагентной системы

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ORCHESTRATOR AGENT                               │
│  (Координация, управление зависимостями, CI/CD)                    │
├────────────┬────────────┬────────────┬────────────┬────────────┬───┤
│ AGENT 1    │ AGENT 2    │ AGENT 3    │ AGENT 4    │ AGENT 5    │   │
│ Core Router│ Mobile Core│ Backhaul   │ OLT GPON   │ Enterprise │   │
│ Agent      │ Agent      │ Agent      │ Agent      │ Agent      │   │
├────────────┼────────────┼────────────┼────────────┼────────────┤   │
│ AGENT 6    │ AGENT 7    │ AGENT 8    │ AGENT 9    │ AGENT 10   │   │
│ Residential│ Linux OS   │ Firmware   │ API/SDK    │ QA/CI/CD   │   │
│ Gateway    │ Agent      │ Agent      │ Agent      │ Agent      │   │
└────────────┴────────────┴────────────┴────────────┴────────────┘
```

## Подпроекты

### 1. Core Router (MTS-CR-9000)
**Каталог:** `core-router/`
**Задачи:**
- Проектирование main board (Tofino 2 + EPYC)
- Проектирование line card (Tofino 2 + ports)
- Разработка драйверов Tofino 2
- Сборка Yocto образа
- Интеграция DPDK
- Разработка P4 pipelines
- Настройка BGP/MPLS/SRv6

**Агент:** `core-router-agent`
**Контекст:** `core-router/spec/core-router-spec.md`, `core-router/chip/board-trace.md`, `core-router/linux/yocto-layer.md`, `core-router/firmware/driver-spec.md`

### 2. Mobile Core (MTS-MC-5000)
**Каталог:** `mobile-core/`
**Задачи:**
- Проектирование main board (ThunderX3 + EPYC)
- Проектирование line card (100G ports)
- Разработка драйверов ThunderX3
- Сборка Yocto образа + K3s
- Разработка UPF/SMF/AMF/PCF
- Интеграция DPDK

**Агент:** `mobile-core-agent`
**Контекст:** `mobile-core/spec/mobile-core-spec.md`, `mobile-core/linux/yocto-k3s-layer.md`, `mobile-core/firmware/driver-spec.md`

### 3. Mobile Backhaul (MTS-MB-3000)
**Каталог:** `mobile-backhaul/`
**Задачи:**
- Проектирование main board (S32G3 + 88Q5242)
- Проектирование line card (10G/1G ports)
- Разработка драйверов S32G3
- Сборка Buildroot образа
- Интеграция DPDK
- Разработка MPLS-TP forwarding
- Разработка PTP grandmaster

**Агент:** `mobile-backhaul-agent`
**Контекст:** `mobile-backhaul/spec/mobile-backhaul-spec.md`, `mobile-backhaul/linux/buildroot-config.md`, `mobile-backhaul/firmware/driver-spec.md`

### 4. OLT GPON (MTS-OLT-2000)
**Каталог:** `olt-gpon/`
**Задачи:**
- Проектирование main board (Tofino 2 + EPYC)
- Проектирование GPON line card (RTL960x)
- Проектирование uplink card (10G/100G)
- Разработка драйверов Tofino 2
- Разработка драйверов RTL960x
- Сборка OpenWrt образа
- Интеграция TR-069
- Разработка OMCI management

**Агент:** `olt-gpon-agent`
**Контекст:** `olt-gpon/spec/olt-gpon-spec.md`, `olt-gpon/linux/openwrt-layer.md`, `olt-gpon/firmware/driver-spec.md`

### 5. Enterprise Router (MTS-ER-1000)
**Каталог:** `enterprise-router/`
**Задачи:**
- Проектирование main board (S32G3 + TomTom)
- Проектирование line card (10G/1G ports)
- Разработка драйверов S32G3
- Сборка OpenWrt образа
- Разработка SD-WAN engine
- Интеграция FRRouting

**Агент:** `enterprise-agent`
**Контекст:** `enterprise-router/spec/enterprise-router-spec.md`, `enterprise-router/linux/openwrt-layer.md`, `enterprise-router/firmware/driver-spec.md`

### 6. Residential Gateway (MTS-RG-500)
**Каталог:** `residential-gateway/`
**Задачи:**
- Проектирование main board (MT7981 + RTL960x)
- Разработка драйверов MT7981
- Разработка драйверов RTL960x
- Сборка OpenWrt образа
- Интеграция WiFi 6 (MT76)
- Интеграция TR-069
- Разработка VoIP (Asterisk)
- Разработка IPTV

**Агент:** `residential-agent`
**Контекст:** `residential-gateway/spec/residential-gateway-spec.md`, `residential-gateway/linux/openwrt-layer.md`, `residential-gateway/firmware/driver-spec.md`

### 7. Linux OS Agent
**Каталог:** `linux/`
**Задачи:**
- Создание meta-mts слоя для Yocto
- Создание board support для каждого устройства
- Написание kernel recipes для каждого чипа
- Написание package recipes для каждого ПО
- Настройка build для каждого устройства
- Тестирование boot sequence
- Тестирование networking stack

**Агент:** `linux-agent`
**Контекст:** `docs/linux-os-selection.md`, все `*/linux/` каталоги

### 8. Firmware Agent
**Каталог:** `firmware/`
**Задачи:**
- Разработка драйверов Tofino 2
- Разработка драйверов ThunderX3
- Разработка драйверов S32G3
- Разработка драйверов RTL960x
- Разработка драйверов MT7981
- Разработка драйверов TomTom
- Тестирование драйверов

**Агент:** `firmware-agent`
**Контекст:** все `*/firmware/driver-spec.md`

### 9. API/SDK Agent
**Каталог:** `api/`
**Задачи:**
- Определение общей схемы YANG
- Реализация REST API gateway
- Реализация gRPC telemetry
- Реализация gRPC config
- Добавление mTLS аутентификации
- Добавление API key аутентификации
- Написание client SDK (Python, Go, Java)
- Добавление OpenAPI/Swagger документации

**Агент:** `api-agent`
**Контекст:** `api/spec/mts-router-api.md`, `api/spec/mts-api-protobuf.md`, `api/examples/mts-api-examples.py`

### 10. QA/CI/CD Agent
**Каталог:** `scripts/`
**Задачи:**
- Настройка CI/CD для сборки образов
- Настройка тестирования
- Настройка деплоя
- Настройка мониторинга
- Настройка сертификации

**Агент:** `qa-agent`
**Контекст:** `scripts/`

## Зависимости между агентами

```
┌─────────────────────────────────────────────────────────────────────┐
│                        ЗАВИСИМОСТИ                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Linux OS Agent ──────────► Все hardware agents                    │
│  Firmware Agent ──────────► Все hardware agents                    │
│  API Agent ───────────────► Все hardware agents                    │
│                                                                     │
│  Core Router Agent ───────► Linux OS Agent                         │
│  Mobile Core Agent ───────► Linux OS Agent                         │
│  Backhaul Agent ──────────► Linux OS Agent                         │
│  OLT GPON Agent ──────────► Linux OS Agent                         │
│  Enterprise Agent ────────► Linux OS Agent                         │
│  Residential Agent ───────► Linux OS Agent                         │
│                                                                     │
│  Все hardware agents ─────► QA/CI/CD Agent                         │
│                                                                     │
│  API Agent ───────────────► QA/CI/CD Agent                         │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Workflow мультиагентной системы

### Фаза 1: Подготовка (недели 1-2)
1. Оркестратор создаёт рабочие директории для каждого агента
2. Linux OS Agent создаёт meta-mts слой
3. Firmware Agent создаёт базовые драйверы
4. API Agent создаёт схему YANG и protobuf

### Фаза 2: Параллельная разработка (недели 3-8)
1. Все 6 hardware agents работают параллельно
2. Каждый агент:
   - Проектирует hardware board
   - Разрабатывает firmware
   - Собирает OS образ
   - Интегрирует API
3. Linux OS Agent адаптирует OS для каждого устройства
4. Firmware Agent дописывает драйверы
5. API Agent дописывает SDK

### Фаза 3: Интеграция (недели 9-10)
1. QA/CI/CD Agent собирает все артефакты
2. Интеграционное тестирование
3. Исправление ошибок

### Фаза 4: Сертификация (недели 11-12)
1. Тестирование каждого устройства
2. Сертификация протоколов
3. Подготовка к производству

## Коммуникация между агентами

###共享 артефакты
- `docs/architecture-overview.md` — общая архитектура
- `docs/chipset-analysis.md` — анализ чипов
- `docs/linux-os-selection.md` — выбор ОС
- `api/spec/mts-router-api.md` — API спецификация
- `api/spec/mts-api-protobuf.md` — protobuf спецификация

### shared schemas
- YANG models (api/spec/)
- Protobuf files (api/spec/)
- Kernel configs (core-router/linux/kernel-config.md)

### shared drivers
- Tofino 2 drivers (core-router/firmware/driver-spec.md)
- ThunderX3 drivers (mobile-core/firmware/driver-spec.md)
- S32G3 drivers (mobile-backhaul/firmware/driver-spec.md)
- RTL960x drivers (olt-gpon/firmware/driver-spec.md, residential-gateway/firmware/driver-spec.md)
- MT7981 drivers (residential-gateway/firmware/driver-spec.md)
- TomTom drivers (enterprise-router/firmware/driver-spec.md)

## Команды для запуска агентов

```bash
# Запуск всех агентов параллельно
./scripts/run-agents.sh --parallel

# Запуск конкретного агента
./scripts/run-agent.sh core-router
./scripts/run-agent.sh mobile-core
./scripts/run-agent.sh mobile-backhaul
./scripts/run-agent.sh olt-gpon
./scripts/run-agent.sh enterprise
./scripts/run-agent.sh residential
./scripts/run-agent.sh linux
./scripts/run-agent.sh firmware
./scripts/run-agent.sh api
./scripts/run-agent.sh qa

# Проверка статуса
./scripts/status.sh

# Сборка образов
./scripts/build-all.sh

# Тестирование
./scripts/test-all.sh

# Деплой
./scripts/deploy.sh
```

## CI/CD Pipeline

```yaml
# .github/workflows/ci-cd.yml
name: MTS Router CI/CD

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build-core-router:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build Yocto
        run: ./scripts/build-core-router.sh
      - name: Test
        run: ./scripts/test-core-router.sh
      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: mts-cr9000-image
          path: build/images/

  build-mobile-core:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build Yocto + K3s
        run: ./scripts/build-mobile-core.sh
      - name: Test
        run: ./scripts/test-mobile-core.sh
      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: mts-mc5000-image
          path: build/images/

  build-mobile-backhaul:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build Buildroot
        run: ./scripts/build-mobile-backhaul.sh
      - name: Test
        run: ./scripts/test-mobile-backhaul.sh
      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: mts-mb3000-image
          path: build/images/

  build-olt-gpon:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build OpenWrt
        run: ./scripts/build-olt-gpon.sh
      - name: Test
        run: ./scripts/test-olt-gpon.sh
      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: mts-olt2000-image
          path: build/images/

  build-enterprise:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build OpenWrt
        run: ./scripts/build-enterprise.sh
      - name: Test
        run: ./scripts/test-enterprise.sh
      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: mts-er1000-image
          path: build/images/

  build-residential:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build OpenWrt
        run: ./scripts/build-residential.sh
      - name: Test
        run: ./scripts/test-residential.sh
      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: mts-rg500-image
          path: build/images/

  api-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Test REST API
        run: ./scripts/test-api-rest.sh
      - name: Test gRPC API
        run: ./scripts/test-api-grpc.sh
      - name: Test SDK
        run: ./scripts/test-sdk.sh

  integration-tests:
    needs: [build-core-router, build-mobile-core, build-mobile-backhaul, build-olt-gpon, build-enterprise, build-residential]
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Integration tests
        run: ./scripts/test-integration.sh