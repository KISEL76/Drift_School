package handlers

import (
	"crypto/sha256"
	"encoding/json"
	"fmt"
	"net/http"
)

// Sign up types
type SignUpRequest struct {
	Nickname    string `json:"nickname"`
	PhoneNumber string `json:"phone_number"`
	Email       string `json:"email"`
	FirstName   string `json:"first_name"`
	LastName    string `json:"last_name"`
	BirthDate   string `json:"birth_date"`
	Password    string `json:"password"`
}

type SignUpResponse struct {
	Message string `json:"message,omitempty"`
	Error   string `json:"error,omitempty"`
}

// Sign Up
func (h *Handler) HandleSignUp(w http.ResponseWriter, r *http.Request) {
	var req SignUpRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(SignUpResponse{Error: "Некорректное тело запроса"})
		return
	}

	var exists bool
	err := h.DB.QueryRow(`
		SELECT EXISTS (
			SELECT 1 FROM users
			WHERE nickname = $1 OR phone_number = $2 OR email = $3
		)
	`, req.Nickname, req.PhoneNumber, req.Email).Scan(&exists)

	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(SignUpResponse{Error: "Ошибка при проверке существования пользователя"})
		return
	}

	if exists {
		w.WriteHeader(http.StatusConflict)
		json.NewEncoder(w).Encode(SignUpResponse{Error: "Пользователь с таким никнеймом, телефоном или email уже существует"})
		return
	}

	hash := sha256.Sum256([]byte(req.Password))
	passwordHash := fmt.Sprintf("%x", hash)

	_, err = h.DB.Exec(`
		INSERT INTO users (nickname, first_name, last_name, phone_number, email, birth_date, password_hash)
		VALUES ($1, $2, $3, $4, $5, $6, $7)
	`, req.Nickname, req.FirstName, req.LastName, req.PhoneNumber, req.Email, req.BirthDate, passwordHash)

	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(SignUpResponse{Error: "Ошибка при создании пользователя"})
		return
	}

	json.NewEncoder(w).Encode(SignUpResponse{Message: "Пользователь успешно зарегистрирован"})
}
