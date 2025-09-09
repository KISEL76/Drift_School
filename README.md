<h1 align="center">Drift School </h1>

## 📖 Introduction

This is my very first attempt at building a **fullstack application** from scratch.  
The project simulates a **drift school platform** with scheduling, feedback, and Telegram notifications.

## 🛠️ Tech Stack

- **Golang 1.23** — Backend API service 
- **PostgreSQL 15** — Database 
- **pgAdmin 4** — Database administration UI 
- **Python (Telegram Bot)** — Notifies students about classes
- **Docker + docker-compose** — Containerization and orchestration
- **Makefile** — Quick commands (`make up`, `make logs`, `make rebuild`, …)
- **Qt C++ 5.12** — Desktop GUI client (outside Docker)

## 📦 Services (docker-compose)

- `backend` → container **school-api**, exposed at `${GO_PORT}:8080`
- `db` → container **school-db**, exposed at `${POSTGRES_PORT}:5432`, persistent volume `db_data`
- `pgadmin` → container **school-pgadmin**, exposed at `5050:80`, volume `pgadmin_data`
- `bot` → container **school-bot**, connected to `db`

All services are linked via `schoolnet` network.

## ⚙️ Environment (.env example)

Create a `.env` file in the project root:

```env
# Backend
GO_PORT=8080

# Postgres
POSTGRES_DB=driftschool
POSTGRES_USER=drift
POSTGRES_PASSWORD=driftpass
POSTGRES_PORT=5432

# pgAdmin
PGADMIN_DEFAULT_EMAIL=admin@example.com
PGADMIN_DEFAULT_PASSWORD=supersecret

# Telegram Bot
TELEGRAM_TOKEN=123456:ABCDEF_your_token_here
```

## 🚀 Quick Start

1. Clone the repository  
   ```bash
   git clone <repo_url>
   cd <folder_name>
   ```

2. Create `.env` (see example above)

3. Start all services:  
   ```bash
   make up
   ```

4. Check logs and container status:  
   ```bash
   make ps
   make logs
   ```

5. Open in browser:  
   - API: [http://localhost:8080](http://localhost:8080)  
   - pgAdmin: [http://localhost:5050](http://localhost:5050)  

6. Test API endpoints (example):  
   ```bash
   curl -i http://localhost:8080/health
   curl http://localhost:8080/api/v1/schedules | jq
   ```

## 🖥️ Qt C++ 5.12 Frontend

The desktop client is built separately from Docker.

1. Install **Qt 5.12** and build tools (qmake/CMake)  
2. Open `frontend/Drift_School_App` folder and open `Drift_School_App.pro` file in Qt and then just push launch button

## 🧰 Makefile Commands

```bash
make up        # build & start all services
make down      # stop containers
make down-v    # stop and remove volumes (reset DB)
make rebuild   # rebuild without cache
make restart   # restart with build
make logs      # follow logs
make ps        # list containers
make prune     # prune docker system (careful!)
```
