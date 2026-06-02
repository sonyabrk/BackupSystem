# сборка
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_QPA_PLATFORM=offscreen

# зависимости
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6sql6-sqlite \
    qt6-l10n-tools \
    libgl1-mesa-dev \
    libglib2.0-dev \
    ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# копии исходников
COPY CMakeLists.txt .
COPY src/       src/
COPY include/   include/
COPY tests/     tests/

# Сборка
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja \
    && cmake --build build --parallel $(nproc)

# финальный образ (без исходников и мусора)
FROM ubuntu:22.04 AS final

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_QPA_PLATFORM=offscreen

# только runtime зависимости
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    libqt6sql6-sqlite \
    libgl1-mesa-glx \
    libglib2.0-0 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# копии только скомпилированных бинарников из стадии builder
COPY --from=builder /app/build/backup_system  ./backup_system
COPY --from=builder /app/build/backup_tests   ./backup_tests

# скрипт запуска тестов
COPY run_tests.sh ./run_tests.sh
RUN chmod +x run_tests.sh

CMD ["./run_tests.sh"]