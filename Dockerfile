FROM ubuntu:22.04

# Установка зависимостей
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    qtbase5-dev \
    qtbase5-dev-tools \
    libdbus-1-dev \
    libdbus-1-3 \
    qtdeclarative5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libqt5dbus5 \
    dbus \
    dbus-x11 \
    git \
    nano \
    sudo \
    libglib2.0-bin \
    ca-certificates \
    --no-install-recommends \
    && rm -rf /var/lib/apt/lists/*

# Настройка среды DBus
RUN mkdir -p /var/run/dbus \
    && dbus-uuidgen > /var/lib/dbus/machine-id

# Создаем группу dbus и пользователя
RUN groupadd -r dbus && \
    useradd -m -G dbus,sudo devuser && \
    echo "devuser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER devuser
WORKDIR /home/devuser

# Создаем папку для конфигураций
RUN mkdir -p /home/devuser/com.system.configurationManager

# Пример конфигурационного файла
RUN echo '{ \
    "Timeout": 2000, \
    "TimeoutPhrase": "Hello from confManagerApplication1!" \
}' > /home/devuser/com.system.configurationManager/confManagerApplication1.json

# Копируем скрипты с правильными правами
COPY --chown=devuser:devuser run_service.sh /home/devuser/
COPY --chown=devuser:devuser run_client.sh /home/devuser/
COPY --chown=devuser:devuser build.sh /home/devuser/

# Устанавливаем права на выполнение
RUN chmod +x /home/devuser/build.sh && \
    chmod +x /home/devuser/run_service.sh && \
    chmod +x /home/devuser/run_client.sh

CMD ["/bin/bash"]