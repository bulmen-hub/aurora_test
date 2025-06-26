# Configuration Manager System

Этот проект реализует систему управления конфигурацией через D-Bus, состоящую из сервиса и клиентского приложения на Qt/C++.

## Основные компоненты

Проект содержит следующие файлы:

- `build.sh`: Скрипт сборки проекта
- `run_service.sh`: Скрипт запуска сервиса
- `run_client.sh`: Скрипт запуска клиента
- `configuration_manager_service/`: Директория сервиса
- `conf_manager_application1/`: Директория клиента

### Сервис (configuration_manager_service)
- Управляет конфигурациями приложений
- Отслеживает изменения конфигурационных файлов
- Предоставляет D-Bus интерфейс для управления конфигурацией

### Клиент (conf_manager_application1)
- Получает конфигурацию от сервиса через D-Bus
- Реагирует на изменения конфигурации в реальном времени
- Реализует таймер с настраиваемыми параметрами

## Требования

- Docker

## Сборка и запуск

### 1. Сборка Docker-образа

```
docker build -t qt-dbus .
```

### 2. Запуск контейнера

```
docker run -it --rm --privileged qt-dbus
```

### 3. Внутри контейнера

Сборка проекта:

```
./build.sh
```

Запуск сервиса в фоновом режиме:

```
./run_service.sh > service.log 2>&1 &
```

Запуск клиента в фоновом режиме:

```
./run_client.sh > client.log 2>&1 &
```

Экспортируем адрес сессионной шины:

```
export DBUS_SESSION_BUS_ADDRESS=$(cat ~/.dbus-session/bus_address)
```

## Тестирование

### Изменение конфигурации через D-Bus

```
gdbus call --session \
  --dest com.system.configurationManager \
  --object-path /com/system/configurationManager/Application/confManagerApplication1 \
  --method com.system.configurationManager.Application.Configuration.ChangeConfiguration \
  "TimeoutPhrase" "<'Please stop me'>"
```

### Проверка реакции клиента

```
tail -f client.log
```

Ожидаемый вывод:

```
Configuration changed, reloading...
New configuration applied: Timeout = 1000 ms, Phrase = "Please stop me"
```

## Принцип работы

1. Сервис при запуске:
   - Создает сессию D-Bus
   - Загружает конфигурации из JSON-файлов в `~/com.system.configurationManager/`
   - Регистрирует интерфейс D-Bus для управления конфигурацией

2. Клиент при запуске:
   - Подключается к сессии D-Bus
   - Получает начальную конфигурацию от сервиса
   - Запускает таймер с параметрами из конфигурации
   - Подписывается на сигналы об изменении конфигурации

3. При изменении конфигурации:
   - Сервис отправляет сигнал `configurationChanged`
   - Клиент получает сигнал и применяет новую конфигурацию
   - Изменения вступают в силу без перезапуска приложений

## Структура конфигурационного файла

Пример (`~/com.system.configurationManager/confManagerApplication1.json`):

```
{
    "Timeout": 2000,
    "TimeoutPhrase": "Hello from confManagerApplication1!"
}
```

## Архитектура D-Bus

```
+------------------+          D-Bus          +-----------------+
| Configuration    | <---------------------->| Client          |
| Manager Service  |     (сигналы/вызовы)    | Application     |
+------------------+                         +-----------------+
       ^
       |
       v
+------------------+
| Конфигурационные |
| JSON-файлы       |
+------------------+
```

## Отладка

Для просмотра логов сервиса:

```
tail -f service.log
```

Для проверки D-Bus соединения (перед проверкой обязательно экспортируйте адрес сессионной шины):

```
dbus-send --session --print-reply --dest=com.system.configurationManager \
  /com/system/configurationManager/Application/confManagerApplication1 \
  com.system.configurationManager.Application.Configuration.GetConfiguration
```
