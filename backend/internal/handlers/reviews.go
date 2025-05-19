package handlers

import (
	"database/sql"
	"encoding/json"
	"net/http"
	"time"

	"drift-backend/internal/utils"
)

// Create riview type
type CreateReviewRequest struct {
	Rating  int    `json:"rating"`
	Comment string `json:"comment"`
}

// Get review type
type ReviewResponse struct {
	ClientName string `json:"client_name"`
	Rating     int    `json:"rating"`
	Comment    string `json:"comment"`
	CreatedAt  string `json:"created_at"`
}

// Get review
func (h *Handler) HandleGetReviews(w http.ResponseWriter, r *http.Request) {
	query := `
		SELECT
			CONCAT(users.first_name, ' ', users.last_name) AS client_name,
			reviews.rating,
			reviews.comment,
			reviews.created_at
		FROM reviews
		INNER JOIN orders ON reviews.order_id = orders.order_id
		INNER JOIN users ON orders.user_id = users.user_id
	`

	rows, err := h.DB.Query(query)
	if err != nil {
		http.Error(w, `{"error":"Не удалось получить отзывы"}`, http.StatusInternalServerError)
		return
	}
	defer rows.Close()

	var reviews []ReviewResponse
	for rows.Next() {
		var r ReviewResponse
		if err := rows.Scan(&r.ClientName, &r.Rating, &r.Comment, &r.CreatedAt); err != nil {
			http.Error(w, `{"error":"Ошибка чтения отзыва"}`, http.StatusInternalServerError)
			return
		}
		reviews = append(reviews, r)
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(reviews)
}

// Create review
func (h *Handler) HandlePostReview(w http.ResponseWriter, r *http.Request) {
	userID, err := utils.ExtractUserIDFromContext(r.Context())
	if err != nil {
		http.Error(w, `{"error":"Не удалось определить пользователя"}`, http.StatusUnauthorized)
		return
	}

	var input CreateReviewRequest
	if err := json.NewDecoder(r.Body).Decode(&input); err != nil {
		http.Error(w, `{"error":"Некорректный JSON"}`, http.StatusBadRequest)
		return
	}

	var orderID int
	err = h.DB.QueryRow(`
		SELECT order_id 
		FROM orders
		WHERE user_id = $1 AND status = 'выполнен'
		ORDER BY order_date DESC
		LIMIT 1
	`, userID).Scan(&orderID)
	if err != nil {
		if err == sql.ErrNoRows {
			http.Error(w, `{"error":"Вы можете оставить отзыв только после выполнения заказа"}`, http.StatusBadRequest)
			return
		}
		http.Error(w, `{"error":"Ошибка при получении заказа"}`, http.StatusInternalServerError)
		return
	}

	_, err = h.DB.Exec(`
		INSERT INTO reviews (order_id, rating, comment, created_at)
		VALUES ($1, $2, $3, $4)
	`, orderID, input.Rating, input.Comment, time.Now().Format("2006-01-02 15:04:05"))
	if err != nil {
		http.Error(w, `{"error":"Не удалось добавить отзыв"}`, http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte(`{"message":"Отзыв добавлен"}`))
}
