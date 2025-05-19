package handlers

import (
	"crypto/sha256"
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

// Login types
type LoginRequest struct {
	Nickname string `json:"nickname"`
	Password string `json:"password"`
}

type LoginResponse struct {
	UserID int    `json:"user_id,omitempty"`
	Role   string `json:"role,omitempty"`
	Token  string `json:"token,omitempty"`
	Error  string `json:"error,omitempty"`
}

var jwtSecret = []byte(os.Getenv("JWT_SECRET"))

// Login
func (h *Handler) HandleLogin(w http.ResponseWriter, r *http.Request) {
	var req LoginRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(LoginResponse{Error: "Некорректный запрос"})
		return
	}

	var (
		storedHash string
		userID     int
	)

	err := h.DB.QueryRow(`
		SELECT 
			user_id, 
			password_hash 
		FROM users 
		WHERE nickname = $1
	`, req.Nickname).Scan(&userID, &storedHash)

	if err != nil {
		w.WriteHeader(http.StatusUnauthorized)
		json.NewEncoder(w).Encode(LoginResponse{Error: "Пользователь не найден или ошибка БД"})
		return
	}

	hash := sha256.Sum256([]byte(req.Password))
	if fmt.Sprintf("%x", hash) != storedHash {
		w.WriteHeader(http.StatusUnauthorized)
		json.NewEncoder(w).Encode(LoginResponse{Error: "Неверный пароль"})
		return
	}

	role := "user"
	if req.Nickname == "Kisel" {
		role = "admin"
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
		"user_id": userID,
		"role":    role,
		"exp":     time.Now().Add(2 * time.Hour).Unix(),
	})

	signedToken, err := token.SignedString(jwtSecret)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(LoginResponse{Error: "Ошибка при создании токена"})
		return
	}

	json.NewEncoder(w).Encode(LoginResponse{
		UserID: userID,
		Role:   role,
		Token:  signedToken,
	})
}
