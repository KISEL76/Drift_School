package middleware

import (
	"context"
	"net/http"
	"os"
	"strings"

	"github.com/golang-jwt/jwt/v5"
)

var jwtSecret = []byte(os.Getenv("JWT_SECRET"))

type contextKey string

const (
	ContextUserID contextKey = "user_id"
	ContextRole   contextKey = "role"
)

func AuthMiddleware(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		header := r.Header.Get("Authorization")
		if header == "" || !strings.HasPrefix(header, "Bearer ") {
			http.Error(w, "Отсутствует или некорректный токен", http.StatusUnauthorized)
			return
		}

		tokenString := strings.TrimPrefix(header, "Bearer ")
		token, err := jwt.Parse(tokenString, func(token *jwt.Token) (interface{}, error) {
			return jwtSecret, nil
		})

		if err != nil || !token.Valid {
			http.Error(w, "Недействительный или просроченный токен", http.StatusUnauthorized)
			return
		}

		claims, ok := token.Claims.(jwt.MapClaims)
		if !ok {
			http.Error(w, "Недопустимые данные токена", http.StatusUnauthorized)
			return
		}

		userID, ok1 := claims["user_id"].(float64)
		role, ok2 := claims["role"].(string)
		if !ok1 || !ok2 {
			http.Error(w, "Некорректная структура токена", http.StatusUnauthorized)
			return
		}

		ctx := context.WithValue(r.Context(), ContextUserID, int(userID))
		ctx = context.WithValue(ctx, ContextRole, role)
		next.ServeHTTP(w, r.WithContext(ctx))
	}
}
