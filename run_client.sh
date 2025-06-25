#!/bin/bash

# Убедимся, что DBus не пытается использовать X11
unset DISPLAY

# Определяем путь к файлу с адресом
RUNTIME_DIR="$HOME/.dbus-session"
BUS_ADDRESS_FILE="$RUNTIME_DIR/bus_address"

# Проверяем существование файла
if [ ! -f "$BUS_ADDRESS_FILE" ]; then
    echo "Error: DBus session bus address not found at $BUS_ADDRESS_FILE"
    exit 1
fi

export DBUS_SESSION_BUS_ADDRESS=$(cat "$BUS_ADDRESS_FILE")
echo "DBUS_SESSION_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS"

# Определяем путь к клиенту
CLIENT_PATH="$HOME/workspace/aurora_test/build_client/conf_manager_application1"

# Проверяем существование файла
if [ ! -f "$CLIENT_PATH" ]; then
    echo "Error: Client executable not found at $CLIENT_PATH"
    exit 1
fi

# Запускаем клиент
"$CLIENT_PATH"