#!/bin/bash

# Убедимся, что DBus не пытается использовать X11
unset DISPLAY

# Создаем директорию для сессионной шины в домашней директории
RUNTIME_DIR="$HOME/.dbus-session"
echo "Creating directory: $RUNTIME_DIR"
mkdir -vp "$RUNTIME_DIR"  # -v для вывода информации, -p для создания родительских каталогов
ls -ld "$RUNTIME_DIR"

# Запускаем сессионную шину DBus
dbus-daemon --session --print-address > "$RUNTIME_DIR/bus_address" &
sleep 1

export DBUS_SESSION_BUS_ADDRESS=$(cat "$RUNTIME_DIR/bus_address")
echo "DBUS_SESSION_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS"

# Определяем путь к сервису
SERVICE_PATH="$HOME/workspace/aurora_test/build_service/configuration_manager_service"

# Проверяем существование файла
if [ ! -f "$SERVICE_PATH" ]; then
    echo "Error: Service executable not found at $SERVICE_PATH"
    exit 1
fi

# Запускаем сервис
"$SERVICE_PATH"