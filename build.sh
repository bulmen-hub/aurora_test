#!/bin/bash

# Создаем рабочую директорию
mkdir -p ~/workspace
cd ~/workspace

# Клонируем репозиторий с проектом
if [ ! -d aurora_test ]; then
    git clone https://github.com/bulmen-hub/aurora_test.git
fi

cd aurora_test

# Создаем директории для сборки
mkdir -p build_service
mkdir -p build_client

# Собираем сервис
cd build_service
cmake ../configuration_manager_service
make

# Проверяем сборку сервиса
if [ ! -f configuration_manager_service ]; then
    echo "Error: Failed to build service"
    exit 1
fi

# Собираем клиент
cd ../build_client
cmake ../conf_manager_application1
make

# Проверяем сборку клиента
if [ ! -f conf_manager_application1 ]; then
    echo "Error: Failed to build client"
    exit 1
fi

echo "Build ready!"
echo "Run service: ./run_service.sh"
echo "Run client: ./run_client.sh"
