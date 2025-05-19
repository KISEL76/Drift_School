package handlers

import "database/sql"

// Handler's type
type Handler struct {
	DB *sql.DB
}
