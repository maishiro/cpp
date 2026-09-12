# --- ビルドステージ ---
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release -j$(nproc)

# --- 実行ステージ ---
FROM ubuntu:22.04 AS runner

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/CMWebAPI .

EXPOSE 5001
CMD ["./CMWebAPI"]