# === CONFIG ===
ENV_FILE=.env

# === COMMANDS ===

up:
	docker compose --env-file $(ENV_FILE) up --build

down:
	docker compose --env-file $(ENV_FILE) down

down-v:
	docker compose --env-file $(ENV_FILE) down -v

rebuild:
	docker compose --env-file $(ENV_FILE) build --no-cache

restart:
	docker compose --env-file $(ENV_FILE) down && docker compose --env-file $(ENV_FILE) up --build

logs:
	docker compose --env-file $(ENV_FILE) logs -f

ps:
	docker compose --env-file $(ENV_FILE) ps

prune:
	docker system prune -a --volumes -f

