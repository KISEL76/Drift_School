package main

import (
	"database/sql"
	"fmt"
	"log"
	"net/http"
	"os"

	"drift-backend/internal/handlers"
	"drift-backend/internal/middleware"

	_ "github.com/lib/pq"
)

func main() {
	dsn := fmt.Sprintf(
		"host=%s port=%s user=%s password=%s dbname=%s sslmode=disable",
		os.Getenv("POSTGRES_HOST"),
		os.Getenv("POSTGRES_PORT"),
		os.Getenv("POSTGRES_USER"),
		os.Getenv("POSTGRES_PASSWORD"),
		os.Getenv("POSTGRES_DB"),
	)

	db, err := sql.Open("postgres", dsn)
	if err != nil {
		log.Fatalf("failed to connect to db: %v", err)
	}
	if err := db.Ping(); err != nil {
		log.Fatalf("cannot connect to db (ping): %v", err)
	}

	h := &handlers.Handler{DB: db}

	// Auth endpoints
	http.HandleFunc("/login", h.HandleLogin)
	http.HandleFunc("/signup", h.HandleSignUp)

	// Instructor endpoints
	http.HandleFunc("/instructors", middleware.AuthMiddleware(func(w http.ResponseWriter, r *http.Request) {
		switch r.Method {
		case http.MethodGet:
			h.HandleGetInstructors(w, r)
		case http.MethodPost:
			h.HandleAddInstructor(w, r)
		default:
			http.Error(w, "Method Not Allowed", http.StatusMethodNotAllowed)
		}
	}))
	http.HandleFunc("/instructors/fire", middleware.AuthMiddleware(h.HandleFireInstructor))

	// Schedule endpoints
	http.HandleFunc("/schedule", middleware.AuthMiddleware(func(w http.ResponseWriter, r *http.Request) {
		switch r.Method {
		case http.MethodGet:
			h.HandleGetSchedule(w, r)
		case http.MethodPost:
			h.HandleCreateSchedule(w, r)
		default:
			http.Error(w, "Method Not Allowed", http.StatusMethodNotAllowed)
		}
	}))
	http.HandleFunc("/schedule/delete", middleware.AuthMiddleware(h.HandleDeleteSchedule))
	http.HandleFunc("/schedule/book", middleware.AuthMiddleware(func(w http.ResponseWriter, r *http.Request) {
		if r.Method == http.MethodPatch {
			h.HandleBookSchedule(w, r)
		} else {
			http.Error(w, "Method Not Allowed", http.StatusMethodNotAllowed)
		}
	}))

	// Services endpoints
	http.Handle("/services", middleware.AuthMiddleware(h.HandleGetServices))

	// Reviews endpoints
	http.HandleFunc("/reviews", middleware.AuthMiddleware(func(w http.ResponseWriter, r *http.Request) {
		switch r.Method {
		case http.MethodGet:
			h.HandleGetReviews(w, r)
		case http.MethodPost:
			h.HandlePostReview(w, r)
		default:
			http.Error(w, "Method Not Allowed", http.StatusMethodNotAllowed)
		}
	}))

	log.Println("API server running on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
